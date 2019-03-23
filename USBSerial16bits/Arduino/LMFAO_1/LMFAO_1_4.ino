/* ========================================================================== */
/*                                                                            */
/*   LMFAO_1_4.ino                                                            */
/*   (c) 2019                                                                 */
/*                                                                            */
/*   Description Sketch pour electronique jedicut-Alden simple + affichage.   */
/*   Contributeurs : 2013 Martin  Principe de l'échange avec Jedicut          */                                                         
/*                   2016 Alain Adaptation Arduino Mega + Ramps 1.4           */
/*                   2017 Vincent Modif USBSerial.dll                         */
/*                   2018 Olivier Suppression des bibliothèques serial USB    */
/*                   2019 Alain Ajout des options                             */
/* ========================================================================== */
/*
 Copyright 2013 Martin
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
The jedicut communication configuration for Clock and Direction must be fixed 
set to the below values, only the wires to the specific board may be different -- wire to -->

Le 18/09/2016 commencé assai avec carte arduino mega 2560 modifier le pins,
pris le port C au lieu du port D .
Le 20/09/2016 modification du code pour avoir les bonnes bornes de sorties
 pour Ramps1.4
Le 21/09/2016 Mise en place de L'afficheur LCD
Le 04/01/2017 Ajout affichage Config
le 30/11/2017 Adaptation du sketch suit à nouveau Plugin USBSerial_2.dll compatible
  avec les entraînements à courroie passer 16 bits la commande de vitesse pour 
  élargissement de la plage de vitesse dans la config de Jedicut.
Le 14/02/2018 Ajout d'un delayMs après la fin du step v1-2-2-1
Le 15/06/2018 Suite à un problème de confusion entre v1.2.2.1 et v1.2.2
 la version 1.2.2.1 devient v1.2.3
Le 19/02/2019 Repris toute la partie communication USB sans passer par les 
bibliothèques Arduino.
Ajout de l'option moteur toujours sous tension pour les machines à couroies
Ajout de l'option de marche drivers par un 1 ao lieu d'un 0 pour les drivers
autres que ceux de la ramp1.4.
Ajout de la correction de la chauffe dans les courbes ou les diagonales.
Version au  9/02/2019 LMFAO_1_4
------------------------------------------------------------------------------- 
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

/* External library */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <LiquidCrystal.h>
#include <avr/iom2560.h>

/* Internal header */
#include "conf.h"
#include "lang.h"


/* DEFINE */


#if BAUDRATE == 9600
	#define UBRR0_BAUDRATE_VALUE 207 // Value for USART clock register for 9600 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#elif  BAUDRATE == 14400
	#define UBRR0_BAUDRATE_VALUE 138 // Value for USART clock register for 14400 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#elif BAUDRATE == 19200
	#define UBRR0_BAUDRATE_VALUE 103 // Value of USART clock register for 19200 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#elif BAUDRATE == 38400
	#define UBRR0_BAUDRATE_VALUE 51 // Value of USART clock register for 38400 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#elif BAUDRATE == 57600
	#define UBRR0_BAUDRATE_VALUE 34 // Value of USART clock register for 57600 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#elif BAUDRATE == 115200
	#define UBRR0_BAUDRATE_VALUE 16	// Value of USART clock register for 115200 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#elif BAUDRATE == 250000
	#define UBRR0_BAUDRATE_VALUE 7	// Value of USART clock register for 250000 Baud with Fosc = 16MHz (cf. Atmega2560 datasheet)
#else
	#error "Missing or invalide Baudrate value in Conf.h" // Baud rate value configured with wrong value or missing declaration in Conf.h
#endif




#define PIN_DEBUG1 13
#define PIN_DEBUG2 19
#define PIN_DEBUG3 6
#define PIN_DEBUG4 7


// stepper driver
#define DRIVER_X1_DIR_MASK 0x10			// Direction mask
#define DRIVER_X2_DIR_MASK 0x20
#define DRIVER_Y1_DIR_MASK 0x40
#define DRIVER_Y2_DIR_MASK 0x80

