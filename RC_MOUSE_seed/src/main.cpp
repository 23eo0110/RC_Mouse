#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// WiFi設定
const char* ssid = "esp32-c softAP";        // WiFiのSSID
const char* password = "hogefugapiyo"; // WiFiのパスワード
const char* hostName = "esp32-c3";      // mDNSでのホスト名

const unsigned int localUdpPort = 8888;  // 受信するUDPポート番号
char incomingPacket[255];  // 受信するデータを格納するバッファ
WiFiUDP udp;
WiFiUDP recieveUdp;

// Motor variables
const int R_Forward = 2; // GPIO2
const int R_Back = 3; // GPIO3
const int L_Forward = 4; // GPIO4
const int L_Back = 5; // GPIO5

double leftForward = 0.0;
double leftBackward = 0.0;
double rightForward = 0.0;
double rightBackward = 0.0;

// バッテリー電圧を読み取るADCピン
const int batteryPin = A1;
const float threshold_1 = 4.1; //over charge threshhold
const float threshold_2 = 3.1; //over discharge threshhold
bool battState = false;
const int tail_lamp = D10; // tail lamp led : D10
const int drv_vdd = D7;  // driver VDD : D7




// タイマー設定
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  static uint32_t Vbatt = 0;
  for(int i = 0; i < 16; i++) {
    Vbatt = Vbatt + analogReadMilliVolts(batteryPin); // ADC with correction
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V

  // 過放電を検出
  if (Vbattf < threshold_2 ) {
    battState = true;
//    digitalWrite(drv_vdd, LOW);
  } else if (Vbattf > threshold_1) {
    battState = false;
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("ESP32-C3 IP address: ");
  Serial.println(ip);

  udp.begin(localUdpPort);
  Serial.printf("UDP server started on port %u\n", localUdpPort);

  // Motor setup
  pinMode(R_Forward, OUTPUT);
  pinMode(R_Back, OUTPUT);
  pinMode(L_Forward, OUTPUT);
  pinMode(L_Back, OUTPUT);

  // バッテリーモニタリングの設定
  pinMode(batteryPin, INPUT); // ADC
  pinMode(tail_lamp, OUTPUT); // set tail lamp pin to output mode
  pinMode(drv_vdd, OUTPUT); // set driver VDD pin to output mode
  digitalWrite(tail_lamp, LOW); // initial state of tail lamp
  digitalWrite(drv_vdd, HIGH); // initial state of driver VDD
  // タイマーの初期化
  timer = timerBegin(0, 80, true); // タイマー0、80分周（1usごとにカウント）、アップカウント
  timerAttachInterrupt(timer, &onTimer, true); // 割込みハンドラを設定
  timerAlarmWrite(timer, 1000000, true); // 1秒ごとに割込み
  timerAlarmEnable(timer); // タイマー割込みを有効化
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Null-terminate the string
    }
    // Serial.printf("Received packet: '%s'\n", incomingPacket);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, incomingPacket);

    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    double left = doc["left_speed"];
    double right = doc["right_speed"];

    if(left > 0){
      analogWrite(L_Forward, left * 255);
      leftForward = left;
      leftBackward = 0;
    } else if(left < 0) {
      analogWrite(L_Back, -left * 255);
      leftForward = 0;
      leftBackward = -left;
    } else {
      analogWrite(L_Forward, 0);
      analogWrite(L_Back, 0);
      leftForward = 0;
      leftBackward = 0;
    }

    if(right > 0){
      analogWrite(R_Forward, right * 255);
      rightForward = right;
      rightBackward = 0;
    } else if(right < 0) {
      analogWrite(R_Back, -right * 255);
      rightForward = 0;
      rightBackward = -right;
    } else {
      analogWrite(R_Forward, 0);
      analogWrite(R_Back, 0);
      rightForward = 0;
      rightBackward = 0;
    }

    Serial.printf(" forward left: %f, right: %f\n", leftForward, rightForward);
    Serial.printf("backward left: %f, right: %f\n\n", leftBackward,rightBackward);

    // 返送(失敗)

    // Serial.printf("port: %s, %d\n", udp.remoteIP().toString(), udp.remotePort());

    // // 応答メッセージの送信
    // int statusNum = recieveUdp.beginPacket(udp.remoteIP(), 8889);
    // Serial.printf("port: %s, %d\n, statusNum: %d", recieveUdp.remoteIP().toString(), recieveUdp.remotePort(), statusNum);
    // // String s = "Received";
    // // byte bytes[s.length()];
    // // s.getBytes(bytes, s.length());
    // // udp.write(bytes, s.length());
    // recieveUdp.println("Received");

    // recieveUdp.endPacket();
  }

    if(battState){
    digitalWrite(tail_lamp, HIGH);
    delay(300);
    digitalWrite(tail_lamp, LOW);
    delay(300);
  }
}
