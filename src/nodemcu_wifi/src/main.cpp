#include <Arduino.h>
#include <Wire.h>
#include <time.h>

#define SDA D1
#define SCL D2
#define i2c_address 0x08

void receiveEvent(int howMany);
void requestEvent();


void setup() {
    Serial.begin(9600);
    Wire.begin(0x08);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
}

void loop() {
    delay(100);
}

/**
 * void receiveEvent(int)
 * Executes whenever data is received from master
*/
void receiveEvent(int howMany) {
    while (0 <Wire.available()) {
        char c = Wire.read();      /* receive byte as a character */
        Serial.print(c);           /* print the character */
    }
    Serial.println();             /* to newline */
}

// function that executes whenever data is requested from master
void requestEvent() {
    Wire.write("Hello NodeMCU");  /*send string on request */
}