#include <Arduino.h>

// バッテリー電圧を読み取るADCピン
const int batteryPin = A0;
// 過放電と判断する電圧の閾値（例: 3.0V）
const float voltageThreshold = 3.0;
// LEDピン
const int ledPin = 13;

void setup() {
  Serial.begin(115200);
  pinMode(batteryPin, INPUT); // ADC
  pinMode(ledPin, OUTPUT);    // LED
  digitalWrite(ledPin, LOW);  // LEDを消灯
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
    Serial.println("Entering deep sleep mode to prevent over-discharge.");
    esp_deep_sleep_start();
  } else {
    // LEDを消灯
    digitalWrite(ledPin, LOW);
  }

  delay(1000);
}
