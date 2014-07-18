/* Home Automation Script
   based on Webduino library
   Vasily Klenov, 2014 */

   /* надо переписать пины на массивы
      и digitalWrite делать только при изменении значения
    */

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
#define PIN_5 45
#define PIN_6 46
#define PIN_7 47

#define PWM_PIN_1 8

int pin_1_value = RELAY_OFF;
int pin_2_value = RELAY_OFF;
int pin_3_value = RELAY_OFF;
int pin_4_value = RELAY_OFF;
int pin_5_value = RELAY_OFF;
int pin_6_value = RELAY_OFF;
int pin_7_value = RELAY_OFF;

int pwm_pin_1_value = LOW; 

/* store the HTML in program memory using the P macro */
P(On) = "<status>ON</status>";
P(Off) = "<status>OFF</status>";

// переписать тоже на макрос?
void printPwmStatus(pwm_value){
  P(pwmMsg) = "<status></status>"; // дописать
  server.printP(pwmMsg);
}

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

        // тут надо преобразовать строку к числу, это если в запросе передается значение для пвм пина
        pwm_value = atol(url_path[1]);

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
          case PIN_5:
            pin_5_value = value;
            break;
          case PIN_6:
            pin_6_value = value;
            break;
          case PIN_7:
            pin_7_value = value;
            break;
          case PWM_PIN_1:
            pwm_pin_1_value = pwm_value;
            break;
         }
       
       }

  server.httpSuccess("text/plain");

  if (type == WebServer::GET)
  { 
    boolean is_on = false;
    int pin_number;
    int pwm_value = -1;
    
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
      case PIN_5:
        if(pin_5_value == RELAY_ON)
        {is_on = true;}
        break;
      case PIN_6:
        if(pin_6_value == RELAY_ON)
        {is_on = true;}
        break;
      case PIN_7:
        if(pin_7_value == RELAY_ON)
        {is_on = true;}
        break;
      case PWM_PIN_1:
        pwm_value = pwm_pin_1_value;
        break;
    }
    
    if (pwm_value >= 0)
    {
      printPwmStatus( pwm_value );
    } else
    {
      if(is_on)
      { 
        server.printP( On );
      } else 
      {
        server.printP( Off );
      }
    }
   
  }
}

void setup()
{ 
  digitalWrite(PIN_1, RELAY_OFF);
  digitalWrite(PIN_2, RELAY_OFF);
  digitalWrite(PIN_3, RELAY_OFF);
  digitalWrite(PIN_4, RELAY_OFF);
  digitalWrite(PIN_5, RELAY_OFF);
  digitalWrite(PIN_6, RELAY_OFF);
  digitalWrite(PIN_7, RELAY_OFF);

  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  pinMode(PIN_3, OUTPUT);
  pinMode(PIN_4, OUTPUT);
  pinMode(PIN_5, OUTPUT);
  pinMode(PIN_6, OUTPUT);
  pinMode(PIN_7, OUTPUT);

  digitalWrite(PWM_PIN_1, LOW);
  pinMode(PWM_PIN_1, OUTPUT);

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
  digitalWrite(PIN_5, pin_5_value);
  digitalWrite(PIN_6, pin_6_value);
  digitalWrite(PIN_7, pin_7_value);

  analogWrite(PWM_PIN_1, pwm_pin_1_value);
}

