#include <SPI.h>
#include <Wire.h>
#include <LeadScout.h>
#include <PBBP.h>
extern "C" {
#include <j0g.h>
}

const uint16_t groupId = 0x1234;
static byte pingCounter = 0;
static NWK_DataReq_t appDataReq;

// TEMPORARY, must change this for wifi, and also have run "scout.sethqtoken(TOKEN)" and "mesh.config(ID)" to provision
WIFI_PROFILE wifis = {"Air Patrol","Jabber","","","",};

// use this if your lead scout doesn't have the backpack bus supporting firmware
bool forceLeadScout = true;

// this stuff should prob all be in the Scout class or somesuch but putting it here to get started
static bool fieldCommands(NWK_DataInd_t *ind);
int leadAnswerID = 0;
static bool leadAnswers(NWK_DataInd_t *ind);
static bool leadAnnouncements(NWK_DataInd_t *ind);
void leadAnnouncementSend(int chan, int from, char *message);
IPAddress server(173,255,220,185);
//IPAddress server(192,168,0,36);
int HQport = 22756;
int whoami;
bool leadScout;
PinoccioWifiClient leadClient;
void leadHQ(void);
void leadSignal(char *json);
void leadIncoming(char *packet, unsigned short *index);
NWK_DataReq_t fieldAnnounceReq;
void fieldAnnounce(char *line); // called by bitlashFilter
void bitlashFilter(byte b);
char *bitlashOutput; // used by bitlashBuffer
void bitlashBuffer(byte b);


// Example code to dump backpack EEPROM contents:
void print_hex(const uint8_t *buf, uint8_t len) {
    while (len--) {
        if (*buf < 0x10) Serial.print("0");
        Serial.print(*buf++, HEX);
    }
}

