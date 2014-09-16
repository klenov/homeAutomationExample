#include <UIPEthernet.h> // https://github.com/ntruchsess/arduino_uip
#include <RestClient.h> //  https://github.com/csquared/arduino-restclient

// можно попробовать переписать на получение стейта
// типа по адресу /rest/items/Light_1/state

#define DEBUG_SWITCHER
#define OPENHAB_HOST "192.168.2.2"
#define OPENHAB_PORT 8080

const char* LIGHTS_URL = "/rest/items/Light_";
const char* CONTENT_TYPE = "Content-Type:text/plain";
const char* ACCEPT       = "Accept:application/json";

const int main_led_pin   = 16;
const int buttons_count = 3; 
// The Ethernet Controller (ENC28J60) uses the SPI pins (10, 11, 12, 13)
const int buttons [buttons_count] = { 2, 3, 4 };
const int leds_1 [buttons_count]  = { 5, 7, 14 };
const int leds_2 [buttons_count]  = { 6, 8, 15 };
boolean buttons_state [buttons_count] = { 0, 0, 0 };
boolean button_was_pressed [buttons_count] = { 0, 0, 0 };
unsigned long last_millis = 0;

unsigned long last_pressed_at = 0;
unsigned long minimal_delay   = 2000;

RestClient client = RestClient(OPENHAB_HOST, OPENHAB_PORT);

void setup()
{ 
  pinMode(main_led_pin, OUTPUT);
  digitalWrite( main_led_pin, HIGH );
  delay(2000);
  digitalWrite( main_led_pin, LOW );

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
    Serial.println(".");
  #endif
  
  // get buttons states from OpenHAB API
  if ( more_than_4000_millis_passed() && api_available() )
  {
    for (int i=0; i<buttons_count; i++) {
      int api_button_state = read_button_state_from_api( i+1 );
      toggle_button_led( i+1, api_button_state);
      buttons_state[i] = api_button_state;
    }
  }
  
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

        send_request_to_api( i+1, buttons_state[i] );
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

void set_headers()
{
  client.setHeader(CONTENT_TYPE);
  client.setHeader(ACCEPT);
}

String json;
int read_button_state_from_api(int button_number)
{
  json = "";
  
  set_headers();
  int status_code = client.get( lights_url_by_button_number(button_number), &json );

  
  #ifdef DEBUG_SWITCHER
    Serial.print("Status code from server: ");
    Serial.println(status_code);
    Serial.print("Response body from server: ");
    Serial.println(json);
  #endif
  
  if( status_code == 200 )
  {     
    return parse_api_response_to_get_state();
  }
  return -1;
}

int parse_api_response_to_get_state()
{
  int index_of_state = json.indexOf("\"state\":");
  if (index_of_state > 0)
  {
    String state;
    state = json.substring(index_of_state + 9, index_of_state + 11);
  
    #ifdef DEBUG_SWITCHER
      Serial.print("STATE: ");
      Serial.println(state);
    #endif
    
    if(state == "ON")
    {
      return 1;
    }
    if(state == "OF")
    {
      return 0;
    }
  }
  return -1;
}

int send_request_to_api(int b_number, int current_state)
{
  char*  query_url = lights_url_by_button_number(b_number);
  set_headers();
  int status_code = client.post( query_url, param_by_state(current_state) );
  
  #ifdef DEBUG_SWITCHER
    Serial.println("POST query:");
    Serial.print(query_url);
    Serial.print("Status code from server: ");
    Serial.println(status_code);
  #endif
  
  return status_code;
}

char* param_by_state(int state)
{
  if (state)
    return "OFF";
  else
    return "ON";
}

void setup_buttons()
{
  for (int i=0; i<buttons_count; i++) {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH); //internal pull-ups
    pinMode(leds_1[i], OUTPUT);
    pinMode(leds_2[i], OUTPUT);
    
    //all off
    digitalWrite(leds_1[i], LOW);
    digitalWrite(leds_2[i], HIGH);
  }
}

char url[40];
char* lights_url_by_button_number(int button_number)
{
  char pin_as_char[2];

  pin_as_char[0] = (char)(((int)'0') + button_number);
  pin_as_char[1] = '\0';

  strcpy (url, LIGHTS_URL);
  strcat (url, pin_as_char);

  return url;
}

void toggle_button_led(int button_number, int state)
{
  if (state == 0 || state == 1)
  {
    digitalWrite(leds_1[button_number-1], state == 1);
    digitalWrite(leds_2[button_number-1], state != 1);
  }

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


// void turn_on_main_led(unsigned char delayms){
//   analogWrite(speaker_pin, 128);
// }