#define DRIVER_X1_STEP_MASK 0x01		// Step Mask
#define DRIVER_X2_STEP_MASK 0x02
#define DRIVER_Y1_STEP_MASK 0x04
#define DRIVER_Y2_STEP_MASK 0x08

#define PIN_X1_DIR A1  					// PF1, X1 Direction
#define PIN_X2_DIR A7  					// PF7, X2 Direction
#define PIN_Y1_DIR 28  					// PA6, Y1 Direction.
#define PIN_Y2_DIR 34  					// PC3, Y2 Direction

#define PIN_X1_STEP A0  				// PF0, X1 Step
#define PIN_X2_STEP A6  				// PF6, X2 Step
#define PIN_Y1_STEP 26  				// PA4, Y1 Step
#define PIN_Y2_STEP 36  				// PC1, Y2 Step
#define PIN_TEST_STEP 11 				// Interrupt frequency test pin

#define PIN_X1_EN 38  					// PD7, X1 Enable
#define PIN_X2_EN A2  					// PF2, X2 Enable
#define PIN_Y1_EN 24  					// PA2, Y1 Enable
#define PIN_Y2_EN 30  					// PC7, Y2 Enable

#define MAX_FEEDRATE_TIMER_VALUE 32767	// Maximum Timer Period for low speed
#define MIN_FEEDRATE_TIMER_VALUE 25		// Minimum Timer Period for high speed

#define MM_PER_SECONDE 62500.0 * MM_PER_STEP

// LCD
#define PIN_LCD_RS 16      				// LCD control and is connected into GADGETS3D  shield LCDRS
#define PIN_LCD_E 17       				// LCD enable pin and is connected into GADGETS3D shield LCDE
#define PIN_LCD_D4 23      				// LCD signal pin, connected to Gadgets3D shield LCD4
#define PIN_LCD_D5 25      				// LCD signal pin, connected to Gadgets3D shield LCD5
#define PIN_LCD_D6 27     	 			// LCD signal pin, connected to Gadgets3D shield LCD6
#define PIN_LCD_D7 29      				// LCD signal pin, connected to Gadgets3D shield LCD7
#define LCD_COLUMN_COUNT 20  			// 20 character per line
#define LCD_LINE_COUNT 4     			// 4 lines
#define LCD_NEW_DIGIT_DISPLAY_COUNT 6	// Number of timer 2 interrupt to generate a new LCD display

// PWM
#define PIN_WIRE_PWM 8 //Sortie de cde de chauffage du fil

#define PIN_RELAY_HEATING 10 //Relais de Chauffage fil "0" ON

// Buffer

// Command Buffer
#define CMD_DATA_SIZE 4                                                         // Size of the Command data in byte (= 4 byte) !! Shall be even number (2, 4, 6, etc..) !!
#define CMD_BUFFER_SIZE 260                                                     // Number of Command the Buffer can stock
#define CMD_BUFFER_MAX 255

//Communication Buffer
#define COM_BUFFER_SIZE 260
#define COM_BUFFER_OVERFLOW_TRIGGER 175 //
#define COM_BUFFER_UNDERFLOW_TRIGGER 75 //

#define TWO_BYTE_CMD 2
#define THREE_BYTE_CMD 3

// Utility
#define ON true
#define OFF false
#define HIGH_BYTE 1
#define LOW_BYTE 0


#define ENABLE_RX_ISR() UCSR0B |= 0x90
#define ENABLE_T1_ISR() TIMSK1 = 0x02
#define ENABLE_T5_ISR() TIMSK5 = 0x02
#define ENABLE_T2_ISR() TIMSK2 = 0x02

#define DISABLE_RX_ISR() UCSR0B &= 0x6F
#define DISABLE_T1_ISR() TIMSK1 = 0x00
#define DISABLE_T5_ISR() TIMSK5 = 0x00
#define DISABLE_T2_ISR() TIMSK2 = 0x00

#define TX_WRITE(x) UDR0 = x
#define RX_READ(x) x = UDR0

