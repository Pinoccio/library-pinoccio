#ifndef LIB_PINOCCIO_SCOUTHANDLER_H_
#define LIB_PINOCCIO_SCOUTHANDLER_H_

#include <Pinoccio.h>
#include <ScoutHandler.h>

class PinoccioScoutHandler {

  public:
    PinoccioScoutHandler();
    ~PinoccioScoutHandler();

    void setup();
    void loop();
    void fieldAnnounce(uint16_t chan, char *message);
    void setVerbose(bool flag);

  protected:
};

static bool hqVerboseOutput;

static void hqConnectHandler(uint8_t cid);
static void hqDisconnectHandler(uint8_t cid);

static char *fieldCommand = NULL;
static int fieldCommandLen = 0;
static bool isAnnouncing = false;
static int fieldAnswerTo = 0;
static char *fieldAnswerChunks;
static int fieldAnswerChunksAt;
static int fieldAnswerRetries;
static NWK_DataReq_t fieldAnswerReq;

// mesh callback for handling incoming commands
static bool fieldCommands(NWK_DataInd_t *ind);

// chunk packet confirmation callback by mesh
static void fieldAnswerChunkConfirm(NWK_DataReq_t *req);

// send the first/next chunk of the answer back and confirm
static void fieldAnswerChunk();
static void fieldAnnounceConfirm(NWK_DataReq_t *req);

// send out any announcement messages on a multicast channel
static void fieldAnnounce(int chan, char *message);

// mesh callback whenever another scout announces something on a channel
static bool fieldAnnouncements(NWK_DataInd_t *ind);

// simple wrapper for the incoming channel announcements up to HQ
static void leadAnnouncementSend(int chan, int from, char *message);

// necessities for tracking state when chunking up a large command into mesh requests
static int leadCommandTo = 0;
static char *leadCommandChunks;
static int leadCommandChunksAt;
static int leadCommandRetries;
static NWK_DataReq_t leadCommandReq;
static void leadCommandChunk(void);
static int leadAnswerID = 0;
static NWK_DataReq_t fieldAnnounceReq;

static bool leadAnswers(NWK_DataInd_t *ind);
static void leadAnnouncementSend(int chan, int from, char *message);
static void leadHQ(void);
static void leadSignal(char *json);
static void leadIncoming(char *packet, unsigned short *index);
void leadHQConnect();

// this is called on the main loop to try to (re)connect to the HQ
static void leadHQHandle(void);

// process a packet from HQ
static void leadIncoming(char *packet, unsigned short *index);

// mesh callback when sending command chunks
static void leadCommandChunkConfirm(NWK_DataReq_t *req);

// called to send the first/next chunk of a command to another scout
static void leadCommandChunk();

// wrapper to send a chunk of JSON to the HQ
static void leadSignal(char *json);

// called whenever another scout sends an answer back to us
static bool leadAnswers(NWK_DataInd_t *ind);


#endif