/*  Copyright 2013 Martin
    Complété en 2016 par A. DENIS
    Complété en 2017 par Vincent
    Complete en 2018 par Olivier
    Complete en 2019 par Alain
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
  le 30/11/2017 Adaptation du sketch suit à nouveau Plugin USBSerial_2.dll compatible
  avec les entraînements à courroie passer 16 bits la commande de vitesse pour
  élargissement de la plage de vitesse dans la config de Jedicut.
  Le 11/02/2019 Separation des fins de course et mise en place de l'écran 
  d'aide à la mise en service.
  le 12/02/2019 Mise en place du homing
  Le 13/02/2019 Recherche Bug sur fdc X2 un debug utilisait l'entrée fdc X2
  Le 14/02/2019 Mise en place chauffe Dynamique.
  Version au 11/04/2019 LMFO_V4_6_0 ajout traitement des pauses
  Version au 02/11/2019 LMFO_V4_7_0 Mise en service de l'encodeur, arret du chauffage
  manuel par l'inter, mise en mémoire du réglge de la valeur de l'encodeur, valeur 
  d'init de l'encodeur dans la config, pendant la découpe possibilité d'être en 
  chaufage manuel et de régler la chauffe. Ajout du langage Allemand donné par Dirk.
  Ajout posssibilité d'inversé le sens de l'encodeur dans la conf;
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
#define MAX_PAUSE_VALUE 65535	// Maximum de temps de pause
#define MIN_PAUSE_VALUE 0		// Minimum de temps de pause

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
#define PIN_WIRE_PWM 8 //Sortie de cde de chauffage di fil
#define PIN_CUTTER_PWM 9 //Sortie de cde de chauffage di fil

//PIN
#define PIN_SWITCH_CONTROL_MODE 40 //Interrupteur de selection PC ou Manu "0" PC
#define PIN_SWITCH_MOTOR A9 //Interrupteur des moteur PAP "0" ON
#define PIN_SWITCH_HEATING_PC 44 //Interrupteur de Chauffage fil "0" PC
#define PIN_SWITCH_HEATING_MANU A10 //Interrupteur de Chauffage Manuel fil "0" M
#define PIN_SWITCH_HEATING_CUTTER A12 //Interrupteur de Chauffage Dutter "0" ON
//Al>
//#define PIN_PUSHBUTTON_SHUNT_ENDSTOP 42 // Bouton poussoir de shunt fin de course pour redémarrer
//<
#define PIN_RELAY_HEATING 10 //Interrupteur de Chauffage fil "0" ON
//Al>
//#define PIN_ENDSTOP_MINI 14 //Somme des 4 fdc mini en série "0" fdc non sollicité
//<
#define PIN_POT_WIRE A5 //potentiomètre chauffage fil
#define PIN_POT_CUTTER A11 //potentiomètre chauffage cutter electrique

#define PIN_ROTARY_ENCODER_A 31   			// Rotary Encoder Channel A
#define PIN_ROTARY_ENCODER_B 33   			// Rotary Encoder Channel B
#define PIN_ROTARY_ENCODER_PUSHBUTTON 35    // Rotary Encoder Channel Push Button
#define PIN_BUZZER 37    					// Beeper is Connected into GADGETS3D shield MEGA_18BEEPER

#define PIN_X1_LIMIT 3 // "0" fdc X1 non sollicité ; "1" avec inversion fdc
#define PIN_Y1_LIMIT 2 // "0" fdc Y1 non sollicité ; "1" avec inversion fdc
#define PIN_X2_LIMIT 15 // "0" fdc X2 non sollicité ; "1" avec inversion fdc
#define PIN_Y2_LIMIT 14 // "0" fdc Y2 non sollicité ; "1" avec inversion fdc

// Mot de la lecture des fins de course 0b 0,0,0,0,fdcY2,fdcY1,fdcX2,fdcX1
byte mask_limits; // Résultat de la lecture des fins de courses
byte Val_X1_Limit; // "0" fdc X1 non sollicité
byte Val_Y1_Limit; // "0" fdc Y1 non sollicité
byte Val_X2_Limit; // "0" fdc X2 non sollicité
byte Val_Y2_Limit; // "0" fdc Y2 non sollicité

#define PIN_BP_HOMING 42 // Bouton poussoir de homing


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


// Switch

#define SWITCH_STATUS_CONTROL_MODE 0
#define SWITCH_STATUS_MOTOR_ENABLE 1
#define SWITCH_STATUS_HEAT_PC 2
#define SWITCH_STATUS_HEAT_MANU 3
#define SWITCH_STATUS_CUTTER_ENABLE 4
//#define SWITCH_STATUS_ENDSTOP_SHUNT 5
#define SWITCH_STATUS_HOMING_OK 5
#define SWITCH_STATUS_ENDSTOP 6

// Mode

#define IsSwitchNotInitialized(x) ((x & 0x1F) != 0x1F)

#define MODE_SWITCH_BIT_CONTROL_MODE 0
#define MODE_SWITCH_BIT_MOTOR_MODE 1
#define MODE_SWITCH_BIT_HEAT_PC 2
#define MODE_SWITCH_BIT_HEAT_MANU 3
#define MODE_SWITCH_BIT_CUTTER 4
//#define MODE_SWITCH_BIT_ENDSTOP_SHUNT 5
#define MODE_SWITCH_BIT_HOMING_OK 5
#define MODE_SWITCH_BIT_ENDSTOP 6

#define MODE_SWITCH_PC(x) ((x & 0x01) == 0x00)
#define MODE_INIT(x) ((x & 0x1F) != 0x1F)
#define HEAT_MODE_MANU(x) ((x & 0x08) == 0x00)
#define HEAT_MODE_PC(x) ((x & 0x04) == 0x00)
#define HEAT_MODE_CUTTER(x) ((x & 0x10) == 0x00)
#define ENDSTOP_ACTIVE(x) ((x & 0x60) == 0x60)
#define MOTOR_DEACTIVATED(x) (x & 0x02)

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
//#define ENABLE_T4_COMP_OUTPUT_C() TCCR4A |= 0x08
//#define DISABLE_T4_COMP_OUTPUT_C() TCCR4A &= 0xF3

//LCD declaration
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_E, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

volatile bool toogle = true;
bool StepperDriverEnableMode = STEPPER_DRIVER_ENABLE_HIGH_LEVEL;


const byte heatPercentToPWMConvTab[101] = {

		0, 3, 6, 8, 11, 13, 16, 18, 21, 23, 26, 29, 31, 34, 36, 39, 41,
		44, 46, 49, 51, 54, 57, 59, 62, 64, 67, 69, 72, 74, 77, 80, 82,
		85, 87, 90, 92, 95, 97, 100, 102, 105, 108, 110, 113, 115, 118,
		120, 123, 125, 128, 131, 133, 136, 138, 141, 143, 146, 148, 151,
		153, 156, 159, 161, 164, 166, 169, 171, 174, 176, 179, 182, 184,
		187, 189, 192, 194, 197, 199, 202, 204, 207, 210, 212, 215, 217,
		220, 222, 225, 227, 230, 233, 235, 238, 240, 243, 245, 248, 250,
		253, 255
};

const bool StepperDriverEnableTab[2][2] = { {ON, ON}, {OFF, ON} };
const bool StepperDriverEnableState[2] = {STEPPER_DRIVER_ENABLE_LOW_LEVEL, STEPPER_DRIVER_ENABLE_HIGH_LEVEL};

enum Mode
{
	MODE_INIT = 0,
	MODE_MANU,
	MODE_PC
};

enum RotaryEncoderState
{
	UNCHANGED = 0,
	POSITIF,
	NEGATIF
};

const byte rotaryEncoderTable[16] = {UNCHANGED, POSITIF, NEGATIF, UNCHANGED, POSITIF, UNCHANGED, UNCHANGED, NEGATIF, NEGATIF, UNCHANGED, UNCHANGED, POSITIF, UNCHANGED, POSITIF, NEGATIF, UNCHANGED};

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
  unsigned int Pause = 0;
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

const byte SwitchStatusTab[7][2] =
{
		{0, 1},
		{0, 2},
		{0, 4},
		{0, 8},
		{0, 16},
		{0, 32},
		{0, 128}
};

struct StructSwitch
{
	bool ControlMode;
	bool MotorEnable;
	bool HeatPC;
	bool HeatManu;
	bool CutterEnable;
	bool HomingOk = 0;
	bool EndStop;
	byte Status;
} Switch;

volatile bool buzzerToogle = false;

// Pour Homing

byte Mot_Dir = 0 ;  // Mot de commande des direction des moteurs
byte Mot_Ck = 0;    // Mot de commande des steps des moteurs

// Mot de commande des steps des moteurs 0b dY2,dY1,dX2,dX1,sY2,sY1,sX2,sX1
byte h_motCkDir = 0; // pour le homing

//Pour chauffe dynamique du fil
//int refChaufDyn ; // consigne avec correction par les steps
bool WireVitesse = 0 ; //ordre de correction de vitesse
// Pour Pause
bool ActivePause = 0 ;
unsigned long kPause = 0;  // millis  + pause en début de pause
unsigned long RePause = 0;  // calcul de la pause restante
unsigned long Cad_Aff = 0;  //cadense affichage pause
// Variable init timer4
  int maRaz ; 
  int maPreset ;
  int encPos ;

const String Version = VERSION;

/**********************************************************************************/
void setup (void)
{
	//serial com Initialization

	UBRR0 = UBRR0_BAUDRATE_VALUE; 	// Configure USART Clock register
	UCSR0A = 0x02;					//
	UCSR0B = 0x18;					//
	UCSR0C = 0x06;					//

	// pins Initialization

	//LCD
	pinMode(PIN_ROTARY_ENCODER_PUSHBUTTON, INPUT);
	digitalWrite(PIN_ROTARY_ENCODER_PUSHBUTTON, HIGH);

	pinMode(PIN_ROTARY_ENCODER_A, INPUT);
	pinMode(PIN_ROTARY_ENCODER_B, INPUT);
	digitalWrite(PIN_ROTARY_ENCODER_A, HIGH);
	digitalWrite(PIN_ROTARY_ENCODER_B, HIGH);
  encPos  = VALEUR_INIT_ENCODEUR * 2; // positionnement à xx% de Heat.WireConsign (xx:2)

	pinMode(PIN_BUZZER, OUTPUT);
	digitalWrite(PIN_BUZZER, LOW);

	// Heating
	pinMode(PIN_RELAY_HEATING, OUTPUT); // PD2, Heat Relay on/off low active for letmathe mdlcnc
	digitalWrite(PIN_RELAY_HEATING, LOW); // Off

	pinMode(PIN_WIRE_PWM, OUTPUT); // PWM pin for wire heating
	pinMode(PIN_CUTTER_PWM, OUTPUT); // PWM pin for the cutter

	pinMode(PIN_POT_WIRE, INPUT); 	// Analog input for wire heat consign potentiometer
	pinMode(PIN_POT_CUTTER, INPUT);	// Analog input for Cutter heat consign potentiometer

	// Switch
	pinMode (PIN_SWITCH_CONTROL_MODE, INPUT);	//Switch control mode PC/Manu ->  low state = mode PC / high state = mode Manu
	digitalWrite(PIN_SWITCH_CONTROL_MODE, HIGH); // Atmega internal Pullup Activated

	pinMode (PIN_SWITCH_MOTOR, INPUT);  //Switch Stepper Driver -> low state = Enabled / high state = OFF or PC control
	digitalWrite(PIN_SWITCH_MOTOR, HIGH); // Atmega internal Pullup Activated

	pinMode (PIN_SWITCH_HEATING_PC, INPUT); //Switch Wire heat mode PC -> Low state = control by PC / high state = OFF or Manu
	digitalWrite(PIN_SWITCH_HEATING_PC, HIGH); // Atmega internal Pullup Activated

	pinMode (PIN_SWITCH_HEATING_MANU, INPUT); //Switch Wire heat mode manu -> low state = manu / high state = OFF or PC
	digitalWrite(PIN_SWITCH_HEATING_MANU, HIGH); // Atmega internal Pullup Activated

	pinMode (PIN_SWITCH_HEATING_CUTTER, INPUT); //Switch Cutter heat mode -> disable if switch control mode is on PC mode else low state = ON / high state = OFF
	digitalWrite(PIN_SWITCH_HEATING_CUTTER, HIGH); // Atmega internal Pullup Activated
  
  pinMode (PIN_X1_LIMIT, INPUT); //Pin sfcd X1 "0" non sollicité
  digitalWrite(PIN_X1_LIMIT, HIGH); // Mise en servive Pullup
  
  pinMode (PIN_Y1_LIMIT, INPUT); //Pin sfcd Y1 "0" non sollicité
  digitalWrite(PIN_Y1_LIMIT, HIGH); // Mise en servive Pullup
  
  pinMode (PIN_X2_LIMIT, INPUT); //Pin sfcd X2 "0" non sollicité
  digitalWrite(PIN_X2_LIMIT, HIGH); // Mise en servive Pullup
  
  pinMode (PIN_Y2_LIMIT, INPUT); //Pin sfcd Y2 "0" non sollicité
  digitalWrite(PIN_Y2_LIMIT, HIGH); // Mise en servive Pullup
//Al>
  pinMode (PIN_BP_HOMING , INPUT); //Pin BP Homing
  digitalWrite(PIN_BP_HOMING , HIGH); // Mise en servive Pullup
 

//	pinMode (PIN_ENDSTOP_MINI, INPUT); // state of the 4 endstops -> low state = All Endstops not activate / high state = at least one Endstop is activate
//	digitalWrite(PIN_ENDSTOP_MINI, HIGH); // Atmega internal Pullup Activated
//	pinMode (PIN_PUSHBUTTON_SHUNT_ENDSTOP, INPUT); // push button shunt endstops -> low state = depressed = Endstops shunted / high state = Endstops not shunted
//	digitalWrite(PIN_PUSHBUTTON_SHUNT_ENDSTOP, HIGH); // Atmega internal Pullup Activated
//<
	pinMode(PIN_DEBUG1, OUTPUT); // Led Pin used to indicate an underflow condition
	pinMode(PIN_DEBUG2, OUTPUT); // Led Pin used to indicate an underflow condition
	pinMode(PIN_DEBUG3, OUTPUT); // Led Pin used to indicate an underflow condition
	pinMode(PIN_DEBUG4, OUTPUT); // Led Pin used to indicate an underflow condition

	// Driver Motor Pins

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

  /*
	// Timer 4 init, used for Wire Heat PWM as Hardware PWM generator
	TIMSK4 = 0x00;	// Reset interrupt flag
	TIFR4 = 0x00;	// disable interrupt
	TCCR4A = 0x0A;	// configure timer in Fast PWM mode, disable Output Compare pins A and B and enable Output Compare pin C
	TCCR4B = 0x1C;	// configure timer in Fast PWM mode and set prescaler to 256
	TCCR4C = 0x00;  // Disable Input capture
	ICR4 = 100;		// Set Top value => Fosc = 16MHz, Prescaler = 256, Compare value = 200 -> Fpwm = 1 / (100 * 256 * 62.5e-9) = 10 KHz
	OCR4C = 0;		// Set compare value => Compare value between 0 = 0% PWM and 200 = 100% PWM
  */

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

	//
	MachineName.getBytes(MachineNameChar, LCD_COLUMN_COUNT);
 // Switch.HomingOk = false ;  // Etat du Homing
	#ifdef DEBUG
	digitalWrite(13, HIGH);
	#endif
  //Al>
  AideMiseServiceFdc(); // aide à la mise en service des fins de course
  //<
  
}

