{   Copyright 2010 Dedalo1111

    This file is part of FDC4_XP.

    FDC4_XP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FDC4_XP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FDC4_XP.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  SysUtils, UType, Windows;

  function  GetDllFamily : byte; export;
  procedure GetDescription(Cible : PChar; tailleCible: integer); export;
  function  EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure ChauffeSendUsart(valChauffe: integer);
  function  CalculerTempsSignalHaut(valChauffe : double) : integer;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  function  EtatMachine : integer; export;
  function  LireChauffeMachine : double; export;
  procedure LibererRessources; export;
  procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur); export;

  procedure PortOut(Port : Word; Data : Byte); stdcall; external 'io.dll';
  function  PortIn(Port : Word) : Byte; stdcall; external 'io.dll';



implementation

const REG_DATA = 0;   // motors activation
const REG_STAT = 1;   // input signals
const REG_CTRL = 2;   // output signals

// REG_STAT bits (input's)
// RS_MAN_AUT = 8;   // bit 3

// REG_CONTROL bits (output's)
// RC_HDT   = 1; // USART data
// RC_HCK   = 2; // USART clock
// RC_HEAT  = 4; // heat on/off
// RC_MOTOR = 8; // motors enabled


const DEBUG        = false;
const DEBUGadapter = false;
const cDebugFile = 'D:\Mis_Documentos\DelphiProjects\Jedicut\debug.log';

var
  ParametreChauffe : TParametreChauffe;
  ParametreCommunication : TParametreCommunication;
  MateriauActif : TMateriau;
  periodeChauffe : integer;
  portAdresseBase : word; // Adresse de base du port parallèle

  RC_HEAT, RC_MOTOR, RC_HCK, RC_HDT, RS_MAN_AUT, RS_MAN_AUTbit : Byte;


// ADRESSE_BASE + 0 : registre de données D0 à D7 (Bit 0 à Bit 7)
// ADRESSE_BASE + 1 : registre de statut /!\ DANGER : LECTURE SEULE /!\
// ADRESSE_BASE + 2 : registre de contrôle


//---------------------------------------------------------------------------------------
//DECIMAL 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
//HEX 	  0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
//BINARY 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111
//---------------------------------------------------------------------------------------

function IntToBin(value: integer; nbytes: integer =2): string;
const
  BCD: array [0..15] of string =
    ('0000', '0001', '0010', '0011', '0100', '0101', '0110', '0111',
    '1000', '1001', '1010', '1011', '1100', '1101', '1110', '1111');
var
  i: integer;
  sHex : string;
begin
  sHex:=IntToHex(value, nbytes);
  Result:='';
  for i := Length(sHex) downto 1 do
    Result := BCD[StrToInt('$' + sHex[i])] + Result;
end;


function IntToXY(value: integer; nbytes: integer =2): string;
const
  BCD: array [0..15] of string =
    ('0000', '---Y', '0010', '0011', '-X--', '0101', '0110', '0111',
    '1000', '1001', '1010', '1011', '1100', '1101', '1110', '1111');
var
  i: integer;
  sHex : string;
begin
  sHex:=IntToHex(value, nbytes);
  Result:='';
  for i := Length(sHex) downto 1 do
    Result := BCD[StrToInt('$' + sHex[i])] + Result;
end;



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
  Description := 'Dédalo-FDC4, universal driver for parallel port with no external timmer, heat on/off control and digital setting. Version 1111.13';
  StrPLCopy(Cible, Description, tailleCible);
end;


{-----------------------------------------------------------------}
{ high-res delay in microsecs }
function Delay_us(elapseus : integer) : boolean;
var
  F,Tstart,Tstop,Tnow : Int64;
begin
      Delay_us:=QueryPerformanceCounter(Tstart);
      QueryPerformanceFrequency(F);
      Tstop:= Tstart + F * elapseus Div 1000000;

      repeat
        //Application.ProcessMessages();
        QueryPerformanceCounter(Tnow);
      until Tnow >= Tstop;
end;


{-----------------------------------------------------------------}
{ Initialiser les paramètre de la chauffe and communications }
procedure InitialiserChauffeEtCommunication(portBase : word ;
                                            ParamChauffe : TParametreChauffe ;
                                            ParamCommunication : TParametreCommunication ;
                                            Materiau : TMateriau);
