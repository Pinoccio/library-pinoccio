#include <Arduino.h>
#include <Pinoccio.h>
#include <bitlash.h>
#include <avr/eeprom.h>

#if defined(__AVR_ATmega128RFA1__)
#include "atmega128rfa1.h"
#elif defined(__AVR_ATmega256RFR2__)
#include "atmega256rfr2.h"
#endif

PinoccioClass Pinoccio;

PinoccioClass::PinoccioClass() {
  shellEnabled = true;
}

PinoccioClass::~PinoccioClass() { }

void PinoccioClass::disableShell() {
  shellEnabled = false;
}

void PinoccioClass::setup() {
  if (shellEnabled) {
    initBitlash(115200);
  }

  SYS_Init();
  HAL_MeasureAdcOffset();
  PHY_RandomReq();

  loadSettingsFromEeprom();
}

void PinoccioClass::loop() {
  SYS_TaskHandler();

  if (shellEnabled) {
    runBitlash();
  }
}

void PinoccioClass::goToSleep(uint32_t sleepForMs) {
  // TODO  http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=136036
  // - put radio to sleep
  // - set all GPIO pins to inputs
  // - turn off ADC
  // - turn off backpack power
  // - put MCU to sleep
}

int8_t PinoccioClass::getTemperature() {
  return HAL_MeasureTemperature();
}

void PinoccioClass::setHQToken(const char *token) {
  for (int i=0; i<32; i++) {
    eeprom_update_byte((uint8_t *)8130+i, token[i]);
  }
}

void PinoccioClass::getHQToken(char *token) {
  for (int i=0; i<32; i++) {
    token[i] = eeprom_read_byte((uint8_t *)8130+i);
  }
}

uint32_t PinoccioClass::getHwSerial() {
  return eeprom_read_dword((uint32_t *)8184);
}

uint16_t PinoccioClass::getHwFamily() {
  return eeprom_read_word((uint16_t *)8188);
}

uint8_t PinoccioClass::getHwVersion() {
  return eeprom_read_byte((uint8_t *)8190);
}

uint8_t PinoccioClass::getEEPROMVersion() {
  return eeprom_read_byte((uint8_t *)8191);
}

void PinoccioClass::sendStateToHQ() {
  // TODO - Send state to HQ, and set pin values and pinmodes from response
}

void PinoccioClass::loadSettingsFromEeprom() {
  // Address 8127 - 3 bytes  - Torch color (R,G,B)
  // Address 8130 - 32 bytes - HQ Token
  // Address 8162 - 16 bytes - Security Key
  // Address 8178 - 1 byte   - Transmitter Power
  // Address 8179 - 1 byte   - Frequency Channel
  // Address 8180 - 2 bytes  - Network Identifier/Troop ID
  // Address 8182 - 2 bytes  - Network Address/Scout ID
  // Address 8184 - 4 bytes  - Unique ID
  // Address 8188 - 2 bytes  - HW family
  // Address 8190 - 1 byte   - HW Version
  // Address 8191 - 1 byte   - EEPROM Version
  byte buffer[32];

  for (int i=0; i<32; i++) {
    buffer[i] = eeprom_read_byte((uint8_t *)8130+i);
  }
  setHQToken((char *)buffer);
  memset(buffer, 0x00, 32);

  for (int i=0; i<16; i++) {
    buffer[i] = eeprom_read_byte((uint8_t *)8162+i);
  }
  meshSetSecurityKey((char *)buffer);
  memset(buffer, 0x00, 16);

  if (eeprom_read_word((uint16_t *)8182) != 0xFFFF ||
      eeprom_read_word((uint16_t *)8180) != 0xFFFF ||
      eeprom_read_byte((uint8_t *)8179) != 0xFF) {
    meshSetRadio(eeprom_read_word((uint16_t *)8182), eeprom_read_word((uint16_t *)8180), eeprom_read_byte((uint8_t *)8179));
  }
  if (eeprom_read_byte((uint8_t *)8178) != 0xFF) {
    meshSetPower(eeprom_read_byte((uint8_t *)8178));
  }
}

