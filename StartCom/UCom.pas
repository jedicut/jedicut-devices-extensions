{   Copyright 2008 Jerome

    This file is part of StartCom.

    StartCom is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    StartCom is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CncNet98.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  SysUtils, UType;

  // Fonction renvoyant le code famille de la dll, ce code indique le type de la dll
  // Les codes possibles : voir UType.pas, fichier commun aux dll
  function GetDllFamily : byte; export

  // Méthode de la dll permettant de définir si la dll propose une IHM d'initialisation
  // GetDllToInit est obligatoirement renseigné si ShowDllForm est renseignée pour une dll de communication
  // function GetDllToInit : integer;

  // Méthode passant le Handle de l'application
  // procedure ShowDllForm(appHandle : HWND);

  procedure GetDescription(Cible : PChar; tailleCible: integer);
  function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer; chauffe : double) : smallInt ; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  function EtatMachine : byte; export;
  function LireChauffeMachine : double; export;

  procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur); export;

  // Vu que dans dll USB
  //procedure LibererRessources; export;
  //function GetChauffeMachine : double; export;

implementation

const TIME_OUT = 10000;

var
  ParametreChauffe : TParametreChauffe;
  ParametreCommunication : TParametreCommunication;
  MateriauActif : TMateriau;
  periodeChauffe : integer;
  pulseChauffe : integer;
  bSignalChauffeHaut : boolean; // Variable permettant de ne positionner le signal de chauffe qu'une seule fois par périodede chauffe
  horloge, ancienneHorloge : byte; // Variable pour détecter le signal d'horloge
  portAdresseBase : word; // Adresse de base du port parallèle

{-----------------------------------------------------------------}
{--- Parallel port specifications ---}
{-----------------------------------------------------------------}
// ADRESSE_BASE + 0 : registre de données D0 à D7 (Bit 0 à Bit 7)
// ADRESSE_BASE + 1 : registre de statut /!\ DANGER : LECTURE SEULE /!\
// ADRESSE_BASE + 2 : registre de contrôle

{-----------------------------------------------------------------}
{--- MM2001 specifications ---}
{-----------------------------------------------------------------}
// Signal de chauffe : broche 16 - bit 2 du registre de commande
// Timer : broche 10 - bit 6 du registre de statut
// Moteur On/Off : bit 3 du registre de contrôle
// Retour du signal de chauffe : broche 11 - Registre Statut Bit 7

{-----------------------------------------------------------------}
{ Renvoie le type de la dll }
function GetDllFamily : byte;
begin
  Result := DLL_FAMILY_COM;
end;

{-----------------------------------------------------------------}
{ Renvoie la description de la dll }
procedure GetDescription(Cible : PChar; tailleCible: integer);
var
  Description : ShortString;
begin
  Description := 'Petite description du Plugin / A small description of the plugin. Version 0.0';
  StrPLCopy(Cible, Description, tailleCible);
end;

{-----------------------------------------------------------------}
{ Méthode de la dll gérant l'alimentation des moteurs }
procedure MoteurOnOff(moteurOn : boolean);
begin
  // Gérer l'alimentation des moteurs
  if moteurOn then
  begin
    // Alimenter les moteurs
  end else begin
    // Emettre la remise à zero des bits moteurs
    // Couper l'alimentation des moteurs et mettre la chauffe à 0
  end;
end;

{-----------------------------------------------------------------}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer; chauffe : double) : smallInt;
var
  codeRetour : smallInt;
begin
  codeRetour := NO_ERROR;
  Result := codeRetour;
end;

{-----------------------------------------------------------------}
{ Initialiser les paramètre de la chauffe chauffe }
procedure InitialiserChauffeEtCommunication(portBase : word ; ParamChauffe : TParametreChauffe ; ParamCommunication : TParametreCommunication ; Materiau : TMateriau);
begin

end;

{-----------------------------------------------------------------}
{ Lire des états de la machine                }
{ - Pour l'instant lecture du mode de chauffe 1 mode manuel, 0 mode PC }
function EtatMachine : byte;
var
  retour : byte;
begin
  // Mode de chauffe : Lecture du bit 5 registre ADRESSE_BASE+1

  Result := retour;
end;

{-----------------------------------------------------------------}
{ Adapter les coordonnées en fonction de la machine }
procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur);
begin

end;

{-----------------------------------------------------------------}
{ Fonction retournant la valeure de la chauffe }
function LireChauffeMachine : double;
var
  retour : double;
begin

  Result := retour;
end;

end.
