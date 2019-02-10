#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>

#define REQ_LEN 64

int fanPin = 4;

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0xEF, 0x13
};

IPAddress ip(192,168,0,106);

// Initialize the Ethernet server library with the IP address and port
EthernetServer server(80);

char buf_req[REQ_LEN] = {0};
char req_index = 0;

#define BUF_LEN  900 // characters reserved for the incoming request
char buf_content[BUF_LEN];
int counter = 0;

//const char token1[] PROGMEM = "GET /off ";

char put_query_url[] = "PUT /uni-bonn/raum1047/led";
char get_query_url[] = "GET /uni-bonn/raum1047/led";

String bad_request_msg = "Bad Request";
String not_found_msg = "Resource not found";
String state_on_msg = "@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n@prefix saref: <https://w3id.org/saref#> .\n\n\t<> a saref:LightingDevice ;\n\tsaref:hasState saref:On .\n";
String state_off_msg = "@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n@prefix saref: <https://w3id.org/saref#> .\n\n\t<> a saref:LightingDevice ;\n\tsaref:hasState saref:Off .\n";
String state_prefix = "@prefix foaf: <http://xmlns.com/foaf/0.1/> .\n@prefix saref: <https://w3id.org/saref#> .\n\n\t<> a saref:LightingDevice ;\n\tsaref:hasState ";




void setup() {
  // setup code to run once:
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("starting... ");
  Serial.println(Ethernet.localIP());
  //starting LED to show that the microcontroller is working
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println(Ethernet.localIP());

  //reserve full buffer with EOS (end of string)
  memset(buf_content, 0, BUF_LEN);
  pinMode(fanPin, OUTPUT);
  
}

void loop() {


  // main code to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    char buffer[746];
    int index = 0;
    //Response of LDFU is processed
    while (client.connected()) {
      if (client.available()) {
       
        char c = client.read();
        //saves the first line of header to assign appropriate response later
        if (req_index < (REQ_LEN - 1)) { 
          buf_req[req_index] = c;
          req_index++;
        }
        Serial.write(c);
        
        //if end of line is reached and currentLineIsBlank is true then following
        if (c == '\n' && currentLineIsBlank) { 
          
          for (int i = 0; i < BUF_LEN; i++) 
          { 
            buf_content[i] = client.read();
            if(index < 745)
            {
              buffer[index] = buf_content[i];
              index++;
              buffer[index] = '\0';
            }
            // if client is not available and therefore full content is saved, break
            if (client.available() == 0) { 
              break;
            }
            counter++; // counter indicates length of the field (900 characters are reserved)
              
          }
            Serial.print(buffer);
          // if loops look for appropriate response
          // ON response
          if ( strstr(buf_req, put_query_url) != 0 ) {
            if ( strstr(buffer, "saref:On") != NULL ) {
              Serial.println("Turning On the pin");
              
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/turtle");
              client.print("Content-Length: ");
              client.println(state_on_msg.length());
              client.println("Connection: close");
              client.println();
              client.println(state_on_msg);
              
              digitalWrite(fanPin, HIGH); 
            }
            else if ( strstr(buffer, "saref:Off") != NULL )  {
              Serial.println("Turning Off the pin");
              
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/turtle");
              client.print("Content-Length: ");
              client.println(state_off_msg.length());
              client.println("Connection: close");
              client.println();
              client.println(state_off_msg);

             digitalWrite(fanPin, LOW);
            } else {
              Serial.println("The request is wrong (http: 400)");
              
              client.println("HTTP/1.1 400 Bad Request"); //204 No Content
              client.println("Content-Type: text/plain");            
              client.print("Content-Length: ");
              client.println(bad_request_msg.length());
              client.println("Connection: close");
              client.println();
              client.println(bad_request_msg);
            }
          }
          else if ( strstr(buf_req, get_query_url) != 0 ) {
            int fan_pin_state = digitalRead(fanPin);
            String fan_pin_state_msg = (fan_pin_state == 1) ? "saref:On .\n" : "saref:Off .\n";
            int content_len = state_prefix.length() + fan_pin_state_msg.length();


            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/turtle");
            client.print("Content-Length: ");
            client.println(content_len);
            client.println("Connection: close");
            client.println();
            client.print(state_prefix);
            client.println(fan_pin_state_msg);           
          }      
          else {
            Serial.println("The url is wrong (http: 404)");
            
            client.println("HTTP/1.1 404 Not Found"); //204 No Content
            client.println("Content-Type: text/plain");            
            client.print("Content-Length: ");
            client.println(not_found_msg.length());
            client.println("Connection: close");
            client.println();
            client.println(not_found_msg);
          }
          req_index = 0;
          memset(buf_req, 0, REQ_LEN);
          counter = 0;
          memset(buf_content, 0, BUF_LEN);
          break;
        }
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
        //        } // first line break
      } // if (client.available())
    } // while (client.connected())
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    Serial.println();
  }
}
