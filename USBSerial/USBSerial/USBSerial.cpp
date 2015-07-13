/*  Copyright 2012 Martin

    This file is part of jedicutplugin.

    Foobar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "USBSerial.h"
#include <stdio.h>
#include <malloc.h>

  TParametreChauffe ParametreChauffe;
  TParametreCommunication* ParametreCommunication;
  TMateriau* MateriauActif;
  unsigned short portAdresseBase; // Adresse de base du port parallèle
  int currentVitesse = 0;
  int currentChauffe = 0;

#ifdef DEBUG_OUTPUT
  extern FILE *fp;    // File pointer for logging
#endif
  extern void writeCommand(char* cmd);

char rotSum  = 0;
char sensSum = 0;
/*********************************************************************************************/
void sendMotorCmd(unsigned char rot, unsigned char sens)
{
char cmd[3];

   // move all different motors with the same command

   if(rot & rotSum)
   {
		cmd[0] = 'M';
		cmd[1] = rotSum+sensSum;
		writeCommand(cmd);

		rotSum = rot;
		sensSum = sens;
   } 
   else
   {
	   rotSum += rot;
	   sensSum |= sens;
   }


	return;
}



/*********************************************************************************************/
JEDICUTPLUGIN_API   unsigned char GetDllFamily()
{
#ifdef DEBUG_OUTPUT
	fprintf(fp,"GetDllFamily:\n");
#endif
	return(0);
}

/*********************************************************************************************/
JEDICUTPLUGIN_API  short EmettreBit(double chauffe) // unsigned char bitRotation, unsigned char bitSens, int vitesse,double chauffe
{
	unsigned char bitRotation;
	int bitRotationW;
	unsigned char bitSens;
	int bitSensW;
	int vitesse;
	char cmd[3];
	 _asm {
		 mov bitRotationW, eax
		 mov bitSensW,edx
		 mov vitesse,ecx
	 }
	bitSens = bitSensW & 0xff;
	bitRotation = bitRotationW & 0xff;
#ifdef DEBUG_OUTPUT
	fprintf(fp,"bitRot(Takt): 0x%02X bitSens(Richt.): 0x%02x vitesse: %d chauffe: %f\n",bitRotation,bitSens,vitesse,chauffe);
#endif

	// check if chauffe changed
	if((int)chauffe != currentChauffe)
	{
		// emit command to set chauffe (only valus between 0 and 255 allowed)
		cmd[0] = 'H';
		cmd[1] = (int)chauffe & 0xff;
		writeCommand(cmd);
		currentChauffe = (int)chauffe;
	}

	// send motor command
	sendMotorCmd(bitRotation,bitSens);

	// check if Vitesse changed
	if(vitesse != currentVitesse)
	{
		// emit command to set vitesse (only valus between 0 and 255 allowed)
		cmd[0] = 'F';
		cmd[1] = vitesse & 0xff;
		writeCommand(cmd);
		currentVitesse = vitesse;
	}


	return(0);
}

/*********************************************************************************************/
JEDICUTPLUGIN_API  void MoteurOnOff() // parameter: bool moteurOn
{
	int moteurOnW;
	bool moteurOn;  
	_asm {
		mov moteurOnW,eax
	}
	moteurOn = (bool)(moteurOnW & 0xff); // only one Byte
	// also turn heating off here TODO
#ifdef DEBUG_OUTPUT
	fprintf(fp,"MoteurOnOff: 0x%02x\n",moteurOn);
#endif

	if(moteurOn) 
	{ 
		writeCommand("A1"); // Motors on

		// Initialise heat and delay values, so they are set new
		// in case of a reset of the controller (added after comment from ZbigPL)
		currentVitesse = 0;
		currentChauffe = 0;		
	}
	else         
	{  
		// is there an outstanding step?
		sendMotorCmd(rotSum, sensSum);
		rotSum=sensSum=0;
		// Heat off, all Motors off
		writeCommand("H\000");     
		writeCommand("A0"); 
	}
}

