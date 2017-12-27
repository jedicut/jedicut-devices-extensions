

/*  Copyright 2013 Martin
    Modifié 2016-2017 par Alain DENIS  alain@aeroden.fr
    This file is part of jedicutplugin.

    fcifmdlcnc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fcifmdlcnc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
	
	This version of the code is for the letmathe mdlcnc board. For 
	other boards this code must be adapted accordingly !!!
*/
/*
The jedicut communication configuration for Clock and Direction must be fixed set to the below values, 
only the wires to the specific board may be different -- wire to -->

Le 18/09/2016 commencé assai avec carte arduino mega 2560 modifier le pins, pris le port C au lieu du port D
Le 20/09/2016 modification du code pour avoir les bonnes bornes de sorties pour Ramps1.4
Le 21/09/2016 Mise en place de L'afficheur LCD
Le 04/01/2017 Ajout affichage Config
le 30/11/2017 Adaptation du sketch suit à nouveau Plugin USBSerial_2.dll compatible
  avec les entraînements à courroie passer 16 bits la commande de vitesse pour 
  élargissement de la plage de vitesse dans la config de Jedicut.


Function               Number in jedicut configuration   Arduino Pin        		e.g. Lethmate MDLCNC board SubD25 pin
                       (fixed !!)
EngineX1 Clock         2                                 53  (PB0)    -- wire to -->        2    
EngineX2 Clock         3                                 52  (PB1)    -- wire to -->        6
EngineY1 Clock         4                                 51  (PB2)    -- wire to -->        4
EngineY2 Clock         5                                 50  (PB3)    -- wire to -->        8
EngineX1 Direction     6                                 33  (PC4)    -- wire to -->        3
EngineX2 Direction     7                                 32  (PC5)    -- wire to -->        7
EngineY1 Direction     8                                 31  (PC6)    -- wire to -->        5
EngineY2 Direction     9                                 30  (PC7)    -- wire to -->        9

All Engines On/Off     -                                 10  (PB4)    -- wire to -->        1 
Heating On/off         -                                 35  (PC2)    -- wire to -->        14
Heating PWM            -                                  7  (PH4)    -- wire to -->        - 
*/ 


#include <avr/interrupt.h>
#include <avr/io.h>
#include <LiquidCrystal.h>
#include "conf.h"

//Définition du LCD

#define LCD_RS 16      // LCD control and is connected into GADGETS3D  shield LCDRS
#define LCD_E 17       // LCD enable pin and is connected into GADGETS3D shield LCDE
#define LCD_D4 23      // LCD signal pin, connected to Gadgets3D shield LCD4
#define LCD_D5 25      // LCD signal pin, connected to Gadgets3D shield LCD5
#define LCD_D6 27      // LCD signal pin, connected to Gadgets3D shield LCD6
#define LCD_D7 29      // LCD signal pin, connected to Gadgets3D shield LCD7
#define COLL 20        // 20 caractères par ligne
#define LIGNE 4         // 4 lignes
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Instanciation du LCD

// Définition du Commutateur rotatif

#define ROT_EN_A 31   // Canal A de l'encodeur rotatif
#define ROT_EN_B 33   // Canal B de l'encodeur rotatif
#define BP_EN 35      // Bouton poussoir de l'encodeur rotatif
//#define BP_STOP 41  // Pas utilisé
#define buzzer 37     // Beeper and is Connected into GADGETS3D shield MEGA_18BEEPER
 
// Ringbuffer for the commands
#define CMD_BUFFER_SIZE 1200  // must be even !
volatile byte cmdArray[CMD_BUFFER_SIZE];
volatile int arrayIdxRead  = 0;
volatile int arrayIdxWrite = 0;
volatile int cmdCounter = 0;
volatile boolean ovf = false;

byte Mot_Dir = 0 ;
byte Mot_Ck = 0;
volatile byte Mot_Off = 1;
volatile byte Jed_Mot_Off =1;
volatile word Freq;
byte refChaufPc ; // ref pour la Chauffe en Auto du PC PWM du chauffage fil 0 à 100
byte Rel_Chauf = 10; //Interrupteur de Chauffage fil "0" ON
volatile bool isrActive = false;
byte X1_Dir = A1;  // PF1, X1 Direction
byte X2_Dir = A7;  // PF7, X2 Direction
byte Y1_Dir = 28;  // PA6, Y1 Direction.
byte Y2_Dir = 34;  // PC3, Y2 Direction

byte X1_Step = A0;  // PF0, X1 Step
byte X2_Step = A6;  // PF6, X2 Step
byte Y1_Step = 26;  // PA4, Y1 Step
byte Y2_Step = 36;  // PC1, Y2 Step
byte Test_Step = 11; // test pour fréquence interruption

byte X1_En = 38;  // PD7, X1 En service
byte X2_En = A2;  // PF2, X2 En service
byte Y1_En = 24;  // PA2, Y1 En service
byte Y2_En = 30;  // PC7, Y2 En service

