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

WIFI_PROFILE profile = {
                /* SSID */ "",
 /* WPA/WPA2 passphrase */ "",
          /* IP address */ "",
         /* subnet mask */ "",
          /* Gateway IP */ "" };

//IPAddress server(66,175,218,211);
IPAddress server(85,119,83,194);

PinoccioWifiClient wifiClient;
mqttClient mqtt(server, 1883, mqttReceive, wifiClient);

int leftMotor = 0;
int rightMotor = 0;
int safetyTimer = 10000; // start in a disabled state

void setup() {
  Pinoccio.init();
  Wifi.begin(&profile); 
  initialize3Pi();
  
  if (mqtt.connect("pinoccio")) {     
    mqtt.subscribe("erictj/3pi-control");
    mqtt.publish("erictj/3pi-telemetry", "Pinoccio 3Pi ready to go!");
  }
}

void loop() {
  Pinoccio.loop();
  mqtt.loop();
  
  if (safetyTimer > 1000) {
    RgbLed.red();
    leftMotor = rightMotor = 0;
    Serial.write(PI_STOP_PID); // stop all motors
  } else {
    safetyTimer++;
    RgbLed.green();
  }
}

void mqttReceive(char* topic, byte* payload, unsigned int length) { 
  RgbLed.cyan();
  safetyTimer = 0;

  getMotorPayload(payload, length);
  
  if (leftMotor >= 0) {
    Serial.write(PI_M1_FORWARD);
  } else {
    Serial.write(PI_M1_BACKWARD);
  }
  Serial.write(abs(leftMotor));
  if (rightMotor >= 0) {
    Serial.write(PI_M2_FORWARD);
  } else {
    Serial.write(PI_M2_BACKWARD);
  }
  Serial.write(abs(rightMotor));
}

void initialize3Pi() {
  Serial.write(PI_SIGNATURE);
  RgbLed.blinkRed();
  RgbLed.blinkRed();
  RgbLed.blinkRed();
  Serial.write(PI_CLEAR_LCD);
  Serial.write(PI_PRINT);
  Serial.write(8);
  Serial.print("Pinoccio");
  Serial.write(PI_LCD_GOTO_XY);
  Serial.write(0);
  Serial.write(1);
  Serial.write(PI_PRINT);
  Serial.write(8);
  Serial.print("Lets go!");
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