#ifdef DEBUG_OUTPUT
void logTBitPort(TBitPort* tbp, char* text)
{
	fprintf(fp,"   %s: adresseBase: %d  iRegistre: %d  iBit: %d\n",text,tbp->adresseBase,tbp->iRegistre,tbp->iBit);
}
#endif
/*********************************************************************************************/
JEDICUTPLUGIN_API  void InitialiserChauffeEtCommunication(TMateriau* Materiau) // unsigned short portBase, TParametreChauffe* ParamChauffe,
                                                             // TParametreCommunication* ParamCommunication, TMateriau* Materiau
{
 int portBaseW;
 int ParamChauffe;
 TParametreCommunication* ParamCommunication;
	 _asm {
		 mov portBaseW, eax
		 mov ParamChauffe,edx
		 mov ParamCommunication,ecx
	 }
	 
	 ParametreChauffe.chauffeActive = (bool)(ParamChauffe & 0x000000ff);
	 ParametreChauffe.chauffeMode = (ParamChauffe & 0x0000ff00)>>8;
	 ParametreChauffe.chauffeDynamique = (ParamChauffe & 0x00ff0000)>>16;
	 ParametreChauffe.chauffeUtilisateur = (ParamChauffe & 0xff000000)>>24;
	 MateriauActif = Materiau;
	 ParametreCommunication = ParamCommunication;
	 portAdresseBase = (unsigned short)(portBaseW & 0xffff);


#ifdef DEBUG_OUTPUT
	 fprintf(fp,"InitialiserChauffeEtCommunication: portBase: 0x%04x\n",portAdresseBase);
	 fprintf(fp,"   chauffeActive: %d  chauffeMode: %d  chauffeDynamique: %d chauffeUtilisateur: %d\n",ParametreChauffe.chauffeActive,ParametreChauffe.chauffeMode,ParametreChauffe.chauffeDynamique, ParametreChauffe.chauffeUtilisateur); 
	 fprintf(fp,"   synchroniserMoteurAvecTimerExterne: %d  frequenceTimer: %lf\n",ParametreCommunication->synchroniserMoteurAvecTimerExterne,ParametreCommunication->frequenceTimer);  
	 logTBitPort(&(ParametreCommunication->BitModeChauffe),"BitModeChauffe");
	 logTBitPort(&(ParametreCommunication->BitHorlogeExterne),"BitHorlogeExterne");
	 logTBitPort(&(ParametreCommunication->BitEmissionChauffe),"BitEmissionChauffe");
	 logTBitPort(&(ParametreCommunication->BitReceptionChauffe),"BitReceptionChauffe");
	 logTBitPort(&(ParametreCommunication->BitAlimMoteur),"BitAlimMoteur");
#endif
}

/*********************************************************************************************/
JEDICUTPLUGIN_API  unsigned char EtatMachine()
{	
#ifdef DEBUG_OUTPUT
	 	fprintf(fp,"EtatMachine:\n");
#endif
		// later here maybe general all motors on etc..
	return(1);
}
 
/*********************************************************************************************/
JEDICUTPLUGIN_API  double LireChauffeMachine()
{
#ifdef DEBUG_OUTPUT
	 	fprintf(fp,"LireChauffeMachine:\n");
#endif
	return(0);
}

/*********************************************************************************************/
JEDICUTPLUGIN_API void GetDescription() // parameters: char* Cible , int tailleCible
{
char* Cible; 
int tailleCible;

	_asm {
      mov Cible,eax
      mov tailleCible,edx
	}
	strncpy(Cible,"Serial Plugin for use with USB2COM adapter or pure serial interface",tailleCible);
}


/*********************************************************************************************/
JEDICUTPLUGIN_API  void AdapterOrdres() // parameters: TArrayOrdresMoteur ArrayOrdres
{
TArrayOrdresMoteur* ArrayOrdres;
	
	// Because of the Delphi calling convention we need to get the parameter from register EAX
	_asm {
      mov ArrayOrdres,eax
	}
TOrdreMoteur  ArrayTampon[4];
TOrdreMoteur* arrayMotoeur = ArrayOrdres->ArrayOrdres;
int i,j;

#ifdef DEBUG_OUTPUT
	fprintf(fp,"AdapterOrdres:\n");
#endif
	return;

  // Initialisation
  for(i=0;i<4;i++)
  {
	  ArrayTampon[i].bitRotation = 0;
	  ArrayTampon[i].bitSense    = 0;
  }

  // Modifiy the motor order
  // maintaining the direction of rotation of each motor

  i=0;
  unsigned char bitRot;
  while(bitRot = arrayMotoeur[i].bitRotation) // because we don't know the size of the array, we trust in the first bitRot==0
  {
	  
	  if(bitRot == 1 || bitRot == 2 || bitRot == 4 || bitRot == 8 ||bitRot == 16 || bitRot == 32 || bitRot == 64 || bitRot == 128 )
	  {
		  for(j=0;j<4;j++)
		  {
			  if(ArrayTampon[j].bitRotation==0 || ArrayTampon[j].bitRotation == arrayMotoeur[i].bitRotation)
			  {
				  ArrayTampon[j].bitSense = arrayMotoeur[i].bitSense;
				  ArrayTampon[j].bitRotation = arrayMotoeur[i].bitRotation;
				  break;
			  }
		  }

		  arrayMotoeur[i].bitSense = ArrayTampon[0].bitSense+ArrayTampon[1].bitSense+ArrayTampon[2].bitSense+ArrayTampon[3].bitSense;

	  }
	  else
	  {
		  // Reset old values
		  for(j=0;j<4;j++)
		  {
			  ArrayTampon[j].bitRotation = 0;
			  ArrayTampon[j].bitSense    = 0;
		  }
	  }

	  // integerize heating
	  arrayMotoeur[i].chauffe = (int)(arrayMotoeur[i].chauffe);
	  i++;
  }


}
