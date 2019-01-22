{   Copyright 2009 Jerome

    This file is part of MDLCNC_XP.

    MDLCNC_XP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MDLCNC_XP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MDLCNC_XP.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  SysUtils, UType;

  function GetDllFamily : byte; export
  procedure GetDescription(Cible : PChar; tailleCible: integer);
  function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur); export;

  // New functions to adapt Jedicut capacities (and simplify settings)
  function GetDllAcceptSmoothMove() : smallInt; export;
  function GetDllAcceptHeatingControl() : smallInt; export;
  function GetDllSendExternalTimer() : smallInt; export;
  function GetDllSendHeatingSignal() : smallInt; export;
  function GetDllSendHeatingStatus() : smallInt; export;
  function GetDllAcceptOnOffControl() : smallInt; export;
  function GetDllPicture() : smallInt; export;

  // Fonctions privées

  procedure PortOut(Port : Word; Data : Byte); stdcall; external 'io.dll';
  function PortIn(Port : Word) : Byte; stdcall; external 'io.dll';

  function CalculerTempsSignalHaut(valChauffe : double) : integer;
implementation

const TIME_OUT = 10000;

var
  ParametreChauffe : TParametreChauffe;
  ParametreCommunication : TParametreCommunication;
  MateriauActif : TMateriau;
  portAdresseBase : word; // Adresse de base du port parallèle

// ADRESSE_BASE + 0 : registre de données D0 à D7 (Bit 0 à Bit 7)
// ADRESSE_BASE + 1 : registre de statut /!\ DANGER : LECTURE SEULE /!\
// ADRESSE_BASE + 2 : registre de contrôle

// Signal de chauffe : broche 16 - bit 2 du registre de commande
// Timer : broche 10 - bit 6 du registre de statut
// Moteur On/Off : bit 3 du registre de contrôle
// Retour du signal de chauffe : broche 11 - Registre Statut Bit 7
// Activation d'un relai via le pin 14 (Registre controle Bit 1) en même temps que l'alimentatino des moteurs,
//  spécifique à la carte de Frank

{-----------------------------------------------------------------}
{ Renvoie le type de la dll }
function GetDllFamily : byte;
begin
  Result := 0;
end;

{-----------------------------------------------------------------}
{ Renvoie la description de la dll }
procedure GetDescription(Cible : PChar; tailleCible: integer);
var
  Description : ShortString;
begin
  Description := 'MDLCNC with switch on pin 14, no timer and no heating control. Version 1.2';
  StrPLCopy(Cible, Description, tailleCible);
end;

{-----------------------------------------------------------------}
{ Méthode de la dll gérant l'alimentation des moteurs }
procedure MoteurOnOff(moteurOn : boolean);
begin
  // Gérer l'alimentation des moteurs
  if moteurOn then
  begin
    // Activer le relais de la chauffe
    PortOut(portAdresseBase + 2, 2);
  end else begin
    // Emettre la remise à zero des bits moteurs
    PortOut(portAdresseBase, 0);
    // Désactiver le relais de la chauffe
    PortOut(portAdresseBase + 2, 0);
  end;
end;

{-----------------------------------------------------------------}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt;
var
  i : integer;
  codeRetour : smallInt;
  tempsBas : integer;
const
  OFFSET_SIGNAL_SENS = 50000;
  TEMPS_STEP_HAUT = 100000;
begin
  codeRetour := NO_ERROR;

  if (vitesse>=0) then
  begin
    // Si on n'a pas choisi l'utilisation du timer externe
    if not ParametreCommunication.synchroniserMoteurAvecTimerExterne then
    begin
      tempsBas := vitesse - TEMPS_STEP_HAUT;
      if (tempsBas<=0) then tempsBas := TEMPS_STEP_HAUT;
      // Emettre le sens de rotation
      PortOut(portAdresseBase, bitSens);
      for i := 0 to OFFSET_SIGNAL_SENS do
      begin
      end;
      // Emettre l'ordre de rotation + sens
      PortOut(portAdresseBase, bitRotation + bitSens);
      for i := 0 to TEMPS_STEP_HAUT do
      begin
      end;
      // Emettre la remise à zero des bits moteurs
      PortOut(portAdresseBase, bitSens);
      for i := 0 to tempsBas do
      begin
      end;
    end;
  end else begin
    Sleep(-1 * vitesse);
  end;

  Result := codeRetour;
end;

{-----------------------------------------------------------------}
{ Calculer le temps durant lequel le signal de chauffe doit être à 1 }
function CalculerTempsSignalHaut(valChauffe : double) : integer;
begin
  Result := 100-Trunc(Int(valChauffe));
end;

{-----------------------------------------------------------------}
{ Initialiser les paramètre de la chauffe chauffe }
procedure InitialiserChauffeEtCommunication(portBase : word ; ParamChauffe : TParametreChauffe ; ParamCommunication : TParametreCommunication ; Materiau : TMateriau);
begin
  portAdresseBase := portBase;
  ParametreChauffe := ParamChauffe;
  ParametreCommunication := ParamCommunication;
  MateriauActif := Materiau;
end;

{-----------------------------------------------------------------}
{ Adapter les coordonnées en fonction de la machine }
procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur);
var
  ArrayTampon : array [0..3] of TOrdreMoteur;
  i, j : integer;
