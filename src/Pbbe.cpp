#include "Pbbe.h"
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "crc.h"


// Some useful constants for parsing the EEPROM

// While these values do not depend on the current layout version, we
// can just use simple constants for them.
static const uint8_t PIN_MASK = 0x3f;
static const uint8_t CHECKSUM_SIZE = 2;
static const uint8_t LAYOUT_VERSION_OFFSET = 0;
static const uint8_t TOTAL_SIZE_OFFSET = 1;
static const uint8_t USED_SIZE_OFFSET = 2;
static const uint8_t UNIQUE_ID_OFFSET = 3;

#if !defined(lengthof)
#define lengthof(x) (sizeof(x)/sizeof(*x))
#endif

#if !defined(offsetof)
#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

//#define DETAIL_PARSE_ERRORS

#ifdef DETAIL_PARSE_ERRORS
#define DEBUG_ERR(x) Serial.println(x)
#define DEBUG_ERR2(x, y) do {Serial.print(x); Serial.println(y);} while(0)
#else
#define DEBUG_ERR(x) do {} while(0)
#define DEBUG_ERR2(x, y) do {} while(0)
#endif

bool Pbbe::parseMinimalHeader(const Eeprom *eep, MinimalHeader *h)
{
  if (!eep || CHECKSUM_SIZE >= eep->size) {
    DEBUG_ERR(F("Eeprom too short for checksum"));
    return false;
  }
  size_t len = eep->size - CHECKSUM_SIZE;

  if (len < 1u) {
    DEBUG_ERR(F("Eeprom too short for layout_version"));
    return false;
  }

  h->layout_version = eep->raw[0];
  h->header_length = 0xc;

  if (h->layout_version == 0 || h->layout_version > 1) {
    DEBUG_ERR(F("Invalid or unsupported layout version"));
    return false;
  }

  // Size should be at least the header + a 1-character name
  if (len < h->header_length + 1u) {
    DEBUG_ERR(F("Eeprom too short for header"));
    return false;
  }

  h->name_length = stringLength(eep, h->header_length);

  if (!h->name_length) {
    DEBUG_ERR(F("Invalid backpack name"));
    return false;
  }

  return true;
}

Pbbe::Header *Pbbe::parseHeaderA(const Eeprom *eep)
{
  MinimalHeader min;

  if (!parseMinimalHeader(eep, &min))
    return NULL;

  Header *h = (Header*)malloc(sizeof(Header) + min.name_length + 1);

  h->layout_version = min.layout_version;
  h->total_eeprom_size = eep->raw[1];
  h->used_eeprom_size = eep->raw[2];
  /* Skip 8 bytes of unique id */
  h->firmware_version = eep->raw[0xb];
  extractString(eep, min.header_length, h->backpack_name, min.name_length);
  h->descriptor_offset = min.header_length + min.name_length;

  return h;
}

size_t Pbbe::stringLength(const Eeprom *eep, size_t offset) {
  // Check that we have at least one character
  if (offset + CHECKSUM_SIZE + 1 > eep->size) {
    DEBUG_ERR2(F("EEPROM too short for string. String start at: "), offset);
    return 0;
  }

  const uint8_t *buf = &eep->raw[offset];
  size_t len = eep->size - offset - CHECKSUM_SIZE;

  // First character is always in
  size_t strlen = 1;
  do {
    // The last character in the string will have the MSB set (which
    // makes the minimum string length 1).
    if ((buf[strlen - 1] & 0x80))
      return strlen;
  } while(strlen++ <= len);
  // Reached end of EEPROM before end of string - invalid string
  DEBUG_ERR2(F("End of EEPROM before end of string. String start at: "), offset);
  return 0;
}

void Pbbe::extractString(const Eeprom *eep, size_t offset, char *str, size_t strlen)
{
  // Add zero termination
  str[strlen] = '\0';

  // Strip the MSB from the last byte, and copy the rest as-is
  str[strlen - 1] = eep->raw[offset + strlen - 1] & 0x7f;
  while (--strlen)
    str[strlen - 1] = eep->raw[offset + strlen - 1];
}

char *Pbbe::parseStringA(const Eeprom *eep, size_t offset)
{
  size_t strlen = stringLength(eep, offset);
  char *str = (char*)malloc(strlen + 1);
  extractString(eep, offset, str, strlen);
  return str;
}

