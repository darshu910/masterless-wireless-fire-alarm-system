#include <WiFi.h>
#include <esp_now.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define MQ2 34
#define BUZZER 25

// Node B & C
uint8_t nodeB[] = {0x1C, 0xC3, 0xAB, 0xC3, 0xD0, 0x24};
uint8_t nodeC[] = {0x1C, 0xC3, 0xAB, 0xC3, 0xEE, 0x70};

int threshold = 2500;
bool fireState = false;

bool n1=false, n2=false, n3=false;

// RECEIVE
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  String msg = String((char*)data);

  if(msg == "N2_FIRE") n2 = true;
  if(msg == "N3_FIRE") n3 = true;

  if(msg == "N2_SAFE") n2 = false;
  if(msg == "N3_SAFE") n3 = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);

  SerialBT.begin("FireAlert");

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, nodeB, 6);
  esp_now_add_peer(&p);

  memcpy(p.peer_addr, nodeC, 6);
  esp_now_add_peer(&p);
}

void loop() {
  int value = analogRead(MQ2);
  n1 = (value > threshold);

  // SEND
  if(n1 && !fireState){
    char msg[]="N1_FIRE";
    esp_now_send(nodeB,(uint8_t*)msg,sizeof(msg));
    esp_now_send(nodeC,(uint8_t*)msg,sizeof(msg));
    fireState=true;
  }

  if(!n1 && fireState){
    char msg[]="N1_SAFE";
    esp_now_send(nodeB,(uint8_t*)msg,sizeof(msg));
    esp_now_send(nodeC,(uint8_t*)msg,sizeof(msg));
    fireState=false;
  }

  int active = n1 + n2 + n3;

  // MESSAGE
  String msg = "Gas leak at ";
  if(n1) msg += "Node A ";
  if(n2) msg += "& Node B ";
  if(n3) msg += "& Node C ";

  // LOGIC
  if(active == 1){
    Serial.println("LOW RISK");
    Serial.println(msg);
    SerialBT.println("LOW RISK");
    SerialBT.println(msg);

    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
  else if(active >= 2){
    Serial.println("HIGH RISK");
    Serial.println(msg);
    SerialBT.println("HIGH RISK");
    SerialBT.println(msg);

    digitalWrite(BUZZER, HIGH);
  }
  else{
    digitalWrite(BUZZER, LOW);
  }

  delay(400);
}