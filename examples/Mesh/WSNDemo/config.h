#ifndef _CONFIG_H_
#define _CONFIG_H_

/*****************************************************************************
*****************************************************************************/
#define APP_ADDR                0x0000
#define APP_CHANNEL             0x0f
#define APP_PANID               0x1234
#define APP_SENDING_INTERVAL    2000
#define APP_ENDPOINT            1
#define APP_OTA_ENDPOINT        2
#define APP_SECURITY_KEY        "TestSecurityKey0"

//#define APP_ENABLE_OTA

//#define PHY_ENABLE_RANDOM_NUMBER_GENERATOR

#define SYS_SECURITY_MODE                   0

#define NWK_BUFFERS_AMOUNT                  3
#define NWK_MAX_ENDPOINTS_AMOUNT            3
#define NWK_DUPLICATE_REJECTION_TABLE_SIZE  10
#define NWK_DUPLICATE_REJECTION_TTL         3000 // ms
#define NWK_ROUTE_TABLE_SIZE                100
#define NWK_ROUTE_DEFAULT_SCORE             3
#define NWK_ACK_WAIT_TIME                   1000 // ms

#define NWK_ENABLE_ROUTING
//#define NWK_ENABLE_SECURITY

#define HAL_ENABLE_UART
#define HAL_UART_CHANNEL                    1
#define HAL_UART_RX_FIFO_SIZE               1
#define HAL_UART_TX_FIFO_SIZE               100

#endif // _CONFIG_H_