#define ENABLE_T2_COMP_OUTPUT_B() TCCR2A |= 0x20
#define DISABLE_T2_COMP_OUTPUT_B() TCCR2A &= 0xCF
#define ENABLE_T4_COMP_OUTPUT_C() TCCR4A |= 0x08
#define DISABLE_T4_COMP_OUTPUT_C() TCCR4A &= 0xF3

//LCD declaration
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

volatile bool toogle = true;
bool StepperDriverEnableMode = STEPPER_DRIVER_ENABLE_HIGH_LEVEL;

const bool StepperDriverEnableTab[2][2] = { {ON, ON}, {OFF, ON} };
const bool StepperDriverEnableState[2] = {STEPPER_DRIVER_ENABLE_LOW_LEVEL, STEPPER_DRIVER_ENABLE_HIGH_LEVEL};

enum Mode
{
	MODE_INIT = 0,
	MODE_MANU,
	MODE_PC
};

enum ParseState
{
	PARSER_STATE_CMD = 0,
	PARSER_STATE_DATA,
	PARSER_STATE_WRITE_CMD_BUFFER
};

union CmdData
{
	unsigned int ui16;
	unsigned char ui8[2];
};

typedef struct TypeBuffer
{
	unsigned char Cmd;
	union CmdData Data;
} TBuffer;

volatile TBuffer CommandBuffer[CMD_BUFFER_SIZE];
volatile byte CommandIndexRead = 0;
volatile byte CommandIndexWrite = 0;
volatile byte CommandCounter = 0;

byte ComOverflow = 0;

volatile byte ParserState = PARSER_STATE_CMD;

volatile struct StructPC
{
  bool Motor = OFF;
  byte Heat = 0;
  unsigned int Feedrate = 0;
} PC;



struct StructHeating
{
	byte WireConsign = 0;
	byte CutterConsign = 0;
  byte WireDynamique = 0;
} Heat;

enum HMI_State
{
	HMI_INIT_SCREEN = 0,
	HMI_INIT_DELAY,
	HMI_PARAMS_SCREEN,
	HMI_PARAMS_DELAY,
	HMI_SWITCH_SCREEN,
	HMI_MODE_SCREEN
};

struct StructHMI
{
	bool ProcessDigit = false;
	byte State = HMI_INIT_SCREEN;
	byte WireConsign = 0;
	byte CutterConsign = 0;
	unsigned int Feedrate = 0;
} HMI;

#ifdef MACHINE_NAME
	const String MachineName = String(MACHINE_NAME);
#else
	const String MachineName = " JediCut-Alden USB";
#endif

unsigned char MachineNameChar[LCD_COLUMN_COUNT];

//Pour chauffe dynamique du fil
bool WireVitesse = 0 ; //ordre de correction de vitesse

// Variable init timer4
  int maRaz ; 
  int maPreset ;

const String Version = VERSION;





