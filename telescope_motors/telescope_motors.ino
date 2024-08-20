#include <esp_now.h>
#include <WiFi.h>
#include <AccelStepper.h>

// when this is defined the board will print debug messages on serial
#define DEBUG

// Motors definition
#define dir1 12
#define stp1 14

#define dir2 0
#define stp2 2

#define MS1 27
#define MS2 26
#define MS3 25
#define EN 33

int ZeroP1 = 450;  // posizione zero del potenziometro1
int ZeroP2 = 450;  // posizione zero del potenziometro2

// store the state: 0 = manual, 1 = inseguimento, 2 = set (not yet implemented)
int state = 0;

AccelStepper stepper1(AccelStepper::DRIVER, stp2, dir2);
AccelStepper stepper2(AccelStepper::DRIVER, stp1, dir1);

typedef struct struct_message {
  // joystick signlas
  int rx;
  int ry;
  bool d;

  // 4 buttons
  bool up;
  bool dn;
  bool right;
  bool left;
} struct_message;

struct_message myData;


void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Rx: ");
  Serial.println(myData.rx);
  Serial.print("Ry: ");
  Serial.println(myData.ry);

  int potenziometro1 = myData.ry;
  int potenziometro2 = myData.rx;  // ascensione retta

  //   int manualSw = myData.right;
  bool clearSw = myData.up;
  bool insegSw = myData.dn;
  bool microStepSw = myData.left;


  // in any state, if the switch is pressed, disable all the modes and move to the manual state
  if (clearSw == false) {
#ifdef DEBUG
    Serial.println("CLEAR");
#endif
    stepper1.setSpeed(0);
    stepper2.setSpeed(0);

    // disable microstep
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, LOW);
    digitalWrite(MS3, LOW);

    state = 0;
  }

  switch (state) {
    case 0:  // manual state
#ifdef DEBUG
      Serial.println("MANUAL state");
#endif
      // move joystick to move the motors
      if (potenziometro1 < 400 || potenziometro1 > 600) {
        digitalWrite(EN, LOW);
        stepper1.setSpeed((potenziometro1 - ZeroP1) / 2);
      } else {
        digitalWrite(EN, HIGH);  // disattivo il driver
        stepper1.setSpeed(0);    // optional ??
      }
      if (potenziometro2 < 400 || potenziometro2 > 600) {
        stepper2.setSpeed((potenziometro2 - ZeroP2) / 6);
      } else {
        stepper2.setSpeed(0);
      }

      // move to INSEGUIMENTO (enable microstep + setSpedd = 19)
      if (insegSw == false) {
        digitalWrite(EN, LOW);

        digitalWrite(MS1, HIGH);  // Pull MS1, MS2, and MS3 high to set logic to 1/16th microstep resolution
        digitalWrite(MS2, HIGH);
        digitalWrite(MS3, HIGH);

        stepper1.setSpeed(19);  // (4255*16/3600)

        state = 1;
      }


      // enable MCROSTEP
      if (microStepSw == false) {
#ifdef DEBUG
        Serial.println("MICROSTEP enabled");
#endif

        digitalWrite(MS1, HIGH);  //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
        digitalWrite(MS2, HIGH);
        digitalWrite(MS3, HIGH);
      }
      break;
    case 1:  // inseguimento state
#ifdef DEBUG
      Serial.println("INSEGUIMENTO state");
#endif
      // moving the joystick has the same effect of the CLEAR button. Stop microstep and move to manual state.
      if (potenziometro1 < 400 || potenziometro1 > 800) {
        stepper1.setSpeed((potenziometro1 - ZeroP1) / 2);

        // disable microstep
        digitalWrite(MS1, LOW);
        digitalWrite(MS2, LOW);
        digitalWrite(MS3, LOW);

        state = 0;  // move to the manual state

      }
      if (potenziometro2 < 400 || potenziometro2 > 800) {
        stepper2.setSpeed((potenziometro2 - ZeroP2) / 6);

        state = 0;  // move to the manual state
      }
      break;
    default:
#ifdef DEBUG
      Serial.println("Unknown state");
#endif
      break;
  }
}

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);

  pinMode(stp1, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(stp2, OUTPUT);
  pinMode(dir2, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);

  stepper1.setMaxSpeed(10000);
  stepper1.setSpeed(0);
  stepper2.setMaxSpeed(10000);
  stepper2.setSpeed(0);


  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  stepper1.runSpeed();
  stepper2.runSpeed();
}