void PinoccioClass::meshSetRadio(const uint16_t theAddress, const uint16_t thePanId, const uint8_t theChannel) {
  NWK_SetAddr(theAddress);
  address = theAddress;
  NWK_SetPanId(thePanId);
  panId = thePanId;
  PHY_SetChannel(theChannel);
  channel = theChannel;
  PHY_SetRxState(true);

  eeprom_update_word((uint16_t *)8182, address);
  eeprom_update_word((uint16_t *)8180, panId);
  eeprom_update_byte((uint8_t *)8179, channel);

  meshSetPower(0);
}


void PinoccioClass::meshSetPower(const uint8_t theTxPower) {
  /* Page 116 of the 256RFR2 datasheet
    0   3.5 dBm
    1   3.3 dBm
    2   2.8 dBm
    3   2.3 dBm
    4   1.8 dBm
    5   1.2 dBm
    6   0.5 dBm
    7  -0.5 dBm
    8  -1.5 dBm
    9  -2.5 dBm
    10 -3.5 dBm
    11 -4.5 dBm
    12 -6.5 dBm
    13 -8.5 dBm
    14 -11.5 dBm
    15 -16.5 dBm
  */
  PHY_SetTxPower(theTxPower);
  txPower = theTxPower;
  eeprom_update_byte((uint8_t *)8178, theTxPower);
}

void PinoccioClass::meshSetSecurityKey(const char *key) {
  NWK_SetSecurityKey((uint8_t *)key);

  for (int i=0; i<16; i++) {
    eeprom_update_byte((uint8_t *)8162+i, key[i]);
  }
}

void PinoccioClass::meshResetSecurityKey(void) {
  const char buf[16] = {0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF,0xFF, 0xFF, 0xFF, 0xFF};
  meshSetSecurityKey(buf);
}

void PinoccioClass::meshSendMessage(MeshRequest request) {
  NWK_DataReq_t* req = request.getRequest();
  Serial.print("sending message to: ");
  Serial.print(req->dstAddr);
  Serial.print(":");
  Serial.println(req->dstEndpoint);
  NWK_DataReq(request.getRequest());
}

void PinoccioClass::meshListen(uint8_t endpoint, bool (*handler)(NWK_DataInd_t *ind)) {
  NWK_OpenEndpoint(endpoint, handler);
}

void PinoccioClass::meshJoinGroup(uint16_t groupAddress) {
  if (!NWK_GroupIsMember(groupAddress)) {
    NWK_GroupAdd(groupAddress);
  }
}

void PinoccioClass::meshLeaveGroup(uint16_t groupAddress) {
  if (NWK_GroupIsMember(groupAddress)) {
    NWK_GroupRemove(groupAddress);
  }
}

bool PinoccioClass::meshIsInGroup(uint16_t groupAddress) {
  return NWK_GroupIsMember(groupAddress);
}

uint16_t PinoccioClass::getAddress() {
  return address;
}

uint16_t PinoccioClass::getPanId() {
  return panId;
}

uint8_t PinoccioClass::getChannel() {
  return channel;
}

uint8_t PinoccioClass::getTxPower() {
  return txPower;
}

const char* PinoccioClass::getTxPowerDb() {
  switch (txPower) {
    case 0:
      return PSTR("3.5 dBm");
      break;
    case 1:
      return PSTR("3.3 dBm");
      break;
    case 2:
      return PSTR("2.8 dBm");
      break;
    case 3:
      return PSTR("2.3 dBm");
      break;
    case 4:
      return PSTR("1.8 dBm");
      break;
    case 5:
      return PSTR("1.2 dBm");
      break;
    case 6:
      return PSTR("0.5 dBm");
      break;
    case 7:
      return PSTR("-0.5 dBm");
      break;
    case 8:
      return PSTR("-1.5 dBm");
      break;
    case 9:
      return PSTR("-2.5 dBm");
      break;
    case 10:
      return PSTR("-3.5 dBm");
      break;
    case 11:
      return PSTR("-4.5 dBm");
      break;
    case 12:
      return PSTR("-6.5 dBm");
      break;
    case 13:
      return PSTR("-8.5 dBm");
      break;
    case 14:
      return PSTR("-11.5 dBm");
      break;
    case 15:
      return PSTR("-16.5 dBm");
      break;
    default:
      return PSTR("unknown");
      break;
  }
}