var
  hFile : TextFile;
begin
  portAdresseBase := portBase;
  ParametreChauffe := ParamChauffe;
  ParametreCommunication := ParamCommunication;
  MateriauActif := Materiau;

  { IMPORTANT NOTICE: (by Dedalo)
    Pin Asignation, names from ParametreCommunication NOT match with
    its driver function. Are populated reading ini from Jedicut
    and reuse historic names function }
  RC_MOTOR  := 1 shl ParametreCommunication.BitAlimMoteur.iBit;
  RC_HEAT   := 1 shl ParametreCommunication.BitModeChauffe.iBit;
  RC_HCK    := 1 shl ParametreCommunication.BitEmissionChauffe.iBit;
  RC_HDT    := 1 shl ParametreCommunication.BitReceptionChauffe.iBit;
  RS_MAN_AUT:= 1 shl ParametreCommunication.BitHorlogeExterne.iBit;
  RS_MAN_AUTbit:=    ParametreCommunication.BitHorlogeExterne.iBit;

  // Compteur d'impulsion de chauffe
  periodeChauffe := CalculerTempsSignalHaut(MateriauActif.pourcentage1);
  ChauffeSendUsart(periodeChauffe);

  if DEBUG then
  begin
    AssignFile(hFile, cDebugFile);
    try
      Append(hFile); // On écrit à la suite du fichier s'il existe
    except
      Rewrite(hFile); // On crée un nouveau fichier si ça n'a pas encore été fait
    end;
    Write(hFile,'InitialiserChauffeEtCommunication');
    Write(hFile,' P:'+IntToHex(portAdresseBase,4)+','
                +IntToHex(ParametreCommunication.BitAlimMoteur.iBit,2)+','
                +IntToHex(RC_MOTOR,2));
    Write(hFile,' PH(on):'+BoolToStr(ParametreChauffe.chauffeActive));
    Write(hFile,' H:'+IntToHex(portAdresseBase,4)+','
                +IntToHex(ParametreCommunication.BitModeChauffe.iBit,2)+','
                +IntToHex(RC_HEAT,2));
    Write(hFile,' CK:'+IntToHex(portAdresseBase,4)+','
                +IntToHex(ParametreCommunication.BitEmissionChauffe.iBit,2)+','
                +IntToHex(RC_HCK,2));
    Write(hFile,' DT:'+IntToHex(portAdresseBase,4)+','
                +IntToHex(ParametreCommunication.BitReceptionChauffe.iBit,2)+','
                +IntToHex(RC_HDT,2));
    //Write(hFile,' M:'+MateriauActif.nom);
    Writeln(hFile);
    CloseFile(hFile);
  end;


end;


