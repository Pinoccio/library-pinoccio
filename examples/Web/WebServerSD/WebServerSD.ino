#include <Wirefree.h>
#include <WifiServer.h>
#include <SD.h>

#define INCLUDE_SD

File myFile;

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ "192.168.1.1",
                 /* subnet mask */ "255.255.255.0",
                  /* Gateway IP */ "192.168.1.1", };

// port 80 is default for HTTP
WifiServer server(80, PROTO_TCP);

void setup()
{
#ifdef INCLUDE_SD
  // CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);

  if (!SD.begin(10)) {
    return;
  }
#endif /* INCLUDE_SD */

  // connect to AP & start server
  Wireless.begin(&wireless_prof, AP_MODE);
  server.begin();

  delay(1000);
}

void loop()
{
  // Listen for incoming clients
  WifiClient client = server.available();
  if (client) {
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c;
        int b;

        while((b = client.read()) == -1);
        c = b;
        Serial.print(c);
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("");

#ifdef INCLUDE_SD
          myFile = SD.open("index.htm");

          if (myFile) {
            // read from the file until there's nothing else in it:
            while (myFile.available()) {
              client.write(myFile.read());
            }
            myFile.close();

            client.println("");
          }
          else
#endif /* INCLUDE_SD */
          {
            client.println("Page not found");
            client.println("");
          }
          
          client.flush();
          
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r'){
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection
    client.stop();
  }
}