void dump_backpacks() {
  Serial.print("Found ");
  Serial.print(Scout.bp.num_slaves);
  Serial.println(" slaves");

  for (uint8_t i = 0; i < Scout.bp.num_slaves; ++i) {
    print_hex(Scout.bp.slave_ids[i], sizeof(Scout.bp.slave_ids[0]));
    Serial.println();
    uint8_t buf[64];
    Scout.bp.readEeprom(i + 1, 0, buf, sizeof(buf));
    Serial.print("EEPROM: ");
    print_hex(buf, sizeof(buf));
    Serial.println();
  }
}
// Geotrust Global CA (used by google.com)
unsigned char cert[] = {0x30, 0x82, 0x03, 0x54, 0x30, 0x82, 0x02, 0x3c, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x03, 0x02, 0x34, 0x56, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x30, 0x32, 0x30, 0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x32, 0x32, 0x30, 0x35, 0x32, 0x31, 0x30, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x30, 0x42, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x0d, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31, 0x1b, 0x30, 0x19, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x12, 0x47, 0x65, 0x6f, 0x54, 0x72, 0x75, 0x73, 0x74, 0x20, 0x47, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x43, 0x41, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xda, 0xcc, 0x18, 0x63, 0x30, 0xfd, 0xf4, 0x17, 0x23, 0x1a, 0x56, 0x7e, 0x5b, 0xdf, 0x3c, 0x6c, 0x38, 0xe4, 0x71, 0xb7, 0x78, 0x91, 0xd4, 0xbc, 0xa1, 0xd8, 0x4c, 0xf8, 0xa8, 0x43, 0xb6, 0x03, 0xe9, 0x4d, 0x21, 0x07, 0x08, 0x88, 0xda, 0x58, 0x2f, 0x66, 0x39, 0x29, 0xbd, 0x05, 0x78, 0x8b, 0x9d, 0x38, 0xe8, 0x05, 0xb7, 0x6a, 0x7e, 0x71, 0xa4, 0xe6, 0xc4, 0x60, 0xa6, 0xb0, 0xef, 0x80, 0xe4, 0x89, 0x28, 0x0f, 0x9e, 0x25, 0xd6, 0xed, 0x83, 0xf3, 0xad, 0xa6, 0x91, 0xc7, 0x98, 0xc9, 0x42, 0x18, 0x35, 0x14, 0x9d, 0xad, 0x98, 0x46, 0x92, 0x2e, 0x4f, 0xca, 0xf1, 0x87, 0x43, 0xc1, 0x16, 0x95, 0x57, 0x2d, 0x50, 0xef, 0x89, 0x2d, 0x80, 0x7a, 0x57, 0xad, 0xf2, 0xee, 0x5f, 0x6b, 0xd2, 0x00, 0x8d, 0xb9, 0x14, 0xf8, 0x14, 0x15, 0x35, 0xd9, 0xc0, 0x46, 0xa3, 0x7b, 0x72, 0xc8, 0x91, 0xbf, 0xc9, 0x55, 0x2b, 0xcd, 0xd0, 0x97, 0x3e, 0x9c, 0x26, 0x64, 0xcc, 0xdf, 0xce, 0x83, 0x19, 0x71, 0xca, 0x4e, 0xe6, 0xd4, 0xd5, 0x7b, 0xa9, 0x19, 0xcd, 0x55, 0xde, 0xc8, 0xec, 0xd2, 0x5e, 0x38, 0x53, 0xe5, 0x5c, 0x4f, 0x8c, 0x2d, 0xfe, 0x50, 0x23, 0x36, 0xfc, 0x66, 0xe6, 0xcb, 0x8e, 0xa4, 0x39, 0x19, 0x00, 0xb7, 0x95, 0x02, 0x39, 0x91, 0x0b, 0x0e, 0xfe, 0x38, 0x2e, 0xd1, 0x1d, 0x05, 0x9a, 0xf6, 0x4d, 0x3e, 0x6f, 0x0f, 0x07, 0x1d, 0xaf, 0x2c, 0x1e, 0x8f, 0x60, 0x39, 0xe2, 0xfa, 0x36, 0x53, 0x13, 0x39, 0xd4, 0x5e, 0x26, 0x2b, 0xdb, 0x3d, 0xa8, 0x14, 0xbd, 0x32, 0xeb, 0x18, 0x03, 0x28, 0x52, 0x04, 0x71, 0xe5, 0xab, 0x33, 0x3d, 0xe1, 0x38, 0xbb, 0x07, 0x36, 0x84, 0x62, 0x9c, 0x79, 0xea, 0x16, 0x30, 0xf4, 0x5f, 0xc0, 0x2b, 0xe8, 0x71, 0x6b, 0xe4, 0xf9, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x53, 0x30, 0x51, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xc0, 0x7a, 0x98, 0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d, 0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xc0, 0x7a, 0x98, 0x68, 0x8d, 0x89, 0xfb, 0xab, 0x05, 0x64, 0x0c, 0x11, 0x7d, 0xaa, 0x7d, 0x65, 0xb8, 0xca, 0xcc, 0x4e, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x35, 0xe3, 0x29, 0x6a, 0xe5, 0x2f, 0x5d, 0x54, 0x8e, 0x29, 0x50, 0x94, 0x9f, 0x99, 0x1a, 0x14, 0xe4, 0x8f, 0x78, 0x2a, 0x62, 0x94, 0xa2, 0x27, 0x67, 0x9e, 0xd0, 0xcf, 0x1a, 0x5e, 0x47, 0xe9, 0xc1, 0xb2, 0xa4, 0xcf, 0xdd, 0x41, 0x1a, 0x05, 0x4e, 0x9b, 0x4b, 0xee, 0x4a, 0x6f, 0x55, 0x52, 0xb3, 0x24, 0xa1, 0x37, 0x0a, 0xeb, 0x64, 0x76, 0x2a, 0x2e, 0x2c, 0xf3, 0xfd, 0x3b, 0x75, 0x90, 0xbf, 0xfa, 0x71, 0xd8, 0xc7, 0x3d, 0x37, 0xd2, 0xb5, 0x05, 0x95, 0x62, 0xb9, 0xa6, 0xde, 0x89, 0x3d, 0x36, 0x7b, 0x38, 0x77, 0x48, 0x97, 0xac, 0xa6, 0x20, 0x8f, 0x2e, 0xa6, 0xc9, 0x0c, 0xc2, 0xb2, 0x99, 0x45, 0x00, 0xc7, 0xce, 0x11, 0x51, 0x22, 0x22, 0xe0, 0xa5, 0xea, 0xb6, 0x15, 0x48, 0x09, 0x64, 0xea, 0x5e, 0x4f, 0x74, 0xf7, 0x05, 0x3e, 0xc7, 0x8a, 0x52, 0x0c, 0xdb, 0x15, 0xb4, 0xbd, 0x6d, 0x9b, 0xe5, 0xc6, 0xb1, 0x54, 0x68, 0xa9, 0xe3, 0x69, 0x90, 0xb6, 0x9a, 0xa5, 0x0f, 0xb8, 0xb9, 0x3f, 0x20, 0x7d, 0xae, 0x4a, 0xb5, 0xb8, 0x9c, 0xe4, 0x1d, 0xb6, 0xab, 0xe6, 0x94, 0xa5, 0xc1, 0xc7, 0x83, 0xad, 0xdb, 0xf5, 0x27, 0x87, 0x0e, 0x04, 0x6c, 0xd5, 0xff, 0xdd, 0xa0, 0x5d, 0xed, 0x87, 0x52, 0xb7, 0x2b, 0x15, 0x02, 0xae, 0x39, 0xa6, 0x6a, 0x74, 0xe9, 0xda, 0xc4, 0xe7, 0xbc, 0x4d, 0x34, 0x1e, 0xa9, 0x5c, 0x4d, 0x33, 0x5f, 0x92, 0x09, 0x2f, 0x88, 0x66, 0x5d, 0x77, 0x97, 0xc7, 0x1d, 0x76, 0x13, 0xa9, 0xd5, 0xe5, 0xf1, 0x16, 0x09, 0x11, 0x35, 0xd5, 0xac, 0xdb, 0x24, 0x71, 0x70, 0x2c, 0x98, 0x56, 0x0b, 0xd9, 0x17, 0xb4, 0xd1, 0xe3, 0x51, 0x2b, 0x5e, 0x75, 0xe8, 0xd5, 0xd0, 0xdc, 0x4f, 0x34, 0xed, 0xc2, 0x05, 0x66, 0x80, 0xa1, 0xcb, 0xe6, 0x33};

