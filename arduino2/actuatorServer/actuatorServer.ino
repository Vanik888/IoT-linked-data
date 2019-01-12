#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>

#define REQ_LEN 64

int fanPin = 4;

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0xEF, 0x13
};

IPAddress ip(192,168,0,112);

// Initialize the Ethernet server library with the IP address and port
EthernetServer server(80);

char buf_req[REQ_LEN] = {0};
char req_index = 0;

#define BUF_LEN  900 // characters reserved for the incoming request
char buf_content[BUF_LEN];
int counter = 0;


// List Of Url That device is going to accept
const char token2[] = "PUT /uni-bonn/raum1047/led";
const char token3[] = "GET /uni-bonn/raum1047/led";

const String prefix = "@prefix foaf: <http://xmlns.com/foaf/0.1/> .@prefix saref: <https://w3id.org/saref#> .<> a saref:LightingDevice ;";
const String triple1 = "saref:hasState saref:";

void setup() {
  // setup code to run once:
  // Open serial communications and wait for port to open:
  Serial.begin(9060);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("starting... ");
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
          //  PUT ON response
          if ((strstr(buf_req, token2) != 0   && strstr(buffer, "On") != NULL))
          {

            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: RDF/XML");
            client.print("Content-Length: ");
            client.println(counter);
            client.println("Connection: close");
            client.println();
            // Turn ON Led
            digitalWrite(fanPin, HIGH);
          }
            // PUT off response
          else if((strstr(buf_req, token2) != 0  && strstr(buffer, "Off") != NULL)){
             client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: RDF/XML");
            client.print("Content-Length: ");
            client.println(counter);
            Serial.print("sending response...");
            client.println("Connection: close");
            client.println();

            //turning fan off
           digitalWrite(fanPin, LOW);


          }
          // GET status of the pin
          else if(strstr(buf_req, token3) != 0){
            String ledState = digitalRead(fanPin) == 0 ? "Off .": "On .";
            String finalLedState =   triple1+ledState;
            int content_len = finalLedState.length();
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/turtle");
            client.print("Content-Length: ");
            client.println("Connection: close");
            client.println();
            client.print(prefix);
            client.print(finalLedState);
          }
          else {

            client.println("HTTP/1.1 404 Not Found"); //204 No Content
            client.println("Content-Type: RDF/XML");
            client.println("Connection: Keep Alive");

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