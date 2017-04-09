 



/*  Copyright 2013 Martin
 *  Complété en 2016 par A. DENIS

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
The jedicut communication configuration for Clock and Direction must be fixed
 set to the below values,
only the wires to the specific board may be different -- wire to -->

Le 18/09/2016 commencé assai avec carte arduino mega 2560 modifier le pins, 
pris le port C au lieu du port D
Le 20/09/2016 modification du code pour avoir les bonnes bornes de sorties 
pour Ramps1.4
Le 21/09/2016 Mise en place de L'afficheur LCD
Le 28/10/2016 changement de TIMER 1 vers 5
Le 29/10 2016 Ajout choix baudrate dans conf.h
Le 2/01/2017 Modification Calcul de frequence
Le 3/01/2017 Modification du mode de timer mis en CTC mode 4
Le 4/01/2017 Mise en place affichage configuration

Function               Number in jedicut configuration   Arduino Pin        		
                       (fixed !!)
EngineX1 Clock         2                                 53  (PB0)   
EngineX2 Clock         3                                 52  (PB1)    
EngineY1 Clock         4                                 51  (PB2)    
EngineY2 Clock         5                                 50  (PB3)    
EngineX1 Direction     6                                 33  (PC4)    
EngineX2 Direction     7                                 32  (PC5)    
EngineY1 Direction     8                                 31  (PC6)   
EngineY2 Direction     9                                 30  (PC7)   

All Engines On/Off     -                                 10  (PB4)    
Heating On/off         -                                 35  (PC2)    
Heating PWM            -                                  7  (PH4)    
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
#define LIGNE 4        // 4 lignes
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Instanciation du LCD

// Définition du Commutateur du panel de contrôle avec LCD

#define ROT_EN_A 31   // Canal A de l'encodeur rotatif
#define ROT_EN_B 33   // Canal B de l'encodeur rotatif
#define BP_EN 35      // Bouton poussoir de l'encodeur rotatif
//#define BP_STOP 41  // Pas utilisé
#define buzzer 37     // Beeper and is Connected into GADGETS3D shield MEGA_18BEEPER


// Ringbuffer for the commands

#define CMD_BUFFER_SIZE 600  // must be even !
volatile byte cmdArray[CMD_BUFFER_SIZE];
volatile int arrayIdxRead  = 0;
volatile int arrayIdxWrite = 0;
volatile int cmdCounter = 0;
volatile boolean ovf = false;

byte Mot_Dir = 0 ;
byte Mot_Ck = 0;
volatile byte Mot_Off = 1;
volatile byte Jed_Mot_Off = 1;
volatile word Freq;
byte refChaufPc ; // ref pour la Chauffe en Auto du PC PWM du chauffage fil 0 à 100
int refChaufManu ; // ref pour la Chauffe en Manuel PWM du chauffage fil 0 à 100
int refChaufCut ; // ref pour la Chauffe en Manuel PWM du chauffage fil 0 à 100

int lastModeA ;
int lastModeB ;
int curModeA ;
int curModeB ;
int encPos ;
int encPosLast ;
int changement ;
byte horEnc;

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

byte C_Pwm = 8; //Sortie de cde de chauffage di fil
byte Cut_Pwm = 9; //Sortie de cde de chauffage di fil
byte Int_PcManu = 40; //Interrupteur de sélection PC ou Manu "0" PC
byte Int_Mot = A9; //Interrupteur des moteur PAP "0" ON
byte Int_Chauf = 44; //Interrupteur de Chauffage fil "0" PC
byte Int_Chauf_M = A10; //Interrupteur de Chauffage Manuel fil "0" M
byte Int_Chauf_Cut = A12; //Interrupteur de Chauffage Dutter "0" ON
byte Rel_Chauf = 10; //Interrupteur de Chauffage fil "0" ON
byte fdc_mini = 14; //Somme des 4 fdc mini en série "0" fdc non sollicité
byte BP_Shunt_fdc = 42 ; // Bouton poussoir de shunt fin de course pour redémarrer

int Pot_Fil = A5 ; //potentiomètre chauffage fil
int ana_Pot_Chauf_M = 0; //stock la valeur lue
int Pot_Cutter = A11 ; //potentiomètre chauffage cutter électrique
int ana_Pot_Cutter = 0; //stock la valeur lue

/**********************************************************************************/
void setup()
{


  //Initialisation Panel
  //pinMode(BUZZER_DIO, OUTPUT);
  pinMode(BP_EN, INPUT);
  digitalWrite(BP_EN, HIGH);

 // pinMode(BP_STOP, INPUT);
 // digitalWrite(BP_STOP, HIGH);
  pinMode(ROT_EN_A, INPUT);
  pinMode(ROT_EN_B, INPUT);
  digitalWrite(ROT_EN_A, HIGH);
  digitalWrite(ROT_EN_B, HIGH);
  lastModeA  = LOW;
  lastModeB  = LOW;
  curModeA  = LOW;
  curModeB  = LOW;
  encPos  = 120; // positionnement à 30% de refChaufManu (120:4)
  encPosLast  = 0;
  horEnc = 1 ;

  refChaufPc = 0 ; //
  refChaufManu = 0;


  Serial.begin(BAUDRATE);    // opens serial port, sets data rate to 115200 bps

  pinMode(13, OUTPUT);     // Led Pin used for signaling an underflow condition

  pinMode(buzzer, OUTPUT);


  pinMode(Rel_Chauf, OUTPUT);      // PD2, Heat Relay on/off low active for letmathe mdlcnc
  digitalWrite(Rel_Chauf, LOW);   // Off

  pinMode(C_Pwm, OUTPUT);      // PWM for wire heating
  analogWrite(C_Pwm, 0);      // Off

  pinMode(Cut_Pwm, OUTPUT);      // PWM pour le cutter
  analogWrite(Cut_Pwm, 0);      // Off

  pinMode(Pot_Fil, INPUT); // entrée analogique Potentiomètre Fil
  pinMode(Pot_Cutter, INPUT); // entrée analogique Potentiomètre cutter

  pinMode (Int_PcManu, INPUT); //Pin Interrupteur PC/Manu "0" PC
  digitalWrite(Int_PcManu, HIGH); // Mise en servive Pullup

  pinMode (Int_Mot, INPUT);  //Pin Interrupteur Moteur PAP "0" ON
  digitalWrite(Int_Mot, HIGH); // Mise en servive Pullup

  pinMode (Int_Chauf, INPUT); //Pin Interrupteur Chauffage Fil "0" ON
  digitalWrite(Int_Chauf, HIGH); // Mise en servive Pullup

  pinMode (Int_Chauf_M, INPUT); //Pin Interrupteur Chauffage Manuel Fil "0" ON
  digitalWrite(Int_Chauf_M, HIGH); // Mise en servive Pullup

  pinMode (Int_Chauf_Cut, INPUT); //Pin Interrupteur Chauffage Manuel Fil "0" ON
  digitalWrite(Int_Chauf_Cut, HIGH); // Mise en servive Pullup

  pinMode (fdc_mini, INPUT); //Pin somme des 4 fdc mini "0" non sollicité
  digitalWrite(fdc_mini, HIGH); // Mise en servive Pullup

  pinMode (BP_Shunt_fdc, INPUT); //Pin somme des 4 fdc mini "1" non sollicité
  digitalWrite(BP_Shunt_fdc, HIGH); // Mise en servive Pullup

  // Driver Motor Pins
  pinMode(X1_Step, OUTPUT);
  pinMode(X2_Step, OUTPUT);
  pinMode(Y1_Step, OUTPUT);
  pinMode(Y2_Step, OUTPUT);
  pinMode(Test_Step, OUTPUT);
  digitalWrite(Test_Step, LOW);

  pinMode(X1_Dir, OUTPUT);
  pinMode(X2_Dir, OUTPUT);
  pinMode(Y1_Dir, OUTPUT);
  pinMode(Y2_Dir, OUTPUT);

  pinMode(X1_En, OUTPUT);
  digitalWrite(X1_En, HIGH);       // forcé à Off
  pinMode(X2_En, OUTPUT);
  digitalWrite(X2_En, HIGH);       // forcé à Off
  pinMode(Y1_En, OUTPUT);
  digitalWrite(Y1_En, HIGH);       // forcé à Off
  pinMode(Y2_En, OUTPUT);
  digitalWrite(Y2_En, HIGH);       // forcé à Manu

  // Message de bienvenue
  lcd.begin(COLL, LIGNE);
  lcd.print (" Jedicut-Alden-USB ");
  testPosIntDem();
  lcd.setCursor(0, 1);
  lcd.print ("Jedicut-Alden v2.4.1");
  lcd.setCursor(7, 2);
  lcd.print (BAUDRATE);
  lcd.setCursor(5, 3);
  lcd.print ((ON_BUZZER) ==1 ?"BUZZER ON":"BUZZER OFF");
  delay(4000);
  lcd.clear(),
  lcd.setCursor(0, 0);
  lcd.print("mm/step "+ String(PAS_STEP,4));
  lcd.setCursor(0, 1);
  lcd.print ("Lim Chauffe = "+ String(LIM_P_FIL,DEC)+" %");
  lcd.setCursor(0, 2);
  lcd.print ("Lim Cutter = " + String(LIM_P_CUTTER,DEC) + " %");
  lcd.setCursor(0, 3);
  lcd.print ((POT_CHAUF) ==1?"Potentiometre Chauf.":"Encodeur Chauffe");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  if ((ON_BUZZER) == 1)
  {
    lcd.setCursor(0, 2);
  lcd.print("Test du Buzzer   ");
  delay( 1000); // le temps de lire le message
  alarmSonor();
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print (" Jedicut-Alden-USB ");  
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print("Mode  Mot  Chau  Cut");
  lcd.setCursor(3, 3);
  lcd.print("%          %    %");

  affIntLcd(); // Affichage état des interrupteur PC/Manu  et Moteur PAP ON/OFF
  affIntChauf();// IAffichage Chauffage fil ON/OFF



  // Initialise Timer5 e, mode 4 CTC
noInterrupts (); // Désactiver toutes les interruptions 
TCCR5A = 0; 
TCCR5B = 0; 
TCNT5 = 0;

OCR5A = 255; // Comparer registre de concordance 16MHz / 256 / 2Hz 
TCCR5B |= (1 << WGM12); // CTC mode
TCCR5B |= (1 << CS12); // 256 prescaler
TIMSK5 |= (1 << OCIE1A); // enable timer compare interrupt
interrupts (); // Activer toutes les interruptions     
 sei();

}
/*********************************************************************************/
void testPosIntDem() // au demarrage test de la position des interrupteurs
{
  lcd.setCursor(0, 2);
  lcd.print("Test des inter.");
  if (digitalRead (Int_PcManu) == LOW) // test si l'inter Mode est sur Manu
  {
    lcd.setCursor(0, 3);
    lcd.print("Mettre Mode en manu");// attente inter Mode sur Manu
    do
    {

    } while (digitalRead (Int_PcManu) == LOW);
  }

  if (digitalRead (Int_Mot) == LOW) // test si l'inter Moteur est sur OFF
  {
    lcd.setCursor(0, 3);
    lcd.print("Mettre Mot. sur OFF"); // attente inter Moteur sur OFF
    do
    {

    } while (digitalRead (Int_Mot) == LOW);
  }
  if ((digitalRead (Int_Chauf) == LOW)| (digitalRead (Int_Chauf_M) == LOW))// test si l'inter Chauffe est sur OFF
  {
    lcd.setCursor(0, 3);
    lcd.print("Mettre Chauf sur OFF"); // attente inter Chauffe sur OFF
    do
    {

    } while ((digitalRead (Int_Chauf) == LOW)| (digitalRead (Int_Chauf_M) == LOW));

  }

    if (digitalRead (Int_Chauf_Cut) == LOW) // test si l'inter Moteur est sur OFF
  {
    lcd.setCursor(0, 3);
    lcd.print("Mettre Cut sur OFF"); // attente inter Moteur sur OFF
    do
    {

    } while (digitalRead (Int_Chauf_Cut) == LOW);
  }
  
    lcd.setCursor(0, 2);
  lcd.print("                    ");// effacement de la ligne
    lcd.setCursor(0, 3);
  lcd.print("                    ");// effacement de la ligne
  
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
  digitalWrite(X1_En, Mot_Off); // Cde Driver X1
  digitalWrite(X2_En, Mot_Off); // Cde Driver X2
  digitalWrite(Y1_En, Mot_Off); // Cde Driver Y1
  digitalWrite(Y2_En, Mot_Off); // Cde Driver Y2
}

/**********************************************************************************/
void ValCommDir()
{
  digitalWrite(X1_Dir, (Mot_Dir & (1 << 4))); // direction de X1
  digitalWrite(X2_Dir, (Mot_Dir & (1 << 5))); // direction de X2
  digitalWrite(Y1_Dir, (Mot_Dir & (1 << 6))); // direction de Y1
  digitalWrite(Y2_Dir, (Mot_Dir & (1 << 7))); // direction de Y2
}

/**********************************************************************************/
void ValCommStep()
{
  digitalWrite(X1_Step, (Mot_Ck & (1 << 0))); // Step de X1
  digitalWrite(X2_Step, (Mot_Ck & (1 << 1))); // Step de X2
  digitalWrite(Y1_Step, (Mot_Ck & (1 << 2))); // Step de Y1
  digitalWrite(Y2_Step, (Mot_Ck & (1 << 3))); // Step de Y2
  digitalWrite(Test_Step, (Mot_Ck & (1 << 0))); // Test_Step
}

/**********************************************************************************/
void arretPropreMachine()
{
  Jed_Mot_Off = 1;
  Gest_Cde_Mot_ON();
  Mot_Ck = 0;
  ValCommStep();
  refChaufPc = 0;
  analogWrite(C_Pwm, refChaufPc); // PWM Arret
  lcd.setCursor(0, 3);
  if (refChaufPc < 100) lcd.print(" ");
  if (refChaufPc < 10) lcd.print(" ");
  lcd.print (refChaufPc, DEC);
  digitalWrite(Rel_Chauf, LOW );
}



/**********************************************************************************/
void handleCommand()
{

  byte val = cmdArray[arrayIdxRead + 1]; // The command parameter value
  switch (cmdArray[arrayIdxRead])
  {
    case 'A':   // All Motors on/off
      if (val == '1' & (digitalRead (Int_Mot) == LOW))  {
        Jed_Mot_Off = 0; // "0" déblocage driver
      }
      else          {
        Jed_Mot_Off = 1; // "1" blocage driver
      }
      Gest_Cde_Mot_ON();

      break;
    case 'H':   // Wire Heat ON/OFF (may be programmed as PWM (analog out))
      refChaufPc = val;
      
      
      if (digitalRead (Int_Chauf) == HIGH)
      {
        refChaufPc = 0;
      }
      if (refChaufPc > 0) {
        digitalWrite(Rel_Chauf , HIGH) ;
        if (refChaufPc > LIM_P_FIL) refChaufPc = LIM_P_FIL ;
      } else {
        digitalWrite(Rel_Chauf, LOW );
      }
      analogWrite(C_Pwm, refChaufPc * 2.55); // PWM for wire heating (stretch 0-100% to a range of 0-255)*/
      lcd.setCursor(0, 3);
      if (refChaufPc < 100) lcd.print(" ");
      if (refChaufPc < 10) lcd.print(" ");
      lcd.print (refChaufPc, DEC);
      
      break;

    case 'M':   // Motor step Command
      sendMotorCmd(val);

      break;

    case 'F':   // Changer la fréquence du temps, le temps écoulé entre deux steps
      // OCR1A values 255 = 250Hz 190 = 328Hz 127 = 500Hz 63 = 1 kHz 31 = 2KHz 15 = 4 kHz
      if (val > 255) val = 255; // restrict from 1 à 255 corresponds  à 250Hz  à 20 kHz
      if (val < 1) val = 1; // restrict from 1 à 255 corresponds  à 250Hz  à 20 kHz
      OCR5A = val;
      Freq = 65535 / (1+(val));
      float MMS1 = Freq * PAS_STEP;
      lcd.setCursor(6, 3);
      if (MMS1 < 10) lcd.print(" ");
      
      lcd.print(MMS1, 1);//affichage mm/s sur LCD Avec i1 chiffre après la virgule
  
      break;
  }

}


/**********************************************************************************/
void alarmSonor()
{
  if ((ON_BUZZER) ==1)
  {
    for (int k = 0; k < 20; k++) // Premier son à une fréquence
    {
      digitalWrite(buzzer, HIGH);// Faire du bruit
      delay(10);// Attendre 10ms
      digitalWrite(buzzer, LOW);// Silence
      delay(10);// Attendre 10ms
    }
    for (int k = 0; k < 40; k++) // Premier son à une fréquence
    {
      digitalWrite(buzzer, HIGH);// Faire du bruit
      delay(10);// Attendre 10ms
      digitalWrite(buzzer, LOW);// Silence
      delay(10);// Attendre 10ms
    }
  }
}

/**********************************************************************************/
void affIntLcd() // Affichage des interrupteurs des fins de course
{
  lcd.setCursor(0, 2);
  if (digitalRead (Int_PcManu) == HIGH)
  {
    lcd.print("Manu");
  }
  else
  {
    lcd.print(" PC ");
  }
  lcd.setCursor(6, 2);
  if (digitalRead (Int_Mot) == HIGH)
  {
    lcd.print("OFF");
  }
  else
  {
    lcd.print(" ON");
  }
  lcd.setCursor(5, 2);
  if (digitalRead (fdc_mini) == LOW)
  {
    lcd.print("I");
  }
  else
  {
    lcd.print("K");
  }
  if (digitalRead (BP_Shunt_fdc) == LOW)
  {
    lcd.setCursor(5, 2);
    lcd.print("I");
  }
}


/*********************************************************************************/
void affIntChauf () // Affichage de l'état des interrupteurs de chauffe et Puissance
{
  lcd.setCursor(12, 2);
  if ((digitalRead (Int_Chauf) == HIGH)& (digitalRead (Int_Chauf_M) == HIGH))
  {
    lcd.print("OFF");
  }
  else
  {
    if ((digitalRead (Int_Chauf) == LOW)& (digitalRead (Int_Chauf_M) == HIGH))
    {
      lcd.print(" PC");
    }
    else
    {
      lcd.print("Man"); 
    }
  }
  lcd.setCursor(11, 3);
  if (refChaufManu < 100) lcd.print(" ");
  if (refChaufManu < 10) lcd.print(" ");
  lcd.print (refChaufManu, DEC);

  lcd.setCursor(17, 2);
  if ((digitalRead (Int_Chauf_Cut) == HIGH))
  {
    lcd.print("OFF");
  }
  else
  {
    lcd.print(" ON");
  }
  lcd.setCursor(16, 3);
  if (refChaufCut < 100) lcd.print(" ");
  if (refChaufCut < 10) lcd.print(" ");
  lcd.print (refChaufCut, DEC);
}

/**********************************************************************************/
void gestFilManu () 
{
  if ((digitalRead (Int_PcManu) == HIGH) & (digitalRead (Int_Chauf_M) == LOW)) 
  {
    if (refChaufManu > 0) 
    {
      digitalWrite(Rel_Chauf , HIGH) ;
    } else 
    {
      digitalWrite(Rel_Chauf, LOW );
    }
    analogWrite(C_Pwm, refChaufManu * 2.55); // PWM for wire heating (stretch 0-100% to a range of 0-255)*/
  } else 
  {
    analogWrite(C_Pwm, 0);
    digitalWrite(Rel_Chauf, LOW );
  }
}

/**********************************************************************************/
void gestChaufCutter () 
{
  if ((digitalRead (Int_PcManu) == HIGH) & (digitalRead (Int_Chauf_Cut) == LOW)) 
  {
    analogWrite(Cut_Pwm, refChaufCut * 2.55); // PWM for wire heating (stretch 0-100% to a range of 0-255)
  } else 
  {
    analogWrite(Cut_Pwm, 0);
  }
}
/**********************************************************************************/
void PwmFilManu() 
{
  if (digitalRead (Int_Chauf_M) == HIGH) 
  {
    refChaufManu = 0;
  } else 
  {
    analogWrite(C_Pwm, refChaufManu * 2.55); // PWM for wire heating (stretch 0-100% to a range of 0-255)*/
  }
}

/**********************************************************************************/
void Mes_Pot_Fil() 
{
  ana_Pot_Chauf_M = analogRead (Pot_Fil);
  refChaufManu = map (ana_Pot_Chauf_M, 0, 1023, 0, LIM_P_FIL);
}

/**********************************************************************************/
void Mes_Pot_Cutter() 
{
  ana_Pot_Cutter = analogRead (Pot_Cutter);
  refChaufCut = map (ana_Pot_Cutter, 0, 1023, 0, LIM_P_CUTTER);
}

/*********************************************************************************/
void encRotatif () 
  {
    lcd.setCursor(10, 3);
    lcd.print(">");
    boolean sortie = true;

    do
    {
      // Lire les valeurs actuelles
      // Définir le changement de variable à 0. S'il y a un changement de position du codeur,
      // Ceci est changé à 1. Cela permet une partie ultérieure de la boucle
      // Une modification a été apportée et il n'a pas à comparer tous les modes
      // encore.
      changement = 0;

      // Lire l'état actuel des broches du codeur courant
      curModeA  = digitalRead (ROT_EN_A );
      curModeB  = digitalRead (ROT_EN_B );
      // Comparer les quatre états possibles pour comprendre ce qui est arrivé
      // Puis encrement / décrémenter la position du capteur de courant
      if (curModeA != lastModeA ) 
      {
        if (curModeA == LOW) 
        {
          if (curModeB == LOW) 
          {
            encPos --;
          } else 
          {
            encPos ++;
          }
        } else 
        {
          if (curModeB  == LOW) 
          {
            encPos ++;
          } else 
          {
            encPos --;
          }
        }
      }
      if (curModeB != lastModeB ) 
      {
        if (curModeB  == LOW) 
        {
          if (curModeA  == LOW) 
            {
            encPos  ++;
            } else
            {
              encPos --;
            }
          } else 
          {
            if (curModeA  == LOW) 
            {
              encPos --;
            } else 
            {
              encPos ++;
            }
          }
        }

        // Définir les modes de broche actuels (HAUT / BAS) pour être les modes de broches dernière des savoir
        // Pour la boucle suivante à comparer à
        lastModeA = curModeA ;
        lastModeB = curModeB ;
        // Si la position de ce codeur a changé, drapeau de la variable de changement afin que nous
        // Savoir à ce sujet plus tard
        if (encPos != encPosLast ) 
        {
        changement = 1;
      }
      if (changement == 1) 
      {
        // Si un codeur a changé, faire quelque chose avec cette information
        // Ici, je vais juste imprimer toutes les positions de l'encodeur
        // Si aucun d'entre eux le changement
        if (encPos < 0) encPos = 0;
        if (encPos > (LIM_P_FIL*4)) encPos = (LIM_P_FIL*4);
        refChaufManu = encPos / 4;
        lcd.setCursor(11, 3);
        if (refChaufManu < 100) lcd.print(" ");
        if (refChaufManu < 10) lcd.print(" ");
        lcd.print (refChaufManu, DEC);
        encPosLast  = encPos ;
      }
      if (digitalRead (BP_EN) == 0) 
      {
        delay(5);
        do 
        {

        } while (digitalRead (BP_EN) == 0);
        sortie = false ;
      }
    } while (sortie);
  lcd.setCursor(10, 3);
  lcd.print(" ");
}

/**********************************************************************************/
ISR(TIMER5_COMPA_vect) {

  if (isrActive) return;
  isrActive = true;
  sei(); // reenable interrupts, to not miss serial data
  do
  {
    // check if the buffer is empty
    if ((arrayIdxRead != arrayIdxWrite) || ovf)
    {
      handleCommand();
      arrayIdxRead += 2;
      if (arrayIdxRead == CMD_BUFFER_SIZE) arrayIdxRead = 0;
      noInterrupts();
      cmdCounter--;
      interrupts();
      if (ovf && (cmdCounter < CMD_BUFFER_SIZE / 2 - 25))
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
  } while (cmdArray[arrayIdxRead] != 'M'); // only motor commands will wait for next sync, all others can be handled immediately

  cli();

  isrActive = false;
}

/**********************************************************************************/
/**** The main loop                                                           *****/
/**********************************************************************************/
void loop()
{
  if ( digitalRead (Int_PcManu) == LOW)
  {
    affIntLcd();
    affIntChauf ();
    interrupts(); //réarmement des interruptions
      do
      {
        if (Serial.available() > 0)
        {
        // Each command consists of 2 bytes
         Serial.readBytes((char*)&cmdArray[arrayIdxWrite], 2);

        // korrekt the write index
        arrayIdxWrite += 2;
          if (arrayIdxWrite == CMD_BUFFER_SIZE) arrayIdxWrite = 0;
        noInterrupts();
        cmdCounter++;
        interrupts();
        // check for oncoming overflow
          if (cmdCounter >= CMD_BUFFER_SIZE / 2 - 20)
          {
           ovf = true;
           Serial.write('S'); // Stop transmission, Buffer full
          }
        }
      }
      while ((digitalRead (Int_PcManu) == LOW) & (digitalRead (Int_Mot) == LOW) & (digitalRead (Int_Chauf) == LOW) & ((digitalRead (fdc_mini) == LOW) | (digitalRead (BP_Shunt_fdc) == LOW)));
    }
  Serial.write('C'); // Continu transmission, afin que Jedicut termine sa séquence sans se bloquer
  noInterrupts(); //Arrêt des interruptions
  arretPropreMachine();
  affIntLcd();
  affIntChauf ();
 //Boucle Manuelle
  while ((digitalRead (Int_PcManu) == HIGH) | (digitalRead (Int_Mot) == HIGH) | (digitalRead (Int_Chauf) == HIGH) | ((digitalRead (fdc_mini) == HIGH) & (digitalRead (BP_Shunt_fdc) == HIGH)) )
  {
    if (digitalRead (fdc_mini) == HIGH)
      {
      alarmSonor();
      }
    affIntLcd();
    if (POT_CHAUF == 0) // Choix entre Potentiomètre ou encodeur
    {
      if (digitalRead (Int_PcManu) == HIGH)
        {
        if (digitalRead (BP_EN) == 0)
          {
          delay(5);
          do
            {

            } while (digitalRead (BP_EN) == 0);//attente relâchement BP

          encRotatif ();
          }
        }
      }
    else
    {  
      Mes_Pot_Fil ();
    }  
    Mes_Pot_Cutter ();
    affIntChauf ();
    gestFilManu ();
    gestChaufCutter ();

  } 
  affIntLcd();
  Serial.flush(); // Effacement du buffer
  interrupts(); //réarmement des interruptions
}

