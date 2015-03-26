/* Web Switch Unit
   This script is a part of my home automation setup 
   Vasily Klenov, 2014

   If you are like me using ethernet shield based on ENC28J60
   then you should replace '#include <Ethernet.h>' with '#include <UIPEthernet.h>'
   inside RestClient.h (and all other libraries if you added any).
   Otherwise if you are using more common ethernet shield
   you should replace '#include <UIPEthernet.h>' with '#include <Ethernet.h>' in this file.

   For debugging via serial-monitor set DEBUG_SWITCHER to non-zero value.
   */

#include <UIPEthernet.h> // https://github.com/ntruchsess/arduino_uip
#include <RestClient.h>  // https://github.com/csquared/arduino-restclient


#define DEBUG_SWITCHER 0
#define OPENHAB_HOST "192.168.2.2"
#define OPENHAB_PORT 8080

#define SIZE(array) (sizeof(array) / sizeof(*array))

// change this to your own unique value
const byte mac[] = { 0x9C, 0x92, 0x27, 0x2D, 0xDA, 0x93 };

const char* CONTENT_TYPE = "Content-Type:text/plain";
const char* ACCEPT       = "Accept:application/json";

const int status_led_pin  = 16;

const int buttons []          = { 3, 4, 5, 6, 7 }; // The Ethernet Controller (ENC28J60) uses the SPI pins 10, 11, 12, 13 on Arduino Nano
boolean button_was_pressed [] = { 0, 0, 0, 0, 0 };

unsigned long last_pressed_at = 0;
const unsigned long minimal_delay   = 1000;

RestClient client = RestClient(OPENHAB_HOST, OPENHAB_PORT);

void setup()
{ 
  pinMode(status_led_pin, OUTPUT);

  #if DEBUG_SWITCHER
    Serial.begin(9600);
    Serial.println("Buttons setup ...");
  #endif

  setup_buttons();
  sturtup_blink();

  #if DEBUG_SWITCHER
    Serial.println("Connecting to network ...");
  #endif

  if (client.begin( const_cast<byte*>(mac) ) == 0 ) {
    #if DEBUG_SWITCHER
      Serial.println("Failed to configure Ethernet using DHCP");
    #endif
  } else
  {
    #if DEBUG_SWITCHER
      Serial.println("OK!");
      Serial.println(Ethernet.localIP());
    #endif
  }
}

void loop()
{
  #if DEBUG_SWITCHER
    //Serial.println(".");
  #endif
  
  delay(100);
  
  // send request to API if button pressed
  for (int i=0; i<SIZE(buttons); i++) {
    if ( millis() - last_pressed_at > minimal_delay )
    {
      if(!button_was_pressed[i] && digitalRead(buttons[i]) == LOW) {
        #if DEBUG_SWITCHER
          Serial.println("Button pressed!");
        #endif
        control_led(HIGH);
        button_was_pressed[i] = true;

        send_request_to_api( i );
        last_pressed_at = millis();
      }

      if(button_was_pressed[i] && digitalRead(buttons[i]) == HIGH)
      {
        button_was_pressed[i] = false;
        control_led(LOW);
      }
    } 
  }
}

void setup_buttons()
{
  for (int i=0; i<SIZE(buttons); i++) {
    pin_as_input(buttons[i]);
  }
}

void pin_as_input(int pin)
{
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH); //internal pull-ups
}

void set_headers()
{
  client.setHeader(CONTENT_TYPE);
  client.setHeader(ACCEPT);
}

int send_request_to_api(int b_number)
{
  set_headers();
  int status_code;

  status_code = client.get( lights_url_by_button_number(b_number) );

  #if DEBUG_SWITCHER
    Serial.print("GET query:");
    Serial.println(lights_url_by_button_number(b_number));
    Serial.print("Status code from server: ");
    Serial.println(status_code);
  #endif
  
  return status_code;
}

char* lights_url_by_button_number(int button_number)
{  
  switch (button_number) {
    case 0:
      return "/CMD?Virtual_Switch_1=TOGGLE";
    case 1:
      return "/CMD?Virtual_Switch_2=TOGGLE";
    case 2:
      return "/CMD?Virtual_Switch_3=TOGGLE";
    case 3:
      return "/CMD?Virtual_Switch_4=TOGGLE";
    case 4:
      return "/CMD?Virtual_Switch_5=TOGGLE";
    default:
      return "";
  }
}

void sturtup_blink()
{
  for (int i=0; i<5; i++) {
    control_led(HIGH);
    delay(250);
    control_led(LOW);
    delay(250);
  }
}

void control_led(int state)
{
    #if DEBUG_SWITCHER
      if(state == LOW)
        Serial.println("led is off");
      else
        Serial.println("led is on");
    #endif
    digitalWrite( status_led_pin, state );
}

