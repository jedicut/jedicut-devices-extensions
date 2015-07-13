{   Copyright 2008 Jerome

    This file is part of VisualCNC_XP.

    VisualCNC_XP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VisualCNC_XP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VisualCNC_XP.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  SysUtils, UType;

  function GetDllFamily : byte; export
  procedure GetDescription(Cible : PChar; tailleCible: integer);
  function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer; chauffe : double) : smallInt; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  function EtatMachine : byte; export;
  function LireChauffeMachine : double; export;
  procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur); export;

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
  periodeChauffe : integer;
  pulseChauffe : integer;
  bSignalChauffeHaut : boolean; // Variable permettant de ne positionner le signal de chauffe qu'une seule fois par périodede chauffe
  horloge, ancienneHorloge : byte; // Variable pour détecter le signal d'horloge
  portAdresseBase : word; // Adresse de base du port parallèle

// ADRESSE_BASE + 0 : registre de données D0 à D7 (Bit 0 à Bit 7)
// ADRESSE_BASE + 1 : registre de statut /!\ DANGER : LECTURE SEULE /!\
// ADRESSE_BASE + 2 : registre de contrôle

// Signal de chauffe : broche 16 - bit 2 du registre de commande
// Timer : broche 10 - bit 6 du registre de statut
// Moteur On/Off : bit 3 du registre de contrôle
// Retour du signal de chauffe : broche 11 - Registre Statut Bit 7

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
  Description := 'Protocole utilisé avec la machine VisualCNC de ITC, compatible Windows XP. Chauffe & utilisation du timer externe. Version 1.3.0.1';
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
    PortOut(portAdresseBase + 2, 0);
  end else
  begin
    // Emettre la remise à zero des bits moteurs
    PortOut(portAdresseBase, 0);
    // Couper l'alimentation des moteurs et mettre la chauffe à 0
    PortOut(portAdresseBase + 2, 8);
  end;
end;

{-----------------------------------------------------------------}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer; chauffe : double) : smallInt;
var
  timeOut : integer;
  i : integer;
  pulse : integer;
  stepOk : boolean;
  codeRetour : smallInt;
  tempsBas : integer; // Si pas de timer
const
  OFFSET_SIGNAL_SENS = 50000; // Si pas de timer
  TEMPS_STEP_HAUT = 100000; // Si pas de timer
begin
  stepOk := false;
  pulse := 0;
  timeOut := 0;
  codeRetour := NO_ERROR;

  if (vitesse>=0) then
  begin
    // Si on a choisit l'utilisation du timer externe
    if (ParametreCommunication.synchroniserMoteurAvecTimerExterne) then
    begin
      if vitesse>1 then vitesse := vitesse-1
      else vitesse := 1;
      while (not stepOk) do  // rajouter la condition que vitesse<>0 pour sortir
      begin
        horloge := PortIn(portAdresseBase+1);
        horloge := horloge and $40; // bit 6
        Inc(timeOut);
        if (timeOut>=TIME_OUT) then
        begin
          codeRetour := ERROR_TIME_OUT;
          break;
        end;
        if ((horloge=$40)and(ancienneHorloge=0)) then
        begin
          timeOut := 0;
          Inc(pulseChauffe);
          Inc(pulse);
          if(pulse=vitesse-1)then
          begin
            // Emettre le sens de rotation
            PortOut(portAdresseBase, bitSens);
          end;
          if(pulse=vitesse) then
          begin
            // Emettre l'ordre de rotation + sens
            PortOut(portAdresseBase, bitRotation + bitSens);
          end;
          if(pulse=vitesse+2)then
          begin
            // Emettre le sens de rotation
            PortOut(portAdresseBase, bitSens);
            stepOk := true; // Pour sortir de la boucle de gestion du pas
          end;
          if ((ParametreChauffe.chauffeActive)and(ParametreChauffe.chauffeUtilisateur)) then
          begin
            // Si Chauffe dynamique
            if ParametreChauffe.chauffeDynamique then
            begin
              if not bSignalChauffeHaut then
              begin
                if pulseChauffe >= periodeChauffe then
                begin
                  // Signal de chauffe à 1
                  PortOut(portAdresseBase+2, 4);
                  bSignalChauffeHaut := true;
                end;
              end;
              // On met à jour le nouveau periodeChauffe
              periodeChauffe := Trunc(Int(chauffe));
            end else begin
              // Chauffe PC mais sans chauffe dynamique
              if (pulseChauffe=periodeChauffe) then
              begin
                // Signal de chauffe à 1
                PortOut(portAdresseBase+2, 4);
              end;
            end;
            if (pulseChauffe=100) then
            begin
              // Signal de chauffe à 0
              PortOut(portAdresseBase+2, 0);
              pulseChauffe:=0;
              bSignalChauffeHaut := false; // Utilisé que pour la chauffe dynamique
            end;
          end;
        end;
        ancienneHorloge := horloge;
      end;
    end else begin
      // Si on n'a pas de chauffe et donc pas de timer externe

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
  // En fait ce n'est pas la période mais le temps ou le signal est à 0
  periodeChauffe := CalculerTempsSignalHaut(MateriauActif.pourcentage1);
  // Compteur d'impulsion de chauffe
  pulseChauffe := 0;
  // Boolean indiquant si le signal de chauffe a été placé au niveau haut
  bSignalChauffeHaut := false;
end;

{-----------------------------------------------------------------}
{ Lire des états de la machine                }
{ - Pour l'instant lecture du mode de chauffe 1 mode manuel, 0 mode PC }
function EtatMachine : byte;
var
  lu : byte;
  retour : byte;
begin
  // Mode de chauffe : Lecture du bit 5 registre ADRESSE_BASE+1
  lu := PortIn(portAdresseBase+1);
  lu := lu and $20; // bit 5

  if (lu=$20) then retour := 0
  else retour := 1;

  Result := retour;
end;

{-----------------------------------------------------------------}
{ Adapter les coordonnées en fonction de la machine }
procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur);
var
  i : integer;
begin
  // Modifier les ordres moteurs pour inclure la gestion de la chauffe
  // chauffe := périodeChauffe (comme pour l'init de la chauffe)
  for i:=0 to Length(ArrayOrdres.ArrayOrdres)-1 do
  begin
    ArrayOrdres.ArrayOrdres[i].chauffe := CalculerTempsSignalHaut(ArrayOrdres.ArrayOrdres[i].chauffe);
  end;
end;

{-----------------------------------------------------------------}
{ Fonction retournant la valeure de la chauffe }
function LireChauffeMachine : double;
var
  ITERATION, timeOut : integer;
  lu : byte;
  periode, nivHaut, nivBas : integer;
  zero : boolean;
begin
  ITERATION := 500000;
  periode := 0;
  zero := false;
  nivHaut := 0;
  nivBas := 0;
  timeOut := 0;

  while ((periode<10)and(timeOut<ITERATION)) do
  begin
    // Contrôle du timeOut
    if (timeOut<ITERATION) then
    begin
      lu := PortIn(portAdresseBase+1);
      lu := lu and $80; // bit 7

      Inc(timeOut);

      if (lu = $80) then
      begin
        Inc(nivHaut);
        if zero then
        begin
          zero := false;
          Inc(periode);
          timeOut := 0; // RAZ du compteur de time out
        end;
      end else begin
        zero := true;
        Inc(nivBas);
      end;
    end;
  end;

  if (periode=0) then
    Result := -1
  else
    Result := nivBas / (nivHaut+nivBas); // nivHaut/(nivHaut+nivBas) donne une chauffe inversée : chauff max = 0%
end;

end.