void setup(void) {
  Scout.setup();

  addBitlashFunction("power.ischarging", (bitlash_function) isBatteryCharging);
  addBitlashFunction("power.percent", (bitlash_function) getBatteryPercentage);
  addBitlashFunction("power.voltage", (bitlash_function) getBatteryVoltage);
  addBitlashFunction("power.enablevcc", (bitlash_function) enableBackpackVcc);
  addBitlashFunction("power.disablevcc", (bitlash_function) disableBackpackVcc);
  addBitlashFunction("power.sleep", (bitlash_function) goToSleep);
  addBitlashFunction("power.report", (bitlash_function) powerReport);

  addBitlashFunction("mesh.config", (bitlash_function) meshConfig);
  addBitlashFunction("mesh.setpower", (bitlash_function) meshSetPower);
  addBitlashFunction("mesh.key", (bitlash_function) meshSetKey);
  addBitlashFunction("mesh.joingroup", (bitlash_function) meshJoinGroup);
  addBitlashFunction("mesh.leavegroup", (bitlash_function) meshLeaveGroup);
  addBitlashFunction("mesh.ingroup", (bitlash_function) meshIsInGroup);
  addBitlashFunction("mesh.ping", (bitlash_function) meshPing);
  addBitlashFunction("mesh.pinggroup", (bitlash_function) meshPingGroup);
  addBitlashFunction("mesh.remoterun", (bitlash_function) meshRemoteRun);
  addBitlashFunction("mesh.broadcastrun", (bitlash_function) meshBroadcastRun);
  addBitlashFunction("mesh.publish", (bitlash_function) meshPublish);
  addBitlashFunction("mesh.subscribe", (bitlash_function) meshSubscribe);
  addBitlashFunction("mesh.report", (bitlash_function) meshReport);

  addBitlashFunction("temperature", (bitlash_function) getTemperature);
  addBitlashFunction("randomnumber", (bitlash_function) getRandomNumber);

  addBitlashFunction("led.off", (bitlash_function) ledOff);
  addBitlashFunction("led.red", (bitlash_function) ledRed);
  addBitlashFunction("led.green", (bitlash_function) ledGreen);
  addBitlashFunction("led.blue", (bitlash_function) ledBlue);
  addBitlashFunction("led.cyan", (bitlash_function) ledCyan);
  addBitlashFunction("led.purple", (bitlash_function) ledPurple);
  addBitlashFunction("led.magenta", (bitlash_function) ledMagenta);
  addBitlashFunction("led.yellow", (bitlash_function) ledYellow);
  addBitlashFunction("led.orange", (bitlash_function) ledOrange);
  addBitlashFunction("led.white", (bitlash_function) ledWhite);
  addBitlashFunction("led.redvalue", (bitlash_function) ledSetRedValue);
  addBitlashFunction("led.greenvalue", (bitlash_function) ledSetGreenValue);
  addBitlashFunction("led.bluevalue", (bitlash_function) ledSetBlueValue);
  addBitlashFunction("led.hexvalue", (bitlash_function) ledSetHexValue);
  addBitlashFunction("led.setrgb", (bitlash_function) ledSetRgb);
  addBitlashFunction("led.report", (bitlash_function) ledReport);

  addBitlashFunction("pin.on", (bitlash_function) pinOn);
  addBitlashFunction("pin.off", (bitlash_function) pinOff);
  addBitlashFunction("pin.makeinput", (bitlash_function) pinMakeInput);
  addBitlashFunction("pin.makeoutput", (bitlash_function) pinMakeOutput);
  addBitlashFunction("pin.read", (bitlash_function) pinRead);
  addBitlashFunction("pin.write", (bitlash_function) pinWrite);
  addBitlashFunction("pin.report", (bitlash_function) pinReport);

  addBitlashFunction("backpack.report", (bitlash_function) backpackReport);

  addBitlashFunction("scout.report", (bitlash_function) getScoutVersion);
  addBitlashFunction("scout.isleadscout", (bitlash_function) isLeadScout);
  addBitlashFunction("scout.sethqtoken", (bitlash_function) setHQToken);
  addBitlashFunction("scout.gethqtoken", (bitlash_function) getHQToken);

  leadScout = Scout.isLeadScout();
  if(forceLeadScout) leadScout = true;

//  meshJoinGroup();
  Scout.meshListen(1, receiveMessage);

  //dump_backpacks();

  whoami = Scout.getAddress();
  if(leadScout)
  {
    Serial.println("Lead Scout, starting WiFi");
    Wifi.begin(&wifis);
    Scout.meshListen(3, leadAnswers);
    Scout.meshListen(4, leadAnnouncements);
    // join all the "channels" to listen for announcements
    for(int i = 1; i < 10; i++) Scout.meshJoinGroup(i);
    Scout.meshJoinGroup(0xbeef); // our internal reporting channel
  }else{
    Serial.println("Field Scout, waiting");
    Scout.meshListen(2, fieldCommands);
  }
  setOutputHandler(&bitlashFilter);
  Serial.println("setup done");
}

void loop(void) {
#if 0
  // SSL example code
  if(leadScout)
  {
    IPAddress google(74,125,136,138);
    int c;
    Gainspan.addCert(String("geotrust"), /* to_flash */ false, cert, sizeof(cert));
    // Timesync with one of 0.pool.ntp.org
    Gainspan.timeSync("87.106.176.225" , 10, 0);
    if (leadClient.connect(google, 443) &&
        leadClient.enableTls("geotrust") &&
	leadClient.print("GET / HTTP/1.0\r\n\r\n")) {
	  Serial.println("TLS connected");
	  while (leadClient.connected())
	   if ((c = leadClient.read()) != -1)
	     Serial.write(c);
    } else {
      Serial.println("TLS connection failed");
    }
    // Don't repeat this. We would put this in setup(), but that breaks
    // in mysterious (think gcc bugs) ways.
    while(true);
  }
#endif
  Scout.loop();
  if(leadScout) leadHQ();
}