/**********************************************************************************/
//==============================================================================
// Aide à la mise en service des fins de course
//==============================================================================
void AideMiseServiceFdc(void)
{
  Heat.WireConsign = map(analogRead (PIN_POT_WIRE), 0, 1023, 0, MAX_PERCENTAGE_WIRE);
  if (Heat.WireConsign> 50)
  {
    Affic_Trame_fdc();
    Test_fdc(); 
  }


}

//------------------------------------------------------------------------------

//==============================================================================
//  Test  des fins de course
//==============================================================================
 void Test_fdc()
 {
 do
 {
  limits_Lect();
  Aff_Test_Fdc();
    
 Heat.WireConsign = map(analogRead (PIN_POT_WIRE), 0, 1023, 0, MAX_PERCENTAGE_WIRE);
 }while (Heat.WireConsign >10);
  limits_Lect();
}
 //-----------------------------------------------------------------------------
 
 //==============================================================================
// Affichage de la Trame Etat Fins de course
//==============================================================================
void Affic_Trame_fdc()
{
  lcd.begin(LCD_COLUMN_COUNT, LCD_LINE_COUNT);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(TEXT17);    // " Etat fin de course"
  lcd.setCursor(0, 1);
  lcd.print ("  X1   Y1   X2   Y2") ;
  lcd.setCursor(0, 3);
  lcd.print (TEXT18);  // "Fin -> Pot Ch < 10%"
}
//------------------------------------------------------------------------------