//==============================================================================
//Setup
//==============================================================================
void setup (void)
{
	//serial com Initialization

	UBRR0 = UBRR0_BAUDRATE_VALUE; 	// Configure USART Clock register
	UCSR0A = 0x02;					//
	UCSR0B = 0x18;					//
	UCSR0C = 0x06;					//

  // Heating
	pinMode(PIN_RELAY_HEATING, OUTPUT); // PD2, Heat Relay on/off low active for letmathe mdlcnc
	digitalWrite(PIN_RELAY_HEATING, LOW); // Off

	pinMode(PIN_WIRE_PWM, OUTPUT); // PWM pin for wire heating


  	pinMode(PIN_X1_EN, OUTPUT);
	digitalWrite(PIN_X1_EN, STEPPER_DRIVER_ENABLE_LOW_LEVEL);      // Disable Driver X1
	pinMode(PIN_X2_EN, OUTPUT);
	digitalWrite(PIN_X2_EN, STEPPER_DRIVER_ENABLE_LOW_LEVEL);      // Disable Driver X2
	pinMode(PIN_Y1_EN, OUTPUT);
	digitalWrite(PIN_Y1_EN, STEPPER_DRIVER_ENABLE_LOW_LEVEL);      // Disable Driver Y1
	pinMode(PIN_Y2_EN, OUTPUT);
	digitalWrite(PIN_Y2_EN, STEPPER_DRIVER_ENABLE_LOW_LEVEL);      // Disable Driver Y2
  
	pinMode(PIN_X1_STEP, OUTPUT);
	pinMode(PIN_X2_STEP, OUTPUT);
	pinMode(PIN_Y1_STEP, OUTPUT);
	pinMode(PIN_Y2_STEP, OUTPUT);
	pinMode(PIN_TEST_STEP, OUTPUT);
	digitalWrite(PIN_TEST_STEP, LOW);

	pinMode(PIN_X1_DIR, OUTPUT);
	pinMode(PIN_X2_DIR, OUTPUT);
	pinMode(PIN_Y1_DIR, OUTPUT);
	pinMode(PIN_Y2_DIR, OUTPUT);


  	// Timer 1 init, used to parse the communication buffer and fill the command buffer
	TIMSK1 = 0x00;	// Reset interrupt flag
	TIFR1 = 0x00;	// Interrupt disabled, enabled only when there is command to execute
	TCCR1A = 0x00;	// configure timer in Clear Timer on Compare mode, disable Output Compare pins
	TCCR1B = 0x0C;	// configure timer in Clear Timer on Compare mode and set prescaler to 256
	TCCR1C = 0x00;
	OCR1A = 1;		// Set Top value => Fosc = 16MHz (62.5 ns), Prescaler = 256, Compare value = 1 -> Fpwm = 1 / ((1 + 1) * 256 * 62.5e-9) ~=  KHz

	// Timer 2 init, used for Cutter Heat PWM as hardware PWM generator and as Buzzer sound generator on timer counter overflow
	TIMSK2 = 0x00;	// Reset interrupt flag
	TIFR2 = 0x00;	// disable interrupt
	TCCR2A = 0x03;	// configure timer in Fast PWM mode, disable Output Compare pin A and enable Output Compare pin B
	TCCR2B = 0x0E;	// configure timer in Fast PWM mode and set prescaler to 256
	OCR2A = 100;	// Set Top value => Fosc = 16MHz, Prescaler = 256, Compare value = 100 -> Fpwm = 1 / (100 * 256 * 62.5e-9) = 625 Hz
	OCR2B = 0;		// Set compare value => Compare value between 0 = 0% PWM and 200 = 100% PWM



   // modification frequence PWM des pin 6, 7, 8, par le timer 4  (chauffe)
  maRaz =7; // 111 pour CS02, CS01, CS00
  maPreset = 2; //010 pour 7800 Hz ; 001 pour 62000Hz
  TCCR4B&=~ maRaz ;
  TCCR4B |= maPreset;
  
  
	// Timer 5 init, used to generate steps at a constant rate
	TIMSK5 = 0x00;	// Reset interrupt flag
	TIFR5 = 0x00;	// Interrupt disabled, enabled only when there is command to execute
	TCCR5A = 0x00;	// configure timer in Clear Timer on Compare mode, disable Output Compare pins
	TCCR5B = 0x0C;	// configure timer in Clear Timer on Compare mode and set prescaler to 256
	TCCR5C = 0x00;
	OCR5A = 255;	// Set Top value => Fosc = 16MHz (62.5 ns), Prescaler = 256, Compare value = 255 -> Fpwm = 1 / (255 * 256 * 62.5e-9) =  245 Hz

	
	MachineName.getBytes(MachineNameChar, LCD_COLUMN_COUNT);


}
//==============================================================================
// RAZ Buffer
//==============================================================================
void BufferFlush (void)
{
	CommandCounter = 0;
	ComOverflow = false;
	TX_WRITE('C');
	CommandIndexRead = 0;
	CommandIndexWrite = 0;
	ParserState = PARSER_STATE_CMD;
}
//------------------------------------------------------------------------------

//==============================================================================
// Stepper Driver Direction
//==============================================================================

