

#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
//#include "utility/Adafruit_PWMServoDriver.h"
#include <VirtualWire.h>


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
   Definisco le dimensioni del display, serviranno per creare l'istanza del display
   e magari riutilizzarle nel codice qualora dovessero servirmi queste informazioni
*/
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Creo istanza del display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);


        

int inseguimento=0;
int microstep=0;
double FattoreScalaAR=4255;// step per fare un ora . Invece 4450 per fare 180 gradi
double FattoreScalaDEC=4450;
double AscensioneRetta=0.00034;
double Declinazione=28.25;
double setAR=9;
double setDEC=0;
bool displ=1;//controllo per accendere display con aggiornamento posizione solo dopo aver impostato le coordinate inizialie
bool set=1;
int posizionePrecedenteAR=6;
int posizionePrecedenteDEC=0;
float ore=3;
float minuti=0;
float secondi=0;
int ZeroP = 520; //posizione zero del potenziometro
int ZeroP2 = 400; //posizione zero del potenziometro

#define stp 10
#define dir 11
#define MS1 6//attenzione invertito CON EN PERCHè INVERTITO NEI COLLEGAMENTI
#define MS2 8
#define MS3 9
#define EN  7
#define stp2 4
#define dir2 3
AccelStepper stepper1(1,10,11);
AccelStepper stepper2(1, 4, 3);
//AccelStepper stepper2(1, 10, 11);
int potenziometro1=1;//
int potenziometro2=1;//


unsigned long previousMillis = 0;        // will store last time LED was updated
const double interval = 4000;           // intervallo stampa su oled
void setup()
{ 
  Serial.begin(9600);
    //definisco pin motore 1
  pinMode(stp2, OUTPUT);
  pinMode(dir2, OUTPUT);
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);

  pinMode(14, INPUT);//pin analogico 0 come ingresso digiale pulsante A1 (set)
  pinMode(16, INPUT);//pin analogico 2 come ingresso digiale pulsante D12 (inseguimento)
  pinMode(12, INPUT);
  pinMode(5, INPUT);


stepper1.setMaxSpeed(10000);
   stepper1.setSpeed(400);
   stepper2.setMaxSpeed(1000);
   stepper2.setSpeed(400);   

  
//Provo ad inizializzare il display all'indirizzo 0x3C (il tuo potrebbe essere diverso)
  if (!display.begin( SSD1306_SWITCHCAPVCC, 0x3C)) {
    /*
      Se non sono riuscito ad inizializzare il display
      creo un loop infinito ed impedisco al programma di andare avanti
    */
    while (true);
  }
  // Pulisco il buffer display
  display.clearDisplay();

  // Applico la pulizia al display
  display.display();
  //I due passaggi qui sopra evitano di mostrare il logo adafruit all'avvio


}

void loop()
{
               //routine di ricezione
              // Serial.println("cavolo");
   //Serial.println(digitalRead(14));   
   //Serial.println(digitalRead(16));
//Serial.println(analogRead(3));
   //Serial.println(analogRead(1));          

//setAR=ore+minuti/60+secondi/3600;
//stampaAR();
    unsigned long currentMillis = millis();

   if (currentMillis - previousMillis >= interval && displ==0  && digitalRead(14)==1) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    stampaTestoSemplice();//stampa su oled
    }
    
    potenziometro1=analogRead(3); 
    potenziometro2=analogRead(1); 
       //Serial.println("uscito");
 if (potenziometro1>600 && digitalRead(14)==1 ) 
   {
    //Serial.println("entrato");
      digitalWrite(EN, LOW);//attivo il driver
      
//      digitalWrite(MS1, LOW); //attivo step normale
//      digitalWrite(MS2, LOW);
//      digitalWrite(MS3, LOW);
    stepper1.setSpeed((potenziometro1-ZeroP)/2 );  
    stepper1.runSpeed();
    //Serial.println (-(potenziom1.MeasData-ZeroP)/1 );
   }
   else if (potenziometro1<450 && digitalRead(14)==1 )
 
   { 
      digitalWrite(EN, LOW);//attivo il driver
       // Serial.println("entrato");
//      digitalWrite(MS1, LOW); //attivo step normale
//      digitalWrite(MS2, LOW);
//      digitalWrite(MS3, LOW);
    stepper1.setSpeed((potenziometro1-ZeroP)/2); 
    stepper1.runSpeed();
        
   }
  else
  { digitalWrite(EN, HIGH);//disattivo il driver
  }


   if (potenziometro2>600 && digitalRead(14)==1) 
   {
      //Serial.println("entrato");
    //Serial.println("potenziometro2");
    stepper2.setSpeed((potenziometro2-ZeroP)/6 );  
    stepper2.runSpeed();
   }
   else if (potenziometro2<450 && digitalRead(14)==1 )
 
   {
    stepper2.setSpeed((potenziometro2-ZeroP2)/6); 
    stepper2.runSpeed();
          //Serial.println("entrato");
   }
  else
  {//stepper2.disableOutputs();
  }
              
