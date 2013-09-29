/*
Wireless Sensor Lamp

This sketch is used in conjunction with the PingServer demo to control
a LED light strip wirelessly.  The PingClient uses the Ping))) sensor 
to determine if an object (i.e. your hand) is within 7 inches of the 
sensor.  If so, it sends a command to the server to inform the server to
toggle the LED light.

More details of the PingServer build-up are available on the DIYSandbox
webiste.

The circuit:
     * Ping))) sensor, attached to pin 7
     
created Jan 21 2012
by c0demonkey

This example code is in the public domain.

*/

#include <Wirefree.h>
#include <WifiClient.h>

WIFI_PROFILE wireless_prof = {
                        /* SSID */ "diysandbox",
         /* WPA/WPA2 passphrase */ "12345678",
                  /* IP address */ NULL,
                 /* subnet mask */ NULL,
                  /* Gateway IP */ NULL, };

String server = "192.168.1.7"; // Hydrogen WebServer

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
WifiClient client(server, "80", PROTO_TCP);

// this constant won't change.  It's the pin number
// of the sensor's output:
const int pingPin = 7;

void setup()
{
  // connect to AP & start server
  Wireless.begin(&wireless_prof);
    
//  delay(1000);
}

void loop()
{
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches;
  
  int output, outputPrev;
  String cmd = "0";
  
  output = 0; outputPrev = 0;
  
  while (1) {
    // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(pingPin, LOW);

    // The same pin is used to read the signal from the PING))): a HIGH
    // pulse whose duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(pingPin, INPUT);
    duration = pulseIn(pingPin, HIGH);

    // convert the time into a distance
    inches = microsecondsToInches(duration);
  
    if (inches < 7) {
      outputPrev = output;
      output = 1;
    } else {
      outputPrev = output;
      output = 0;
    }
    
    if (output == outputPrev) {
      // two readings in a row are the same, so check if we need to do something
      if ((output == 1) && (cmd == "0")) {
        cmd = "1";
        break;
      } 
    }
    delay(1000);
  }
  
  // if you get a connection, report back via serial:
  if (client.connect()) {
    Serial.println("connection Success..");
    
    // Make a HTTP request:
    client.print(cmd);
    client.flush();
  } 
  else {
    // kf you didn't get a connection to the server:
    Serial.println("connection failed..");
  }
  
  Serial.println("disconnecting.");
  client.stop();
 
  delay(3000);
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