int C_Pwm = 8; //Sortie de cde de chauffage di fil



/**********************************************************************************/
void setup() 
{


  Serial.begin(BAUDRATE);    // opens serial port, sets data rate to 115200 bps
  
  pinMode(13,OUTPUT);      // Led Pin used for signaling an underflow condition
  

  
  pinMode(35,OUTPUT);       // PD2, Heat Relay on/off low active for letmathe mdlcnc
  digitalWrite(35, HIGH);   // Off
  
  pinMode(C_Pwm,OUTPUT);       // PWM for wire heating
  analogWrite(C_Pwm,0);       // Off



  // Driver Motor Pins  
  pinMode(X1_Step,OUTPUT); 
  pinMode(X2_Step,OUTPUT);  
  pinMode(Y1_Step,OUTPUT);  
  pinMode(Y2_Step,OUTPUT);  
  
  
  pinMode(X1_Dir,OUTPUT);   
  pinMode(X2_Dir,OUTPUT);   
  pinMode(Y1_Dir,OUTPUT);   
  pinMode(Y2_Dir,OUTPUT); 

  pinMode(X1_En,OUTPUT); 
  digitalWrite(X1_En, HIGH);       // forcé à Off  
  pinMode(X2_En,OUTPUT);
  digitalWrite(X2_En, HIGH);       // forcé à Off     
  pinMode(Y1_En,OUTPUT);
  digitalWrite(Y1_En, HIGH);       // forcé à Off     
  pinMode(Y2_En,OUTPUT);
  digitalWrite(Y2_En, HIGH);       // forcé à Off  
// Message de bienvenue
  lcd.begin(COLL, LIGNE);
  lcd.print ("  * JEDICUT-USB *");
  lcd.setCursor(0,1);
  lcd.print("Jedicut-ALDEN v1.2.2");
  lcd.setCursor(2, 3);
  lcd.print ("Baudrate "+ String(BAUDRATE));
   delay(3000);
  lcd.clear(),
  lcd.setCursor(0, 1);
  lcd.print("mm/step "+ String(PAS_STEP,4));
  lcd.setCursor(0, 2);
  lcd.print ("Lim Chauffe = "+ String(LIM_P_FIL,DEC)+" %");
    delay(3000);
  lcd.setCursor(0, 0);
  lcd.print ("  * JEDICUT-USB *");
  lcd.setCursor(0,1);
  lcd.print("Jedicut-ALDEN v1.2.2");  
  lcd.setCursor(0,2);
  lcd.print("Chauffe    Vitesse");
  lcd.setCursor(0,3);
  lcd.print("     %         mm/s");
  lcd.setCursor(0,3);
 
  
  // using timer5 (because timer2 is used for pwm on pin3 and timer0 is used for delay functions)
  // using Fast PWM (Mode 15)
  TCCR5A = 0x03;          // wgm11=1 wgm0=10
  TCCR5B = 0x1C;          // wgm13=1 wgm12=1    cs12=1 cs11=0 cs10=0 /256 prescaler
  OCR5A  = 255;           // Timer compare register. 255 = 250Hz, 127 = 500Hz, 63 = 1 KHz
  TIMSK5 = (1 << OCIE5A); // Timer interrupt enable for match to OCR1A
  TIFR5  = 0;             // Clear flags.
  TCNT5  = 0;
  sei();
  
}

/**********************************************************************************/
void sendMotorCmd(byte cmd)
{
  Mot_Dir = (Mot_Dir & 0x0F) | (cmd & 0xf0); // Directions first!
  ValCommDir();
  delayMicroseconds(1); // eventually wait a little bit

  Mot_Ck = (Mot_Ck & 0xF0) | (cmd & 0x0f); // and step
  ValCommStep();
  delayMicroseconds(5); // eventually wait a little bit
  // and falling edge of step pulse
  Mot_Ck = (Mot_Ck & 0xF0);
  ValCommStep();
}


/**********************************************************************************/
void Gest_Cde_Mot_ON()
{
  Mot_Off = Jed_Mot_Off ;
  digitalWrite(X1_En,Mot_Off); // Cde Driver X1
  digitalWrite(X2_En,Mot_Off); // Cde Driver X2
  digitalWrite(Y1_En,Mot_Off); // Cde Driver Y1
  digitalWrite(Y2_En,Mot_Off); // Cde Driver Y2
}

/**********************************************************************************/
void ValCommDir()
{
  digitalWrite(X1_Dir,(Mot_Dir&(1<<4))); // direction de X1
  digitalWrite(X2_Dir,(Mot_Dir&(1<<5))); // direction de X2
  digitalWrite(Y1_Dir,(Mot_Dir&(1<<6))); // direction de Y1
  digitalWrite(Y2_Dir,(Mot_Dir&(1<<7))); // direction de Y2
}

