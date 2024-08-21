#include <esp_now.h>
#include <WiFi.h>
#include <AccelStepper.h>

// when this is defined the board will print debug messages on serial
#define DEBUG

// Motors definition
#define dir1 33
#define stp1 32

#define dir2 2
#define stp2 0

#define MS1 27
#define MS2 26
#define MS3 25
#define EN 12


#define LASER_PIN 5

int ZeroP1 = 450;  // posizione zero del potenziometro1
int ZeroP2 = 450;  // posizione zero del potenziometro2

// store the state: 0 = MANUAL, 1 = INSEGUIMENTO, 2 INSEGUIMENTO_MANUALE, 3 = set (not yet implemented)
int state = 0;

bool lastLaserSwitchState = true;
byte ledState = LOW;

AccelStepper stepper1(AccelStepper::DRIVER, stp2, dir2);
AccelStepper stepper2(AccelStepper::DRIVER, stp1, dir1);

typedef struct struct_message {
  // joystick signlas
  int rx;
  int ry;
  bool sw;

  // 4 buttons
  bool up;
  bool dn;
  bool right;
  bool left;
} struct_message;

struct_message myData;


void startInseguimentoWithMicroStep() {
  digitalWrite(EN, LOW);

  enableMicroStep();

  stepper1.setSpeed(19);  // (4255*16/3600)
}

void enableMicroStep() {
  digitalWrite(MS1, HIGH);  // Pull MS1, MS2, and MS3 high to set logic to 1/16th microstep resolution
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);
}


void disableMicroStep() {
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);
}


void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Rx: ");
  Serial.println(myData.rx);
  Serial.print("Ry: ");
  Serial.println(myData.ry);

  int potenziometro1 = myData.ry;
  int potenziometro2 = myData.rx;  // ascensione retta

  bool laserSw = myData.sw;
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

    disableMicroStep();

    digitalWrite(LASER_PIN, LOW);

    state = 0;
  }


  // always control the laser switch
  if (laserSw != lastLaserSwitchState) {
    lastLaserSwitchState = laserSw;
    if (laserSw == false) { // button has been pressed
      ledState = (ledState == HIGH) ? LOW: HIGH;
      digitalWrite(LASER_PIN, ledState);
    }
  }

  switch (state) {
    case 0:  // MANUAL state
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

      // move to INSEGUIMENTO (enable microstep + setSpeed = 19)
      if (insegSw == false) {
        startInseguimentoWithMicroStep();
        state = 1;
      }


      if (microStepSw == false) {
#ifdef DEBUG
        Serial.println("MICROSTEP enabled");
#endif

        enableMicroStep();
      }
      break;
    case 1:  // INSEGUIMENTO state
#ifdef DEBUG
      Serial.println("INSEGUIMENTO state");
#endif

      if (microStepSw == false) {
#ifdef DEBUG
        Serial.println("MICROSTEP enabled");
#endif
        enableMicroStep();
      }

      // moving the joystick enter into the INSEGUIMENTO_MANUALE state
      if (potenziometro1 < 400 || potenziometro1 > 600) {
        stepper1.setSpeed((potenziometro1 - ZeroP1) / 2);
        state = 2;
      }
      if (potenziometro2 < 400 || potenziometro2 > 600) {
        stepper2.setSpeed((potenziometro2 - ZeroP2) / 6);
        state = 2;
      }
      break;
    case 2:  // INSEGUIMENTO_MANUALE state
#ifdef DEBUG
      Serial.println("INSEGUIMENTO_MANUALE state");
#endif
      // INSEGUIMENTO_MANUALE: move the motors with the speed equal to the joystick with microstep.
      // If the joystick is not moved, come back to inseguimento (microstep + speed=19)
      if ((potenziometro1 >= 400 && potenziometro1 <= 600) && (potenziometro2 >= 400 && potenziometro2 <= 600)) {
        startInseguimentoWithMicroStep();
        state = 1;
      } else {
        if (potenziometro1 < 400 || potenziometro1 > 600) {
          stepper1.setSpeed((potenziometro1 - ZeroP1) / 2);
        }
        if (potenziometro2 < 400 || potenziometro2 > 600) {
          stepper2.setSpeed((potenziometro2 - ZeroP2) / 6);
        }
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
  pinMode(LASER_PIN, OUTPUT);

  stepper1.setMaxSpeed(10000);
  stepper1.setSpeed(0);
  stepper2.setMaxSpeed(10000);
  stepper2.setSpeed(0);

  digitalWrite(LASER_PIN, ledState);


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
