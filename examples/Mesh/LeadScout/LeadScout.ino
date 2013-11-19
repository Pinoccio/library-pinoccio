#include <SPI.h>
#include <Wire.h>
#include <LeadScout.h>
extern "C" {
#include <j0g.h>
}

WIFI_PROFILE wifiprofile = {
                        /* SSID */ "Air Patrol",
         /* WPA/WPA2 passphrase */ "UberJabber",
                  /* IP address */ "",
                 /* subnet mask */ "",
                  /* Gateway IP */ "", };


//IPAddress server(192,168,0,2); // peer device IP address
IPAddress server(173,255,220,185); // peer device IP address
int port = 22756;
char token[] = "perin";
static NWK_DataReq_t appDataReq, pingReq;
int retries;
int meshAddress = 1;


// globals
#define RFCHUNK 100
PinoccioWifiClient client;
#define BUFSIZE 1024
char bufout[BUFSIZE];
char *bufin;
int offset = 0;
// <8 keypairs in the json
#define INDEXSIZE 16
unsigned short index[INDEXSIZE];

void HQ();
void signal(char *json);
void ten4();
void serialHandler(byte b);
static bool receiveMessage(NWK_DataInd_t *ind);


void setup() {
  Scout.setup();
  for(int i = 1; i < 10; i++) Scout.meshJoinGroup(i);
  Scout.meshSetRadio(meshAddress);
  Scout.meshSetSecurityKey("TestSecurityKey1");
  Wifi.begin(&wifiprofile);
  HQ();
  setOutputHandler(&serialHandler);
  Scout.meshListen(1, receiveMessage);
  Scout.meshListen(2, receiveMulticast);
}

