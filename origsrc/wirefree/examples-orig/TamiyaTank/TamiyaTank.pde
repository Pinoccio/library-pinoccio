/*

Tamiya Tank Demo

This sketch drives a Tamiya tank platform.  It uses two digital pins to control
each set of treads.  Currently, the demo tank only controls the treads in a 
forward motion.  Therefore, it's only possible for the tank to currently go
forward, left and right.  

More details of the tank build-up are avaiable on the DIYSandbox website.

The tank creates a network named 'DIYTank', which you can connect to with a
mobild phone.  Viewing the webpage http://192.168.1.1 will show a simple webpage
which will allow you to control the direction of the tank.

The circuit:
     * pin 7: negative active (0V) enables the right track so the tank will
              turn left
     * pin 8: negative active (0V) enables the left trank so the tank with
              turn right
            
created Jan 21 2012
by c0demonkey
             
This example code is in the public domain.

*/

#include <Wirefree.h>
#include <WifiServer.h>

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
  // connect as AP & start server
  Wireless.begin(&wireless_prof, AP_MODE);
  server.begin();
  
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  
  delay(1000);
}

void loop()
{
  // Listen for incoming clients
  WifiClient client = server.available();
  int count = 0;
  
  if (client) {
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
          //client.println("");     
                     
          // Output HTML page
          client.println("<html><head><title>DIYTank</title></head>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>");
          client.println("<body><p><form method=\"get\" action=\"\"><table border=\"0\"><tr>");
          client.println("<td></td><td><input type=\"radio\" name=\"d\" value=\"f\">Fwd</td><td><td></tr>");
          client.println("<tr><td><input type=\"radio\" name=\"d\" value=\"l\">Left</td>");
          client.println("<td><input type=\"radio\" name=\"d\" value=\"s\">Stop</td>");
          client.println("<td><input type=\"radio\" name=\"d\" value=\"r\">Right</td></tr></table>");
          client.println("<br /><input type=\"submit\" value=\"Go!\">");
          client.println("</form></body></html>");  
          client.println("");  

          client.flush();

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
             c = b;
             if ( c == 'l') {
               digitalWrite(7, LOW);
               digitalWrite(8, HIGH);
               Wireless.setLED(LED_RED);
             }
             else if ( c == 'f' ) {
               digitalWrite(7, LOW);
               digitalWrite(8, LOW);
               Wireless.setLED(LED_GREEN);
             }
             else if ( c == 'r' ) {
               digitalWrite(7, HIGH);
               digitalWrite(8, LOW);
               Wireless.setLED(LED_BLUE);
             }
             else if ( c == 's' ) {
               digitalWrite(7, HIGH);
               digitalWrite(8, HIGH);
               Wireless.setLED(LED_WHITE);
             }
          }          
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection         
    client.stop();            
  }
}


