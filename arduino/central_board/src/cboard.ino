
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#include "advancedSerial.h"

#define SIZE(array) (sizeof(array) / sizeof(*array))

#define MQTT_PINS_TOPIC "home/control/cboard/pins/"
#define MQTT_INFO_TOPIC "home/info/cboard/pins/"

#define MQTT_PINS_FORMAT "XX"

const char *mqtt_control_prefix = MQTT_PINS_TOPIC;
const char *mqtt_info_prefix    = MQTT_INFO_TOPIC;
const char *mqtt_pins_format    = MQTT_PINS_FORMAT;

bool published = false;

struct MqttPin {
  int8_t pin_number;
  bool    pin_state;
};

MqttPin pin_40 = { 40, true };
MqttPin pin_42 = { 42, true };
MqttPin pin_43 = { 43, true };
MqttPin pin_44 = { 44, true };
MqttPin pin_45 = { 45, true };
MqttPin pin_46 = { 46, true };
MqttPin pin_47 = { 47, false }; // led strip

MqttPin mqtt_pins[] = { pin_40, pin_42, pin_43, pin_44, pin_45, pin_46, pin_47 };


/* #################  Ethernet                                                */

uint8_t mac[] = { 0xA9, 0x10, 0x14, 0xE7, 0xAC, 0xCC };
IPAddress     ip(192, 168, 2, 15);
IPAddress server(192, 168, 2,  3);

EthernetClient ethClient;

/* #################  MQTT                                                    */

PubSubClient client(ethClient);
long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("centralBoard")) {
    // Once connected, publish an announcement...
    // client.publish("outTopic", "hello world");

    // ... and resubscribe
    client.subscribe( MQTT_PINS_TOPIC "+" ); // sic! '+' is a wildcard
  }

  return client.connected();
}


void publishPinState(int16_t pin_index) {
  aSerial.l(Level::v).pln("publishPinState");
  MqttPin mqtt_pin = mqtt_pins[pin_index];

  char char_buffer[ strlen(MQTT_INFO_TOPIC) + strlen(MQTT_PINS_FORMAT) ] = MQTT_INFO_TOPIC;

  char pin_as_str[3] = "";
  itoa(mqtt_pin.pin_number, pin_as_str, 10);
  aSerial.l(Level::vvvv).p("pin_as_str = ").pln(pin_as_str);

  strcat(char_buffer, pin_as_str);
  aSerial.l(Level::vvvv).p("char_buffer = ").pln(char_buffer);

  char payload[] = "0";
  if( mqtt_pin.pin_state )
    payload[0] = '1';

  boolean retained = true;
  client.publish(char_buffer, payload, retained);
  delay(20);
}

void publishPinStates() {
  for (int i=0; i<SIZE(mqtt_pins); i++) {
    publishPinState( i );
  }
}

bool equalsToPinsTopic(char* recieved_topic) {
  return strncmp( mqtt_control_prefix, recieved_topic, strlen(mqtt_control_prefix) ) == 0;
}

void mqttLoop() {
  if (!client.connected()) {
    uint32_t now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;

        if( !published ) {
          publishPinStates();
          published = true;
        }
      }
    }
  } else {
    // Client connected
    client.loop();
  }
}

int16_t get_pin_number(char first_digit, char second_digit) {
  char str[3] = "XX";
  str[0] = first_digit;
  str[1] = second_digit;

  return atoi(str);
}

int16_t find_pin_index(int16_t pin) {
   for (int i=0; i<SIZE(mqtt_pins); i++)
     if ( pin == mqtt_pins[i].pin_number )
        return(i);

   return(-1);
}

void changePinState(int16_t pin_number, bool new_state) {
  aSerial.l(Level::vvv).p("changePinState, pin = ").p(pin_number).p(" state = ").pln(new_state);

  int16_t pin_index = find_pin_index(pin_number);

  if(pin_index < 0)
    return;

  aSerial.l(Level::vvv).p("pin_state = ").p(mqtt_pins[pin_index].pin_state).p(" pin_index = ").pln(pin_index);

  if( mqtt_pins[pin_index].pin_state != new_state ) {
    mqtt_pins[pin_index].pin_state = new_state;
    digitalWrite(mqtt_pins[pin_index].pin_number, new_state);
    publishPinState(pin_index);
  }
}

void callback(char* recieved_topic, byte* payload, unsigned int length) {
  aSerial.l(Level::v).p("Message arrived : ").pln(recieved_topic);

  uint16_t recieved_topic_len = strlen(recieved_topic);
  uint16_t pins_topic_len     = strlen(mqtt_control_prefix);

  uint16_t len_diff = recieved_topic_len - pins_topic_len;

  if( len_diff !=  strlen(mqtt_pins_format) ) {
    aSerial.l(Level::v).pln("ERROR: incorrect mqtt topic lenght");
    return;
  }

  if( !equalsToPinsTopic(recieved_topic) ) {
    aSerial.l(Level::v).pln("ERROR: unknown mqtt topic");
    return;
  }

  char first_digit  = recieved_topic[recieved_topic_len - 2];
  char second_digit = recieved_topic[recieved_topic_len - 1];

  if( !isdigit(first_digit) || !isdigit(second_digit) ) {
    aSerial.l(Level::v).pln("ERROR: mailformed mqtt topic tail - NAN");
    return;
  }

  if( payload[0] != '0' && payload[0] != '1') {
    aSerial.l(Level::v).pln("ERROR: mailformed payload");
    return;
  }

  bool state = ( payload[0] == '1' );
  int16_t p_number = get_pin_number(first_digit, second_digit);
  changePinState(p_number, state);

}

void setupInputPins() {
  for (int i=0; i<SIZE(mqtt_pins); i++) {
    digitalWrite(mqtt_pins[i].pin_number, mqtt_pins[i].pin_state);
    pinMode(mqtt_pins[i].pin_number, OUTPUT);
  }
}

/* #################  SETUP                                                   */

void setup(void) {
  Serial.begin(9600);
  aSerial.setPrinter(Serial);
  aSerial.setFilter(Level::vvv);

  aSerial.l(Level::v).pln("Setup ...");

  setupInputPins();

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  delay(3000);
}


/* #################  LOOP                                                    */

void loop() {
  mqttLoop();
}