Pbbe::DescriptorList *Pbbe::parseDescriptorListA(const Eeprom *eep, const Header *h)
{
  uint8_t offset;

  // Use previously parsed values if given, otherwise, do a minimal
  // header parse to figure out where it ends.
  if (h) {
    offset = h->descriptor_offset;
  } else {
    MinimalHeader min;
    if (!parseMinimalHeader(eep, &min))
      return NULL;
    offset = min.header_length + min.name_length;
  }

  // First, count the number of descriptors
  uint8_t count = 0;
  uint8_t i = offset;
  while (i < eep->size - CHECKSUM_SIZE) {
    if (eep->raw[i] == 0xff) {
      // Skip empty descriptors
      i++;
    } else {
      MinimalDescriptor min;
      if (!parseMinimalDescriptor(eep, i, &min))
        return NULL;
      count++;
      i += min.descriptor_length + min.name_length;
    }
  }

  // Then allocate memory for the list
  const size_t size = sizeof(DescriptorList) + count * sizeof(DescriptorInfo);
  DescriptorList *list = (DescriptorList*)malloc(size);
  list->num_descriptors = count;

  // And loop over the descriptors again to fill the list
  count = 0;
  i = offset;
  DescriptorInfo *group = NULL;
  while (i < eep->size - CHECKSUM_SIZE) {
    if (eep->raw[i] == 0xff) {
      // Skip empty descriptors
      i++;
    } else {
      MinimalDescriptor min;
      if (!parseMinimalDescriptor(eep, i, &min))
        return NULL;
      DescriptorInfo& info = list->info[count];
      info.type = min.type;
      info.offset = i;
      info.parsed = NULL;
      if (min.type == DT_GROUP) {
        group = &info;
        info.group = NULL;
      } else {
        info.group = group;
      }
      count++;
      i += min.descriptor_length + min.name_length;
    }
  }

  return list;
}

struct DescriptorTypeInfo {
  /** Length of the descriptor in EEPROM, excluding the name */
  uint8_t descriptor_length;

  /** Does this descriptor support having a name at all? */
  bool supports_name;

  /** Size of the Descriptor subclass, excluding the name */
  uint8_t parsed_size;

  /**
   * Default name, used for some descriptors when supports_name above is
   * true, but the the "has name" flag in the actual descriptor is not
   * set. */
  char default_name[8];
};

// TODO: Use a macro to generate the elements in this array, to simplify
// things while at the same time adding static_asserts to verify some
// assumptions (such as that the name field is the last one). Also, the
// supports_name might be automatically filled by doing things like
// http://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
// (might need C++11, though).
static const DescriptorTypeInfo descriptor_type_info[] PROGMEM = {
/* Note that the .default_name field names are commented out, because of
 * http://gcc.gnu.org/bugzilla/show_bug.cgi?id=55227 */
  [Pbbe::DT_RESERVED] = { 0 },
  [Pbbe::DT_GROUP] = {
    .descriptor_length = 1,
    .supports_name = true,
    // Use offsetof since GroupDescriptor is empty, but sizeof can never
    // be 0 (but offsetof is).
    .parsed_size = offsetof(Pbbe::GroupDescriptor, name),
  },
  [Pbbe::DT_POWER_USAGE] = {
    .descriptor_length = 5,
    .supports_name = false,
    .parsed_size = sizeof(Pbbe::PowerUsageDescriptor),
  },
  [Pbbe::DT_DATA] = {
    .descriptor_length = 2, /* Excluding data _and_ name! */
    .supports_name = true,
    .parsed_size = sizeof(Pbbe::DataDescriptor),
 /* .default_name = */ "data",
  },
  [Pbbe::DT_IOPIN] = {
    .descriptor_length = 2,
    .supports_name = true,
    .parsed_size = sizeof(Pbbe::IoPinDescriptor),
  },
  [Pbbe::DT_UART] = {
    .descriptor_length = 4,
    .supports_name = true,
    .parsed_size = sizeof(Pbbe::UartDescriptor),
 /* .default_name = */ "uart",
  },
  [Pbbe::DT_I2C_SLAVE] = {
    .descriptor_length = 3,
    .supports_name = true,
    .parsed_size = sizeof(Pbbe::I2cSlaveDescriptor),
 /* .default_name = */ "i2c",
  },
  [Pbbe::DT_SPI_SLAVE] = {
    .descriptor_length = 3,
    .supports_name = true,
    .parsed_size = sizeof(Pbbe::SpiSlaveDescriptor),
 /* .default_name = */ "spi",
  },
};

