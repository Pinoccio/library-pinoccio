#include <Pinoccio.h>

WIFI_PROFILE profile = {
        /* SSID */ "",
        /* WPA/WPA2 passphrase */ "",
        /* IP address */ "",
        /* subnet mask */ "",
        /* Gateway IP */ "", };

// port 80 is default for HTTP
PinoccioWifiServer server(80, PROTO_TCP);

void setup() {
  Serial.println("Starting up");
  Pinoccio.init();
  Serial.begin(115200);
  Serial.println("Starting wireless...");

  Wifi.begin(&profile);
  server.begin();

  Serial.println("Done");
  delay(1000);
}

void loop() {
  Pinoccio.taskHandler();
  
  // Listen for incoming clients
  PinoccioWifiClient client = server.available();
  int count = 0;

  if (client) {
    Serial.print('Client connected, sending response...');
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c;
        int  b;

        while((b = client.read()) == -1);
        c = b;
         // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          count = 0;

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("");

          // Output HTML page
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("  <head>");
          client.println("    <title>Pinoccio - LED Demo</title>");
          client.println("    <link rel='stylesheet' href='http://pinocc.io/blog/wp-content/themes/smartstart/style.css?ver=1000' type='text/css' media='all'>");
          client.println("  </head>");
          client.println("  <body>");
          client.println("    <header id='header' class='container clearfix'>");
          client.println("      <div id='masthead' class='container'>");
          client.println("        <a href='http://pinocc.io/' id='logo' title='Pinoccio'>");
          client.println("          <img src='http://pinocc.io/blog/wp-content/uploads/2012/09/pinoccio-logo-face.png' alt='Pinoccio'>");
          client.println("        </a>");
          client.println("      </div>");
          client.println("  </header>");
          client.println("  <center>");
          client.println("  <br>");
          client.println("  <h1>RGB LED Demo</h1>");
          client.println("  <p><form method='get' action=''><select name='l'>");
          client.println("  <option value='r'>Red</option>");
          client.println("  <option value='g'>Green</option>");   
          client.println("  <option value='b'>Blue</option>");
          client.println("  <option value='c'>Cyan</option>"); 
          client.println("  <option value='m'>Magenta</option>");   
          client.println("  <option value='y'>Yellow</option>");
          client.println("  <option value='w'>White</option>"); 
          client.println("  </select><input type='submit' value='Change' /></form></p>");
          client.println("  </center>");
          client.println("  </body>");
          client.println("</html>");

          client.println("");

          client.flush();
          Serial.println('Done');
          break;
        }
        if (c == '\n') {
        // you're starting a new line
          currentLineIsBlank = true;
          count = 0;
        }
        else if (c != '\r'){
        // you've gotten a character on the current line
          currentLineIsBlank = false;
          count++;
          if ((c == '=') && (count == 8)){
            while((b = client.read()) == -1);
            Serial.print(b);
            c = b;
            if ( c == 'r')
              RgbLed.setRed();
            else if ( c == 'g')
              RgbLed.setGreen();
            else if ( c == 'b')
              RgbLed.setBlue();
            else if ( c == 'c')
              RgbLed.setCyan();
            else if ( c == 'm')
              RgbLed.setMagenta();
            else if ( c == 'y')
              RgbLed.setYellow();
            else if ( c == 'w')
              RgbLed.setWhite();
          }
        }
      }
    } 
    // give the web browser time to receive the data
    delay(250);
    // close the connection
    Serial.print("Disconnecting...");
    client.stop();
    Serial.println("Done");
  }
}