////////////////////
// lead scout stuff

// this is called on the main loop to try to (re)connect to the HQ
void leadHQ(void)
{
  char auth[256], token[33];
  static char *buffer = NULL;
  uint8_t block[128];
  char *nl;
  int rsize, len, i;
  unsigned short index[32]; // <10 keypairs in the incoming json

  if (!leadClient.connected()) {
    Serial.println("HQ connecting");
    if (leadClient.connect(server, HQport)) {
      Serial.println("HQ contacted");
      Pinoccio.getHQToken(token);
      token[32] = 0;
      sprintf(auth,"{\"type\":\"token\",\"token\":\"%s\"}\n", token);
      leadSignal(auth);
    } else {
      Serial.println("HQ offline");
      leadClient.stop();
      delay(1000); // TODO how to handle this better?
      return;
    }
  }
  // only continue if new data to read
  if(!leadClient.available()) return;

  // check to initialize our read buffer
  if(!buffer)
  {
    buffer = (char*)malloc(1);
    *buffer = 0;
  }

  // get all waiting data and look for packets
  while(rsize = leadClient.read(block, 256)){
    len = strlen(buffer);

    // process chunk of incoming data
    buffer = (char*)realloc(buffer, len+rsize+1);
    if(!buffer) return; // TODO, realloc error, need to restart?
    memcpy(buffer+len, block, rsize);
    buffer[len+rsize] = 0; // null terminate

    // look for a packet
    Serial.print("looking for packet in: ");
    Serial.println(buffer);
    nl = strchr(buffer, '\n');
    if(!nl) continue;

    // null terminate just the packet and process it
    *nl = 0;
    j0g(buffer, index, 32);
    if(*index) leadIncoming(buffer, index);
    else Serial.println("JSON parse failed");

    // advance buffer and resize, minimum is just the buffer end null
    nl++;
    len = strlen(nl);
    memmove(buffer, nl, len+1);
    buffer = (char*)realloc(buffer, len+1); // shrink
  }
}

// when we can't process a command for some internal reason
void leadCommandError(int from, int id, char *reason)
{
  char *err;
  err = (char*)malloc(strlen(reason)+128);
  sprintf(err,"{\"type\":\"reply\",\"from\":%d,\"id\":%d,\"err\":true,\"reply\":\"%s\"}\n",from,id,reason);
  leadSignal(err);
  free(err);
}

// necessities for tracking state when chunking up a large command into mesh requests
int leadCommandTo = 0;
char *leadCommandChunks;
int leadCommandChunksAt;
int leadCommandRetries;
NWK_DataReq_t leadCommandReq;
void leadCommandChunk(void);

// process a packet from HQ
void leadIncoming(char *packet, unsigned short *index)
{
  char *type, *command;
  int to, ret, len;
  unsigned long id;

  type = j0g_str("type", packet, index);
  Serial.println(type);

  if(strcmp(type, "online") == 0)
  {
    // TODO anything hard-coded to do once confirmed online?
  }

  if(strcmp(type, "command") == 0)
  {
    to = atoi(j0g_str("to", packet, index));
    id = strtoul(j0g_str("id", packet, index), NULL, 10);
    command = j0g_str("command", packet, index);
    if(!to || !id || !command)
    {
      Serial.println("invalid command, requires to, id, command");
      return;
    }

    // handle internal ones first
    if(to == whoami)
    {
      // reusing same buffer for bitlash response
      bitlashOutput = (char*)malloc(100);
      sprintf(bitlashOutput,"{\"type\":\"reply\",\"from\":%d,\"id\":%lu,\"end\":true,\"reply\":\"",to,id);
      setOutputHandler(&bitlashBuffer);
      ret = (int)doCommand(command);
      setOutputHandler(&bitlashFilter);
      len = strlen(bitlashOutput);
      strcpy(bitlashOutput+len, "\"}\n");
      leadSignal(bitlashOutput);
      free(bitlashOutput);
      return;
    }

    // we can only send one command at a time
    if(leadCommandTo)
    {
      // TODO we could stop reading the HQ socket in this mode and then never get a busy?
      return leadCommandError(to,id,"busy");
    }

    // send over mesh to recipient and cache id for any replies
    leadAnswerID = id;
    leadCommandTo = to;
    leadCommandChunks = (char*)malloc(strlen(command)+1);
  	strcpy(leadCommandChunks,command);
    leadCommandChunksAt = 0;
    leadCommandRetries = 0;
    leadCommandChunk();
  }
}

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
    if(strlen(leadCommandChunks+leadCommandChunksAt) > 100)
      {
        leadCommandChunksAt += 100;
        leadCommandChunk();
        return; // don't free yet
      }
  } else {
		leadCommandRetries++;
		if(leadCommandRetries > 3)
		{
	    Serial.print("error: ");
	    Serial.println(req->status, HEX);
      leadCommandError(leadCommandTo, leadAnswerID, "no response");
		}else{
      Serial.println("RETRY");
			NWK_DataReq(req);
			return; // don't free yet
		}
  }
  leadCommandTo = 0;
	free(leadCommandChunks);
}

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk()
{
  int len = strlen(leadCommandChunks+leadCommandChunksAt);
  if(len > 100) len = 100;
  else len++; // null terminator at end
  leadCommandReq.dstAddr = leadCommandTo;
  leadCommandReq.dstEndpoint = 2;
  leadCommandReq.srcEndpoint = 3;
  leadCommandReq.options = NWK_OPT_ENABLE_SECURITY;
  leadCommandReq.data = (uint8_t*)(leadCommandChunks+leadCommandChunksAt);
  leadCommandReq.size = len;
  leadCommandReq.confirm = leadCommandChunkConfirm;
  NWK_DataReq(&leadCommandReq);
  //RgbLed.blinkCyan(200);
  Serial.print(leadCommandTo, DEC);
  Serial.print(" len ");
  Serial.print(len, DEC);
  Serial.println("->chunk");
}

