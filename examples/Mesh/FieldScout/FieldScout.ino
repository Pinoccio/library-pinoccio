#include <SPI.h>
#include <Wire.h>
#include <Scout.h>

int meshAddress = 2; // unique id of the scout
int retries;

#define RFCHUNK 100
#define BUFSIZE 1024
char bufin[BUFSIZE], bufout[BUFSIZE];
int offset = 0;

static NWK_DataReq_t appDataReq;
void serialHandler(byte b);


void setup() {
  Scout.setup();
  Scout.meshSetRadio(meshAddress);
  Scout.meshSetSecurityKey("TestSecurityKey1");
  
  Serial.println("Waiting for ping packets:");
  Scout.meshListen(1, receiveMessage);
  setOutputHandler(&serialHandler);
}

void loop() {
  Scout.loop();
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

void doMessage(int from, char *message)
{
  int ret;
	Serial.println(message);
	offset=0;
  ret = (int)doCommand(message);
  Serial.println(ret);
  bufout[offset] = 0; // null terminate
  if(offset > 1) offset -= 2; // remove trailing "\n"
  if(!offset) strcpy(bufout, "(empty)");
	sendMessage(1, bufout);
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
