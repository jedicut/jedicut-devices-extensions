#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-03-10 19:42:54

#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <LiquidCrystal.h>
#include <avr/iom2560.h>
#include "conf.h"
#include "lang.h"

void setup (void) ;
void AideMiseServiceFdc(void);
void Test_fdc(void);
void Affic_Trame_fdc(void);
void limits_Lect(void);
void Aff_Test_Fdc (void);
void HomingManage (void)
void Arm_Homing (void);
void Desar_Homing (void);
void homing_set (void);
void cycle_1(void);
void cycle_2(void);
void cycle_3(void);
void cycle_4(void);
void prepos_set(void);
void EndStopManage (void);
void Trait_Arr_fdc(void);
void testPosIntDem(void);

inline void PauseManage (unsigned int pms)
inline void StepperDriverDir (byte dir) ;
inline void StepperDriverStep (byte step) ;
inline void ProcessStep(byte cmd) ;
inline void StepperDriverEnable (bool en) ;
void StepperDriverManage (void) ;
void CleanMotorHalt (void) ;
inline void HeatingRelay (byte en) ;
inline void ComputeRotaryEncoderHeatConsign (void) ;
inline void HeatingManage (byte mode) ;
void ResetHeat (void) ;
inline void SoundAlarm (bool en) ;
inline void ProcessCommand (void) ;
inline void IsrProcessBuffer (void) ;
ISR(TIMER1_COMPA_vect) ;
ISR(TIMER2_COMPA_vect) ;
ISR(TIMER5_COMPA_vect) ;
ISR(USART0_RX_vect) ;
void BufferFlush (void) ;
inline void CmdBufferWrite (unsigned char *Data) ;
inline void CheckComBufferOverflow (void) ;
inline void CheckComBufferUnderflow (void) ;
inline void DataProcess (unsigned char *data) ;
inline void ComParse (void) ;
void GetSwitchStatus (void) ;
void HMI_WriteModeManu (void) ;
void HMI_WriteModePC (void) ;
inline void HMI_InitScreen (void) ;
inline void HMI_ParamsScreen (void) ;
inline void HMI_InitSwitchScreen (void) ;
inline bool HMI_SwitchInitScreen (void) ;
inline void HMI_ModeScreen (void) ;
inline void HMI_ManuDigitScreen (void) ;
inline void HMI_PcDigitScreen (void) ;
inline void HMI_DigitScreen (void) ;
inline void HMI_Manage (void) ;
inline void ModeManage (void) ;
void loop (void) ;

#include "LMFAO_V3_3.ino"


#endif
