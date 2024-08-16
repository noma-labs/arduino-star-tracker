#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>



Servo servo;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int rx;
  int ry;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Rx: ");
  Serial.println(myData.rx);
  Serial.print("Ry: ");
  Serial.println(myData.ry);
  Serial.print("Button: ");
  Serial.println(myData.d);
  Serial.println();
  int mrx = map(myData.rx, 0, 4095, 0, 180);
  Serial.print("Write: ");
  Serial.println(mrx);
  servo.write(mrx);
  int mry = map(myData.ry, 0, 4095, 0, 180);
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  servo.attach(12);


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // WiFi.mode(WIFI_MODE_STA);
  // Serial.println(WiFi.macAddress());
  // mac of esp32D   : 7C:9E:BD:F3:B7:E8
  // mac of esp32D(A): C8:2B:96:B9:56:AC

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {}