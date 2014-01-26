#ifndef LIB_PINOCCIO_SHELL_H_
#define LIB_PINOCCIO_SHELL_H_

#include "bitlash.h"
#include "src/bitlash.h"

#include "utility/sysConfig.h"
#include "utility/phy.h"
#include "utility/hal.h"
#include "utility/sys.h"
#include "utility/nwk.h"
#include "utility/sysTimer.h"
#include "utility/halSleep.h"
#include "utility/halTemperature.h"
#include "utility/meshRequest.h"
#include "avr/sleep.h"

class PinoccioShell {

  public:
    PinoccioShell();
    ~PinoccioShell();

    void setup();
    void loop();
    void allReportHQ();

    void startShell();
    void disableShell();

    char *bitlashOutput;
  protected:
    bool isShellEnabled;
};

extern PinoccioShell Shell;

static numvar getTemperature(void);
static numvar getRandomNumber(void);
static numvar uptimeReport(void);

static numvar isBatteryCharging(void);
static numvar getBatteryPercentage(void);
static numvar getBatteryVoltage(void);
static numvar enableBackpackVcc(void);
static numvar disableBackpackVcc(void);
static numvar goToSleep(void);
static numvar powerReport(void);

static numvar ledEnableContinuousBlink(void);
static numvar ledDisableContinuousBlink(void);
static numvar ledBlink(void);
static numvar ledBlinkTorch(void);
static numvar ledOff(void);
static numvar ledRed(void);
static numvar ledGreen(void);
static numvar ledBlue(void);
static numvar ledCyan(void);
static numvar ledPurple(void);
static numvar ledMagenta(void);
static numvar ledYellow(void);
static numvar ledOrange(void);
static numvar ledWhite(void);
static numvar ledTorch(void);
static numvar ledSetHexValue(void);
static numvar ledSetRgb(void);
static numvar ledSaveTorch(void);
static numvar ledReport(void);

static numvar meshConfig(void);
static numvar meshSetPower(void);
static numvar meshSetDataRate(void);
static numvar meshSetKey(void);
static numvar meshResetKey(void);
static numvar meshJoinGroup(void);
static numvar meshLeaveGroup(void);
static numvar meshIsInGroup(void);
static numvar meshPing(void);
static numvar meshPingGroup(void);
static numvar meshSend(void);
static numvar meshVerbose(void);
static numvar meshReport(void);
static numvar meshRouting(void);
static numvar meshAnnounce(void);

static numvar pinOn(void);
static numvar pinOff(void);
static numvar pinMakeInput(void);
static numvar pinMakeInputPullup(void);
static numvar pinMakeOutput(void);
static numvar pinRead(void);
static numvar pinWrite(void);
static numvar pinThreshold(void);
static numvar pinReport(void);

static numvar backpackReport(void);
static numvar backpackList(void);

static numvar scoutReport(void);
static numvar isScoutLeadScout(void);
static numvar setHQToken(void);
static numvar getHQToken(void);
static numvar wdtBoot(void);

static numvar startStateChangeEvents(void);
static numvar stopStateChangeEvents(void);
static numvar setEventPeriods(void);
static numvar setEventVerbose(void);

static numvar wifiReport(void);
static numvar wifiList(void);
static numvar wifiConfig(void);
static numvar wifiDhcp(void);
static numvar wifiStatic(void);
static numvar wifiReassociate(void);
static numvar wifiCommand(void);
static numvar wifiPing(void);
static numvar wifiDNSLookup(void);
static numvar wifiGetTime(void);
static numvar wifiSleep(void);
static numvar wifiWakeup(void);
static numvar wifiVerbose(void);

static void pingScout(int address);
static void pingGroup(int address);
static void pingConfirm(NWK_DataReq_t *req);
static bool receiveMessage(NWK_DataInd_t *ind);
static void sendMessage(int address, char *data, bool getAck);
static void sendConfirm(NWK_DataReq_t *req);

static void digitalPinEventHandler(uint8_t pin, uint8_t value);
static void analogPinEventHandler(uint8_t pin, uint16_t value);
static void batteryPercentageEventHandler(uint8_t value);
static void batteryVoltageEventHandler(uint8_t value);
static void batteryChargingEventHandler(uint8_t value);
static void batteryAlarmTriggeredEventHandler(uint8_t value);
static void temperatureEventHandler(uint8_t value);

void bitlashFilter(byte b); // watches bitlash output for channel announcements
void bitlashBuffer(byte b); // buffers bitlash output from a command

#endif