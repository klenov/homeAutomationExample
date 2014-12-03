/* Arduino Relays Unit Web Server
   This script is a part of my home automation setup 
   Based on Webduino library https://github.com/sirleech/Webduino
   Vasily Klenov, 2014

   I use this script with Arduino Mega 2560 connected with Ethernet Shield

  https://github.com/sirleech/Webduino/blob/master/examples/Web_Parms/Web_Parms.ino
    */

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"

#define DEBUG_RELAYS_UNIT 0

#define VERSION_STRING "0.0.2"

#define CRLF "\r\n"

#define SIZE(array) (sizeof(array) / sizeof(*array))

// CHANGE THIS TO YOUR OWN UNIQUE VALUE
static uint8_t mac[6] = { 0xA9, 0x10, 0x14, 0xE7, 0xAC, 0xCC };  

// CHANGE THIS TO MATCH YOUR HOST NETWORK
static uint8_t ip[4] = { 192, 168, 2, 15 };

#define PREFIX ""
WebServer webserver(PREFIX, 80);

#define RELAY_ON  0
#define RELAY_OFF 1

//const int relays_count = 7;
const int relays[]        = { 40, 42, 43, 44, 45, 46, 47 };
int relays_states[] = { RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF };

char* pin_number_as_char = "XX";

/* store the HTML in program memory using the P macro */
P(On)      = "<status>ON</status>";
P(Off)     = "<status>OFF</status>";
P(Hello) = 
    "<html>"
    "<head>"
    "<title>Arduino Web Server</title>"
    "</head>"
    "<body>"
    "<h1>Home Automation Web Server</h1>"
    "based on <a href=\"https://github.com/sirleech/Webduino\">Webduino library</a><br/>"
    "Vasily Klenov, 2014<br/>"
    "<b>version</b>"
    VERSION_STRING
    "</body></html>";

void httpNotFound(WebServer &server){
  P(failMsg) =
    "HTTP/1.0 404 Bad Request" CRLF
    "Content-Type: text/html" CRLF
    CRLF
    "<h2>File Not Found</h2>";
  server.printP(failMsg);
}  

void helloCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  server.httpSuccess();

  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type != WebServer::HEAD)
  {
    server.printP(Hello);
  }
}

/* It handles both GET and PUT requests.  For a GET, it returns a pin
 * value.  For a PUT it set the pin value. */
void pinsCmd(WebServer &server, WebServer::ConnectionType type, char **url_path, char *url_tail, bool tail_complete)
{

  if(strncmp(url_path[0],"pins",5)!=0){
    httpNotFound(server);
    return;
  }

  char* pin_param = url_path[1];
  int relay_index = find_index(pin_param);

  if (type == WebServer::PUT)
  {
    #ifdef DEBUG_RELAYS_UNIT
      Serial.println("Processing PUT query...");
    #endif

    int new_state = RELAY_OFF;

    if ( relay_index >= 0 )
    {
      server.httpSuccess();
      /* for url /pins/40/ON url_path[2] will contain 'ON' 
         and  url_path[1] will contain '40' */
      if(strcmp(url_path[2], "ON") == 0){ // поменять на strncmp ?????
        new_state = RELAY_ON;
      }

      if ( relays_states[relay_index] != new_state )
      {
        digitalWrite(relays[relay_index], new_state);
        relays_states[relay_index] = new_state;
      }

      return;
    } else
    {
      httpNotFound(server);
      return;
    }
  }

  if (type == WebServer::GET)
  { 
    #ifdef DEBUG_SWITCHER
      Serial.println("Processing GET query...");
    #endif
    int relay_index = find_index(pin_param);
    
    if ( relay_index >= 0 )
    {
      server.httpSuccess();
      if( relays_states[relay_index] == RELAY_OFF)
        server.printP( Off );
      else
        server.printP( On );
      return;
    } else
    {
      httpNotFound(server);
      return;
    }
  }

  server.httpFail();
}

void setup()
{ 
  for (int i=0; i<SIZE(relays); i++) {
    digitalWrite(relays[i], RELAY_OFF);
    pinMode(relays[i], OUTPUT);
  }

  #ifdef DEBUG_RELAYS_UNIT
    Serial.begin(9600);
    Serial.println("connecting to network");
  #endif

  // setup the Ehternet library
  Ethernet.begin(mac, ip);
  
  webserver.setDefaultCommand(&helloCmd);
  webserver.setUrlPathCommand(&pinsCmd);

  /* start the server to wait for connections */
  webserver.begin();
}

void loop()
{
  // process incoming connections one at a time forever
  webserver.processConnection();
}

int find_index(char *value)
{
   for (int i=0; i<SIZE(relays); i++)
   {
     pin_number_as_char = itoa(relays[i], pin_number_as_char, 10); // itoa reference http://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html#gaa571de9e773dde59b0550a5ca4bd2f00
     if (strcmp(pin_number_as_char, value) == 0)
     {
        return(i); 
     }
   }
   return(-1);
}

