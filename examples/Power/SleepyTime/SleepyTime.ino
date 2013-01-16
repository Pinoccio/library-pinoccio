#include "config.h"
#include <Pinoccio.h>

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

void PHY_EdConf(int8_t ed) {
  Serial.println(ed - PHY_RSSI_BASE_VAL);
}

void setup() {
  Pinoccio.init();
  Serial.begin(115200);

  appSleepTimer.interval = APP_SLEEP_INTERVAL;
  appSleepTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appSleepTimer.handler = appSleepTimerHandler;

  sleepCtr = 0;
}

void loop() {
  Pinoccio.taskHandler();

  switch (appState) {
    case APP_STATE_START_SEND:
      if (sleepCtr < 5) {
        Serial.println("Radio send");
      
        PHY_SetChannel(APP_CHANNEL);
        PHY_EdReq();
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
