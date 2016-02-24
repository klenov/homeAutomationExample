#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SIZE(array) (sizeof(array) / sizeof(*array))

typedef void (*VoidFunction) (void);

const char wifi_ssid[]      = "cherdak";
const char wifi_password[]  = "rodchenko";

const char openhab_ip[]  = "192.168.2.2";

HTTPClient http;

/* BUTTONS */
const int buttons []                   = { D5, D1, D2, D3, D4 };
volatile boolean button_was_pressed [] = {  0,  0,  0,  0,  0 }; // на D0 почему-то не заработало

volatile unsigned long last_pressed_at = 0;
const unsigned long minimal_delay   = 2000;

inline void set_pressed_flag(int index, bool flag) {
  if ( millis() - last_pressed_at > minimal_delay ) {
    last_pressed_at = millis();
    button_was_pressed[index] = flag;
    Serial.print("set_pressed_flag");
    Serial.println(index);
  }
}

void int0() {
  set_pressed_flag(0, true);
}

void int1() {
  set_pressed_flag(1, true);
}

void int2() {
  set_pressed_flag(2, true);
}

void int3() {
  set_pressed_flag(3, true);
}

void int4() {
  set_pressed_flag(4, true);
}

VoidFunction interrupt_f[] = { int0, int1, int2, int3, int4 };

void pin_as_input(int pin) {
  pinMode(pin, INPUT_PULLUP);
}

void setup_buttons() {
  for (int i=0; i<SIZE(buttons); i++) {
    pin_as_input(buttons[i]);
    attachInterrupt( buttons[i], interrupt_f[i], FALLING );
  }
}

char* lights_url_by_button_number(int button_number) {  
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

int send_request_to_api(int b_number) {
  
  http.begin(openhab_ip, 8080, lights_url_by_button_number(b_number));

  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Accept", "application/json");

  int httpCode = http.GET();
  if(httpCode > 0) {
    Serial.print("[HTTP] GET... code: ");
    Serial.println(httpCode);
  } else {
    Serial.print("[HTTP] GET... failed, error: ");
    Serial.println( http.errorToString(httpCode).c_str() );
  }

  http.end();
}

void setup() {

  Serial.begin(115200);
  Serial.println("Setup..");

  WiFi.begin(wifi_ssid, wifi_password);

  http.setReuse(true);

  setup_buttons();
  // pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {
    // wait for WiFi connection
    while ( WiFi.status() != WL_CONNECTED ) {
      Serial.print(".");
      delay(500);
    }

    for (int i=0; i<SIZE(buttons); i++) {
      //Serial.println(i);

      // Serial.println("timeout:");
      if( button_was_pressed[i] ) {
        noInterrupts();
        button_was_pressed[i] = false;

        Serial.println("Button pressed!");

        // control_led(HIGH);

        send_request_to_api( i );
        interrupts();
      }
    }

  // delay(500);
  // Serial.print("loop");
}

