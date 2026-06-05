#include <WiFi.h>
#include <esp_now.h>

#define MQ2 34
#define BUZZER 25

// Node A & C
uint8_t nodeA[] = {0x1C, 0xC3, 0xAB, 0xB4, 0x6A, 0xAC};
uint8_t nodeC[] = {0x1C, 0xC3, 0xAB, 0xC3, 0xEE, 0x70};

int threshold = 2100;
bool fireState = false;

bool n1=false, n2=false, n3=false;

// RECEIVE
void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len) {
  String msg = String((char*)data);

  if(msg == "N1_FIRE") n1 = true;
  if(msg == "N3_FIRE") n3 = true;

  if(msg == "N1_SAFE") n1 = false;
  if(msg == "N3_SAFE") n3 = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t p = {};
  memcpy(p.peer_addr, nodeA, 6);
  esp_now_add_peer(&p);

  memcpy(p.peer_addr, nodeC, 6);
  esp_now_add_peer(&p);
}

void loop() {
  int value = analogRead(MQ2);
  n2 = (value > threshold);

  if(n2 && !fireState){
    char msg[]="N2_FIRE";
    esp_now_send(nodeA,(uint8_t*)msg,sizeof(msg));
    esp_now_send(nodeC,(uint8_t*)msg,sizeof(msg));
    fireState=true;
  }

  if(!n2 && fireState){
    char msg[]="N2_SAFE";
    esp_now_send(nodeA,(uint8_t*)msg,sizeof(msg));
    esp_now_send(nodeC,(uint8_t*)msg,sizeof(msg));
    fireState=false;
  }

  int active = n1 + n2 + n3;

  if(active >= 2){
    digitalWrite(BUZZER, HIGH);
  }
  else if(active == 1){
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
  else{
    digitalWrite(BUZZER, LOW);
  }

  delay(300);
}