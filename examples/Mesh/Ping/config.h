#ifndef _CONFIG_H_
#define _CONFIG_H_

/*****************************************************************************
*****************************************************************************/
#define APP_ADDR                  0
#define APP_CHANNEL               0x1a // channel 26, at the end of the 2.4GHz spectrum
#define APP_PANID                 0x4567
#define APP_ENDPOINT              1
#define APP_SECURITY_KEY          "TestSecurityKey0"
#define APP_FLUSH_TIMER_INTERVAL  1000

#define SYS_SECURITY_MODE                   0

#define NWK_BUFFERS_AMOUNT                  3
#define NWK_MAX_ENDPOINTS_AMOUNT            3
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   800 // ms

//#define PHY_ENABLE_ENERGY_DETECTION
#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY

#define HAL_ENABLE_UART
#define HAL_UART_CHANNEL                    0
#define HAL_UART_RX_FIFO_SIZE               200
#define HAL_UART_TX_FIFO_SIZE               200

#endif // _CONFIG_H_