// wrapper to send a chunk of JSON to the HQ
void leadSignal(char *json)
{
  if(!leadClient.connected())
  {
    Serial.println("HQ offline, can't signal");
    Serial.println(json);
    return;
  }
  Serial.println("HQ signalling");
  Serial.println(json);
  leadClient.write(json);
  leadClient.flush();
}

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind) {
	bool end = false;
  int at;
  char sig[256];
  //RgbLed.blinkGreen(200);

  if(ind->options&NWK_IND_OPT_MULTICAST)
  {
    Serial.println("MULTICAST on wrong endpoint");
    return true;
  }

  Serial.println("Received answer");
  if(ind->data[ind->size-1] == 0)
  {
    end = true;
    ind->size--;
  }
  sprintf(sig,"{\"type\":\"reply\",\"id\":%d,\"from\":%d,\"data\":\"", leadAnswerID, ind->srcAddr);
  at = strlen(sig);
  memcpy(sig+at, ind->data, ind->size);
  sprintf(sig+at+ind->size, "\",\"end\":%s}\n",end?"true":"false");
  leadSignal(sig);

	return true;
}

// simple wrapper for the incoming channel announcements up to HQ
void leadAnnouncementSend(int chan, int from, char *message)
{
  char sig[256];
  sprintf(sig,"{\"type\":\"channel\",\"id\":%d,\"from\":%d,\"data\":\"%s\"}\n", chan, from, message);
  leadSignal(sig);
}

// mesh callback whenever another scout announces something on a channel
static bool leadAnnouncements(NWK_DataInd_t *ind) {
  //RgbLed.blinkBlue(200);

  // be safe
  if(!ind->options&NWK_IND_OPT_MULTICAST) return true;

  Serial.print("MULTICAST");
  leadAnnouncementSend(ind->dstAddr, ind->srcAddr, (char*)ind->data);
  return true;
}

/////////////////////
// field scout stuff

// necessities to track state whenever we have to chunk up large bitlash answers to a command
int fieldAnswerTo = 0;
char *fieldAnswerChunks;
int fieldAnswerChunksAt;
int fieldAnswerRetries;
NWK_DataReq_t fieldAnswerReq;
void fieldAnswerChunk(void);

// chunk packet confirmation callback by mesh
static void fieldAnswerChunkConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
    if(strlen(fieldAnswerChunks+fieldAnswerChunksAt) > 100)
      {
        fieldAnswerChunksAt += 100;
        fieldAnswerChunk();
        return; // don't free yet
      }
  } else {
		fieldAnswerRetries++;
		if(fieldAnswerRetries > 3)
		{
	    Serial.print("error: ");
	    Serial.println(req->status, HEX);
		}else{
      Serial.println("RETRY");
			NWK_DataReq(req);
			return; // don't free yet
		}
  }
  fieldAnswerTo = 0;
	free(fieldAnswerChunks);
}

// send the first/next chunk of the answer back
static void fieldAnswerChunk()
{
  int len = strlen(fieldAnswerChunks+fieldAnswerChunksAt);
  if(len > 100) len = 100;
  else len++; // null terminator at end
  fieldAnswerReq.dstAddr = fieldAnswerTo;
  fieldAnswerReq.dstEndpoint = 3;
  fieldAnswerReq.srcEndpoint = 2;
  fieldAnswerReq.options = NWK_OPT_ENABLE_SECURITY;
  fieldAnswerReq.data = (uint8_t*)(fieldAnswerChunks+fieldAnswerChunksAt);
  fieldAnswerReq.size = len;
  fieldAnswerReq.confirm = fieldAnswerChunkConfirm;
  NWK_DataReq(&fieldAnswerReq);
  //RgbLed.blinkCyan(200);
  Serial.print(fieldAnswerTo, DEC);
  Serial.print(" len ");
  Serial.print(len, DEC);
  Serial.println("->chunk");
}

// necessities to buffer up large incoming commands
char *fieldCommand = NULL;
int fieldCommandLen = 0;

