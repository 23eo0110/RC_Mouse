#include <Arduino.h>
// pin definitions
const int batteryPin = A1; //Battery Check Pin
const float threshold_1 = 4.1; //over discharge threshhold（<=3.1V）
const float threshold_2 = 3.1; //over discharge threshhold（<=3.1V）
bool battState = false;
const int vbusPin = D0; // check VBUS : D0
const int tail_lamp = D10; // tail lamp led : D10
const int drv_vdd = D7;  // driver VDD : D7


// タイマー設定
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(batteryPin); // ADC with correction
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V

  if (Vbattf < threshold_2 ) {
    battState = true;
    digitalWrite(drv_vdd, LOW);
  } else if (Vbattf > threshold_1) {
    battState = false;
  }
}


// function to handle VBUS change
void IRAM_ATTR handleVbusChange() {
  if (digitalRead(vbusPin) == HIGH) {
    digitalWrite(drv_vdd, LOW);
  } else {
    digitalWrite(drv_vdd, HIGH);
  }
}

void setup() {
  pinMode(batteryPin, INPUT); // ADC
  pinMode(vbusPin, INPUT); // VBUS
  pinMode(tail_lamp, OUTPUT); // set tail lamp pin to output mode
  pinMode(drv_vdd, OUTPUT); // set driver VDD pin to output mode
  digitalWrite(tail_lamp, LOW); // initial state of tail lamp
  digitalWrite(drv_vdd, HIGH); // initial state of driver VDD

  // タイマーの初期化
  timer = timerBegin(0, 80, true); // タイマー0、80分周（1usごとにカウント）、アップカウント
  timerAttachInterrupt(timer, &onTimer, true); // 割込みハンドラを設定
  timerAlarmWrite(timer, 1000000, true); // 1秒ごとに割込み
  timerAlarmEnable(timer); // タイマー割込みを有効化

 // VBUSピンの割込みを設定（setup()内へ記述）
  attachInterrupt(digitalPinToInterrupt(vbusPin), handleVbusChange, CHANGE);
}

void loop() {
  if(battState){
    digitalWrite(tail_lamp, HIGH);
    delay(300);
    digitalWrite(tail_lamp, LOW);
    delay(300);
  }
}
