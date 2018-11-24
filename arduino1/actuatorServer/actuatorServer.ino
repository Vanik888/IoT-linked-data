#include <SPI.h>
#include <Ethernet.h>

#define REQ_LEN 20

int fanPin = 4;

byte mac[] = {
  0x90, 0xA2, 0xDA, 0x10, 0xBB, 0xB7
};
IPAddress ip(192, 168, 0, 104); //172, 22, 177, 231

// Initialize the Ethernet server library with the IP address and port
EthernetServer server(80);

char buf_req[REQ_LEN] = {0};
char req_index = 0;

#define BUF_LEN  900 // characters reserved for the incoming request
char buf_content[BUF_LEN];
int counter = 0;

//const char token1[] PROGMEM = "GET /off ";
char token1[] = "GET /off ";
char token2[] = "PUT /off";
char token3[] = "PUT /on";
char token4[] = "GET /on";
char token5[] = "GET / ";

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
  //starting LED to show that the microcontroller is working
  pinMode(LED_BUILTIN, OUTPUT);

//  Serial.println(Ethernet.localIP());

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
          //saves content in field
          Serial.println("Start to read the request body");
          for (int i = 0; i < BUF_LEN; i++) { 
            buf_content[i] = client.read();;
            Serial.println(buf_content[i]);
            counter++; // counter indicates length of the field (900 characters are reserved)
            // if client is not available and therefore full content is saved, break
            if (client.available() == 0) { 
              break;
            }            

          }
          Serial.println("Finish to read the request body");
          Serial.print("counter: " );
          Serial.println(counter);
          // if loops look for appropriate response
          // OFF response
          if ((strstr(buf_req, token1) != 0) || (strstr(buf_req, token2) != 0)) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/rdf+xml");
            client.print("Content-Length: ");
            client.println(counter);
            client.println("Connection: close");
            client.println();
            //sends received content of the request back
            for (int i = 0; i < counter; i++) {
              client.print(buf_content[i]);
            }
            //turning fan off
            digitalWrite(fanPin, HIGH);
          }
          // ON response
          else if ((strstr(buf_req, token3) != 0) || (strstr(buf_req, token4) != 0)) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/rdf+xml");
            client.print("Content-Length: ");
            client.println(counter);
            client.println("Connection: close");
            client.println();
            for (int i = 0; i < counter; i++) {
              client.print(buf_content[i]);
            }
            //turning fan on
            digitalWrite(fanPin, LOW);
          }
          // GET on ip adresse
          else if (strstr(buf_req, token5) != 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Connection: close");
            client.println();
          }
          // wrong request
          else {
            client.println("HTTP/1.1 404 Not Found"); //204 No Content
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