// mesh callback for handling incoming commands
static bool fieldCommands(NWK_DataInd_t *ind) {
	int total, ret;
  //RgbLed.blinkGreen(200);

  Serial.print("Received command");
  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);
  Serial.print("  ");
  Serial.print("rssi: ");
  Serial.print(ind->rssi, DEC);
  Serial.println();

  if(fieldAnswerTo)
  {
    Serial.println("can't receive command while sending answer");
    return false;
  }

  // commands may be larger than one packet, copy and buffer up
  total = fieldCommandLen + ind->size;
  fieldCommand = (char*)realloc(fieldCommand, total);
  if(!fieldCommand) return false; // TODO we need to restart or something, no memory
  memcpy(fieldCommand+fieldCommandLen,ind->data,ind->size);
  fieldCommandLen = total;
  // when null terminated, do the message
  if(fieldCommand[fieldCommandLen-1] != 0){
    Serial.println("waiting for more");
    return true;
  }

  // run the command and chunk back the results
  setOutputHandler(&bitlashBuffer);
  bitlashOutput = (char*)malloc(1);
  bitlashOutput[0] = 0;
  Serial.print("running command ");
  Serial.println(fieldCommand);
  ret = (int)doCommand(fieldCommand);
  Serial.print("got result ");
  Serial.println(ret);
  setOutputHandler(&bitlashFilter);
  fieldCommandLen = 0;

  // send data back in chunks
  fieldAnswerTo = ind->srcAddr;
  fieldAnswerChunks = bitlashOutput;
  fieldAnswerChunksAt = 0;
  fieldAnswerRetries = 0;
  fieldAnswerChunk();

	return true;
}

// can only send one at a time, locks up!
bool announcing = false;
static void fieldAnnounceConfirm(NWK_DataReq_t *req) {
  announcing = false;
}

// check a line of bitlash output for any announcements and send them
void fieldAnnounce(char *line)
{
  char *message;
  int chan, len;

  // any lines looking like "CHAN:4 message" will send "message" to channel 4
  if(strncmp("CH4N:",line,5) != 0) return;
  message = strchr(line,' ');
  if(!message) return;
  *message = 0;
  message++;
  chan = atoi(line+5);
  if(!chan || announcing) return;

  len = strlen(message);
  Serial.print("announcing to ");
  Serial.print(chan, DEC);
  Serial.print(" ");
  Serial.println(message);

  // when lead scout, shortcut
  if(leadScout) return leadAnnouncementSend(chan, whoami, message);

  Scout.meshJoinGroup(chan); // must be joined to send
  fieldAnnounceReq.dstAddr = chan;
  fieldAnnounceReq.dstEndpoint = 4;
  fieldAnnounceReq.srcEndpoint = whoami;
  fieldAnnounceReq.options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  fieldAnnounceReq.data = (uint8_t*)message;
  fieldAnnounceReq.size = len+1;
  fieldAnnounceReq.confirm = fieldAnnounceConfirm;
  announcing = true;
  NWK_DataReq(&fieldAnnounceReq);
  //RgbLed.blinkGreen(200);
}


/****************************\
*      BUILT-IN HANDLERS    *
\****************************/
numvar getTemperature(void) {
  return Scout.getTemperature();
}

numvar getRandomNumber(void) {
  return random();
}

/****************************\
*      POWER HANDLERS       *
\****************************/
numvar isBatteryCharging(void) {
  return Scout.isBatteryCharging();
}

numvar getBatteryPercentage(void) {
  return Scout.getBatteryPercentage();
}

numvar getBatteryVoltage(void) {
  return Scout.getBatteryVoltage();
}

numvar enableBackpackVcc(void) {
  Scout.enableBackpackVcc();
  return true;
}

numvar disableBackpackVcc(void) {
  Scout.disableBackpackVcc();
  return true;
}

numvar goToSleep(void) {
  // TODO: not implemented yet
  //Pinoccio.goToSleep(getarg(1));
}

numvar powerReport(void) {
  // TODO: return JSON formmated report of power details
  // ie: {pct:85,vlt:4.1,chg:false,vcc:true}
  return true;
}

/****************************\
*      RGB LED HANDLERS     *
\****************************/
numvar ledOff(void) {
  RgbLed.turnOff();
}

numvar ledRed(void) {
  RgbLed.red();
}

numvar ledGreen(void) {
  RgbLed.green();
}

numvar ledBlue(void) {
  RgbLed.blue();
}

numvar ledCyan(void) {
  RgbLed.cyan();
}

numvar ledPurple(void) {
  RgbLed.purple();
}

numvar ledMagenta(void) {
  RgbLed.magenta();
}

numvar ledYellow(void) {
  RgbLed.yellow();
}

numvar ledOrange(void) {
  RgbLed.orange();
}

numvar ledWhite(void) {
  RgbLed.white();
}

numvar ledSetRedValue(void) {
  RgbLed.setRedValue(getarg(1));
}

numvar ledSetGreenValue(void) {
  RgbLed.setGreenValue(getarg(1));
}

numvar ledSetBlueValue(void) {
  RgbLed.setBlueValue(getarg(1));
}

numvar ledSetHexValue(void) {
  Serial.println((char *)getstringarg(1));
  if (isstringarg(1)) {
    RgbLed.setHex((char *)getstringarg(1));
    return true;
  } else {
    return false;
  }
}

numvar ledSetRgb(void) {
  RgbLed.setRedValue(getarg(1));
  RgbLed.setGreenValue(getarg(2));
  RgbLed.setBlueValue(getarg(3));
}

numvar ledReport(void) {
  Serial.print("{\"r\":");
  Serial.print(RgbLed.getRedValue());
  Serial.print(",\"g\":");
  Serial.print(RgbLed.getGreenValue());
  Serial.print(",\"b\":");
  Serial.print(RgbLed.getBlueValue());
  Serial.println("}");
}

/****************************\
*    MESH RADIO HANDLERS    *
\****************************/
numvar meshConfig(void) {
  uint16_t panId = 0x4567;
  uint8_t channel = 0x1a;
  if (getarg(0) == 2) {
    panId = getarg(2);
    channel = getarg(3);
  }
  Scout.meshSetRadio(getarg(1), panId, channel);
}