//==============================================================================
// Lecture des fins de course et constitution d'un mot état des fins de course
//==============================================================================
 
void limits_Lect()
{
  mask_limits = (((digitalRead (PIN_X1_LIMIT)^ INV_FDC_X1) <<0)|((digitalRead (PIN_Y1_LIMIT)^INV_FDC_Y1) <<2)|((digitalRead (PIN_X2_LIMIT)^INV_FDC_X2) <<1)|((digitalRead (PIN_Y2_LIMIT)^INV_FDC_Y2) <<3));

  Val_X1_Limit = bitRead(mask_limits,0);
  Val_X2_Limit = bitRead(mask_limits,1);
  Val_Y1_Limit = bitRead(mask_limits,2);
  Val_Y2_Limit = bitRead(mask_limits,3);
}
//------------------------------------------------------------------------------

//==============================================================================
// Affichage de l'etat des fins de course
//==============================================================================
void Aff_Test_Fdc ()
 {
  lcd.setCursor(2, 2);
  lcd.print (Val_X1_Limit,DEC);
  lcd.setCursor(7, 2);
  lcd.print (Val_Y1_Limit,DEC);
  lcd.setCursor(12, 2);
  lcd.print (Val_X2_Limit,DEC);
  lcd.setCursor(17, 2);
  lcd.print (Val_Y2_Limit,DEC);   
  delay(100);// Attendre 100ms
 }
//------------------------------------------------------------------------------

//==============================================================================
// Homing Manage
//==============================================================================
void HomingManage ()
{
  if (Switch.HomingOk == false ) 
  {
  
    if ((digitalRead (PIN_SWITCH_MOTOR) == LOW))
    {
    StepperDriverEnable(ON);
    Arm_Homing ();
    
    lcd.setCursor(0, 1);
    lcd.print(TEXT14);       //"MODE  MOT Chau  CUT"
    GetSwitchStatus ();
    HMI_ModeScreen ();
    
      
    }
  }
  else
  {
    if (Switch.MotorEnable == 1)  Desar_Homing ();
  }
}
//------------------------------------------------------------------------------
//==============================================================================
// Armement Homing
//==============================================================================

void Arm_Homing ()
{
  if (SEQ_HOMING == 1)
  {

    lcd.setCursor(0, 1);
    lcd.print(TEXT19);  // "   Attente Homing   "
    lcd.setCursor(0, 2);
    lcd.print("C1a C1 C2 C3 C4 Ppos");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    do
    {
    }
    while ((digitalRead (PIN_BP_HOMING)== HIGH)) ; 
    homing_set();
    if (PREPOS ==1)
    {
      prepos_set();
    }
    lcd.setCursor(0, 1);
    lcd.print(TEXT20);  //" Homing Termine     "
    delay (1000);
    
    if (MOTEUR_ON_ASSERVI == 1)
    {
      StepperDriverEnable(ON); 
    }



      
      Switch.HomingOk = true ;
  } 

  // Si on ne veut pas de Homing par la config, on vient la
  // pour faire le premier passage 

  else
  {
      Switch.HomingOk = true ;
  } 
}
//------------------------------------------------------------------------------

//==============================================================================
// Desarmement Homing
//==============================================================================
void Desar_Homing ()
{
 Switch.HomingOk = false ;

}

//------------------------------------------------------------------------------

