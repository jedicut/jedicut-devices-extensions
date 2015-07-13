{   Copyright 2008 Jerome

    This file is part of MaxCom.

    MaxCom is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MaxCom is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MaxCom.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  SysUtils, UType;

  function GetDllFamily : byte; export
  procedure GetDescription(Cible : PChar; tailleCible: integer);
  function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt ; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  function EtatMachine : byte; export;

implementation

var
  portAdresseBase : word; // Adresse de base du port parallèle

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
  Description := 'Protocole utilisé avec la machine prototype, compatible Windows 95/98. Version 0.7.1';
  StrPLCopy(Cible, Description, tailleCible);
end;

{-----------------------------------------------------------------}
{ Méthode d'émission des bits en assembleur }
procedure aout(adresse : word ; dat : byte);assembler;
asm
  mov al,dat
  mov dx, adresse
  out dx,al
end;

{-----------------------------------------------------------------}
{ Méthode de la dll gérant l'alimentation des moteurs }
procedure MoteurOnOff(moteurOn : boolean);
begin
  // Gérer l'alimentation des moteurs
  if moteurOn then
  begin
    // Alimenter les moteurs et mettre la chauffe à 0
    aout(portAdresseBase + 2, 0);
  end else
  begin
    // Emettre la remise à zero des bits moteurs
    aout(portAdresseBase, 0);
    // Couper l'alimentation des moteurs et mettre la chauffe à 0
    aout(portAdresseBase + 2, 8);
  end;
end;

{-----------------------------------------------------------------}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt;
var
  dat : byte;
  i : integer;
begin
  // Calcul
  dat := bitRotation + bitSens;

  // Envoi du bit
  aout(portAdresseBase, dat);

  // Temporisation
  if (vitesse >= 0) then
  begin
    for i := 0 to vitesse do
    begin
    end;
  end
  else
  begin
    Sleep(-1 * vitesse);
  end;

  // Envoi de 0 (horloge)
  aout(portAdresseBase, 0);

  Result := NO_ERROR;
end;

{-----------------------------------------------------------------}
{ Initialiser les paramètre de la chauffe chauffe }
procedure InitialiserChauffeEtCommunication(portBase : word ; ParamChauffe : TParametreChauffe ; ParamCommunication : TParametreCommunication ; Materiau : TMateriau);
begin
  portAdresseBase := portBase;
  // Chauffe non supportée par cette machine
end;

{-----------------------------------------------------------------}
{ Lire des états de la machine                }
{ - Pour l'instant lecture du mode de chauffe 1 mode manuel, 0 mode PC }
function EtatMachine : byte;
begin
  Result := 1;
end;


end.
