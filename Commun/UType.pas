{   Copyright 2011 Jerome

    This file is part of OpenDxf.

    OpenDxf is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenDxf is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenDxf.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UType;

interface

{ Caractéristique d'un matériaux }
{ }
type
  PMateriau = ^TMateriau;
  TMateriau = packed record
    vitesse1 : double;
    pourcentage1 : double;
    vitesse2 : double;
    pourcentage2 : double;
  end;

{ Type d'identification d'un registre du port parallèle }
{ - 0 : registre de données
  - 1 : registre de statut /!\ LECTURE SEULE /!\
  - 2 : registre de commande }
{ }
type
  TBitPort = packed record
    adresseBase, iRegistre, iBit : integer;
  end;

{ Caractéristique de la communication }
{ }
type
  TParametreCommunication = packed record
    BitModeChauffe, BitHorlogeExterne, BitEmissionChauffe, BitReceptionChauffe, BitAlimMoteur : TBitPort;
    synchroniserMoteurAvecTimerExterne : boolean; // true si un timer externe est utilisé pour gérer le mouvement des moteurs
	frequenceTimer : double; // Fréquence en kHz
  end;

{ Caractéristique de la chauffe }
{ }
type
  TParametreChauffe = packed record
    chauffeActive, chauffeMode, chauffeDynamique : boolean;
    chauffeUtilisateur : boolean; // parametre utile lors du pilotage manuel de la machine pour activer/désactiver la chauffe
  end;


{ Informations nécessaires pour UNE rotation moteur }
{ }
type
  TOrdreMoteur = packed record
    bitRotation : Byte; // Bit de rotation moteur
    bitSens : Byte;     // Bit de sens moteur
    vitesse : integer;  // Durées d'impulsion (vitesse)
    chauffe : double;   // Pourcentage de chauffe
    keyPoint : boolean;
  end;

{ Tableau d'ordres moteurs }
{ }
type
  TArrayOrdresMoteur = packed record
    ArrayOrdres : array of TOrdreMoteur;
  end;

{ Définition des constantes communes aux dll }
const DLL_FAMILY_COM = 0; // Jedicut calculate and send steps
const DLL_FAMILY_COM_SEGMENT = 7; // Jedicut calculate segment (cutting wizard needed). Another software have to interpolate this
const DLL_FAMILY_FILE_PROFIL = 1;
const DLL_FAMILY_FILE_PROFIL_READ_ONLY = 2;
const DLL_FAMILY_FILE_PROFIL_WRITE_ONLY = 3;
const DLL_FAMILY_FILE_CUT = 4;
const DLL_FAMILY_FILE_CUT_READ_ONLY = 5;
const DLL_FAMILY_FILE_CUT_WRITE_ONLY = 6;

{ Définition des constantes spécifiques aux dll de communication}
const DLL_CAPABILITY_SMOOTH_TRUE = 1;
const DLL_CAPABILITY_SMOOTH_FALSE = 0;

const NO_ERROR = 0;
const ERROR_TIME_OUT = -1;
const ERROR_ON_SENDING = -2;
const ERROR_EMERGENCY = -3;
const ERROR_LIMIT_SWITCH = -4;

{--- Définition des constantes spécifiques aux dll de fichier ---}

{ Définition d'un point d'un profil }
type
  TPointProfil = record
    X : double;
    Y : double;
    keyPoint : boolean;
    valChauffe : double;
  end;

{ Définition des coordonnées d'un profil }
type
  TCoordonneesProfil = record
    coordonneesExDecoupe : array of TPointProfil;
    coordonneesInDecoupe : array of TPointProfil;
  end;

implementation


end.