//==============================================================================
// Séquence Homing
//==============================================================================

void homing_set ()
{


    lcd.setCursor(0, 1);
    lcd.print(TEXT13);    //"   Homing en cours  "
  
    cycle_1();
    cycle_2();
    cycle_3();
    cycle_4();


}
//------------------------------------------------------------------------------

//==============================================================================
// Cycle 1 Remontée des axe Y de Xmm si vous avez choisi l'option 
// Retour à zéro X1 et X2 en premier, Y1 et Y2 en second
//==============================================================================
void cycle_1()
{ 
 byte tr_mask_limits =0 ;
  //calcul de la tempo entre chaque step pour la vitesse d'approche
  unsigned int tempo_step = MM_PER_STEP / VIT_RECH_FDC * 1000000 ; 
  if (POS_SECU_Y == 1 )
  {
    unsigned int Nbre_pas = MM_POS_SECU_Y / MM_PER_STEP ;
    
    lcd.setCursor(0, 3);
    lcd.print("===");
    
    for (unsigned int i = Nbre_pas ; i > 0 ; i--)
    {
      h_motCkDir = 0x0C; // 00001100
      ProcessStep(h_motCkDir);
      delayMicroseconds(tempo_step);
    }
  }

  // Recherche fdc X1 et X2
  
  lcd.setCursor(3, 3);
  lcd.print("==");
  
  do
  {
    limits_Lect();
    tr_mask_limits = mask_limits ^ 0xFF ;
    h_motCkDir = 0xF3; // 11110011
    h_motCkDir = h_motCkDir & tr_mask_limits ;
    ProcessStep(h_motCkDir);
    delayMicroseconds(tempo_step); 
  }
  while (mask_limits < 0x03);  

  // Recherche fdc Y1 et Y2
  
  lcd.setCursor(5, 3);
  lcd.print("==");
 
  do
  {
    limits_Lect();
    tr_mask_limits = mask_limits ^ 0xFF ;
    h_motCkDir = 0xFC; //11111100
    h_motCkDir = h_motCkDir & tr_mask_limits ;
    ProcessStep(h_motCkDir);
    delayMicroseconds(tempo_step); 
  }
  while (mask_limits < 0x0F); 
}
//------------------------------------------------------------------------------

//==============================================================================
//Cycle 2 retour hors fin de course les 4 axes en même temps
//==============================================================================
void cycle_2()
{ 
  lcd.setCursor(7, 3);
  lcd.print("==");
  
  byte tr_mask_limits =0 ;
  
  //calcul de la tempo entre chaque step pour la vitesse lente (ajuste)  
  unsigned int tempo_step = MM_PER_STEP / VIT_AJUST_FDC * 1000000 ; 
  
  // Avance des 4 axes jusqu'à ce que les fins de course soient sollicités
  do
  {
    limits_Lect();
    tr_mask_limits = mask_limits ^ 0x00 ;
    h_motCkDir = 0x0F; //00001111
    h_motCkDir = h_motCkDir & tr_mask_limits ;
    ProcessStep(h_motCkDir);
    delayMicroseconds(tempo_step);
  }
  while (mask_limits > 0x00); // Sortie de la boucle les 4 fdc non sollicités.
}
//------------------------------------------------------------------------------
//==============================================================================
// Cycle 3 : retour vers fins de course les 4 axes en même temps
//==============================================================================

void cycle_3()
{ 
  lcd.setCursor(9, 3);
  lcd.print("===");
  
  byte tr_mask_limits =0 ;
 
  //calcul de la tempo entre chaque step pour la vitesse lente (ajuste)  
  unsigned int tempo_step = MM_PER_STEP / VIT_AJUST_FDC * 1000000 ;   
  do
  {
    limits_Lect();
    tr_mask_limits = mask_limits ^ 0xFF ;
    h_motCkDir = 0xFF; //11111111
    h_motCkDir = h_motCkDir & tr_mask_limits ;
    ProcessStep(h_motCkDir);
    delayMicroseconds(tempo_step);
  }
  while (mask_limits < 0x0F); // Sortie de la boucle les 4 fdc sont sollicités. 
}
//------------------------------------------------------------------------------

//==============================================================================
// Cycle 4 : retour hors fin de course les 4 axes en même temps
//==============================================================================
void cycle_4()
{
  lcd.setCursor(12, 3);
  lcd.print("====");
  
  byte tr_mask_limits =0 ;
 
  //calcul de la tempo entre chaque step pour la vitesse lente (ajuste) 
  unsigned int tempo_step = MM_PER_STEP / VIT_AJUST_FDC * 1000000 ; 
  
  do
  {
    limits_Lect();
    tr_mask_limits = mask_limits ^ 0x00 ;
    h_motCkDir = 0x0F;  // 00001111
    h_motCkDir = h_motCkDir & tr_mask_limits ;
    ProcessStep(h_motCkDir);
    delayMicroseconds(tempo_step);
  }
  while (mask_limits > 0x00); // Sortie de la boucle les 4 fdc non sollicités.
 }
//------------------------------------------------------------------------------

//==============================================================================
// Si option "Prépositionnement" Les axes Y rejoignent les mm demandés en option
// puis  les axes X rejoignent les mm demandés
//==============================================================================
void prepos_set()
{ 
  lcd.setCursor(16, 3);
  lcd.print("====");

  //calcul de la tempo pour la vitesse de déplacement (Grande Vitesse).
  unsigned int tempo_step = MM_PER_STEP / VIT_RECH_FDC * 1000000 ; 
  
  // Prepositionnement des axes Y
  unsigned int Nbre_pas = MM_PREPOS_Y / MM_PER_STEP ;
  for (unsigned int i = Nbre_pas ; i > 0 ; i--)
  {
    h_motCkDir = 0x0C; // 00001100
    ProcessStep(h_motCkDir);
    delayMicroseconds(tempo_step);
  }
  // Prépositionnement des axes X
  Nbre_pas = MM_PREPOS_X / MM_PER_STEP ;
  for (unsigned int i = Nbre_pas ; i > 0 ; i--)
  {
    h_motCkDir = 0x03; // 00000011
       ProcessStep(h_motCkDir);
       delayMicroseconds(tempo_step);
  }
  
}
//------------------------------------------------------------------------------

//==============================================================================
// End Stop Manage  sur une detection de End Stop en Mode Auto
//==============================================================================
void EndStopManage ()
{

  if (Switch.HomingOk == true )
  {
  //arretPropreMachine();

  limits_Lect();
  if (mask_limits > 0x00)
    {
    Trait_Arr_fdc(); 
    Desar_Homing (); 
    }
  }


}