static const uint16_t i2c_speeds[] PROGMEM = {
  [0] = 100,  // Standard mode (100kbit/s)
  [1] = 400,  // Fast-mode (400 kbit/s)
  [2] = 1000, // Fast-mode plus (1 Mbit/s)
  [3] = 3400, // High-speed mode (3.4 Mbit/s)
};

static const uint32_t uart_speeds[] PROGMEM = {
  [0] = 0, // Unspecified
  [1] = 300,
  [2] = 600,
  [3] = 1200,
  [4] = 2400,
  [5] = 4800,
  [6] = 9600,
  [7] = 19200,
  [8] = 38400,
  [9] = 57600,
  [10] = 115200,
};

const Pbbe::PhysicalPinInfo Pbbe::physical_pin_info[] PROGMEM = {
  [0]  = {LogicalPin::NONE, "NC"},
  [1]  = {LogicalPin::NONE, "VUSB"},
  [2]  = {BACKPACK_BUS, "BKPK"},
  [3]  = {LogicalPin::NONE, "RST"},
  [4]  = {SCK, "SCK"},
  [5]  = {MISO, "MISO"},
  [6]  = {MOSI, "MOSI"},
  [7]  = {SS, "SS"},
  [8]  = {0, "RX0"},
  [9]  = {1, "TX0"},
  [10] = {2, "D2"},
  [11] = {3, "D3"},
  [12] = {4, "D4"},
  [13] = {5, "D5"},
  [14] = {6, "D6"},
  [15] = {7, "D7"},
  [16] = {8, "D8"},
  [17] = {LogicalPin::NONE, "3V3"},
  [18] = {LogicalPin::NONE, "GND"},
  [19] = {LogicalPin::NONE, "VBAT"},
  [20] = {13, "RX1"},
  [21] = {14, "TX1"},
  [22] = {SCL, "SCL"},
  [23] = {SDA, "SDA"},
  [24] = {LogicalPin::NONE, "REF"},
  [25] = {A0, "A0"},
  [26] = {A1, "A1"},
  [27] = {A2, "A2"},
  [28] = {A3, "A3"},
  [29] = {A4, "A4"},
  [30] = {A5, "A5"},
  [31] = {A6, "A6"},
  [32] = {A7, "A7"},
};

bool Pbbe::parseMinimalDescriptor(const Eeprom *eep, size_t offset, MinimalDescriptor *d) {
  if (offset + CHECKSUM_SIZE + 1 > eep->size) {
    DEBUG_ERR2(F("EEPROM too short for descriptor type. Descriptor start at: "), offset);
    return false;
  }

  const uint8_t *buf = &eep->raw[offset];
  size_t len = eep->size - offset - CHECKSUM_SIZE;

  d->type = (DescriptorType)buf[0];

  if (d->type == DT_RESERVED || d->type > lengthof(descriptor_type_info)) {
    DEBUG_ERR2(F("Invalid or unknown descriptor type. Descriptor start at: "), offset);
    return false;
  }

  d->descriptor_length = pgm_read_byte(&descriptor_type_info[d->type].descriptor_length);
  bool has_name = pgm_read_byte(&descriptor_type_info[d->type].supports_name);

  if (len < d->descriptor_length) {
    DEBUG_ERR2(F("EEPROM too short for descriptor. Descriptor start at: "), offset);
    return false;
  }

  switch (d->type) {
    case DT_DATA:
      // Read the length of the data in the descriptor
      d->descriptor_length += buf[1];
      if (len < d->descriptor_length) {
        DEBUG_ERR2(F("EEPROM too short for data descriptor. Descriptor start at: "), offset);
        return false;
      }

      if (!(buf[1] & 0x80))
        has_name = false;
      break;
    case DT_I2C_SLAVE:
    case DT_SPI_SLAVE:
      if (!(buf[1] & 0x80))
        has_name = false;
      break;
    case DT_UART:
      if (!(buf[3] & 0x80))
        has_name = false;
      break;
    default:
      /* Nothing */
      break;
  }

  if (has_name) {
    d->name_length = stringLength(eep, offset + d->descriptor_length);
    if (!d->name_length) {
      DEBUG_ERR2(F("Invalid descriptor name. Descriptor start at: "), offset);
      return false;
    }
  } else {
    d->name_length = 0;
  }
  return true;
}

