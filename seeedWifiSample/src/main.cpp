#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char* ssid = "esp32-c softAP";        // WiFiのSSID
const char* password = "hogefugapiyo"; // WiFiのパスワード
const char* hostName = "esp32-c3";      // mDNSでのホスト名

const unsigned int localUdpPort = 8888;  // 受信するUDPポート番号
char incomingPacket[255];  // 受信するデータを格納するバッファ
WiFiUDP udp;
WiFiUDP recieveUdp;

double leftForward = 0.0;
double leftBackward = 0.0;
double rightForward = 0.0;
double rightBackward = 0.0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("ESP32-C3 IP address: ");
  Serial.println(ip);

  udp.begin(localUdpPort);
  Serial.printf("UDP server started on port %u\n", localUdpPort);
}


void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Null-terminate the string
    }
    // Serial.printf("Received packet: '%s'\n", incomingPacket);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, incomingPacket);

    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    double left = doc["left_speed"];
    double right = doc["right_speed"];

    if(left > 0){
      leftForward = left;
      leftBackward = 0;
    } else if(left < 0) {
      leftForward = 0;
      leftBackward = -left;
    } else {
      leftForward = 0;
      leftBackward = 0;
    }

    if(right > 0){
      rightForward = right;
      rightBackward = 0;
    } else if(right < 0) {
      rightForward = 0;
      rightBackward = -right;
    } else {
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
}