//==============================================================================
// Traitement d'un arrêt par fin de course
//==============================================================================
void Trait_Arr_fdc()
{
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(TEXT11);    //" Etat fin de course"
  lcd.setCursor(0, 1);
  lcd.print ("  X1   Y1   X2   Y2") ;
  lcd.setCursor(0, 3);
  lcd.print(TEXT12);    //" --> BP Homing"
  Aff_Test_Fdc ();
  do
    {
      //alarmSonor();  
    }while (digitalRead (PIN_BP_HOMING)== HIGH);
     SoundAlarm(OFF);
  do
    {
 
    }while (digitalRead (PIN_BP_HOMING)== LOW);
    
  lcd.clear();
  testPosIntDem();
  lcd.clear();
  	lcd.setCursor(0, 0);
  	lcd.print(MachineName);
  	lcd.setCursor(0, 1);
  	lcd.print(TEXT14);       //"MODE  MOT Chau  CUT"
  	lcd.setCursor(0, 3);
  	lcd.print("             0%   0%");
}
//------------------------------------------------------------------------------

//==============================================================================
// Au demarrage test de la position des interrupteurs ou sur arret fdc
//==============================================================================

void testPosIntDem()
{
  lcd.setCursor(0, 2);
  lcd.print(TEXT6); // "Test des inter."
  if (digitalRead (PIN_SWITCH_CONTROL_MODE) == LOW) // test si l'inter Mode est sur Manu
  {
    lcd.setCursor(0, 3);
    lcd.print(TEXT7);// "Mettre Mode en manu" attente inter Mode sur Manu
    do
    {

    } 
    while (digitalRead (PIN_SWITCH_CONTROL_MODE) == LOW);
  }

  if (digitalRead (PIN_SWITCH_MOTOR) == LOW) // test si l'inter Moteur est sur OFF
  {
    lcd.setCursor(0, 3);
    lcd.print(TEXT8); // "Mettre Mot. sur OFF" attente inter Moteur sur OFF
    do
    {

    } while (digitalRead (PIN_SWITCH_MOTOR) == LOW);
  }
  if ((digitalRead (PIN_SWITCH_HEATING_PC == LOW) | (digitalRead (PIN_SWITCH_HEATING_MANU) == LOW))) // test si l'inter Chauffe est sur OFF
  {
    lcd.setCursor(0, 3);
    lcd.print(TEXT9); // "Mettre Chauf sur OFF" attente inter Chauffe sur OFF
    do
    {

    } while ((digitalRead (PIN_SWITCH_HEATING_PC) == LOW) | (digitalRead (PIN_SWITCH_HEATING_MANU) == LOW));

  }

  if (digitalRead (PIN_SWITCH_HEATING_CUTTER) == LOW) // test si l'inter Moteur est sur OFF
  {
    lcd.setCursor(0, 3);
    lcd.print(TEXT10); // "Mettre Cut sur OFF" attente inter Moteur sur OFF
    do
    {

    } while (digitalRead (PIN_SWITCH_HEATING_CUTTER) == LOW);
  }



}
//------------------------------------------------------------------------------

//==============================================================================
// Pause Manage  sur une detection de End Stop en Mode Auto
//==============================================================================
inline void PauseManage (void)
{
  
  if (ActivePause == 1)
  {
    if ( millis() > Cad_Aff)
    {
    RePause = (kPause - millis())/1000 ;
    Cad_Aff = millis() + 250;
    lcd.setCursor(7, 0);
  	lcd.print ("   ") ;
  	lcd.setCursor(7, 0);
    lcd.print (RePause) ;
    }
    
    if ( millis() > kPause)
    {
    ActivePause = 0 ;
    kPause = 0;
    lcd.setCursor(0, 0);
	  lcd.print ("                   ") ;
   	lcd.setCursor(0, 0);
	  lcd.print(MachineName);
    ENABLE_T5_ISR();
    }
  }
}

//------------------------------------------------------------------------------

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

/**********************************************************************************/

inline void StepperDriverStep (byte step)
{
	bitWrite(PORTF, 0, (step & DRIVER_X1_STEP_MASK));
	bitWrite(PORTF, 6, (step & DRIVER_X2_STEP_MASK));
	bitWrite(PORTA, 4, (step & DRIVER_Y1_STEP_MASK));
	bitWrite(PORTC, 1, (step & DRIVER_Y2_STEP_MASK));
	bitWrite(PORTB, 5, (step & DRIVER_X1_STEP_MASK));
}

/**********************************************************************************/
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

/**********************************************************************************/
inline void StepperDriverEnable (bool en)
{
	bool enable = StepperDriverEnableState[en];

	StepperDriverEnableMode = en;

 
	digitalWrite(PIN_X1_EN, enable); // Cde Driver X1
	digitalWrite(PIN_X2_EN, enable); // Cde Driver X2
 	digitalWrite(PIN_Y1_EN, enable); // Cde Driver Y1
 	digitalWrite(PIN_Y2_EN, enable); // Cde Driver Y2
}

/**********************************************************************************/

//==============================================================================
//Gestion de la marche asservie des Drivers
//==============================================================================
void  StepperDriverManage (void)
{
    if ((MOTEUR_ON_ASSERVI == 1))
    {
      if ((!Switch.ControlMode) && (!Switch.MotorEnable) && (StepperDriverEnableMode != PC.Motor))
      {
        StepperDriverEnable(PC.Motor);
      }
      else
      {
      if  (Switch.MotorEnable) Desar_Homing () ;
      }
    }
    else 
    {
      if ((Switch.MotorEnable) && (StepperDriverEnableMode != OFF))
        { 
          StepperDriverEnable(OFF); // blocage des drivers
          Desar_Homing () ;
        }
        else
        {
        if((!Switch.MotorEnable) && (StepperDriverEnableMode != ON)) 
        StepperDriverEnable(ON);
        } 
     }     
                                                  

 }
//------------------------------------------------------------------------------
/**********************************************************************************/
void CleanMotorHalt (void)
{
	PC.Motor = OFF;
	StepperDriverStep (OFF);
}

/**********************************************************************************/

inline void HeatingRelay (byte en)
{
	digitalWrite (PIN_RELAY_HEATING, en);
}

/**********************************************************************************/