// buffer must be a null terminated string, returns updated buffer pointer for next call
// uses realloc to change size as needed between calls
char *sockread(PinoccioWifiClient socket, char *buffer)
{
  uint8_t block[128];
  char *nl;
  int rsize, len, i;
  
  if(!buffer)
  {
    buffer = (char*)malloc(1);
    *buffer = 0;
  }
  len = strlen(buffer);
  
  // read another block
  rsize = socket.read(block, 127);
  if(!rsize) return buffer;
  Serial.print("incoming block:");
  for(i=0;i<rsize;i++)
  {
    Serial.print(i,DEC);
    Serial.print(":");
    Serial.print(block[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  buffer = (char*)realloc(buffer, len+rsize+1);
  if(!buffer) return NULL; // TODO, realloc error, need to restart?
  memcpy(buffer+len, block, rsize);
  buffer[len+rsize] = 0; // null terminate
  
  // look for a packet
  Serial.print("looking for packet in: ");
  Serial.println(buffer);
  nl = strchr(buffer, '\n');
  if(!nl) return buffer;

  // null terminate just the packet and process it
  *nl = 0;
  j0g(buffer, index, INDEXSIZE);
  if(*index) ten4(buffer);

  // advance buffer and resize, minimum is just the buffer end null
  nl++;
  len = strlen(nl);
  memmove(buffer, nl, len+1);
  buffer = (char*)realloc(buffer, len+1); // shrink
  return buffer;
}

void loop()
{
  Scout.loop();
  if (client.available()) bufin = sockread(client, bufin);
  if (!client.connected()) {
    Serial.println("HQ failed");
    client.stop();
    delay(1000);
    HQ();
  }  

}

char *chunks;
int chunkat;
int chunkto;
static void sendMessageConfirm(NWK_DataReq_t *req) {
  Serial.print("  Message confirmation - ");
  if (req->status == NWK_SUCCESS_STATUS) {
    Serial.println("success");
    if(strlen(chunks+chunkat) > RFCHUNK)
      {
        chunkat += RFCHUNK;
        sendMessageChunk();
        return; // don't free yet
      } 
  } else {
		retries++;
		if(retries > 3)
		{
	    Serial.print("error: ");
	    Serial.println(req->status, HEX);
		  sprintf(bufout,"{\"type\":\"reply\",\"reply\":\"%d: error\"}\n",req->dstAddr);
		  signal(bufout);
		}else{
      Serial.println("RETRY");
			NWK_DataReq(req);
			return; // don't free yet
		}		
  }
	free(chunks);
}

static void sendMessageChunk() {  
  int len = strlen(chunks+chunkat);
  if(len > RFCHUNK) len = RFCHUNK;
  else len++; // null terminator at end
  appDataReq.dstAddr = chunkto;
  appDataReq.dstEndpoint = meshAddress; // just a hard-mapping convention
  appDataReq.srcEndpoint = 1;
  appDataReq.options = NWK_OPT_ENABLE_SECURITY;
  appDataReq.data = (uint8_t*)(chunks+chunkat);
  appDataReq.size = len;
  appDataReq.confirm = sendMessageConfirm;
  NWK_DataReq(&appDataReq);
  RgbLed.blinkCyan(200);

  Serial.print(len, DEC);
  Serial.println("->chunk");
}

static void sendMessage(int to, char *message) {
  int len = strlen(message);
  chunkto = to;
	chunks = (char*)malloc(len+1);
	memcpy(chunks,message,len+1);
  chunkat = 0;
  sendMessageChunk();
}

static void pingConfirm(NWK_DataReq_t *req) {
  if (req->status == NWK_SUCCESS_STATUS) {
	  RgbLed.blinkGreen(100);
    Serial.println("ping success");
	  sprintf(bufout,"{\"type\":\"reply\",\"reply\":\"%d: online\"}\n", req->dstAddr);
		signal(bufout);
  } else {
	  RgbLed.blinkRed(100);
    Serial.print("ping error ");
    Serial.println(req->status, HEX);
	  NWK_DataReq(req);
  }
}

static void pingFind(int to) {  
  pingReq.dstAddr = to;
  pingReq.dstEndpoint = 1;
  pingReq.srcEndpoint = 1;
  pingReq.options = NWK_OPT_ENABLE_SECURITY;
  pingReq.data = 0;
  pingReq.size = 0;
  pingReq.confirm = pingConfirm;
  NWK_DataReq(&pingReq);
}

void HQ()
{
  char auth[64];
  if (client.connect(server, port)) {
    Serial.println("HQ contacted");
    sprintf(auth,"{\"type\":\"token\",\"token\":\"%s\"}\n", token);
    signal(auth);
  } else {
    Serial.println("HQ offline");
  }
		
}

void signal(char *json)
{
  Serial.println("outgoing");
  Serial.println(json);
  client.write(json);
  client.flush();
//  RgbLed.blinkGreen(10);
}

void ten4(char *packet)
{
  char *type, *command;
  int start, ret;
  type = j0g_str("type", packet, index);
  Serial.println(type);
  if(strcmp(type, "online") == 0)
  {
    sprintf(bufout,"{\"type\":\"report\",\"report\":\"temp %d\"}\n", Scout.getTemperature()*100);
    signal(bufout);
    setOutputHandler(&serialHandler);
  }
  if(strcmp(type, "command") == 0)
  {
    command = (char*)malloc(strlen(j0g_str("command", packet, index))+1);
    strcpy(command, j0g_str("command", packet, index));
    Serial.println(command);
		if(strncmp("add",command,3) == 0)
		{
			ret = atoi(command+4);
			Serial.print("adding ");
			Serial.print(ret,DEC);
			Serial.println("");
			if(ret <= 0)
			{
				Serial.println("invalid");
			  strcpy(bufout,"{\"type\":\"reply\",\"reply\":\"invalid id\"}\n");
				signal(bufout);
				free(command);
				return;
			}
			pingFind(ret);
		  Scout.meshListen(ret, receiveMessage);
			free(command);
			return;
		}
		if(strncmp("ping",command,4) == 0 && strlen(command) > 7)
		{
			ret = atoi(command+5);
			Serial.print("commanding ");
			Serial.print(ret,DEC);
			Serial.println("");
			sendMessage(ret, command+7);			
			free(command);
			return;
		}
    // reusing same buffer for bitlash response
    strcpy(bufout,"{\"type\":\"reply\",\"reply\":\"");
    start = offset = strlen(bufout);
    ret = (int)doCommand(command);
    free(command);
    Serial.println(ret);
    bufout[offset] = 0; // null terminate
    if(offset > start) offset -= 2; // remove trailing "\n"
    if(strlen(bufout) == start) strcpy(bufout+offset, "(empty)\"}\n");
    else strcpy(bufout+offset,"\"}\n");
    signal(bufout);
  }
}

void serialHandler(byte b) {
  char str[100];
  if(offset+1 > BUFSIZE-100)
  {
    Serial.println("bitlash buffer overrun");
    return;
  }
  sprintf(str,"bl: %d %d",offset, b);
  Serial.println(str);
  if(b == '\r') return; // skip CR
  // escape newlines and quotes
  if(b == '\n')
  {
    bufout[offset++] = '\\';
    b = 'n';
  }
  if(b == '"')
  {
    bufout[offset++] = '\\';
    b = '"';
  }
  bufout[offset] = b;
  offset++;
}

void doMessage(int from, char *message)
{
  Serial.print("doMessage:");
  Serial.println(message);
  sprintf(bufout,"{\"type\":\"reply\",\"reply\":\"%d: %s\"}\n", from, message);
  signal(bufout);
}

char *message = NULL;
int messagelen = 0;

static bool receiveMessage(NWK_DataInd_t *ind) {
	int total;
  RgbLed.blinkGreen(200);
  
  Serial.print("Received message - ");
  Serial.print("lqi: ");
  Serial.print(ind->lqi, DEC);

  Serial.print("  ");

  Serial.print("rssi: ");
  Serial.print(ind->rssi, DEC);
  Serial.print("  ");

  Serial.print("data: ");
  for (int i=0; i<ind->size; i++) {
	  Serial.print(" ");
    Serial.print(ind->data[i], HEX);
  }
  Serial.println("");

  if(ind->options&NWK_IND_OPT_MULTICAST)
  {
    Serial.println("MULTICAST on wrong endpoint");
    return true;
  }

  total = messagelen + ind->size;
  message = (char*)realloc(message, total);
  if(!message) return false; // TODO we need to restart, no memory
  memcpy(message+messagelen,ind->data,ind->size);
  messagelen = total;
  // when null terminated, do the message
  if(!message[messagelen-1]){
    doMessage(ind->srcAddr, message);
    messagelen = 0;
  }else{
    Serial.print(messagelen,DEC);
    Serial.print(message[messagelen],DEC);
    Serial.println("waiting for more");  
  }
	return true;
}

static bool receiveMulticast(NWK_DataInd_t *ind) {
  char sig[256];
  RgbLed.blinkBlue(200);
  
  // be safe
  if(!ind->options&NWK_IND_OPT_MULTICAST) return true;

  Serial.print("MULTICAST");
  sprintf(sig,"{\"type\":\"channel\",\"id\":%d,\"from\":%d,\"data\":\"%s\"}\n", ind->dstAddr, ind->srcAddr, (char*)ind->data);
  signal(sig);
  return true;
}
