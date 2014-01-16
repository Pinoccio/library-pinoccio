// Note: Programming of the Flash with gs_flashprogram only seems to
// work through UART and needs ESCAPE_NON_PRINT disabled
//#define USE_SPI
//#define ESCAPE_NON_PRINT

#ifdef USE_SPI
#include <SPI.h>
#endif

void display_byte(uint8_t c)
{
#ifdef ESCAPE_NON_PRINT
  if (!isprint(c) && !isspace(c)) {
    Serial.write('\\');
    if (c < 0x10) Serial.write('0');
    Serial.print(c, HEX);
    return;
  }
#endif
  Serial.write(c);
}
void setup() {
  Serial.begin(115200);
#ifdef USE_SPI
  // Max 3.5Mhz
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.begin();
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
#else
  Serial1.begin(115200);
#endif
  // uncomment these if you want to put the WiFi module into firmware update mode
  //  pinMode(6, OUTPUT);
  //  digitalWrite(6, HIGH);

  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, LOW);
  delay(1000);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
}

#ifdef USE_SPI

#define  GS_SPI_ESC_CHAR               (0xFB)    /* Start transmission indication */
#define  GS_SPI_IDLE_CHAR              (0xF5)    /* synchronous IDLE */
#define  GS_SPI_XOFF_CHAR              (0xFA)    /* Stop transmission indication */
#define  GS_SPI_XON_CHAR               (0xFD)    /* Start transmission indication */      
#define  GS_SPI_INVALID_DATA_ALL_ONE   (0xFF)    /* Invalid character possibly recieved during reboot */
#define  GS_SPI_INVALID_DATA_ALL_ZERO  (0x00)    /* Invalid character possibly recieved during reboot */
#define  GS_SPI_READY_ACK              (0xF3)    /* SPI link ready */
#define  GS_SPI_ESC_XOR                (0x20)    /* Xor mask for escaped data bytes */

uint8_t send_receive(uint8_t c) {
  digitalWrite(7, LOW);
  c = SPI.transfer(c);
  digitalWrite(7, HIGH);
  return c;
}

// Send a byte as-is, don't do any additional stuffing
void send_byte(uint8_t c) {
  uint8_t in = send_receive(c);
  switch (in) {
   case GS_SPI_XON_CHAR:
    case GS_SPI_XOFF_CHAR:
    case GS_SPI_IDLE_CHAR:
    case GS_SPI_INVALID_DATA_ALL_ONE:
   case GS_SPI_INVALID_DATA_ALL_ZERO:
    case GS_SPI_READY_ACK:
      // Ignore these for now...
      break;
    case GS_SPI_ESC_CHAR:
      in = send_receive(GS_SPI_IDLE_CHAR) ^ GS_SPI_ESC_XOR;
      // fallthrough
    default:
      display_byte(in);
  }
}

// Send the given byte, adding stuffing if needed
void send_and_stuff_byte(uint8_t c) {
  if( (GS_SPI_ESC_CHAR  == c) ||
      (GS_SPI_XON_CHAR  == c) ||
      (GS_SPI_XOFF_CHAR == c) ||
      (GS_SPI_IDLE_CHAR == c) ||
      (GS_SPI_INVALID_DATA_ALL_ONE == c) ||
      (GS_SPI_INVALID_DATA_ALL_ZERO == c)||
      (GS_SPI_READY_ACK == c) ) {
          send_byte(GS_SPI_ESC_CHAR);
          send_byte(c ^ GS_SPI_ESC_XOR);
      } else {
          send_byte(c);
      }
}

void loop() {
  int out = Serial.read();
  if (out == -1)
    send_byte(GS_SPI_IDLE_CHAR);
  else
    send_and_stuff_byte(out);
}

#else // !SPI
void loop() {
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    display_byte(Serial1.read());
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
}
#endif