inline void ComputeRotaryEncoderHeatConsign (void)
{
	bool rotEncA, rotEncB;
	byte rotStatus = 0;

	if(!digitalRead(PIN_ROTARY_ENCODER_PUSHBUTTON))
	{
		while(!digitalRead(PIN_ROTARY_ENCODER_PUSHBUTTON));
      lcd.setCursor(10, 3);
      lcd.print(">");    // Affichage de ">" pour dire que l'on est en réglage
		do
		{
			rotEncA = digitalRead(PIN_ROTARY_ENCODER_A);
			rotEncB = digitalRead(PIN_ROTARY_ENCODER_B);
			bitWrite(rotStatus, 3, rotEncA);
			bitWrite(rotStatus, 1, rotEncB);
      
      #ifdef ROT_ENC_INVERS
      
    			if(rotaryEncoderTable[rotStatus] & 0x01)
    			{
          if(encPos < MAX_PERCENTAGE_WIRE *2)   // on multiplie par 2 pour avoir ensuite une définition de 1%
           encPos ++;
    			}
    			else if(rotaryEncoderTable[rotStatus] & 0x02)
    			{
          if(encPos != 0)
          encPos --;
    			}      
      
      #else
      
    			if(rotaryEncoderTable[rotStatus] & 0x02)
    			{
          if(encPos < MAX_PERCENTAGE_WIRE *2)   // on multiplie par 2 pour avoir ensuite une définition de 1%
           encPos ++;
    			}
    			else if(rotaryEncoderTable[rotStatus] & 0x01)
    			{
          if(encPos != 0)
          encPos --;
    			}
      
      #endif
      
      bitWrite(rotStatus, 2, rotEncA);
			bitWrite(rotStatus, 0, rotEncB);
      Heat.WireConsign = encPos / 2;   //on divise par 2 car "encPos " est incrémenté 2 par 2
      lcd.setCursor(11, 3);
      if (Heat.WireConsign < 100) lcd.print(" ");
      if (Heat.WireConsign < 10) lcd.print(" ");
      lcd.print (Heat.WireConsign, DEC);
		}while(digitalRead(PIN_ROTARY_ENCODER_PUSHBUTTON) && !digitalRead(PIN_SWITCH_HEATING_MANU ));
		while(!digitalRead(PIN_ROTARY_ENCODER_PUSHBUTTON) && !digitalRead(PIN_SWITCH_HEATING_MANU ));
	
  lcd.setCursor(10, 3);
  lcd.print(" ");  // Affichage de " " pour dire que l'on sort du réglage
  }
  else
  {
   Heat.WireConsign = encPos / 2;
      lcd.setCursor(11, 3);
      if (Heat.WireConsign < 100) lcd.print(" ");
      if (Heat.WireConsign < 10) lcd.print(" ");
      lcd.print (Heat.WireConsign, DEC);  
  }
}

/**********************************************************************************/

inline void HeatingManage (byte mode)
{
	if(mode == MODE_MANU)
	{
		if(!Switch.HeatManu)
		{
		#ifdef HEAT_CONSIGN_ROTARY_ENCODER
				ComputeRotaryEncoderHeatConsign();
		#else
				Heat.WireConsign = map(analogRead (PIN_POT_WIRE), 0, 1023, 0, 100);
        if (Heat.WireConsign > MAX_PERCENTAGE_WIRE) Heat.WireConsign = MAX_PERCENTAGE_WIRE ;
		#endif
		}
		else
		{
			Heat.WireConsign = 0;
		}


		if(Switch.CutterEnable)
		{
			DISABLE_T2_COMP_OUTPUT_B();
			Heat.CutterConsign = 0;
			OCR2B = 0;
		}
		else
		{
			ENABLE_T2_COMP_OUTPUT_B();
			Heat.CutterConsign = map (analogRead (PIN_POT_CUTTER), 0, 1023, 0, 100);
      if (Heat.CutterConsign > MAX_PERCENTAGE_CUTTER) Heat.CutterConsign = MAX_PERCENTAGE_CUTTER ;
			OCR2B = Heat.CutterConsign;
		}
	}
	else
	{
		if(!Switch.HeatPC)
		{
			Heat.WireConsign = PC.Heat;
		}
		else if(!Switch.HeatManu)
		{
		#ifdef HEAT_CONSIGN_ROTARY_ENCODER
				ComputeRotaryEncoderHeatConsign();
		#else
		    Heat.WireConsign = map(analogRead (PIN_POT_WIRE), 0, 1023, 0, 100);
        if (Heat.WireConsign > MAX_PERCENTAGE_WIRE) Heat.WireConsign = MAX_PERCENTAGE_WIRE ;
		#endif
		}
		else
			Heat.WireConsign = 0;
	}

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
  if ((CHAUFFE_ASSERV == 1)& (mode == MODE_PC ))
    {
    if (WireVitesse == 1) //Correction de la chauffe en fonction des steps
      {
        Heat.WireDynamique= (Heat.WireConsign * CORRECT_CHAUFFE);
        if (Heat.WireDynamique > MAX_PERCENTAGE_WIRE) Heat.WireDynamique = MAX_PERCENTAGE_WIRE ;
      }
      else Heat.WireDynamique = Heat.WireConsign ;

      analogWrite(PIN_WIRE_PWM, (Heat.WireDynamique * 2.55));
    }
    else
    {
    	//OCR4C = Heat.WireConsign; // PWM for wire heating (stretch 0-100% to a range of 0-254)*/
      analogWrite(PIN_WIRE_PWM, (Heat.WireConsign * 2.55));
    }
}

/**********************************************************************************/

void ResetHeat (void)
{
	Heat.WireConsign = 0;
	PC.Heat = 0;
	HeatingRelay( OFF );
	//OCR4C = 0; // PWM for wire heating (stretch 0-100% to a range of 0-255)*/
  analogWrite(PIN_WIRE_PWM, 0);
	Heat.CutterConsign = 0;
	OCR2B = 0;
}

/**********************************************************************************/

inline void SoundAlarm (bool en)
{
	if(en)
		ENABLE_T2_ISR();
	else
	{
		DISABLE_T2_ISR();
		digitalWrite(PIN_BUZZER, LOW);
	}
}

/*********************************************************************************/

inline void ProcessCommand (void)
{
	switch(CommandBuffer[CommandIndexRead].Cmd)
	{
		case 'M':
			ProcessStep(CommandBuffer[CommandIndexRead].Data.ui8[HIGH_BYTE]);
			break;

		case 'A':   // All Motors on/off
			digitalWrite(PIN_DEBUG1, toogle);
			toogle = !toogle;
			PC.Motor = CommandBuffer[CommandIndexRead].Data.ui8[HIGH_BYTE];
      if (MOTEUR_ON_ASSERVI)
			StepperDriverEnable(StepperDriverEnableTab[Switch.MotorEnable][PC.Motor]);
      
			break;                       

		case 'F':   // Change the stepper Feedrate
			// OCR5A values 255 = 250Hz 190 = 328Hz 127 = 500Hz 63 = 1 kHz 31 = 2KHz 15 = 4 kHz
			PC.Feedrate = CommandBuffer[CommandIndexRead].Data.ui16;
			OCR5A = PC.Feedrate;
			TCNT5 = 0;
			break;
      
      case 'P':   // Pause
      DISABLE_T5_ISR();
      ActivePause = 1;
			PC.Pause = CommandBuffer[CommandIndexRead].Data.ui16;
     /*unsigned int H_Pause = 0;
      unsigned int L_Pause = 0;
      H_Pause = PC.Pause;
      L_Pause = PC.Pause;
       H_Pause << 8 ;
       L_Pause >> 8 ;
       PC.Pause = H_Pause | L_Pause ;
       */
      kPause = 0;
    	kPause = millis() + PC.Pause;
      Cad_Aff = millis() + 250;
      lcd.setCursor(0, 0);
    	lcd.print ("Pause      secondes ") ;
    	lcd.setCursor(7, 0);
      lcd.print (PC.Pause /1000) ;
			break;

		case 'H':   // Wire Heat ON/OFF (may be programmed as PWM (analog out))
			PC.Heat = CommandBuffer[CommandIndexRead].Data.ui8[HIGH_BYTE];
			break;

    default:
      break;
	}
}

