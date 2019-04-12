/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2012 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#ifdef __IN_ECLIPSE__


#include "Arduino.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <LiquidCrystal.h>
#include <avr/iom2560.h>
#include "conf.h"
#include "lang.h"




void setup (void) ;
void BufferFlush (void) 
inline void PauseManage (unsigned int pms)
void StepperDriverDir (byte dir);
void StepperDriverStep (byte step);
void StepperDriverEnable (bool en);
void ProcessStep(byte cmd);
void StepperDriverManage (void);
void CleanMotorHalt (void);
void HeatingRelay (byte en);
ISR(TIMER1_COMPA_vect) ;
ISR(TIMER2_COMPA_vect) ;
ISR(TIMER5_COMPA_vect) ;
ISR(USART0_RX_vect) ;
void HeatingManage (void);
void HMI_InitScreen (void);
void HMI_ParamsScreen (void);
void HMI_InitSwitchScreen (void);
bool HMI_SwitchInitScreen (void);
void HMI_PcDigitScreen (void);
void HMI_DigitScreen (void);
void ProcessCommand (void);
void IsrProcessBuffer (void);
void CheckComBufferOverflow (void);
void CheckComBufferUnderflow (void);
void CmdBufferWrite (unsigned char *Data);
void DataProcess (unsigned char *data);
void ComParse (void);
void HMI_Manage (void);
void ModeManage (void)
void loop (void) ;




#include "LMFAO_1_4.ino"


#endif
