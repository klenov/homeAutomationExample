/* Home Automation Script
   based on Webduino library
   Vasily Klenov, 2014 */

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"

// CHANGE THIS TO YOUR OWN UNIQUE VALUE
static uint8_t mac[6] = { 0xA9, 0x10, 0x14, 0xE7, 0xAC, 0xCC };  

// CHANGE THIS TO MATCH YOUR HOST NETWORK
static uint8_t ip[4] = { 192, 168, 2, 15 };

/* all URLs on this server will start with /pins because of how we
 * define the PREFIX value.  We also will listen on port 80, the
 * standard HTTP service port */
#define PREFIX "/pins"
WebServer webserver(PREFIX, 80);

#define RELAY_ON  0
#define RELAY_OFF 1

#define PIN_1 40
#define PIN_2 42
#define PIN_3 43
#define PIN_4 44

int pin_1_value = RELAY_OFF;            
int pin_2_value = RELAY_OFF;           
int pin_3_value = RELAY_OFF;  
int pin_4_value = RELAY_OFF;  

/* store the HTML in program memory using the P macro */
P(On) = "<status>ON</status>";
P(Off) = "<status>OFF</status>";

void httpNotFound(WebServer &server){
  P(failMsg) =
    "HTTP/1.0 404 Bad Request" CRLF
    WEBDUINO_SERVER_HEADER
    "Content-Type: text/html" CRLF
    CRLF
    "<h2>BAD REQUEST!</h2>";
  server.printP(failMsg);
}

/* This command is set as the default command for the server.  It
 * handles both GET and PUT requests.  For a GET, it returns a pin
 * value.  For a PUT it set the pin value. */
void pinsCmd(WebServer &server, WebServer::ConnectionType type, char **url_path, char *url_tail, bool tail_complete)
{
     if (type == WebServer::PUT)
      {
        int value = RELAY_OFF;
        int pin_number;
        
        /* for url /pins/40/ON url_path[1] will contain 'ON' and  url_path[0] will contain '40' */
        if(strcmp(url_path[1], "ON") == 0){
         value = RELAY_ON;
        }      
        
        pin_number = atol(url_path[0]);
        
        switch (pin_number) {
          case PIN_1:
            pin_1_value = value;
            break;
          case PIN_2:
            pin_2_value = value;
            break;
          case PIN_3:
            pin_3_value = value;
            break; 
          case PIN_4:
            pin_4_value = value;
            break;  
         }
       
       }

  server.httpSuccess("text/plain");

  if (type == WebServer::GET)
  { 
    boolean is_on = false;
    int pin_number;
    
    pin_number = atol(url_path[0]);
              
     switch (pin_number) {
          case PIN_1:
            if(pin_1_value == RELAY_ON)
            {is_on = true;}
            break;
          case PIN_2:
            if(pin_2_value == RELAY_ON)
            {is_on = true;}
            break;
           case PIN_3:
            if(pin_3_value == RELAY_ON)
            {is_on = true;}
            break;  
           case PIN_4:
            if(pin_4_value == RELAY_ON)
            {is_on = true;}
            break; 
         }
          
          
    if(is_on)     
   { 
     server.printP( On );
   } else
   {
     server.printP( Off );
   }
   
  }
}

void setup()
{ 
  digitalWrite(PIN_1, RELAY_OFF);
  digitalWrite(PIN_2, RELAY_OFF);
  digitalWrite(PIN_3, RELAY_OFF);
  digitalWrite(PIN_4, RELAY_OFF);

  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  pinMode(PIN_3, OUTPUT);
  pinMode(PIN_4, OUTPUT);

  //Serial.begin(9600);

  // setup the Ehternet library
  Ethernet.begin(mac, ip);

  webserver.setUrlPathCommand(&pinsCmd);

  /* start the server to wait for connections */
  webserver.begin();
}

void loop()
{
  // process incoming connections one at a time forever
  webserver.processConnection();
//  Serial.print(red);
//  Serial.print(" ");
//  Serial.print(green);
//  Serial.print(" ");
//  Serial.println(blue);

  digitalWrite(PIN_1, pin_1_value);
  digitalWrite(PIN_2, pin_2_value);
  digitalWrite(PIN_3, pin_3_value);
  digitalWrite(PIN_4, pin_4_value);
 
}