/**********************************************************************************/

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

/**********************************************************************************/

ISR(TIMER1_COMPA_vect)
{

}

/**********************************************************************************/

ISR(TIMER2_COMPA_vect)
{
	buzzerToogle = !buzzerToogle;
	digitalWrite(PIN_BUZZER, buzzerToogle);
}

/**********************************************************************************/

ISR(TIMER5_COMPA_vect)
{
	digitalWrite(PIN_DEBUG3, HIGH);
	IsrProcessBuffer();
	digitalWrite(PIN_DEBUG3, LOW);
}

/**********************************************************************************/

ISR(USART0_RX_vect)
{
	digitalWrite(PIN_DEBUG4, HIGH);
	ComParse();
	digitalWrite(PIN_DEBUG4, LOW);
}


/**********************************************************************************/

void BufferFlush (void)
{
	CommandCounter = 0;
	ComOverflow = false;
	TX_WRITE('C');
	CommandIndexRead = 0;
	CommandIndexWrite = 0;
	ParserState = PARSER_STATE_CMD;
}

/**********************************************************************************/

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

/**********************************************************************************/

inline void CheckComBufferOverflow (void)
{
	if((CommandCounter >= COM_BUFFER_OVERFLOW_TRIGGER) && (!ComOverflow))
	{
		ComOverflow = true;
		digitalWrite(PIN_DEBUG2, HIGH);
		TX_WRITE('S');
	}
}

/**********************************************************************************/

inline void CheckComBufferUnderflow (void)
{
	if((CommandCounter <= COM_BUFFER_UNDERFLOW_TRIGGER) && (ComOverflow))
	{
		ComOverflow = false;
		digitalWrite(PIN_DEBUG2, LOW);
		TX_WRITE('C');
	}
}

/**********************************************************************************/

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

		case 'P':
   /* char H_Pause = 0;
    char L_Pause = 0;
    
    L_Pause = data[1];
    H_Pause = data[2];
    data[1] = H_Pause;
    data[2] = L_Pause;
    */
		i.ui8[HIGH_BYTE] = data[1];
 		i.ui8[LOW_BYTE] = data[2];

			if (i.ui16 > MAX_PAUSE_VALUE)
				i.ui16 = MAX_PAUSE_VALUE;
			else if (i.ui16 < MIN_PAUSE_VALUE )
				i.ui16 = MIN_PAUSE_VALUE;
       
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

/**********************************************************************************/

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
			else if((data == 'F')|| (data == 'P'))
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

/**********************************************************************************/

void GetSwitchStatus (void)
{
	Switch.Status = 0;

	Switch.ControlMode = digitalRead(PIN_SWITCH_CONTROL_MODE);
	Switch.Status += SwitchStatusTab[SWITCH_STATUS_CONTROL_MODE][Switch.ControlMode];
	Switch.MotorEnable = digitalRead(PIN_SWITCH_MOTOR);
	Switch.Status += SwitchStatusTab[SWITCH_STATUS_MOTOR_ENABLE][Switch.MotorEnable];
	Switch.HeatPC = digitalRead(PIN_SWITCH_HEATING_PC);
	Switch.Status += SwitchStatusTab[SWITCH_STATUS_HEAT_PC][Switch.HeatPC];
	Switch.HeatManu = digitalRead(PIN_SWITCH_HEATING_MANU);
	Switch.Status += SwitchStatusTab[SWITCH_STATUS_HEAT_MANU][Switch.HeatManu];
	Switch.CutterEnable = digitalRead(PIN_SWITCH_HEATING_CUTTER);
	Switch.Status += SwitchStatusTab[SWITCH_STATUS_CUTTER_ENABLE][Switch.CutterEnable];
  //Al>
	//Switch.EndStopShunt = digitalRead(PIN_BP_HOMING);
	//Switch.Status += SwitchStatusTab[SWITCH_STATUS_ENDSTOP_SHUNT][Switch.EndStopShunt];
  //Switch.HomingOk = Switch.HomingOk;
	Switch.Status += SwitchStatusTab[SWITCH_STATUS_HOMING_OK][Switch.HomingOk];
  //<
  //Al>
  byte SomEndStop = (((digitalRead (PIN_X1_LIMIT)^ INV_FDC_X1) <<0)|((digitalRead (PIN_Y1_LIMIT)^INV_FDC_Y1) <<2)|((digitalRead (PIN_X2_LIMIT)^INV_FDC_X2) <<1)|((digitalRead (PIN_Y2_LIMIT)^INV_FDC_Y2) <<3));
  if (SomEndStop== 0) Switch.EndStop = 0 ; else Switch.EndStop = 1;
 
 //<
  Switch.Status += SwitchStatusTab[SWITCH_STATUS_ENDSTOP][Switch.EndStop];

#ifdef DEBUG
	Switch.ControlMode = false;
	Switch.MotorEnable = false;
	Switch.Status &= 0xFE;
#endif
}

/*********************************************************************************/

void HMI_WriteModeManu (void)
{
	lcd.setCursor(0, 2);
	lcd.print("MANU ");
	lcd.setCursor(0, 3);
	lcd.print("          ");
 }

/*********************************************************************************/

void HMI_WriteModePC (void)
{
	lcd.setCursor(0, 2);
	lcd.print(" PC  I  ON  PC   OFF");
	lcd.setCursor(6, 3);
	lcd.print("mm/s");
}

/*********************************************************************************/

inline void HMI_InitScreen (void)
{
	// Welcome text
	lcd.begin(LCD_COLUMN_COUNT, LCD_LINE_COUNT);
	lcd.print(" Jedicut-Alden-USB");
	lcd.setCursor(7, 1);
	lcd.print(Version);
	lcd.setCursor(7, 2);
	lcd.print(BAUDRATE);
	lcd.setCursor(5, 3);
#ifdef BUZZER_ON
	lcd.print("BUZZER ON");
	SoundAlarm(ON);
	delay(500);
	SoundAlarm(OFF);
#else
	lcd.print("BUZZER OFF");
#endif
}

/*********************************************************************************/

inline void HMI_ParamsScreen (void)
{
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("mm/step " + String(MM_PER_STEP, 5));
	lcd.setCursor(0, 1);
	lcd.print (TEXT1 + String(MAX_PERCENTAGE_WIRE, DEC) + "%");
	lcd.setCursor(0, 2);
	lcd.print (TEXT2 + String(MAX_PERCENTAGE_CUTTER, DEC) + "%");
	lcd.setCursor(0, 3);
	#ifdef HEAT_CONSIGN_ROTARY_ENCODER
	lcd.print(TEXT4);
	#else
	lcd.print(TEXT3);
	#endif
}

