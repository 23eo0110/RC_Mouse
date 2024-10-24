#include <Arduino.h>

// Battery Check Pin
const int batteryPin = A0;
// over discharge threshhold（<=3.2V）
const float voltageThreshold = 3.2;
// LED Pin
const int ledPin = D10;

void setup() {
  Serial.begin(115200);
  pinMode(batteryPin, INPUT); // ADC
  pinMode(ledPin, OUTPUT);    // LED
  digitalWrite(ledPin, LOW);  // LED OFF
}

void loop() {
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(batteryPin); // ADC with correction
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V
  Serial.println(Vbattf, 3);

  // 過放電を検出
  if (Vbattf < voltageThreshold) {
    // 過放電保護動作（LEDを点灯）
    digitalWrite(ledPin, HIGH);
    // デバイスをスリープモードに移行
    //Serial.println("Entering deep sleep mode to prevent over-discharge.");
    //esp_deep_sleep_start();
  } else {
    // LEDを消灯
    digitalWrite(ledPin, LOW);
  }

  delay(1000);
}
