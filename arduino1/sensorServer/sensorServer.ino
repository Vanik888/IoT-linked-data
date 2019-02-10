/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 
 */

#include <SPI.h>
#include <Ethernet.h>

#define REQ_LEN 64
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0xBB, 0xB7
};
IPAddress ip(192, 168, 0, 105);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

const int sensorPin = 0;
const char token1[] = "GET /uni-bonn/raum1047/temperature";

const String prefix = "@prefix ssn: <http://purl.oclc.org/NET/ssnx/ssn#> .\n@prefix cdt: <http://w3id.org/lindt/custom_datatypes#> .\n@prefix xsd:  <http://www.w3.org/2001/XMLSchema#> .\n@prefix sosa: <http://www.w3.org/ns/sosa/> .\n\n";
const String triple1 = "\t<> a sosa:Observation ;\n\tsosa:hasSimpleResult \"";
const String triple2 = " Cel\"^^cdt:temperature .\n";

char buf_req[REQ_LEN] = {0};
int req_index = 0;

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  
  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  //configures reference voltage for temperature sensor to 1.1 instead of 5V
  analogReference(INTERNAL);
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
   
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (req_index < (REQ_LEN - 1)) { 
          buf_req[req_index] = c;
          req_index++;
        }

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          Serial.print("token1= ");
          Serial.println(token1);
          Serial.print("url= ");
          Serial.println(buf_req);
          
          if (strstr(buf_req, token1) != 0) {            

            int sensor_val = analogRead(sensorPin);
            int temp = sensor_val / 9.31; //10mV equals to 1 degree -> 10mV/ (1.1V /1024)=9.31       
            String temp_str = String(temp);
            
            Serial.print("temperature string=");
            Serial.print(temp_str);
            int content_len = prefix.length() + triple1.length() + temp_str.length() + triple2.length();
            Serial.print("prefix-length=");
            Serial.println(prefix.length());
            
            Serial.print("content-length=");
            Serial.println(content_len);
            Serial.println(prefix);


            Serial.print(triple1);
            Serial.print(temp_str);
            Serial.println(triple2);

            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/turtle");
            client.print("Content-Length: ");
            client.println(content_len);
            client.println("Connection: close");
            client.println();
            client.print(prefix);


            client.print(triple1);
            client.print(temp_str);
            client.print(triple2);


            Serial.print("Url matched. Send the temperature: ");
            Serial.println(temp_str);
          }          
          else {
            Serial.println("Unknown url. Send 404.");
            client.println("HTTP/1.1 404 Not Found"); 
            client.println("Content-Type: text/turtle");
            client.println("Content-Length: 0");
            client.println("Connection: close");            
          }
          req_index = 0;
          memset(buf_req, 0, REQ_LEN);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
