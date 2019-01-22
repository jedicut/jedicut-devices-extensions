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

	This structures definedin this fale are based on the delphi records
	defined within the software jedicut.
*/

#ifdef JEDICUTPLUGIN_EXPORTS
#define JEDICUTPLUGIN_API __declspec(dllexport)
#else
#define JEDICUTPLUGIN_API __declspec(dllimport)
#endif

#pragma pack(1) // needed to fit the delphi records


typedef struct {
double vitesse1;
double pourcentage1;
double vitesse2;
double pourcentage2;
} TMateriau;


typedef struct {
	unsigned char bitRotation;
	unsigned char bitSense;
	int           vitesse;
	double        chauffe;
	unsigned char keyPoint;  
} TOrdreMoteur;
 

typedef struct {
	TOrdreMoteur* ArrayOrdres;  
} TArrayOrdresMoteur;


typedef struct {
	int adresseBase;
	int iRegistre;
	int iBit;
} TBitPort;

typedef struct {
	TBitPort BitModeChauffe;
	TBitPort BitHorlogeExterne;
	TBitPort BitEmissionChauffe; 
	TBitPort BitReceptionChauffe;
	TBitPort BitAlimMoteur;
	bool synchroniserMoteurAvecTimerExterne;
	double frequenceTimer;
 } TParametreCommunication;
  
  
typedef struct {
	bool chauffeActive;
	bool chauffeMode;
	bool chauffeDynamique;
	bool chauffeUtilisateur;
} TParametreChauffe;


//extern "C" {
JEDICUTPLUGIN_API  unsigned char  GetDllFamily();  

JEDICUTPLUGIN_API  short EmettreBit(double chauffe);  // unsigned char bitRotation, unsigned char bitSens, int vitesse, double chauffe
JEDICUTPLUGIN_API  void MoteurOnOff(); // parameter: bool moteurOn
JEDICUTPLUGIN_API  void InitialiserChauffeEtCommunication(TMateriau* Materiau); // unsigned short portBase, TParametreChauffe ParamChauffe,
                                                                                // TParametreCommunication ParamCommunication, TMateriau Materiau
JEDICUTPLUGIN_API  unsigned char EtatMachine();
JEDICUTPLUGIN_API  double LireChauffeMachine();
JEDICUTPLUGIN_API  void GetDescription(); // Parameter: char* Cible , int tailleCible
JEDICUTPLUGIN_API  void AdapterOrdres();  // parameters: TArrayOrdresMoteur ArrayOrdres

JEDICUTPLUGIN_API  short GetDllAcceptSmoothMove();
JEDICUTPLUGIN_API  short GetDllAcceptHeatingControl();
JEDICUTPLUGIN_API  short GetDllSendExternalTimer();
JEDICUTPLUGIN_API  short GetDllSendHeatingSignal();
JEDICUTPLUGIN_API  short GetDllSendHeatingStatus();
JEDICUTPLUGIN_API  short GetDllAcceptOnOffControl();
JEDICUTPLUGIN_API  short GetDllPicture();

// $(SolutionDir)$(Configuration)\

//}