{-----------------------------------------------------------------}
{ Méthode de la dll gérant l'alimentation des moteurs }
procedure MoteurOnOff(moteurOn : boolean);
var
  hFile : TextFile;
begin

  if DEBUG then
  begin
    AssignFile(hFile, cDebugFile);
    try
      Append(hFile); // On écrit à la suite du fichier s'il existe
    except
      Rewrite(hFile); // On crée un nouveau fichier si ça n'a pas encore été fait
    end;
  end;


  // Gérer l'alimentation des moteurs
  if moteurOn then
  begin

    if DEBUG then Writeln(hFile, 'Motores ON');

    // Alimenter les moteurs
    PortOut(portAdresseBase+REG_CTRL, RC_HCK+RC_HDT+RC_HEAT);  //Enable & heat ON
    Sleep(1000);
  end else
  begin
    // Emettre la remise à zero des bits moteurs
    PortOut(portAdresseBase+REG_DATA, 0);
    // Couper l'alimentation des moteurs et mettre la chauffe à 0
    PortOut(portAdresseBase+REG_CTRL, RC_HCK+RC_HDT+RC_MOTOR); // clear all bits

    if DEBUG then Writeln(hFile, 'Motores OFF');

  end;

  if DEBUG then CloseFile(hFile);

end;




{-----------------------------------------------------------------}
{
EmettreBit: R44(-X---X--), S00, V06, H25, D6
EmettreBit: R11(---Y---Y), S00, V06, H25, D6
EmettreBit: R40(-X--0000), S00, V50, H25, D80
EmettreBit: R04(0000-X--), S00, V50, H25, D80
EmettreBit: R10(---Y0000), S00, V50, H25, D80
EmettreBit: R01(0000---Y), S00, V50, H25, D80
}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt;
const
  cPERIODUS=125;      //Period in usec. for 4Khz x 2 motor steps
  cDELAYSENSEDIR=60;  //Delay to stablish sense prior to pulse step.
  cPULSEWIDTH=10;     //Pulse step width in uSeg.
  cPCTIME=6;          //Own PC instruction time
var
  hFile : TextFile;
begin
  Result := NO_ERROR;

  if DEBUG then
  begin
    AssignFile(hFile, cDebugFile);
    try
      Append(hFile); // On écrit à la suite du fichier s'il existe
    except
      Rewrite(hFile); // On crée un nouveau fichier si ça n'a pas encore été fait
    end;
  end;

  if (vitesse>0) then    //velocidad
  begin
    // Si on n'a pas choisi l'utilisation du timer externe
    if not ParametreCommunication.synchroniserMoteurAvecTimerExterne then
    begin

      if DEBUG then
      begin
        Write(hFile,'EmettreBit: R'+IntToHex(bitRotation,2)+'('+IntToXY(bitRotation,2)+')');
        Write(hFile,', S'+IntToHex(bitSens,2));
        Write(hFile,', V'+IntToHex(vitesse,2));
        Write(hFile,', H'+FloatToStr(chauffe));
        Write(hFile,', D'+IntToStr(vitesse));
        Writeln(hFile);
      end;

      // Set dir bits
      PortOut(portAdresseBase, bitSens);
      if not Delay_us(cDELAYSENSEDIR) then
        Result:=ERROR_ON_SENDING; //Just if high res counters not availables (older windows)

      // Set dir + step
      PortOut(portAdresseBase, bitRotation + bitSens);
      Delay_us(cPULSEWIDTH);
      // Restore step bit
      PortOut(portAdresseBase, bitSens);

      // Delay to complish with speed
      Delay_us((vitesse*cPERIODUS)-(cDELAYSENSEDIR+cPULSEWIDTH+cPCTIME));

    end else
      Result:=ERROR_TIME_OUT;
  end else
    begin
      Sleep(-1*vitesse);
      if DEBUG then
        Writeln(hFile,'EmettreBit:---PAUSE:'+IntToStr(vitesse));
    end;

  if DEBUG then CloseFile(hFile);

end;



{-----------------------------------------------------------------}
{ Méthode per d'envoi des bits propre à USART MCU}
procedure ChauffeSendUsart(valChauffe: integer);
var
  data, idx : integer;
begin
  for idx:=0 to 7 do
  begin
    data := (valChauffe shr idx) and $01 ;
    PortOut(portAdresseBase+REG_CTRL, (RC_HCK+data) xor $03);  //hardware inverted
    Delay_us(5000);
    PortOut(portAdresseBase+REG_CTRL, RC_HCK+RC_HDT);
    Delay_us(5000);
  end;
end;


{-----------------------------------------------------------------}
{ Calculer le temps durant lequel le signal de chauffe doit être à 1 }
function CalculerTempsSignalHaut(valChauffe : double) : integer;
begin
  Result := Trunc(valChauffe*2.5); //range 0-250
end;



{-----------------------------------------------------------------}
{ Fonction retournant la valeure de la chauffe }
function LireChauffeMachine : double;
begin
  Result:= 0.0;
end;


{-----------------------------------------------------------------}
{ Lire des états de la machine                }
{ - Pour l'instant lecture du mode de chauffe 1 mode manuel, 0 mode PC }
function EtatMachine : integer;
  var
  hFile : TextFile;
begin

  Result := ((PortIn(portAdresseBase+REG_STAT) and RS_MAN_AUT) shr RS_MAN_AUTbit) xor 1;

  if DEBUG then
    begin
    AssignFile(hFile, cDebugFile);
    try
      Append(hFile); // On écrit à la suite du fichier s'il existe
    except
      Rewrite(hFile); // On crée un nouveau fichier si ça n'a pas encore été fait
    end;
    writeln(hFile, 'Status:'+IntToStr(Result));
    CloseFile(hFile);
  end;

end;

{-----------------------------------------------------------------}
{ release resources                }
procedure LibererRessources();
var
  hFile : TextFile;
begin
    if DEBUG then
  begin
    AssignFile(hFile, cDebugFile);
    try
      Append(hFile); // On écrit à la suite du fichier s'il existe
    except
      Rewrite(hFile); // On crée un nouveau fichier si ça n'a pas encore été fait
    end;
    writeln(hFile, 'RESOURCES RELEASED');
    CloseFile(hFile);
  end;
end;



{-----------------------------------------------------------------}
{ Adapter les coordonnées en fonction de la machine }
procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur);
var
  hFile : TextFile;
  ArrayTampon : array [0..3] of TOrdreMoteur;
  i, j : integer;
  sBin,sHex : string;
begin

  if DEBUG and DEBUGadapter then
  begin
    AssignFile(hFile, cDebugFile);
    try
      Append(hFile); // On écrit à la suite du fichier s'il existe
    except
      Rewrite(hFile); // On crée un nouveau fichier si ça n'a pas encore été fait
    end;
    writeln(hFile, 'AdapterOrdres '+IntToStr(Length(ArrayOrdres.ArrayOrdres)));
  end;


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

    if DEBUG and DEBUGadapter then
    begin
      Write(hFile,IntToStr(i)+':');
      sHex:=IntToHex(ArrayOrdres.ArrayOrdres[i].bitRotation,2);
      sBin:=IntToXY(ArrayOrdres.ArrayOrdres[i].bitRotation,2);
      Write(hFile,sHex+'('+sBin+'),');
      Write(hFile,'V:'+IntToStr(ArrayOrdres.ArrayOrdres[i].vitesse)+',' );
      sBin:=IntToBin(ArrayOrdres.ArrayOrdres[i].bitSens);
      Write(hFile,sBin+'<-->');
    end;

    // Si on a des mouvements moteur par moteur (pas 2 mouvements moteurs au même  instant)
    // Si hay movimientos motor a motor (no dos movimientos motores al mismo tiempo)
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

      //2 mouvements moteurs au même instant
      //ArrayOrdres.ArrayOrdres[i].vitesse:=ArrayOrdres.ArrayOrdres[i].vitesse * 2;

    end else begin
      // Réinitialisation des historiques suite à un mouvement simutané de 2 moteurs pour lequel il ne faut pas gérer d'historique de sens de rotation
      // Inicialización del histórico, un movimiento simultáneo de 2 motores no debe hacer uso del histórico de rotazión
      for j := 0 to 3 do
      begin
        ArrayTampon[j].bitRotation := 0;
        ArrayTampon[j].bitSens := 0;
      end;

    end;

    // Modifier les ordres moteurs pour inclure la gestion de la chauffe
    // chauffe := périodeChauffe (comme pour l'init de la chauffe)
    ArrayOrdres.ArrayOrdres[i].chauffe := CalculerTempsSignalHaut(ArrayOrdres.ArrayOrdres[i].chauffe);

    if DEBUG and DEBUGadapter then
    begin
      sHex:=IntToHex(ArrayOrdres.ArrayOrdres[i].bitRotation,2);
      sBin:=IntToXY(ArrayOrdres.ArrayOrdres[i].bitRotation,2);
      Write(hFile,sHex+'('+sBin+'),');
      Write(hFile,'V:'+IntToStr(ArrayOrdres.ArrayOrdres[i].vitesse)+',' );
      sBin:=IntToBin(ArrayOrdres.ArrayOrdres[i].bitSens);
      Writeln(hFile,sBin);
    end;

  end;


  if DEBUG and DEBUGadapter then CloseFile(hFile);

  // Optimiser les vitesses de déplacement
  // Fonctionnalité en BETA, qui semble ne pas fonctionner
  // CompresserOrdresMoteur(ArrayOrdres);
end;


end.