/**********************************************************************************/
void ValCommStep()
{
 digitalWrite(X1_Step,(Mot_Ck&(1<<0))); // Step de X1 
 digitalWrite(X2_Step,(Mot_Ck&(1<<1))); // Step de X2
 digitalWrite(Y1_Step,(Mot_Ck&(1<<2))); // Step de Y1
 digitalWrite(Y2_Step,(Mot_Ck&(1<<3))); // Step de Y2
 digitalWrite(Test_Step, (Mot_Ck & (1 << 0))); // Test_Step
}

/**********************************************************************************/
void handleCommand()
{

 unsigned int val = cmdArray[arrayIdxRead+1]; // The command parameter value
unsigned char BufferRecep[4]={0};
  BufferRecep[0]=cmdArray[arrayIdxRead+0];
  BufferRecep[1]=cmdArray[arrayIdxRead+1];
  BufferRecep[2]=cmdArray[arrayIdxRead+2];
  BufferRecep[3]=cmdArray[arrayIdxRead+3];
  
  switch(cmdArray[arrayIdxRead])
  {
    case 'A':   // All Motors on/off
    {
      if(val == '1')  {Jed_Mot_Off = 0;} // "0" déblocage driver
      else          {Jed_Mot_Off = 1;} // "1" blocage driver
      Gest_Cde_Mot_ON();
     break;
    }
    case 'H':   // Wire Heat ON/OFF (may be programmed as PWM (analog out))
    {
        refChaufPc = val;
       if (refChaufPc > 0) {
        digitalWrite(Rel_Chauf , HIGH) ;
         if (refChaufPc > LIM_P_FIL) refChaufPc = LIM_P_FIL ;
      } 
        else {
        digitalWrite(Rel_Chauf, LOW );
      }
      analogWrite(C_Pwm, refChaufPc * 2.55); // PWM for wire heating (stretch 0-100% to a range of 0-255)*/

      lcd.setCursor(1,3);
      if (refChaufPc < 100) lcd.print(" ");
      if (refChaufPc < 10) lcd.print(" ");
      lcd.print(refChaufPc, DEC);
   break;
    }   
    case 'M':   // Motor step Command
    {
      sendMotorCmd(val);
    break;
    }
    case 'F':   // Changer la fréquence du temps, le temps écoulé entre deux steps
    {
		// OCR5A values 255 = 250Hz 190 = 328Hz 127 = 500Hz 63 = 1 kHz 31 = 2KHz 15 = 4 kHz
      val = (BufferRecep[1] * 256) + BufferRecep[2];
      if (val > 32766) val = 32766; // restrict from 1 à 255 corresponds  à 250Hz  à 20 kHz
      if (val < 1) val = 1; // restrict from 1 à 255 corresponds  à 250Hz  à 20 kHz
      OCR5A = val;
      Freq = 62500 / (1+(val));
      float MMS1 = Freq * PAS_STEP;
      lcd.setCursor(10,3);
      if (MMS1 < 10) lcd.print(" ");
      lcd.print(MMS1, 1);//affichage mm/s sur LCD Avec 1 chiffre après la virgule
      
    break;
    }
  }

}


/**********************************************************************************/



/**********************************************************************************/
ISR(TIMER5_COMPA_vect) {
  
  if (isrActive) return;
  isrActive = true;  
  sei(); // reenable interrupts, to not miss serial data
  
do {
      // check if the buffer is empty
      if((arrayIdxRead != arrayIdxWrite) || ovf)
      {
        handleCommand();
        arrayIdxRead += 4;
        if(arrayIdxRead==CMD_BUFFER_SIZE) arrayIdxRead=0;
	      noInterrupts();
        cmdCounter--;
        interrupts();
        if (ovf && (cmdCounter<CMD_BUFFER_SIZE/4-25))
        {
          Serial.write('C');
          ovf = false;
        } 
        digitalWrite(13, LOW); // underflow led off
      }
      else
      {
        // underflow !!
        digitalWrite(13, HIGH); // underflow led on
        break;
      }
    } while(cmdArray[arrayIdxRead] != 'M'); // only motor commands will wait for next sync, all others can be handled immediately
    
  cli();
  
  isrActive = false;    
}

/**********************************************************************************/
/**** The main loop                                                           *****/
/**********************************************************************************/
void loop() 
{ 

  if (Serial.available() > 0)
  {    
    // Each command consists of 2 bytes
    Serial.readBytes((char*)&cmdArray[arrayIdxWrite],4);

    // korrekt the write index
    arrayIdxWrite+=4;
    if(arrayIdxWrite==CMD_BUFFER_SIZE) arrayIdxWrite=0;
	  
    noInterrupts();
    cmdCounter++;
    interrupts();
	  
    
    // check for oncoming overflow    
    if(cmdCounter >= CMD_BUFFER_SIZE/4-20)
    {
      ovf = true;
      Serial.write('S'); // Stop transmission, Buffer full
      }
  }
}