inline void StepperDriverDir (byte dir)
{
//AL >
  dir = (dir ^ INV_DIR_MASK );
  //<
	bitWrite(PORTF, 1, (dir & DRIVER_X1_DIR_MASK));
	bitWrite(PORTF, 7, (dir & DRIVER_X2_DIR_MASK));
	bitWrite(PORTA, 6, (dir & DRIVER_Y1_DIR_MASK));
	bitWrite(PORTC, 3, (dir & DRIVER_Y2_DIR_MASK));
}

//------------------------------------------------------------------------------

//==============================================================================
// Stepper Driver Step
//==============================================================================

inline void StepperDriverStep (byte step)
{
	bitWrite(PORTF, 0, (step & DRIVER_X1_STEP_MASK));
	bitWrite(PORTF, 6, (step & DRIVER_X2_STEP_MASK));
	bitWrite(PORTA, 4, (step & DRIVER_Y1_STEP_MASK));
	bitWrite(PORTC, 1, (step & DRIVER_Y2_STEP_MASK));
	bitWrite(PORTB, 5, (step & DRIVER_X1_STEP_MASK));
}

//------------------------------------------------------------------------------

//==============================================================================
// Stepper Driver Enable
//==============================================================================

inline void StepperDriverEnable (bool en)
{
	bool enable = StepperDriverEnableState[en];
  //bool enable = 0;
	StepperDriverEnableMode = en;

 
	digitalWrite(PIN_X1_EN, enable); // Cde Driver X1
	digitalWrite(PIN_X2_EN, enable); // Cde Driver X2
 	digitalWrite(PIN_Y1_EN, enable); // Cde Driver Y1
 	digitalWrite(PIN_Y2_EN, enable); // Cde Driver Y2
}

//------------------------------------------------------------------------------

//==============================================================================
// Process Stepp
//==============================================================================

inline void ProcessStep(byte cmd)
{
	StepperDriverDir(cmd);
//	delayMicroseconds(1); 		// wait 1 us before sending the steps

	StepperDriverStep(cmd);
  
  //Calcul compensation Chauffe artifice vitesse 

  if (CHAUFFE_ASSERV == 1)
  {
    WireVitesse = ((cmd & DRIVER_X1_STEP_MASK)&&(cmd & DRIVER_Y1_STEP_MASK)||(cmd & DRIVER_X2_STEP_MASK) && (cmd & DRIVER_Y2_STEP_MASK) );
  }
//	delayMicroseconds(3); 		// wait 3 us before sending falling pulse for step
	StepperDriverStep(0);
}

//------------------------------------------------------------------------------

//==============================================================================
//Relais Heating
//==============================================================================
inline void HeatingRelay (byte en)
{
	digitalWrite (PIN_RELAY_HEATING, en);
}

//==============================================================================
// ISR Timer 1
//==============================================================================

ISR(TIMER1_COMPA_vect)
{

}
//------------------------------------------------------------------------------

//==============================================================================
//ISR Timer 2 Buzzer
//==============================================================================

ISR(TIMER2_COMPA_vect)
{
//	buzzerToogle = !buzzerToogle;
//	digitalWrite(PIN_BUZZER, buzzerToogle);
}

//------------------------------------------------------------------------------

//==============================================================================
// ISR Timer 5 cadence de la vitesse
//==============================================================================

ISR(TIMER5_COMPA_vect)
{
	//digitalWrite(PIN_DEBUG3, HIGH);
	IsrProcessBuffer();
	//digitalWrite(PIN_DEBUG3, LOW);
}

//------------------------------------------------------------------------------

//==============================================================================
// ISR Liaison USB
//==============================================================================

ISR(USART0_RX_vect)
{
	//digitalWrite(PIN_DEBUG4, HIGH);
	ComParse();
	//digitalWrite(PIN_DEBUG4, LOW);
}

//------------------------------------------------------------------------------

//==============================================================================
// Heating Manage
//==============================================================================

