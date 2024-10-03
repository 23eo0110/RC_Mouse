#include <Arduino.h>

//Motor variables
const int R_Forward = 2; // GPIO2
const int R_Back = 3; // GPIO3
const int L_Forward = 4; // GPIO4
const int L_Back = 5; // GPIO5

void DRV8835(int in1, int in2, int in3, int in4) {
  analogWrite(R_Forward, in1);
  analogWrite(R_Back, in2);
  analogWrite(L_Forward, in3);
  analogWrite(L_Back, in4);
}

void stop() {
  DRV8835(0, 0, 0, 0);
  delay(1000);
}

void setup() {

  // Motor setup
   pinMode(R_Forward, OUTPUT);
   pinMode(R_Back, OUTPUT);
   pinMode(L_Forward, OUTPUT);
   pinMode(L_Back, OUTPUT);

}

void loop() {
  for (int i = 0; i <= 255; i++) {
  DRV8835(i, 0, 0, 0); // Right motor forward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(0, 0, i, 0); // Left motor forward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(0, i, 0, 0); // Right motor backward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(0, 0, 0, i); // Left motor backward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(i, 0, i, 0); // Right motor forward, Left motor forward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(0, i, 0, i); // Right motor backward, Left motor backward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(i, 0, 0, i); // Right motor forward, Left motor backward
  delay(10);
  }

  stop();

  for (int i = 0; i <= 255; i++) {
  DRV8835(0, i, i, 0); // Right motor backward, Left motor forward
  delay(10);
  }

  stop();

}
