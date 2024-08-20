#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define ryPin 34  // NOTE: other analog pin were not working (like 15 and 2)
#define rxPin 35

#define upPin 25
#define dnPin 26
#define switchPin 32
#define leftPin 27
#define rightPin 33
#define jolly 14  // not yet implemented


typedef struct struct_message {
  int rx;  //  X-axis analog output voltage
  int ry;  //  Y-axis analog output voltage
  bool sw;

  // 4 buttons on the controller
  bool up;
  bool dn;
  bool right;
  bool left;

  // bool jolly;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery fail");
  }
}

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // TODO: use the mac of the peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(ryPin, INPUT);
  pinMode(rxPin, INPUT);

  pinMode(upPin, INPUT_PULLUP);
  pinMode(dnPin, INPUT_PULLUP);
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
}


void loop() {
  myData.rx = map(analogRead(rxPin), 0, 4095, 0,  1024); 
  myData.ry = map(analogRead(ryPin), 0, 4095, 0, 1024);
  myData.sw = digitalRead(switchPin);
  myData.dn = digitalRead(dnPin);
  myData.up = digitalRead(upPin);
  myData.left = digitalRead(leftPin);
  myData.right = digitalRead(rightPin);

  Serial.print("x:");
  Serial.println(myData.rx);
  Serial.print("y:");
  Serial.println(myData.ry);
  Serial.print("dn:");
  Serial.println(myData.dn);
  Serial.print("up:");
  Serial.println(myData.up);
  Serial.print("left:");
  Serial.println(myData.left);
  Serial.print("right:");
  Serial.println(myData.right);
  Serial.print("switch:");
  Serial.println(myData.sw);
  Serial.println();


  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result != ESP_OK) {
    Serial.println("Error sending the data");
  }
  // delay(100);
}