inline void HeatingManage (void)
{	
		Heat.WireConsign = PC.Heat;
  	if (Heat.WireConsign > 0)
    	{
    		HeatingRelay( ON );
    		//ENABLE_T4_COMP_OUTPUT_C();
    	}
    	else
    	{
    		HeatingRelay( OFF );
    		//DISABLE_T4_COMP_OUTPUT_C();
    	}
      
    if ((CHAUFFE_ASSERV == 1))
    {
    if (WireVitesse == 1) //Correction de la chauffe en fonction des steps
      {
        Heat.WireDynamique= (Heat.WireConsign * CORRECT_CHAUFFE);
        if (Heat.WireDynamique > MAX_PERCENTAGE_WIRE) Heat.WireDynamique = MAX_PERCENTAGE_WIRE ;
        analogWrite(PIN_WIRE_PWM, Heat.WireDynamique * 2.55);
      }
      
      else
      {
        Heat.WireDynamique = Heat.WireConsign ;
        analogWrite(PIN_WIRE_PWM, Heat.WireDynamique * 2.55);
      }
    }
    else
    {
      analogWrite(PIN_WIRE_PWM, Heat.WireConsign * 2.55);
    }

}

//------------------------------------------------------------------------------

//==============================================================================
//Affichage accueil
//==============================================================================

inline void HMI_InitScreen (void)
{
	// Welcome text
	lcd.begin(LCD_COLUMN_COUNT, LCD_LINE_COUNT);
	lcd.print(" Jedicut-Alden-USB");
	lcd.setCursor(7, 1);
	lcd.print(Version);
	lcd.setCursor(7, 2);
	lcd.print(BAUDRATE);

}

//------------------------------------------------------------------------------

//==============================================================================
//Affichage configuration
//==============================================================================
inline void HMI_ParamsScreen (void)
{
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("mm/step " + String(MM_PER_STEP, 5));
	lcd.setCursor(0, 1);
	lcd.print (TEXT1);
  lcd.setCursor(15, 1);
  lcd.print (String(MAX_PERCENTAGE_WIRE, DEC) + "%");
	lcd.setCursor(0, 2);
  if (MOTEUR_ON_ASSERVI)
	lcd.print (TEXT2);
  else
  lcd.print (TEXT3);
	lcd.setCursor(0, 3);
	if (CHAUFFE_ASSERV)
	lcd.print(TEXT4);
	else
	lcd.print(TEXT5);


}

//------------------------------------------------------------------------------
//==============================================================================
//Preparation pour le test des switchs
//==============================================================================
inline void HMI_InitSwitchScreen (void)
{
	lcd.clear();
	lcd.setCursor(0, 1);
	lcd.print("Fin init.");
}

//-----------------------------------------------------------------------------
//==============================================================================
//Verification de la position des switchs
//==============================================================================

inline bool HMI_SwitchInitScreen (void)
{

	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(MachineName);
  lcd.setCursor(0,1);
  lcd.print("Jedicut-ALDEN ");
  lcd.setCursor(14, 1);
  lcd.print(Version); 
  lcd.setCursor(0,2);
  lcd.print(TEXT6);
  lcd.setCursor(0,3);
  lcd.print("  0.00mm/s    %     ");

	return true;
}

//------------------------------------------------------------------------------




//==============================================================================
//Mise a jour des valeurs de fonctionnement
//==============================================================================
inline void HMI_PcDigitScreen (void)
{
	char line[21] = {"  0.00mm/s    %     "};
	float mmPerSec;
	byte val, valDec;

	if((Heat.WireConsign != HMI.WireConsign) || (PC.Feedrate != HMI.Feedrate) || (HMI.ProcessDigit))
	{
		HMI.WireConsign = Heat.WireConsign;
		HMI.Feedrate = PC.Feedrate;

		if(HMI.Feedrate > 0)
		{
			mmPerSec = MM_PER_SECONDE / (float)(HMI.Feedrate);
			val = (byte)mmPerSec;
			valDec = ((unsigned int)(mmPerSec * 100.0) % 100);

			line[0] = val >= 100 ? ('0' + (val/100)) : ' ';
			line[1] = val >= 10 ? ('0' + ((val/10)%10)) : ' ';
			line[2] = '0' + (val % 10);
			line[4] = valDec >= 10 ? ('0' + ((valDec/10)%10)) : '0';
			line[5] = '0' + (valDec % 10);
		}

		line[11] = HMI.WireConsign >= 100 ? ('0' + (HMI.WireConsign/100)) : ' ';
		line[12] = HMI.WireConsign >= 10 ? ('0' + ((HMI.WireConsign/10)%10)) : ' ';
		line[13] = '0' + (HMI.WireConsign % 10);

		lcd.setCursor(0, 3);
		lcd.print(line);
		HMI.ProcessDigit = false;
	
	}
}
//------------------------------------------------------------------------------

