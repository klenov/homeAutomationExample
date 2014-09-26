/* Home Automation Script
   based on Webduino library
   Vasily Klenov, 2014 */

#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"

#define DEBUG_RELAYS_UNIT 0

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

const int relays_count = 7;
const int relays [buttons_count]        = { 40, 42, 43, 44, 45, 46, 47 };
const int relays_states [buttons_count] = { RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF };

/* store the HTML in program memory using the P macro */
P(On) = "<status>ON</status>";
P(Off) = "<status>OFF</status>";

/* This command is set as the default command for the server.  It
 * handles both GET and PUT requests.  For a GET, it returns a pin
 * value.  For a PUT it set the pin value. */
void pinsCmd(WebServer &server, WebServer::ConnectionType type, char **url_path, char *url_tail, bool tail_complete)
{
  if (type == WebServer::PUT)
  {
    #ifdef DEBUG_SWITCHER
      Serial.println("Processing PUT query...");
    #endif

    int new_state = RELAY_OFF;
    int pin_number = atol(url_path[0]);
    int relay_index = find_index(relays, relays_count, pin_number);

    if ( relay_index >= 0 )
    {
      /* for url /pins/40/ON url_path[1] will contain 'ON' 
         and  url_path[0] will contain '40' */
      if(strcmp(url_path[1], "ON") == 0){
        new_state = RELAY_ON;
      }

      if ( relays_states[relay_index] != new_state )
      {
        digitalWrite(pin_number, new_state);
        relays_states[relay_index] = new_state;
      }

      server.httpSuccess();
      return;
    }
  }

  if (type == WebServer::GET)
  { 
    #ifdef DEBUG_SWITCHER
      Serial.println("Processing GET query...");
    #endif
    int pin_number = atol(url_path[0]);
    int relay_index = find_index(relays, relays_count, pin_number);
    
    if ( relay_index >= 0 )
    {
      if( relays_states[relay_index] == RELAY_OFF)
        server.printP( Off );
      else
        server.printP( On );

      server.httpSuccess();
      return;
    }
  }
  server.httpFail();
}

void setup()
{ 
  for (int i=0; i<relays_count; i++) {
    digitalWrite(relays[i], RELAY_OFF);
    pinMode(relays[i], OUTPUT);
  }

  #ifdef DEBUG_SWITCHER
    Serial.begin(9600);
    Serial.println("connect to network");
  #endif

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
}

int find_index(int array[], int num_elements, int value)
{
   for (int i=0; i<num_elements; i++)
   {
     if (array[i] == value)
     {
        return(i); 
     }
   }
   return(-1);
}

