#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define ryPin 34
#define rxPin 35

#define upPin 14    
#define dnPin 16    
#define leftPin 5   
#define rightPin 5  

typedef struct struct_message {
  int rx;  //  X-axis analog output voltage
  int ry;  //  Y-axis analog output voltage
  bool sw;

  // 4 buttons on the controller
  bool up;     
  bool dn;     
  bool right;  
  bool left;   
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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

  pinMode(upPin, INPUT);
  pinMode(dnPin, INPUT);
  pinMode(leftPin, INPUT);
  pinMode(rightPin, INPUT);
}


void loop() {
  myData.rx = analogRead(rxPin);
  myData.ry = analogRead(ryPin);
  myData.sw = false;
  myData.dn = digitalRead(dnPin);
  myData.up = digitalRead(upPin);
  myData.left = digitalRead(leftPin);
  myData.right = digitalRead(rightPin);

  Serial.print("x:");
  Serial.println(myData.rx);
  Serial.print("y:");
  Serial.print(myData.ry);
  Serial.print("dn:");
  Serial.print(myData.dn);
  Serial.print("up:");
  Serial.print(myData.up);
  Serial.print("left:");
  Serial.print(myData.left);
  Serial.print("right:");
  Serial.print(myData.right);


  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
  delay(100);
}