bool Pbbe::parseDescriptorA(const Eeprom *eep, DescriptorInfo *info)
{
  // Do a first-pass parse
  MinimalDescriptor min;
  if (!parseMinimalDescriptor(eep, info->offset, &min))
    return false;

  const DescriptorTypeInfo &tinfo = descriptor_type_info[min.type];
  const uint8_t *buf = &eep->raw[info->offset];

  uint8_t parsed_size = pgm_read_byte(&tinfo.parsed_size);
  bool supports_name = pgm_read_byte(&tinfo.supports_name);

  // Figure out how much memory to allocate. That's the parsed_size,
  // plus the length of the name plus trailing zero.
  uint8_t size = parsed_size;

  // Add extra allocation size if needed
  switch (min.type) {
    case DT_DATA:
      // Allocate the actual data after the name
      size += buf[1];
      break;
    default:
      // nothing
      break;
  }

  // Add allocation size for the name, if any
  if (supports_name) {
    if (min.name_length)
      size += min.name_length;
    else
      size += strlen_P(tinfo.default_name);
    // Trailing zero
    size++;
  }

  // Allocate memory
  info->parsed = (Descriptor*)malloc(size);
  if (!info->parsed) {
    Serial.println(F("Failed to allocate memory for parsed descriptor"));
    return false;
  }

  // Parse descriptor
  switch (min.type) {
    case DT_SPI_SLAVE: {
      SpiSlaveDescriptor& d = static_cast<SpiSlaveDescriptor&>(*info->parsed);
      d.ss_pin = buf[1] & PIN_MASK;
      d.speed = buf[2];
      break;
    }
    case DT_UART: {
      UartDescriptor& d = static_cast<UartDescriptor&>(*info->parsed);
      d.tx_pin = buf[1] & PIN_MASK;
      d.tx_pin = buf[2] & PIN_MASK;
      if ((buf[3] & 0xf) >= lengthof(uart_speeds)) {
        DEBUG_ERR2(F("Invalid UART speed. Descriptor start at: "), info->offset);
        goto fail;
      }

      d.speed = pgm_read_dword(&uart_speeds[buf[3] & 0xf]);
      break;
    }
    case DT_IOPIN: {
      IoPinDescriptor& d = static_cast<IoPinDescriptor&>(*info->parsed);
      d.pin = buf[1] & PIN_MASK;
      break;
    }
    case DT_GROUP: {
      // GroupDescriptor& d = static_cast<GroupDescriptor&>(*info->parsed);
      // Only has a name
      break;
    }
    case DT_POWER_USAGE: {
      PowerUsageDescriptor& d = static_cast<PowerUsageDescriptor&>(*info->parsed);
      d.power_pin = buf[1] & PIN_MASK;
      d.minimum = buf[2];
      d.typical = buf[3];
      d.maximum = buf[4];
      break;
    }
    case DT_I2C_SLAVE: {
      I2cSlaveDescriptor& d = static_cast<I2cSlaveDescriptor&>(*info->parsed);
      d.addr = buf[1] & 0x7f;
      if ((buf[2] & 0x3) >= lengthof(i2c_speeds)) {
        DEBUG_ERR2(F("Invalid I2c speed. Descriptor start at: "), info->offset);
        goto fail;
      }
      d.speed = pgm_read_word(&i2c_speeds[buf[2] & 0x3]);
      break;
    }
    case DT_DATA: {
      DataDescriptor& d = static_cast<DataDescriptor&>(*info->parsed);
      // Read the length of the data in the descriptor
      d.length = buf[1];
      // We allocated extra memory at the end of the buffer, after the
      // name. Make d->data point to that memory.
      d.data = (uint8_t *)info->parsed + size - d.length;
      memcpy(d.data, &buf[2], d.length);
      break;
    }
    default: {
      // Should never occur
      break;
    }
  }

  char *name;
  name = (char *)info->parsed + parsed_size;
  // Skip to the name, if any and copy it
  if (supports_name) {
    if (min.name_length) {
      size_t offset = info->offset + min.descriptor_length;
      extractString(eep, offset, name, min.name_length);
    } else {
      strcpy_P(name, tinfo.default_name);
    }
  }

  return true;

fail:
  free(info->parsed);
  info->parsed = NULL;
  return false;
}

