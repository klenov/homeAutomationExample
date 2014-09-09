#include <UIPEthernet.h> // https://github.com/ntruchsess/arduino_uip
#include <RestClient.h> //  https://github.com/csquared/arduino-restclient

#define DEBUG_SWITCHER
#define OPENHAB_HOST "192.168.2.2"
#define OPENHAB_PORT 8080

const char* LIGHTS_URL = "/rest/items/Light_";
const char* CONTENT_TYPE = "Content-Type:text/plain";
const char* ACCEPT       = "Accept:application/json";

const int buttons_count = 3; 
// The Ethernet Controller (ENC28J60) uses the SPI pins (10, 11, 12, 13)
const int buttons [buttons_count] = { 2, 3, 4 };
const int button_shift = -1; // to get 'Light_1' for pin 2
const int leds_1 [buttons_count]  = { 5, 7, 14 };
const int leds_2 [buttons_count]  = { 6, 8, 15 };
boolean buttons_state [buttons_count] = { 0, 0, 0 };

RestClient client = RestClient(OPENHAB_HOST, OPENHAB_PORT);

void setup()
{ 
  #ifdef DEBUG_SWITCHER
    Serial.begin(9600);
    Serial.println("connect to network");
  #endif
 
 setup_buttons();
 client.dhcp();
}

void loop()
{
  delay(3000);
  Serial.println("OK");

  // update all buttons from api
  // for (int i=0; i<buttons_count; i++) {
   
  // }
  int button_number = 3;
  int button_state = read_button_state_from_api(button_number);
  toggle_button_led(button_number, button_state);
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
    #ifdef DEBUG_SWITCHER
      Serial.println(json);
    #endif
    
    return parse_api_response_to_get_state();
  }
  return 0;
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
  }
  return 0;
}

int send_request_to_api(char const *item_name, char const *data_params)
{
  char*  query_url = "/rest/items";
  strcat( query_url, item_name);
  set_headers();
  int status_code = client.post(query_url, data_params);
  
  #ifdef DEBUG_SWITCHER
    Serial.println("POST query:");
    Serial.print(query_url);
    Serial.print("Status code from server: ");
    Serial.println(status_code);
  #endif
  
  return status_code;
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

char url[20];
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
  digitalWrite(leds_1[button_number-1], state == 1);
  digitalWrite(leds_2[button_number-1], state != 1);
}