//==============================================================================
//P�riode d'affichage
//==============================================================================

inline void HMI_DigitScreen (void)
{
	static unsigned long i = 0;

	if(i < millis())
	{
		i = millis() + 100;


			HMI_PcDigitScreen();
	}
}

//------------------------------------------------------------------------------

//==============================================================================
// Process Commandes
//==============================================================================

inline void ProcessCommand (void)
{
 
	switch(CommandBuffer[CommandIndexRead].Cmd)
	{
		case 'M':
			ProcessStep(CommandBuffer[CommandIndexRead].Data.ui8[HIGH_BYTE]);
			break;

		case 'A':   // All Motors on/off
			//digitalWrite(PIN_DEBUG1, toogle);
			toogle = !toogle;
			PC.Motor = CommandBuffer[CommandIndexRead].Data.ui8[HIGH_BYTE];
      if (MOTEUR_ON_ASSERVI)
			StepperDriverEnable(StepperDriverEnableTab[1][PC.Motor]);
     
			break;                       

		case 'F':   // Change the stepper Feedrate
			// OCR5A values 255 = 250Hz 190 = 328Hz 127 = 500Hz 63 = 1 kHz 31 = 2KHz 15 = 4 kHz
			PC.Feedrate = CommandBuffer[CommandIndexRead].Data.ui16;
			OCR5A = PC.Feedrate;
			TCNT5 = 0;      
			break;

		case 'H':   // Wire Heat ON/OFF (may be programmed as PWM (analog out))
			PC.Heat = CommandBuffer[CommandIndexRead].Data.ui8[HIGH_BYTE];
			break;

    default:
      break;
	}
}

//------------------------------------------------------------------------------

//==============================================================================
// Isr Process Buffer
//==============================================================================
inline void IsrProcessBuffer (void)
{
	while(CommandCounter != 0)
	{
		ProcessCommand();
		CommandIndexRead++;
		CommandCounter--;
		CheckComBufferUnderflow();
		if(CommandBuffer[CommandIndexRead].Cmd == 'M')
			break;
	}
}

//------------------------------------------------------------------------------
//==============================================================================
// Test Buffer Overflow
//==============================================================================

inline void CheckComBufferOverflow (void)
{
	if((CommandCounter >= COM_BUFFER_OVERFLOW_TRIGGER) && (!ComOverflow))
	{
		ComOverflow = true;
		//digitalWrite(PIN_DEBUG2, HIGH);
		TX_WRITE('S');
	}
}
//------------------------------------------------------------------------------

//==============================================================================
// Test Buffer Underflow
//==============================================================================

inline void CheckComBufferUnderflow (void)
{
	if((CommandCounter <= COM_BUFFER_UNDERFLOW_TRIGGER) && (ComOverflow))
	{
		ComOverflow = false;
	//	digitalWrite(PIN_DEBUG2, LOW);
		TX_WRITE('C');
	}
}

//------------------------------------------------------------------------------

//==============================================================================
// Ecriture des commandes dans le Buffer
//==============================================================================

inline void CmdBufferWrite (unsigned char *Data)
{
	CommandBuffer[CommandIndexWrite].Cmd = Data[0];
	CommandBuffer[CommandIndexWrite].Data.ui8[HIGH_BYTE]= Data[1];
	CommandBuffer[CommandIndexWrite].Data.ui8[LOW_BYTE] = Data[2];
	CommandIndexWrite++;
	if(CommandCounter < 255)
		CommandCounter++;
	CheckComBufferOverflow();
}
//------------------------------------------------------------------------------

//==============================================================================
// IData Process
//==============================================================================