Pbbe::Eeprom *Pbbe::getEeprom(PBBP &pbbp, uint8_t addr)
{
  uint8_t buf[3];
  // Read the first 3 bytes
  if (!pbbp.readEeprom(addr, 0, buf, sizeof(buf))) {
    Serial.println("EEPROM read failed: ");
    pbbp.printLastError(Serial);
    return NULL;
  }

  // Check EEPROM version
  if (buf[0] == 0 || buf[0] > 1) {
    Serial.print("Unsupported EEPROM version: ");
    Serial.print(buf[0]);
    return NULL;
  }

  // Get the used size of the EEPROM
  uint8_t used_size = buf[2];
  // Allocate memory for that
  Eeprom *eep = (Eeprom*)malloc(sizeof(Eeprom) + used_size);
  if (!eep) {
    Serial.println("Memory allocation for EEPROM failed");
    return NULL;
  }

  eep->size = used_size;

  // And read the full EEPROM
  if (!pbbp.readEeprom(addr, 0, eep->raw, used_size)) {
    Serial.println("EEPROM read failed: ");
    pbbp.printLastError(Serial);
    free(eep);
    return NULL;
  }

  // Verify checksum
  uint16_t calc_checksum = eepromChecksum(eep->raw, used_size - CHECKSUM_SIZE);
  uint16_t read_checksum = eep->raw[used_size - 2] << 8 | eep->raw[used_size - 1];
  if (read_checksum != calc_checksum) {
    Serial.println("EEPROM checksum incorrect");
    free(eep);
    return NULL;
  }

  return eep;
}

bool Pbbe::writeEeprom(PBBP &pbbp, uint8_t addr, const Eeprom *eeprom)
{
  if (!pbbp.writeEeprom(addr, 0, eeprom->raw, eeprom->size)) {
    Serial.println("EEPROM write failed: ");
    pbbp.printLastError(Serial);
    return false;
  }
  return true;
}

Pbbe::Eeprom *Pbbe::updateEeprom(Eeprom *eep, size_t offset, const uint8_t *buf, uint8_t length)
{
  if (!eep && offset != 0) {
    Serial.println(F("Cannot write EEPROM from sketch with non-zero offset"));
    return NULL;
  }
  if (eep && offset > eep->size) {
    Serial.println(F("Cannot update EEPROM starting past the end of the EEPROM"));
    return NULL;
  }

  // Enlarge the buffer if we would write past the end or allocate one
  // in the first place if none was passed in
  if (!eep || offset + length + CHECKSUM_SIZE > eep->size) {
    eep = (Eeprom*)realloc(eep, sizeof(*eep) + offset + length + CHECKSUM_SIZE);
    if (!eep) {
      Serial.println("Failed to allocate memory for EEPROM update");
      return NULL;
    }
    eep->size = offset + length + CHECKSUM_SIZE;
  }

  // Update the contents, but skip readonly bytes
  for (size_t i = 0; i < length; ++i)
    if (!isReadonly(eep, offset + i))
      eep->raw[offset + i] = buf[i];

  // See if the new contents still look valid
  DescriptorList *list = parseDescriptorListA(eep);
  if (!list) {
    Serial.println("Could not parse updated EEPROM");
    return NULL;
  }

  // Now, find out where the end of the last descriptor is and update
  // the "used size" in the header accordingly. This can change when
  // writing 0xff at the end of the EEPROM, or when the EEPROM was
  // ressized above.
  size_t last_offset = list->info[list->num_descriptors - 1].offset;
  MinimalDescriptor min;
  parseMinimalDescriptor(eep, last_offset, &min);
  size_t used_size = last_offset + min.descriptor_length + min.name_length + CHECKSUM_SIZE;
  free(list);
  list = NULL;

  eep->raw[USED_SIZE_OFFSET] = used_size;
  // Shrink the buffer if needed
  if (used_size != eep->size) {
    eep = (Eeprom *)realloc(eep, sizeof(Eeprom) + used_size);
    eep->size = used_size;
  }

  // Update the checksum
  uint16_t checksum = eepromChecksum(eep->raw, used_size - CHECKSUM_SIZE);
  eep->raw[used_size - CHECKSUM_SIZE] = checksum >> 8;
  eep->raw[used_size - CHECKSUM_SIZE + 1] = checksum & 0xff;

  return eep;
}

uint8_t Pbbe::uniqueIdChecksum(uint8_t *buf)
{
  return pinoccio_crc_generate<uint8_t>(PBBP::UNIQUE_ID_CRC_POLY, 0, buf, PBBP::UNIQUE_ID_LENGTH - 1);
}

uint16_t Pbbe::eepromChecksum(uint8_t *buf, size_t length)
{
  return pinoccio_crc_generate<uint16_t>(EEPROM_CRC_POLY, 0, buf, length);
}

bool Pbbe::isReadonly(const Eeprom *eep, size_t offset) {
  return offset >= UNIQUE_ID_OFFSET &&
         offset < UNIQUE_ID_OFFSET + PBBP::UNIQUE_ID_LENGTH;
}

/* vim: set filetype=cpp sw=2 sts=2 expandtab: */