// int inseguimento=1;
////  

if ((digitalRead(5)==0))//disattivo inseguimento (digital pin 5 che equivale d8) ,  disattivo microstep
  {   
      //digitalWrite(EN, HIGH);//disattivo il driver del 1 motore
      //Serial.println("entrato");
      digitalWrite(MS1, LOW); 
      digitalWrite(MS2, LOW);
      digitalWrite(MS3, LOW);
      set=1;//per settare valori dec ar
      displ=0;
      inseguimento=0;
      microstep=0;
  }

  
 if ((digitalRead(16)==0))//
 { inseguimento=1;
 microstep=0;
  }
 //attivo inseguimento con microstep  
   if (inseguimento==1)
  {   
     digitalWrite(EN, LOW);
     // Serial.println("entrato inseguimento");
     digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
     digitalWrite(MS2, HIGH);
     digitalWrite(MS3, HIGH);
    stepper1.setSpeed(19); //(4255*16/3600)
    stepper1.runSpeed();
 set=1;//per settare valori dec ar
 displ=0;

        }
//Serial.println(inseguimento);
 if ((digitalRead(12)==0))//attivo microstep digital pin 12 che equivale a d9
  {   
     // digitalWrite(EN, LOW);
     // Serial.println("entrato");
      digitalWrite(MS1, HIGH); //Pull MS1,MS2, and MS3 high to set logic to 1/16th microstep resolution
      digitalWrite(MS2, HIGH);
      digitalWrite(MS3, HIGH);
  set=1;//per settare valori dec ar
  displ=0;
  microstep=1;
        }
 
// 
//if ((digitalRead(16)==0)&& digitalRead(14)==0 && set==1)//setto coordinate iniziali (schiaccio contemporaneamento A1 e D12)
//  { 
//    stepper1.currentPosition()==posizionePrecedenteAR;
//    stepper2.currentPosition()==posizionePrecedenteDEC; 
//    
//   set=0;
//   displ=0;//accende il display con aggiornamento posizione solo quando ho già impostato coordinate iniziali
//          }
////SETTO ORE MINUTI E SECONDI CON JOY
//
 if (digitalRead(14)==0 && digitalRead(16)==1){//schiaccio pulsante A1 per settare coordinate. Il pulsante D12 non deve essere schiacciato
  //IMPOSTO ORE
   if (potenziometro1>600 ) 
   {
    ore=ore + 1;
    
   }
   else if (potenziometro1<450)
 
   {
      ore=ore - 1;      
   }

     //IMPOSTO MINUTI
    if (potenziometro2>600 && potenziometro2<900 ) 
     {
     secondi=secondi + 1;
     }
     else if (potenziometro2<450 && potenziometro2>100)
 
     {
      secondi=secondi - 1;      
     }
  //imposto secondi

    if (potenziometro2>990 ) 
     {
     minuti=minuti + 1;
     }
     else if (potenziometro2<40)
 
     {
      minuti=minuti - 1;      
     }

 setAR=ore+minuti/60+secondi/3600;
 stampaAR();
 delay(100);
 setDEC=0;
 stepper1.currentPosition()==posizionePrecedenteAR;
 stepper2.currentPosition()==posizionePrecedenteDEC;
 set=0;
displ==0;
   }//Serial.println("ci sonooo");
//Serial.println(stepper1.currentPosition());
}








