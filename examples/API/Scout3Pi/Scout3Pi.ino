#include "config.h"
#include <Pinoccio.h>

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

int leftMotor = 0;
int rightMotor = 0;
int safetyTimer = 50000; // start in a disabled state

void initialize3Pi() {
  Serial1.write(PI_SIGNATURE);
  RgbLed.blinkRed();
  RgbLed.blinkRed();
  RgbLed.blinkRed();
  Serial1.write(PI_CLEAR_LCD);
  Serial1.write(PI_PRINT);
  Serial1.write(8);
  Serial1.print("Pinoccio");
  Serial1.write(PI_LCD_GOTO_XY);
  Serial1.write(0);
  Serial1.write(1);
  Serial1.write(PI_PRINT);
  Serial1.write(8);
  Serial1.print("Lets go!");
}

void getMotorPayload(byte* payload, unsigned int length) {
  int i=0, j=0;
  char buf[10];
  int forward = 0;
  int turn = 0;
  
  while (payload[i] != ':') {
    buf[j++] = payload[i++];
  }
  buf[i++] = '\0';
  leftMotor = atoi(buf);
  j = 0;
  
  while (i < length) {
    buf[j++] = payload[i++];
  }
  buf[j++] = '\0';
  rightMotor = atoi(buf);
}

static bool appDataInd(NWK_DataInd_t *ind) {
  Serial.println("Received control message");
  safetyTimer = 0;

  getMotorPayload(ind->data, ind->size);
  Serial.print("leftMotor: ");
  Serial.println(leftMotor);
  Serial.print("rightMotor: ");
  Serial.println(rightMotor);

  if (leftMotor >= 0) {
    Serial1.write(PI_M1_FORWARD);
  } else {
    Serial1.write(PI_M1_BACKWARD);
  }
  Serial1.write(abs(leftMotor));
  if (rightMotor >= 0) {
    Serial1.write(PI_M2_FORWARD);
  } else {
    Serial1.write(PI_M2_BACKWARD);
  }
  Serial1.write(abs(rightMotor));
}

void setup() {
  Pinoccio.init();
  initialize3Pi();
  
  RgbLed.blinkRed(200);
  RgbLed.blinkGreen(200);
  RgbLed.blinkBlue(200);
  Serial.println("3pi Scout ready for duty");

  NWK_SetAddr(APP_MESH_ADDR);
  NWK_SetPanId(APP_MESH_PANID);
  PHY_SetChannel(APP_MESH_CHANNEL);
  PHY_SetRxState(true);
  
  NWK_OpenEndpoint(1, appDataInd);
}

void loop() {
  Pinoccio.loop();
  
  if (safetyTimer > 10000) {
    RgbLed.red();
    leftMotor = rightMotor = 0;
    Serial1.write(PI_STOP_PID); // stop all motors
  } else {
    safetyTimer++;
    RgbLed.green();
  }
}