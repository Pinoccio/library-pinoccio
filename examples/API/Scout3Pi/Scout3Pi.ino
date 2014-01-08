#include <Scout.h>
#include <Wire.h>
#include <SPI.h>

// http://www.pololu.com/docs/0J21/10.a
enum {
  PI_SIGNATURE           = 0x81,
  PI_RAW_SENSORS         = 0x86,
  PI_CALIBRATED_SENSORS  = 0x87,
  PI_TRIMPOT             = 0xB0,
  PI_BATTERY_MV          = 0xB1,
  PI_PLAY_MUSIC          = 0xB3,
  PI_CALIBRATE           = 0xB4,
  PI_RESET_CALIBRATION   = 0xB5,
  PI_LINE_POSITION       = 0xB6,
  PI_CLEAR_LCD           = 0xB7,
  PI_PRINT               = 0xB8,
  PI_LCD_GOTO_XY         = 0xB9,
  PI_AUTOCALIBRATE       = 0xBA,
  PI_START_PID           = 0xBB,
  PI_STOP_PID            = 0xBC,
  PI_M1_FORWARD          = 0xC1,
  PI_M1_BACKWARD         = 0xC2,
  PI_M2_FORWARD          = 0xC5,
  PI_M2_BACKWARD         = 0xC6
};

int8_t leftMotor = 0;
int8_t rightMotor = 0;
int safetyTimer = 1100; // start in a disabled state
int stopped = true;

void initialize3Pi() {
  Serial1.begin(115200);
  delay(100);
  RgbLed.blinkRed();
  Serial1.write(PI_SIGNATURE);
  Serial1.write(PI_CLEAR_LCD);
  Serial1.write(PI_PRINT);
  Serial1.write(0x08);
  Serial1.print("Pinoccio");
  Serial1.write(PI_LCD_GOTO_XY);
  Serial1.write((byte)0x00);
  Serial1.write(0x01);
  Serial1.write(PI_PRINT);
  Serial1.print(8);
  Serial1.print("Lets go!");
}

void setup() {
  Scout.setup();
  initialize3Pi();

  RgbLed.blinkRed(200);
  RgbLed.blinkGreen(200);
  RgbLed.blinkBlue(200);
  Serial.println("3pi Scout ready for duty");

  Scout.meshListen(1, receiveMessage);
}

void loop() {
  Scout.loop();

  if (safetyTimer > 1000) {
    RgbLed.red();
    leftMotor = rightMotor = 0;
    if (!stopped) {
      Serial1.write(PI_STOP_PID); // stop all motors
      stopped = true;
    }
  } else {
    safetyTimer++;
    RgbLed.green();
  }
}

static bool receiveMessage(NWK_DataInd_t *ind) {
  Serial.println("Received control message");
  safetyTimer = 0;
  stopped = false;

  getMotorPayload(ind->data, ind->size);
  Serial.print(abs(leftMotor));
  Serial.print(":");
  Serial.println(abs(rightMotor));

  if (leftMotor >= 0) {
    Serial1.write(PI_M1_FORWARD);
  } else {
    Serial1.write(PI_M1_BACKWARD);
  }
  Serial1.write((byte)abs(leftMotor));

  if (rightMotor > 0) {
    Serial1.write(PI_M2_FORWARD);
    Serial1.write(abs(rightMotor));
  } else if (rightMotor < 0) {
    Serial1.write(PI_M2_BACKWARD);
    Serial1.write(abs(rightMotor));
  } else {
    Serial1.write(PI_M2_FORWARD);
    Serial1.write((byte)0x00);
  }

  return true;
}

void getMotorPayload(byte* payload, unsigned int length) {
  int i=0, j=0;
  char buf[10];

  while (payload[i] != ':') {
    buf[j++] = payload[i++];
  }
  buf[i++] = '\0';
  leftMotor = atoi(buf);
  if (leftMotor < -127 || leftMotor > 127) {
    leftMotor = 0;
  }
  j = 0;

  while (i < length) {
    buf[j++] = payload[i++];
  }
  buf[j++] = '\0';
  rightMotor = atoi(buf);
  if (rightMotor < -127 || rightMotor > 127) {
    rightMotor = 0;
  }
}