void stampaTestoSemplice() {
// Extract the integer part of the number and print it
   double number;//AR
  double number1;//DEC
//  if(stepper1.currentPosition()==posizionePrecedenteAR)//se non cambio posizione step non mi calcolo nuove coordinate
//  {number=setAR+interval; // tengo conto del tempo passato
//    }
//     if(stepper1.currentPosition()==posizionePrecedenteAR )//stampa il dato set position
//  {
//    Serial.println();
//    number=setAR+12;
//    }

    if(inseguimento==1 )// inseguimento
  {
     //number=-stepper1.currentPosition()/(FattoreScalaAR*16)+setAR;//se attivo il microstep
     number=setAR;//se attivo il microstep. le AR non cambiano 
    stepper1.setCurrentPosition(0);
    posizionePrecedenteAR=stepper1.currentPosition();
    setAR=number;
    }

//
 if( microstep==1)//inseguimento
  {
     number=-stepper1.currentPosition()/(FattoreScalaAR*16)+interval/3600000+setAR;//se attivo il microstep. tengo anche conto del tempo che passa
    stepper1.setCurrentPosition(0);
    posizionePrecedenteAR=stepper1.currentPosition();
    setAR=number;
    }
    
    else if(microstep==0 && inseguimento==0)
    {//
     // number=setAR+1;
    number=-stepper1.currentPosition()/FattoreScalaAR+setAR+interval/3600000;
//    //number=24;
         stepper1.setCurrentPosition(0);
      posizionePrecedenteAR=stepper1.currentPosition();
      setAR=number;
    }

     if(stepper2.currentPosition()==posizionePrecedenteDEC)
     {
    }

    if(stepper2.currentPosition()== posizionePrecedenteDEC && set==0)//stampa il dato set position
    {
    number1=setDEC;
    }
   else if(inseguimento==1 || microstep==1)//microstep ed inseguimento
  {
     number1=-((180*stepper2.currentPosition())/(FattoreScalaDEC))/4+setDEC;
     stepper2.setCurrentPosition(0);
    posizionePrecedenteDEC=stepper2.currentPosition();
    setDEC=number1;
    }
         else
    {  number1=-(180*stepper2.currentPosition())/(FattoreScalaDEC)+setDEC;
        stepper2.setCurrentPosition(0);
    posizionePrecedenteDEC=stepper2.currentPosition();
    setDEC=number1;
    }


  //Ripulisco il buffer
  display.clearDisplay();

  //Setto il colore del testo a "bianco"
  display.setTextColor( WHITE);

  //Setto dimensione del testo
  display.setTextSize(2);

  //Sposto il cursore a metà altezza del display
 display.setCursor(0, 0);
//  //Stampo una scritta
 display.println("AR= ");
 //ASCENSIONE RETTA
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  display.setCursor(40, 0);
  display.println(int_part);
//   display.display();
remainder= remainder * 60;
  int_part = (unsigned long)remainder;
  remainder = remainder - (double)int_part;
  
  display.setCursor(70, 0);
  display.println(int_part);
  remainder= remainder * 60;
  
int_part = (unsigned long)remainder;
  remainder = remainder - (double)int_part;
 display.setCursor(100, 0);
  display.println(int_part);
 
//  remainder = remainder*100;
// 
//int_part = (unsigned long)remainder;
//display.setCursor(50, 20);
//  display.println(int_part);
  //display.println(stepper1.currentPosition());

//DECLINAZIONE
  //Sposto il cursore a metà altezza del display
  display.setCursor(0, 45);
  //Stampo una scritta
  display.println("DE=");
  unsigned long int_part1 = (unsigned long)number1;
  double remainder1 = number1 - (double)int_part1;
  display.setCursor(35, 45);
  display.println(int_part1);
//   display.display();
remainder1= remainder1 * 60;
  int_part1 = (unsigned long)remainder1;
  remainder1 = remainder1 - (double)int_part1;
  
  display.setCursor(75, 45);
  display.println(int_part1);
  remainder1= remainder1 * 60;
  
int_part1 = (unsigned long)remainder1;
  remainder1 = remainder1 - (double)int_part1;
 display.setCursor(103, 45);
  display.println(int_part1);
// 
//  remainder1 = remainder1*100;
 
//int_part1 = (unsigned long)remainder1;
//display.setCursor(50, 20);
//  display.println(int_part1);

  //La mando in stampa
  display.display();

}



void stampaAR() {
// Extract the integer part of the number and print it
   double number=setAR;//AR
  

  //Ripulisco il buffer
  display.clearDisplay();

  //Setto il colore del testo a "bianco"
  display.setTextColor( WHITE);

  //Setto dimensione del testo
  display.setTextSize(2);

  //Sposto il cursore a metà altezza del display
 display.setCursor(0, 0);
//  //Stampo una scritta
 display.println("setAR=");
 //ASCENSIONE RETTA
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  display.setCursor(40, 20);
  display.println(int_part);
//   display.display();
remainder= remainder * 60;
  int_part = (unsigned long)remainder;
  remainder = remainder - (double)int_part;
  
  display.setCursor(70, 20);
  display.println(int_part);
  remainder= remainder * 60;
  
int_part = (unsigned long)remainder;
  remainder = remainder - (double)int_part;
 display.setCursor(100, 20);
  display.println(int_part);
 
  remainder = remainder*100;
 
//int_part = (unsigned long)remainder;
//display.setCursor(50, 20);
//  display.println(int_part);
  //display.println(stepper1.currentPosition());

  //La mando in stampa
  display.display();

}