numvar meshSetPower(void) {
  Scout.meshSetPower(getarg(1));
}

numvar meshSetKey(void) {
  Pinoccio.meshSetSecurityKey((const char *)getstringarg(1));
}

numvar meshJoinGroup(void) {
  Scout.meshJoinGroup(groupId);
}

numvar meshLeaveGroup(void) {
  Scout.meshLeaveGroup(groupId);
}

numvar meshIsInGroup(void) {
  return Scout.meshIsInGroup(groupId);
}

numvar meshPing(void) {
  pingScout(getarg(1));
}

numvar meshPingGroup(void) {
  pingGroup(groupId);
}

// Run a command that's defined on another scout.  ie: meshRemoteRun(scoutId, "remote command string");
numvar meshRemoteRun(void) {
  MeshRequest request;
  request.setDstAddress(getarg(1));
  request.setDstEndpoint(1);
  request.setSrcEndpoint(1);

  if (!isstringarg(2)) {
    Serial.println("Second argument must be a valid Bitlash command (and a string)");
    return false;
  }

  if (sizeof(getarg(2)) > 100) {
    Serial.println("Size of payload cannot exceed 100 bytes");
    return false;
  }

  uint8_t size = strlen((const char *)getstringarg(2));

  Serial.println(getarg(0));
  Serial.println(getarg(1));
  Serial.println((const char *) getstringarg(2));
  Serial.println(size);

  request.setHeader(NWK_PAYLOAD_HEADER_FINAL);
  request.setPayload((byte *)getstringarg(2), size+1);

  NWK_DataReq_t* dataReq = request.getRequest();
  Serial.println("Data request");
  Serial.println(dataReq->dstAddr);
  Serial.println(dataReq->dstEndpoint);
  Serial.println(dataReq->srcEndpoint);
  Serial.println(dataReq->size);
  Serial.println(dataReq->data[0], HEX);
  for (int i=1; i<dataReq->size; i++) {
    Serial.write((const char*)dataReq->data[i]);
  }
  Pinoccio.meshSendMessage(request);
  return true;
}

numvar meshBroadcastRun(void) {
  // TODO: run a function that's defined on on all boards in a troop
  // ie: meshRemoteRun("remoteFunctionName");
  return true;
}

numvar meshPublish(void) {
  // TODO: send a payload to a multicast address on the mesh
  // ie: meshPublish(multicastId, callbackFunctionThatReturnsPayloadToPublish, frequencyToPublish);
  return true;
}

numvar meshSubscribe(void) {
  // TODO: subscribe to/join a multicast address on the mesh
  // ie: meshSubscribe(multicastId, callbackFunctionThatIsCalledWhenMessageArrives);
  NWK_GroupAdd(getarg(1));
  if (!NWK_GroupIsMember(getarg(1))) {
    Serial.println("Error attempting to subscribe to topic");
    return false;
  }
  return true;
}

numvar meshReport(void) {
  // TODO: return JSON formatted report of radio details
  // ie: {"id":34,"pid":1,"ch":26,"sec":true}
  Serial.println("-- Mesh Radio Settings --");
  Serial.print(" - Address: ");
  Serial.println(Scout.getAddress());
  Serial.print(" - Pan ID: 0x");
  Serial.println(Scout.getPanId(), HEX);
  Serial.print(" - Channel: ");
  Serial.println(Scout.getChannel());
  Serial.print(" - Tx Power: ");
  // gotta read these from program memory (for SRAM savings)
  char c;
  const char *dbString = Scout.getTxPowerDb();
  while((c = pgm_read_byte(dbString++))) {
     Serial.write(c);
  }
  Serial.println();
  Serial.print(" - In group: ");
  if (Scout.meshIsInGroup(groupId)) {
    Serial.println("Yes");
  } else {
    Serial.println("No");
  }
  Serial.println(" - Routing: ");
  Serial.println("|    Fixed    |  Multicast  |    Score    |    DstAdd   | NextHopAddr |    Rank     |     LQI    |");
  NWK_RouteTableEntry_t *table = NWK_RouteTable();
  for (int i=0; i < NWK_ROUTE_TABLE_SIZE; i++) {
    if (table[i].dstAddr == NWK_ROUTE_UNKNOWN) {
      continue;
    }
    Serial.print("|      ");
    Serial.print(table[i].fixed);
    Serial.print("      |      ");
    Serial.print(table[i].multicast);
    Serial.print("      |      ");
    Serial.print(table[i].score);
    Serial.print("      |     0x");
    Serial.print(table[i].dstAddr, HEX);
    Serial.print("     |     0x");
    Serial.print(table[i].nextHopAddr, HEX);
    Serial.print("     |     ");
    Serial.print(table[i].rank);
    Serial.print("     |     ");
    Serial.print(table[i].lqi);
    Serial.println("    |");
  }
}

/****************************\
*        I/O HANDLERS       *
\****************************/
numvar pinOn(void) {
  pinMode(getarg(1), OUTPUT);
  digitalWrite(getarg(1), HIGH);
  return true;
}

numvar pinOff(void) {
  pinMode(getarg(1), OUTPUT);
  digitalWrite(getarg(1), LOW);
  return true;
}

numvar pinMakeInput(void) {
  pinMode(getarg(1), INPUT);
  return true;
}

numvar pinMakeOutput(void) {
  pinMode(getarg(1), OUTPUT);
  return true;
}