/*********************************************************************************/

inline void HMI_InitSwitchScreen (void)
{
	lcd.clear();
	lcd.setCursor(0, 1);
	lcd.print(TEXT6);
}

/*********************************************************************************/

inline bool HMI_SwitchInitScreen (void)
{
	static byte old = 0;
	byte status = Switch.Status;

#ifdef DEBUG
	status = 0x1F;
#endif

	if(IsSwitchNotInitialized(status))
	{
		if(old != status)
		{

			lcd.setCursor(0, 2);
			if(!Switch.ControlMode) // test si l'inter Mode est sur Manu
				lcd.print(TEXT7);// attente inter Mode sur Manu
			else if(!Switch.MotorEnable)
				lcd.print(TEXT8); // attente inter Moteur sur OFF
			else if(!Switch.HeatPC || !Switch.HeatManu)
				lcd.print(TEXT9); // attente inter Chauffe sur OFF
			else if(!Switch.CutterEnable)
				lcd.print(TEXT10); // attente inter Moteur sur OFF

			old = status;
		}

		return false;
	}

	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(MachineName);
	lcd.setCursor(0, 1);
	lcd.print(TEXT14);       //"MODE  MOT Chau  CUT"
	lcd.setCursor(0, 3);
	lcd.print("             0%   0%");

#ifdef DEBUG
	digitalWrite(13, LOW);
#endif

	return true;
}

/*********************************************************************************/

inline void HMI_ModeScreen (void)
{
	static byte old = 0;
	char line[21] = {"                    "};

	if(old != Switch.Status)
	{
		if(Switch.ControlMode)
		{
			line[0] = 'M';
			line[1] = 'A';
			line[2] = 'N';
			line[3] = 'U';

			if(Switch.MotorEnable)
			{
				line[7] = 'O';
				line[8] = 'F';
				line[9] = 'F';
			}
			else
			{
				line[8] = 'O';
				line[9] = 'N';
			}

			if(!Switch.HeatManu)
			{
				line[12] = 'M';
				line[13] = 'A';
				line[14] = 'N';
			}
			else if(!Switch.HeatPC)
			{
				line[12] = 'D';
				line[13] = 'I';
				line[14] = 'S';
			}
			else
			{
				line[12] = 'O';
				line[13] = 'F';
				line[14] = 'F';
			}

			if(Switch.CutterEnable)
			{
				line[17] = 'O';
				line[18] = 'F';
				line[19] = 'F';
			}
			else
			{
				line[18] = 'O';
				line[19] = 'N';
			}
		}
		else
		{
			line[1] = 'P';
			line[2] = 'C';

			if(Switch.MotorEnable)
			{
        line[7] = 'O';
        line[8] = 'F';
        line[9] = 'F';
			}
			else
			{
				line[8] = 'P';
				line[9] = 'C';
			}

			if(!Switch.HeatManu)
			{
				line[12] = 'M';
				line[13] = 'A';
				line[14] = 'N';
			}
			else if(!Switch.HeatPC)
			{
				line[13] = 'P';
				line[14] = 'C';
			}
			else
			{
				line[12] = 'O';
				line[13] = 'F';
				line[14] = 'F';
			}

			line[17] = 'D';
			line[18] = 'I';
			line[19] = 'S';
		}


	//	if(Switch.EndStop && Switch.EndStopShunt)
   if(Switch.EndStop && !Switch.ControlMode)
		{
			SoundAlarm(ON);
			line[5] = 'K';
		}
		else
		{
			SoundAlarm(OFF);
			line[5] = 'I';
		}

		lcd.setCursor(0, 2);
		lcd.print(line);

		HMI.ProcessDigit = true;

		old = Switch.Status;
	}
}

/*********************************************************************************/

inline void HMI_ManuDigitScreen (void)
{
	char line[21] = {"              %    %"};

	if((Heat.WireConsign != HMI.WireConsign) || (Heat.CutterConsign != HMI.CutterConsign) || (HMI.ProcessDigit))
	{
		HMI.WireConsign = Heat.WireConsign;
		HMI.CutterConsign = Heat.CutterConsign;

		line[11] = HMI.WireConsign >= 100 ? ('0' + (HMI.WireConsign/100)) : ' ';
		line[12] = HMI.WireConsign >= 10 ? ('0' + ((HMI.WireConsign/10)%10)) : ' ';
		line[13] = '0' + (HMI.WireConsign % 10);

		line[16] = HMI.CutterConsign >= 100 ? ('0' + (HMI.CutterConsign/100)) : ' ';
		line[17] = HMI.CutterConsign >= 10 ? ('0' + ((HMI.CutterConsign/10)%10)) : ' ';
		line[18] = '0' + (HMI.CutterConsign % 10);

		lcd.setCursor(0, 3);
		lcd.print(line);
		HMI.ProcessDigit = false;
	}
}

/*********************************************************************************/

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

/*********************************************************************************/

inline void HMI_DigitScreen (void)
{
	static unsigned long i = 0;

	if(i < millis())
	{
		i = millis() + 100;

		if(Switch.ControlMode)
			HMI_ManuDigitScreen();
		else
			HMI_PcDigitScreen();
	}
}

/*********************************************************************************/

inline void HMI_Manage (void)
{
	static unsigned long i = 0;

	switch(HMI.State)
	{
		case HMI_MODE_SCREEN:
			HMI_ModeScreen();
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

/*********************************************************************************/

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
			if(!Switch.ControlMode && !Switch.EndStop && !Switch.MotorEnable && Switch.HomingOk )
			{
				BufferFlush();
				ENABLE_T5_ISR();
				ENABLE_T1_ISR();
				ENABLE_RX_ISR();
				modeState = MODE_PC;
			}
			else
      {
        EndStopManage ();
        if (Switch.MotorEnable == 0)
          {
          HomingManage () ;
          }
				HeatingManage(MODE_MANU);
      }
			break;

		case MODE_PC:
		//	if(Switch.ControlMode)
      if(Switch.ControlMode || Switch.EndStop || Switch.MotorEnable)
			{
				DISABLE_RX_ISR();
				DISABLE_T1_ISR();
				DISABLE_T5_ISR();

				OCR5A = 255;

				BufferFlush();
				CleanMotorHalt();
				ResetHeat();

				modeState = MODE_MANU;
			}
			else
				HeatingManage(MODE_PC);
			break;
	}
}

/**********************************************************************************/
/**** The main loop                                                           *****/
/**********************************************************************************/
void loop (void)
{
	GetSwitchStatus();
	StepperDriverManage();
  PauseManage(); 
	ModeManage();
	HMI_Manage();
}
