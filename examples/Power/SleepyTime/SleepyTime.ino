#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>

typedef enum AppState_t
{
  APP_STATE_START_SEND,
  APP_STATE_GO_TO_SLEEP,
  APP_STATE_SLEEPING,
} AppState_t;

static AppState_t appState = APP_STATE_START_SEND;
static SYS_Timer_t appSleepTimer;
static uint8_t sleepCtr;


static void appSleepTimerHandler(SYS_Timer_t *timer) {
  appState = APP_STATE_START_SEND;
  (void)timer;
}

void setup() {
  Scout.setup();

  appSleepTimer.interval = 5000;
  appSleepTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appSleepTimer.handler = appSleepTimerHandler;

  sleepCtr = 0;
}

void loop() {
  Scout.loop();

  switch (appState) {
    case APP_STATE_START_SEND:
      if (sleepCtr < 5) {
        Serial.println("Radio send");

        appState = APP_STATE_GO_TO_SLEEP;
      } else {
        Serial.println("Deep sleep");

        do {
          set_sleep_mode(SLEEP_MODE_PWR_DOWN);
          sleep_mode();
        } while(0);
      }
      break;

    case APP_STATE_GO_TO_SLEEP:
      if (!NWK_Busy()) {
        Serial.println("Radio sleep");
        sleepCtr++;
        NWK_SleepReq();
        appState = APP_STATE_SLEEPING;
      }
      break;

    default:
      break;
  }
}
