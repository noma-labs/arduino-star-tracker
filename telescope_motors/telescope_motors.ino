#include <esp_now.h>
#include <WiFi.h>
#include <AccelStepper.h>

// first motot
// #define dir 12
// #define stp 14
// secondo motor
#define dir 9
#define stp 10

#define MS1 27
#define MS2 26
#define MS3 25
#define EN 33

AccelStepper stepper1(AccelStepper::DRIVER, stp, dir);
// AccelStepper stepper2(1, 4, 3);



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
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Rx: ");
  Serial.println(myData.rx);
  Serial.print("Ry: ");
  Serial.println(myData.ry);

  if (myData.ry > 2000) {
    Serial.println("forward");
    StepForwardDefault();
  }
  if (myData.ry < 1500) {
    Serial.println("backward");
    ReverseStepDefault();
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);

  // resetBEDPins();

  stepper1.setMaxSpeed(10000);
  stepper1.setSpeed(400);


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

//Reset Big Easy Driver pins to default states
void resetBEDPins() {
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
  digitalWrite(EN, HIGH);
}



//Default microstep mode function
void StepForwardDefault() { 
   digitalWrite(EN, LOW);//attivo il driver

  stepper1.setSpeed(400);
  stepper1.runSpeed();

}


//Reverse default microstep mode function
void ReverseStepDefault() {
    digitalWrite(EN, LOW);//attivo il driver
  stepper1.setSpeed(-400);
  stepper1.runSpeed();

}