inline void DataProcess (unsigned char *data)
{
	CmdData i;

	switch (data[0])
	{
		case 'A':
			if(data[1] == '1')
				data[1] = ON;
			else
				data[1] = OFF;
			break;

		case 'F':
			i.ui8[HIGH_BYTE] = data[1];
			i.ui8[LOW_BYTE] = data[2];
			if (i.ui16 > MAX_FEEDRATE_TIMER_VALUE)
				i.ui16 = MAX_FEEDRATE_TIMER_VALUE;
			else if (i.ui16 < MIN_FEEDRATE_TIMER_VALUE )
				i.ui16 = MIN_FEEDRATE_TIMER_VALUE;

			data[1] = i.ui8[HIGH_BYTE];
			data[2] = i.ui8[LOW_BYTE];
			break;


		case 'H':
			if (data[1] > MAX_PERCENTAGE_WIRE)
				data[1] = MAX_PERCENTAGE_WIRE;
			break;

		default:
			break;
	}
}
//------------------------------------------------------------------------------
//==============================================================================
// ITraitement des arriv�e de commande par USB 
//==============================================================================

inline void ComParse (void)
{
	static byte i = 0;
	static unsigned char Cmd[CMD_DATA_SIZE];
	static byte CmdSize = 0;
	unsigned char data;

	RX_READ(data);

	switch (ParserState)
	{
		case PARSER_STATE_CMD:
			if((data == 'A') || (data == 'H') || (data == 'M'))
			{
				Cmd[0] = data;
				i = 1;
				CmdSize = TWO_BYTE_CMD;
				ParserState = PARSER_STATE_DATA;
			}
			else if(data == 'F')
			{
				Cmd[0] = data;
				i = 1;
				CmdSize = THREE_BYTE_CMD;
				ParserState = PARSER_STATE_DATA;
			}
			break;

		case PARSER_STATE_DATA:
			Cmd[i++] = data;
			if(i >= CmdSize)
			{
				DataProcess(Cmd);
				CmdBufferWrite(Cmd);
				i = 0;
				ParserState = PARSER_STATE_CMD;
			}
			break;

		default:
			break;
	}
}

//------------------------------------------------------------------------------

inline void HMI_Manage (void)
{
	static unsigned long i = 0;

	switch(HMI.State)
	{
		case HMI_MODE_SCREEN:
		
			HMI_DigitScreen();
			break;

		case HMI_INIT_SCREEN:
			HMI_InitScreen();
			i = millis() + 3000;
			HMI.State = HMI_INIT_DELAY;
			break;

		case HMI_INIT_DELAY:
			if(i < millis())
				HMI.State = HMI_PARAMS_SCREEN;
			break;

		case HMI_PARAMS_SCREEN:
			HMI_ParamsScreen();
			i = millis() + 3000;
			HMI.State = HMI_PARAMS_DELAY;
			break;

		case HMI_PARAMS_DELAY:
			if(i < millis())
			{
				HMI_InitSwitchScreen();
				HMI.State = HMI_SWITCH_SCREEN;
			}
			break;

		case HMI_SWITCH_SCREEN:
			if( HMI_SwitchInitScreen())
				HMI.State = HMI_MODE_SCREEN;
			break;
	}
}
//==============================================================================
// Mode Manage
//==============================================================================

inline void ModeManage (void)
{
	static byte modeState = MODE_INIT;

	switch(modeState)
	{
		case MODE_INIT:
			if(HMI.State == HMI_MODE_SCREEN)
  		  modeState = MODE_MANU;
			break;

		case MODE_MANU:
         if (MOTEUR_ON_ASSERVI == 0)
         StepperDriverEnable (ON);
			
				BufferFlush();
				ENABLE_T5_ISR();
				ENABLE_T1_ISR();
				ENABLE_RX_ISR();
				modeState = MODE_PC;
			break;

		case MODE_PC:


				HeatingManage();
			break;
	}
}
//------------------------------------------------------------------------------
//==============================================================================
//    The main loop                                                           
//==============================================================================
void loop (void)
{
  
	ModeManage();
  HMI_Manage();

}

//------------------------------------------------------------------------------