numvar pinRead(void) {
  return digitalRead(getarg(1));
}

numvar pinWrite(void) {
  // TODO: set a PWM pin's value from 0 - 255
  return true;
}

numvar pinThreshold(void) {
  // TODO: create a threshold function with the following format:
  // threshold(pin, value, fnToCallIfValueLessThan, fnToCallIfValueEqual, fnToCallIfValueGreaterThan)
  return true;
}

numvar pinReport(void) {
  // TODO: return JSON formmated report of all IO pins and their values
  return true;
}

/****************************\
*     BACKPACK HANDLERS     *
\****************************/
numvar backpackReport(void) {
  Serial.println("[{\"name\":\"wifi\",\"version\":\"1.0\"},{\"name\":\"environment\",\"version\":\"2.0\"}]");
}

/****************************\
 *   SCOUT REPORT HANDLERS  *
\****************************/
numvar getScoutVersion(void) {
  Serial.println("1.0");
}

numvar isLeadScout(void) {
  Serial.println("true");
  //return leadScout;
}

numvar setHQToken(void) {
  Pinoccio.setHQToken((const char *)getstringarg(1));
}

numvar getHQToken(void) {
  char token[33];
  Pinoccio.getHQToken((char *)token);
  token[32] = 0;
  Serial.println(token);
}


/****************************\
 *     HELPER FUNCTIONS     *
\****************************/
static void pingScout(int address) {
  appDataReq.dstAddr = address;

  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_ACK_REQUEST|NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = pingConfirm;
  NWK_DataReq(&appDataReq);
  //RgbLed.blinkCyan(200);

  Serial.print("PING ");
  Serial.print(address);
  Serial.print(": ");

  pingCounter++;
}

static void pingGroup(int address) {
  appDataReq.dstAddr = address;

  appDataReq.dstEndpoint = 1;
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_MULTICAST|NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = &pingCounter;
  appDataReq.size = sizeof(pingCounter);
  appDataReq.confirm = pingConfirm;
  NWK_DataReq(&appDataReq);

  Serial.print("PING ");
  Serial.print(address, HEX);
  Serial.print(": ");

  pingCounter++;
}

static void pingConfirm(NWK_DataReq_t *req) {
  Serial.print("dstAddr: ");
  Serial.println(req->dstAddr, HEX);
  Serial.print("dstEndpoint: ");
  Serial.println(req->dstEndpoint);
  Serial.print("srcEndpoint: ");
  Serial.println(req->srcEndpoint);
  Serial.print("options: ");
  Serial.println(req->options, BIN);
  Serial.print("size: ");
  Serial.println(req->size);
  Serial.print("status: ");
  Serial.println(req->status, HEX);

  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.print("1 byte from ");
    Serial.print(req->dstAddr);
    Serial.print(" RSSI=-");
    Serial.println(req->control);
  } else {
    Serial.print("Error: ");
    switch (req->status) {
      case NWK_OUT_OF_MEMORY_STATUS:
        Serial.print("Out of memory: ");
        break;
      case NWK_NO_ACK_STATUS:
      case NWK_PHY_NO_ACK_STATUS:
        Serial.print("No acknowledgement received: ");
        break;
      case NWK_NO_ROUTE_STATUS:
        Serial.print("No route to destination: ");
        break;
      case NWK_PHY_CHANNEL_ACCESS_FAILURE_STATUS:
        Serial.print("Physical channel access failure: ");
        break;
      default:
        Serial.print("unknown failure: ");
    }
    Serial.print("(");
    Serial.print(req->status, HEX);
    Serial.println(")");
  }
}

static bool receiveMessage(NWK_DataInd_t *ind) {
  Serial.print("Received message - ");
  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);

  Serial.print("  ");

  Serial.print("rssi: ");
  Serial.print(abs(ind->rssi), DEC);
  Serial.print("  ");

  Serial.print("data: ");
  for (int i=0; i<ind->size; i++) {
    Serial.print(ind->data[i], DEC);
  }
  Serial.println("");
  NWK_SetAckControl(abs(ind->rssi));

  // run the Bitlash callback function, if defined
  char *callback = "mesh.receive";
  if (findscript(callback)) {
    doCommand(callback);
  }
  return true;
}

// watches bitlash output for channel announcements
void bitlashFilter(byte b) {
  static char buf[101];
  static int offset = 0;

  Serial.write(b); // cc to serial
  if(b == '\r') return; // skip CR

  // newline or max len announces and resets
  if(b == '\n' || offset == 100)
  {
    buf[offset] = 0;
    fieldAnnounce(buf);
    offset=0;
    return;
  }
  if(b == '"')
  {
    buf[offset++] = '\\';
    b = '"';
  }
  buf[offset] = b;
  offset++;
}

// buffers bitlash output from a command
void bitlashBuffer(byte b) {
  int len;

  Serial.write(b); // cc to serial
  if(b == '\r') return; // skip CR

  len = strlen(bitlashOutput);
  bitlashOutput = (char*)realloc(bitlashOutput, len+3); // up to 2 bytes w/ escaping, and the null term

  // escape newlines and quotes
  if(b == '\n')
  {
    bitlashOutput[len++] = '\\';
    b = 'n';
  }
  if(b == '"')
  {
    bitlashOutput[len++] = '\\';
    b = '"';
  }
  bitlashOutput[len] = b;
  bitlashOutput[len+1] = 0;
}
