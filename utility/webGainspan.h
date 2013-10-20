#ifndef  _PINOCCIO_WEB_GAINSPAN_H_
#define  _PINOCCIO_WEB_GAINSPAN_H_

#include <avr/pgmspace.h>
#include "WString.h"

typedef uint8_t SOCKET;

class SOCK_STATUS {
public:
  static const uint8_t CLOSED      = 0x00;
  static const uint8_t INIT        = 0x01;
  static const uint8_t LISTEN      = 0x02;
  static const uint8_t ESTABLISHED = 0x03;
  static const uint8_t CLOSE_WAIT  = 0x04;
};

class IPPROTO {
public:
  static const uint8_t TCP        = 6;
  static const uint8_t UDP        = 7;
  static const uint8_t UDP_CLIENT = 8;
};

// command identifiers
// config
#define CMD_DISABLE_ECHO     0
// wifi
#define CMD_SET_WPA_PSK     1
#define CMD_SET_SSID        2
#define CMD_DISCONNECT      5
#define CMD_GET_MAC_ADDR    8
//network
#define CMD_DISABLE_DHCP    3
#define CMD_ENABLE_DHCP     4
#define CMD_TCP_LISTEN      6
#define CMD_TCP_CONN        7
#define CMD_DNS_LOOKUP      9
#define CMD_CLOSE_CONN      10
#define CMD_NETWORK_SET     11
#define CMD_WIRELESS_MODE   12
#define CMD_ENABLE_DHCPSVR  13
#define CMD_UDP_LISTEN      14
#define CMD_UDP_CONN        15
#define CMD_NET_STATUS      16
#define CMD_AT              17
#define CMD_GET_VERSION     18

#define CMD_INVALID         255

// device operation modes
#define DEV_OP_MODE_COMMAND 0
#define DEV_OP_MODE_DATA    1
#define DEV_OP_MODE_DATA_RX 2

// device wireless connection state
#define DEV_CONN_ST_DISCONNECTED 0
#define DEV_CONN_ST_CONNECTED    1

// connection ID
#define INVALID_CID 255

// wireless connection params
typedef struct _GS_PROFILE {
  String ssid;
  String security_key;
  String ip;
  String subnet;
  String gateway;
} GS_PROFILE;

typedef struct _SOCK_TABLE {
  uint8_t status;
  uint8_t protocol;
  uint16_t port;
  uint8_t cid;
} SOCK_TABLE;

class webGainspan {
public:
  uint8_t mode;
  uint8_t init(uint32_t baud=115200);
  void configure(GS_PROFILE* prof);
  uint8_t connect();
  uint8_t connected();
  void process();
  uint8_t connectSocket(SOCKET s, String ip, String port);
  String dns_lookup(String url);
  void send_data(String data);
  void esc_seq_start();
  void esc_seq_stop();
  String get_dev_id();
  String getAppVersion();
  String getGepsVersion();
  String getWlanVersion();

  void configSocket(SOCKET s, uint8_t protocol, uint16_t port);
  void execSocketCmd(SOCKET s, uint8_t cmd);
  uint8_t readSocketStatus(SOCKET s);
  uint8_t getSocketProtocol(SOCKET s);
  uint8_t isDataOnSock(SOCKET s);
  uint16_t readData(SOCKET s, uint8_t* buf, uint16_t len);
  uint16_t writeData(SOCKET s, const uint8_t*  buf, uint16_t  len);
  
  static const uint16_t SSIZE = 256; // Max Tx buffer siz


  String readline(void);
  uint8_t send_cmd(uint8_t cmd);
  uint8_t parse_resp(uint8_t cmd);
  uint8_t send_cmd_w_resp(uint8_t cmd);
  void parse_cmd(String buf);
  void parse_data(String buf);

  void flush();
  
private:
  String security_key;
  String ssid;
  String local_ip;
  String subnet;
  String gateway;
  uint8_t serv_cid;
  uint8_t client_cid;
  uint8_t dev_mode;
  String ip;
  String port;
  uint8_t connection_state;
  String dev_id;
  String appVersion;
  String gepsVersion;
  String wlanVersion;
  String dns_url_ip;
  uint8_t tx_done;

  SOCK_TABLE sock_table[4];
  uint8_t socket_num;
  SOCKET dataOnSock;
  String srcIPUDP;
  String srcPortUDP;

  void (*rx_data_handler)(String data);
};

extern webGainspan Gainspan;

#endif // _PINOCCIO_WEB_GAINSPAN_H_