#include <UIPEthernet.h> // https://github.com/ntruchsess/arduino_uip
#include <RestClient.h> //  https://github.com/csquared/arduino-restclient

// можно попробовать переписать на получение стейта
// типа по адресу /rest/items/Light_1/state

#define DEBUG_SWITCHER
#define OPENHAB_HOST "192.168.2.2"
#define OPENHAB_PORT 8080

const char* CONTENT_TYPE = "Content-Type:text/plain";
const char* ACCEPT       = "Accept:application/json";

const int main_led_pin   = 16;
const int buttons_count = 3; 
// The Ethernet Controller (ENC28J60) uses the SPI pins (10, 11, 12, 13)
const int buttons [buttons_count] = { 5, 3, 4 };

char* lights_url  = "/CMD?Light_X=TOGGLE";
char* pin_as_char = "X";
boolean button_was_pressed [buttons_count] = { 0, 0, 0 };
unsigned long last_millis = 0;

unsigned long last_pressed_at = 0;
unsigned long minimal_delay   = 2000;

RestClient client = RestClient(OPENHAB_HOST, OPENHAB_PORT);

void setup()
{ 
  pinMode(main_led_pin, OUTPUT);

  #ifdef DEBUG_SWITCHER
    Serial.begin(9600);
    Serial.println("connect to network");
  #endif
 
 setup_buttons();
 sturtup_blink();
 byte mac[] = { 0x9C, 0x92, 0x27, 0x2D, 0xDA, 0x93 };
 if (client.begin(mac) == 0) {
    #ifdef DEBUG_SWITCHER
      Serial.println("Failed to configure Ethernet using DHCP");
    #endif
  }
}

void loop()
{
  #ifdef DEBUG_SWITCHER
    //Serial.println(".");
  #endif
  
  // send request to API if button pressed
  for (int i=0; i<buttons_count; i++) {
    if ( millis() - last_pressed_at > minimal_delay )
    {
      if(!button_was_pressed[i] && digitalRead(buttons[i]) == LOW) {
        #ifdef DEBUG_SWITCHER
          Serial.println("Button pressed!");
        #endif
        digitalWrite( main_led_pin, HIGH );
        button_was_pressed[i] = true;

        send_request_to_api( i+1 );
        last_pressed_at = millis();
      }

      if(button_was_pressed[i] && digitalRead(buttons[i]) == HIGH)
      {
        button_was_pressed[i] = false;
        digitalWrite( main_led_pin, LOW );
      }
    } 
  }
  
}

void setup_buttons()
{
  for (int i=0; i<buttons_count; i++) {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH); //internal pull-ups
  }
}

void set_headers()
{
  client.setHeader(CONTENT_TYPE);
  client.setHeader(ACCEPT);
}

int send_request_to_api(int b_number)
{
  lights_url[11] = lights_url_by_button_number(b_number);
  set_headers();
  int status_code = client.get( lights_url );

  #ifdef DEBUG_SWITCHER
    Serial.println("GET query:");
    Serial.print(lights_url);
    Serial.print("Status code from server: ");
    Serial.println(status_code);
  #endif
  
  return status_code;
}

char lights_url_by_button_number(int button_number)
{  
  return (char)(((int)'0') + button_number);
}

int api_available()
{
  set_headers();
  int status_code = client.get( "/rest" );
  if ( status_code == 200 )
  { return 1; }
  else
  { return 0; }  
}

int more_than_4000_millis_passed()
{
  unsigned long current_millis = millis();
  if ( current_millis >= last_millis  )
  {
    if ( current_millis - last_millis > 4000 )
    {
      last_millis = current_millis;
      return 1;
    }
  }
  else
  {
    last_millis = current_millis;
  }
  return 0;
}

void sturtup_blink()
{
  for (int i=0; i<30; i++) {
    digitalWrite( main_led_pin, HIGH );
    delay(250);
    digitalWrite( main_led_pin, LOW );
    delay(250);
  }
}


