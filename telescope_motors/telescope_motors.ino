#include <esp_now.h>
#include <WiFi.h>
#include <AccelStepper.h>

// when this is defined the board will print debug messages on serial
// #define DEBUG


int inseguimento = 0;
int microstep = 0;
double FattoreScalaAR = 4255;  // step per fare un ora . Invece 4450 per fare 180 gradi
double FattoreScalaDEC = 4450;
double AscensioneRetta = 0.00034;
double Declinazione = 28.25;
double setAR = 9;
double setDEC = 0;
bool set = 1;
int posizionePrecedenteAR = 6;
int posizionePrecedenteDEC = 0;
float ore = 3;
float minuti = 0;
float secondi = 0;
int ZeroP = 520;   //posizione zero del potenziometro
int ZeroP2 = 400;  //posizione zero del potenziometro

// Motos definition
#define dir1 12
#define stp1 14
#define dir2 9
#define stp2 10

#define MS1 27
#define MS2 26
#define MS3 25
#define EN 33


AccelStepper stepper1(AccelStepper::DRIVER, stp2, dir2);
AccelStepper stepper2(AccelStepper::DRIVER, stp1, dir1);

typedef struct struct_message {
  // joystick signlas
  int rx;
  int ry;
  bool d;

  // 4 buttons
  bool up;     // "CLEAR" switch: deactivate modes
  bool dn;     // "INSEGUIMNETO" swith: enable automatic tracker
  bool right;  // "MANUAL" switch: move the motors usign the joystick
  bool left;   // "MICROSTEP" swith: enable micro step
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Rx: ");
  Serial.println(myData.rx);
  Serial.print("Ry: ");
  Serial.println(myData.ry);

  int potenziometro1 = myData.rx;
  int potenziometro2 = myData.ry;

  int manualSw = myData.right;
  bool clearSw = myData.up;
  bool insegSw = myData.dn;
  bool microStepSw = myData.left;

  // MANUAL mode: move manually the motors using the joystick (x -> motor1, y-> motor2)
  if (manualSw == true) {
#ifdef DEBUG
    Serial.println("MANUAL mode enabled");
#endif
    if (potenziometro1 > 600) {
      digitalWrite(EN, LOW);  //activate the driver
      stepper1.setSpeed((potenziometro1 - ZeroP) / 2);
    } else if (potenziometro1 < 450) {
      digitalWrite(EN, LOW);  //attivo il driver
      stepper1.setSpeed((potenziometro1 - ZeroP) / 2);
    } else {
      digitalWrite(EN, HIGH);  //disattivo il driver
    }
    if (potenziometro2 > 600) {
      stepper2.setSpeed((potenziometro2 - ZeroP) / 6);
    } else if (potenziometro2 < 450) {
      stepper2.setSpeed((potenziometro2 - ZeroP2) / 6);
    }
  }


  // CLEAR mode:
  //    - disable INSEGUIMENTO and MICROSTEP
  //    - enable SET mode
  if (clearSw == true) {
#ifdef DEBUG
    Serial.println("CLEAR enabled");
#endif
    //disattivo inseguimento (digital pin 5 che equivale d8) ,  disattivo microstep
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, LOW);
    digitalWrite(MS3, LOW);
    set = 1;  //per settare valori dec ar
    inseguimento = 0;
    microstep = 0;
  }

  // INSEGUIMENTO mode:
  //    - enable tracker
  //    - disable microstep
  //if ((digitalRead(INSG) == 0)) TODO: read zero value to enable  ??
  if (insegSw == false) {
    inseguimento = 1;
    microstep = 0;
  }
  // attivo inseguimento con microstep
  if (inseguimento == 1) {
#ifdef DEBUG
    Serial.println("INSEGUIMENTO mode enable");
#endif
    digitalWrite(EN, LOW);
    digitalWrite(MS1, HIGH);  //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
    digitalWrite(MS2, HIGH);
    digitalWrite(MS3, HIGH);
    stepper1.setSpeed(19);  //(4255*16/3600)
    set = 1;                //per settare valori dec ar
  }


  // MICROSTEP mode  (attivo microstep digital pin 12 che equivale a d9)
  // TODO: read zero value ??
  // if (microStepSw == 0)
  if (microStepSw == true) {
#ifdef DEBUG
    Serial.println("MICROSTEP mode anebled");
#endif
    digitalWrite(MS1, HIGH);  //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
    digitalWrite(MS2, HIGH);
    digitalWrite(MS3, HIGH);
    set = 1;  //per settare valori dec ar
    microstep = 1;
  }


  //  SET mode: change the hout. minutes. seconds.   
  //        - is enable id the manual mode is disable and inseguimento is enabled ?
  //        - manully set hour, minute, and seconds
  // TODO: read are in the opposit logic ??
  // if (digitalRead(SET) == 0 && digitalRead(INSG) == 1) {  //schiaccio pulsante A1 per settare coordinate. Il pulsante D12 non deve essere schiacciato
  if (manualSw == false && insegSw == true) {

    if (potenziometro1 > 600) {
      ore = ore + 1;
    } else if (potenziometro1 < 450) {
      ore = ore - 1;
    }

    if (potenziometro2 > 600 && potenziometro2 < 900) {
      secondi = secondi + 1;
    } else if (potenziometro2 < 450 && potenziometro2 > 100) {
      secondi = secondi - 1;
    }

    if (potenziometro2 > 990) {
      minuti = minuti + 1;
    } else if (potenziometro2 < 40) {
      minuti = minuti - 1;
    }

    setAR = ore + minuti / 60 + secondi / 3600;
    delay(100);
    setDEC = 0;
    stepper1.currentPosition() == posizionePrecedenteAR;
    stepper2.currentPosition() == posizionePrecedenteDEC;
    set = 0;
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
  stepper2.setMaxSpeed(1000);
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
