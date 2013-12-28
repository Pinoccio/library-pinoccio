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
#define CMD_SSLOPEN         19
#define CMD_TCERTADD        20
#define CMD_TCERTDEL        21
#define CMD_NTIMESYNC       22
#define CMD_WAUTO           23
#define CMD_NAUTO           24
#define CMD_PROFILESAVE     25
#define CMD_PROFILEDEFAULT  26
#define CMD_PROFILEGET      27
#define CMD_CURCID          28
#define CMD_PSDPSLEEP       29
#define CMD_STORENWCONN     30
#define CMD_RESTORENWCONN   31
#define CMD_NCMAUTO_START   32
#define CMD_LIST_SSIDS      33
#define CMD_RESET           34
#define CMD_NCMAUTO_STOP    35
#define CMD_PROFILEERASE    36
#define CMD_L4RETRY_COUNT   37

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
  webGainspan();
  uint8_t mode;
  bool debugAutoConnect;

  uint8_t setup(uint32_t baud=115200);
  uint8_t init();
  void configure(GS_PROFILE* prof);
  bool autoConfigure(const char *ssid, const char *passphrase, String ip, String port);
  uint8_t connect();
  uint8_t autoConnect();
  uint8_t connected();
  void process();
  uint8_t connectSocket(SOCKET s, String ip, String port);
  uint8_t connectToExistingCID(SOCKET s, int cid);
  String dnsLookup(String url);
  void send_data(String data);
  void esc_seq_start();
  void esc_seq_stop();
  String get_dev_id();
  String getAppVersion();
  String getGepsVersion();
  String getWlanVersion();
  uint8_t getAutoConnectSocket();

  void configSocket(SOCKET s, uint8_t protocol, uint16_t port);
  void execSocketCmd(SOCKET s, uint8_t cmd);
  uint8_t readSocketStatus(SOCKET s);
  uint8_t getSocketProtocol(SOCKET s);
  uint8_t isDataOnSock(SOCKET s);
  uint16_t readData(SOCKET s, uint8_t* buf, uint16_t len);
  uint16_t writeData(SOCKET s, const uint8_t*  buf, uint16_t  len);

  /**
   * Perform TLS handshaking. Should be called after a connection is
   * opened, but before any data is sent. After this, all data sent will
   * be encrypted.
   *
   * The certname is the name of a certificate previously set through
   * addCert. The certificate should be a CA certificate. If the server
   * supplies a certificate that is signed by this particular CA, then
   * the TLS handshake succeeds. If the server certificate is not signed
   * by this CA (or is invalid for other reasons, like expiry date), the
   * connection is closed and 0 is returned.
   *
   * Note that no checking of the server certificate's commonName
   * happens! If you pass in a (commercial) CA certificate, _any_
   * certificate issued by that CA will be accepted, not just the ones
   * with a specific hostname inside.
   *
   * Also make sure that the current time is correctly set, otherwise
   * the server certificate will likely be considered expired or not yet
   * valid even when it isn't.
   */
  uint8_t enableTls(SOCKET s, String certname);

  /**
   * Save the given certificate to the wifi gainspan's flash or RAM
   * (depending on to_flash). The name can be any string and should be
   * passed to enableTls later. The buffer should contain the ca
   * certificate in (binary) DER format. */
  uint8_t addCert(String certname, bool to_flash, const uint8_t *buf, uint16_t len);
  uint8_t delCert(String certname);
  /**
   * Do an SNTP timesync to an NTP server.
   * A one-shot sync is performed immediately and, if interval is
   * non-zero, more syncs are performed every interval seconds.
   * timeout is the number of seconds to wait for the server's response.
   */
  uint8_t timeSync(String ntp_server, uint8_t timeout, uint16_t interval);

  static const uint16_t SSIZE = 256; // Max Tx buffer siz

  String readline(void);
  uint8_t send_cmd(uint8_t cmd);
  uint8_t send_raw_cmd(const char *cmd);
  uint8_t parse_resp(uint8_t cmd);
  uint8_t parse_raw_resp();
  uint8_t send_cmd_w_resp(uint8_t cmd);
  uint8_t send_raw_cmd_w_resp(const char *cmd);
  void parse_cmd(String buf);
  void parse_data(String buf);

  void showHex(const char b, const bool newline, const bool show0x);
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
  String certname;
  bool to_flash;
  uint16_t cert_size;
  uint8_t timeout;
  uint16_t interval;
  uint8_t connection_state;
  String dev_id;
  String appVersion;
  String gepsVersion;
  String wlanVersion;
  String dns_url_ip;
  uint8_t tx_done;

  SOCK_TABLE sock_table[4];
  uint8_t socket_num;
  uint8_t autoConnectSocket;
  SOCKET dataOnSock;
  String srcIPUDP;
  String srcPortUDP;

  void (*rx_data_handler)(String data);
};

extern webGainspan Gainspan;

#endif // _PINOCCIO_WEB_GAINSPAN_H_