begin
  // Inititalisation du tableau
  for i:=0 to 3 do
  begin
    ArrayTampon[i].bitRotation := 0;
    ArrayTampon[i].bitSens := 0;
  end;
  // Modifier les ordres moteurs
  // Objectif : maintenir les sens de rotation de chaque moteur entre leurs ordres de rotation
  for i:=0 to Length(ArrayOrdres.ArrayOrdres)-1 do
  begin
    // Si on a des mouvements moteur par moteur (pas 2 mouvements moteurs au même  instant)
    if (ArrayOrdres.ArrayOrdres[i].bitRotation in [1,2,4,8,16,32,64,128]) then
    begin
      for j := 0 to 3 do
      begin
        // On cherche l'historique du sens moteur i
        if ((ArrayTampon[j].bitRotation = 0)or
            (ArrayTampon[j].bitRotation = ArrayOrdres.ArrayOrdres[i].bitRotation)) then
        begin
          // On recopie le byte moteur
          ArrayTampon[j].bitRotation := ArrayOrdres.ArrayOrdres[i].bitRotation;
          // On change la valeur de l'historique
          ArrayTampon[j].bitSens := ArrayOrdres.ArrayOrdres[i].bitSens;
          break;
        end;
      end;
      // On affecte le sens de rotation en additionnant tous les historiques
      ArrayOrdres.ArrayOrdres[i].bitSens :=
        ArrayTampon[0].bitSens +
        ArrayTampon[1].bitSens +
        ArrayTampon[2].bitSens +
        ArrayTampon[3].bitSens;
    end else begin
      // Réinitialisation des historiques suite à un mouvement simutané de 2 moteurs pour lequel il ne faut pas gérer d'historique de sens de rotation
      for j := 0 to 3 do
      begin
        ArrayTampon[j].bitRotation := 0;
        ArrayTampon[j].bitSens := 0;
      end;
    end;
  end;

  // Modifier les ordres moteurs pour inclure la gestion de la chauffe
  // chauffe := périodeChauffe (comme pour l'init de la chauffe)
  for i:=0 to Length(ArrayOrdres.ArrayOrdres)-1 do
  begin
    ArrayOrdres.ArrayOrdres[i].chauffe := CalculerTempsSignalHaut(ArrayOrdres.ArrayOrdres[i].chauffe);
  end;

end;


{-----------------------------------------------------------------}
{ New functions to adapt Jedicut capacities (and simplify settings)
{-----------------------------------------------------------------}

{-----------------------------------------------------------------}
{ Enable smooth movement - dependence on EmettreBit function }
{ 0=false | 1=true }
function GetDllAcceptSmoothMove() : smallInt; export;
begin
  Result := 1;
end;

{-----------------------------------------------------------------}
{ What kind of heating control propose the cnc controller ? }
{ 0=false
  1=true static with pin number
  2=true dynamic with pin number
  3=true static without pin number
  4=true dynamic without pin number
}
function GetDllAcceptHeatingControl() : smallInt; export;
begin
  Result := 0;
end;

{-----------------------------------------------------------------}
{ Can cnc controller have an external timer in output ? }
{ 0=false | 1=true }
function GetDllSendExternalTimer() : smallInt; export;
begin
  Result := 1;
end;

{-----------------------------------------------------------------}
{ Does cnc controller have a heating signal in output ? }
{ 0=false | 1=true }
function GetDllSendHeatingSignal() : smallInt; export;
begin
  Result := 0;
end;

{-----------------------------------------------------------------}
{ Does cnc controller have a heating status signal in output ? }
{ 0=false | 1=true }
function GetDllSendHeatingStatus() : smallInt; export;
begin
  Result := 1;
end;

{-----------------------------------------------------------------}
{ Does cnc controller need on/off motor signal pin number ? }
{ 0=false | 1=true }
function GetDllAcceptOnOffControl() : smallInt; export;
begin
  Result := 1;
end;

{-----------------------------------------------------------------}
{ The kind of plugin of communication }
{ DLL_IMG_PARALLEL_PORT = 0
  DLL_IMG_GCODE = 1
  DLL_IMG_ARDUINO = 2
}
function GetDllPicture() : smallInt; export;
begin
  Result := 0;
end;



end.
