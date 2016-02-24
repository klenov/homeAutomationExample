#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char wifi_ssid[]      = "cherdak";
const char wifi_password[]  = "rodchenko";

const char openhab_ip[]  = "192.168.2.2";


HTTPClient http;

void setup() {

    Serial.begin(9600);

    WiFi.begin(wifi_ssid, wifi_password);

    http.setReuse(true);
    // pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {
    // wait for WiFi connection
    while ( WiFi.status() != WL_CONNECTED ) {
        Serial.print(".");
        delay(500);
    }

    http.begin(openhab_ip, 8080, "/CMD?Light_4=TOGGLE");

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

    delay(5000);
}

