/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://www.arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */


// the setup function runs once when you press reset or power the board

int led_strip_state = 0;
int light_state = 1;
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(42, OUTPUT);
  digitalWrite(42, light_state);

  pinMode(47, OUTPUT);
  digitalWrite(47, led_strip_state);
  Serial.begin(9600);
}


int inByte = 0;         // incoming serial byte

void loop() {
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    // read first analog input, divide by 4 to make the range 0-255:
    if(inByte == 'l') {
      light_state = ! light_state;
      digitalWrite(42, light_state);
    }

    if(inByte == 'o') {
      led_strip_state = ! led_strip_state;
      digitalWrite(47, led_strip_state);
    }

  }
}
