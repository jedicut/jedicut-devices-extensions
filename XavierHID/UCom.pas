{   Copyright 2012 Jerome

    This file is part of XavierHID.

    XavierHID is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    XavierHID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with XavierHID.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  SysUtils, UType, JvComponentBase, Windows, Forms, JvHidControllerClass, Dialogs;

  function GetDllFamily : byte; export
  procedure GetDescription(Cible : PChar; tailleCible: integer);
  function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt; export;
  function EtatMachine : byte; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur); export;
  function LireChauffeMachine : double; export;

  procedure LibererRessources; export;
  function GetChauffeMachine : double; export;

  // --- Méthodes utilisées pour les évènements de TJvHidDevice
  type
    THidEvent = class
    public
      //function HidCtlEnumerate(HidDev: TJvHidDevice; const Idx: Integer): Boolean;
      //procedure HidCtlDeviceChange(Sender: TObject);
      procedure HidCtlArrival(HidDev: TJvHidDevice);
      procedure HidCtlRemoval(HidDev: TJvHidDevice);
      procedure OnHidDeviceRead(HidDev: TJvHidDevice; ReportID: Byte; const Data: Pointer; Size: Word);
    end;
  // ----------------------------------------------------------

implementation

const XAVIER_VID = $052B;
const XAVIER_PID = $00AA;

// Code fonction de la machine
const XAVIER_FUNC_MOTOR_ON = $44; // D
const XAVIER_FUNC_MOTOR_OFF = $46; // F
const XAVIER_FUNC_PAUSE = $50; // P
const XAVIER_FUNC_CHAUFFE = $43; // C
const XAVIER_FUNC_ROTATE = $4D; // M
const XAVIER_FUNC_MODE_CHAUFFE = $49; // I

var
  HidCtl: TJvHidDeviceController;
  XavierDevice : TJvHidDevice;
  HidEvent : THidEvent;
  bMachineEnAttente : boolean; // La machine attend l'envoie d'ordre par le PC
  bEtatMachine : boolean; // Les moteurs sont ils allumés
  bMachineConnected : boolean; // Boolean indiquant si la machine est connectée ou non. Utilisé uniquement pour détecter le timeout lié à un débranchement de la machine du port USB
  // Gestion de la chauffe
  MateriauActif : TMateriau;
  ParametreChauffe : TParametreChauffe;
  machineValChauffe : double;
  bMachineChauffePC : boolean;
  decoupeValChauffe : integer; // Valeur de la chauffe du précédent ordre de rotation

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
  Description := 'XavierHID - Interface CNC USB pour MM2001. Version 1.2';
  StrPLCopy(Cible, Description, tailleCible);
end;

{-----------------------------------------------------------------}
{ Méthode de la dll gérant l'alimentation des moteurs }
procedure MoteurOnOff(moteurOn : boolean);
var
  i : Integer;
  Buf : array [0..64] of Byte;
  Written : Cardinal;
  ToWrite : Cardinal;
  MessageToPrint : string;
begin
  if Assigned(XavierDevice) then
  begin
    //--- Gérer l'alimentation des moteurs
    ToWrite := XavierDevice.Caps.OutputReportByteLength;
    for i:=0 to ToWrite do
    begin
      Buf[i] := 0;
    end;
    if ToWrite <> 0 then
    begin
      if not bEtatMachine then
        // Allumer les moteurs
        Buf[1] := XAVIER_FUNC_MOTOR_ON
      else
        // Eteindre les moteurs
        Buf[1] := XAVIER_FUNC_MOTOR_OFF;
      // Envoyer le signal
      if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
      begin
          MessageToPrint := 'Erreur dans l''envoi des données';
          MessageDlg(MessageToPrint, mtError, [mbOK], 0);
      end;
    end;

    // Changer l'état de la machine
    bEtatMachine:= not bEtatMachine;

    //--- Gérer le signal de chauffe
    // Si on vient d'allumer les moteurs, c'est qu'il faut mettre en route la chauffe
    if bEtatMachine then
    begin
      if ((ParametreChauffe.chauffeActive)and(ParametreChauffe.chauffeUtilisateur)and bEtatMachine) then
      begin
        ToWrite := XavierDevice.Caps.OutputReportByteLength;
        for i:=0 to ToWrite do
        begin
          Buf[i] := 0;
        end;
        if ToWrite <> 0 then
        begin
          // Nom commande : C
          Buf[1] := XAVIER_FUNC_CHAUFFE;
          // Cette valeur n'est utilisée que pour l'initialisation de la chauffe.
          // Le mieux serait peut être d'avoir la valeur du premier point de la
          // découpe mais c'est impossible ici
          Buf[2] := Trunc(Int(MateriauActif.pourcentage1));
          decoupeValChauffe := Buf[2]; // Initialisation de la variable
          // Envoyer le signal
          if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
          begin
              MessageToPrint := 'Erreur dans l''envoi des données';
              MessageDlg(MessageToPrint, mtError, [mbOK], 0);
          end;
        end;
      end;
    end else begin
      // Si on vient d'éteindre les moteurs, alors il faut couper la chauffe
      // TODO : A coder si nécessaire
    end;
  end else begin
    MessageToPrint := 'Pas de machine';
    MessageDlg(MessageToPrint, mtError, [mbOK], 0);
  end;

  // Relaché la méthode OnHidDevice est obligatoire quand le dialogue avec la machine est terminé (car on libère la dll)
  // OBLIGATOIRE sinon plantage de l'application à la fin de l'envoi des pas
  if Assigned(XavierDevice) then
    if not bEtatMachine then
    begin
      XavierDevice.OnData := nil;
    end else begin
      XavierDevice.OnData := HidEvent.OnHidDeviceRead;
    end;
end;

{-----------------------------------------------------------------}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt;
var
  codeRetour : smallInt;
  i : Integer;
  Buf: array [0..64] of Byte;
  Written: Cardinal;
  ToWrite: Cardinal;
  TwiceBitRot : byte;
begin
  codeRetour := NO_ERROR;
  TwiceBitRot := 0;

  if (bitRotation<>0)  then
  begin
    // bMachineEnAttente=true si la machine est prête à recevoir des ordres
    if Assigned(XavierDevice) then
    begin
      // Attendre que la machine soit prête
      while not bMachineEnAttente do
      begin
//        Sleep(1);
        if not bMachineConnected then
        begin
          codeRetour := ERROR_TIME_OUT;
          break;
        end;
      end;
      if bMachineEnAttente then
      begin
        bMachineEnAttente := false;
        //Buf[0] := 0;
        ToWrite := XavierDevice.Caps.OutputReportByteLength;
        if XavierDevice.Caps.OutputReportByteLength <> 0 then
        begin
          // Initialiser le paquet avec des 0
          for i:=0 to XavierDevice.Caps.OutputReportByteLength do
          begin
            Buf[i] := 0;
          end;
          // Construire le paquet
          // Attention, ce sont des bytes !
          Buf[1]:= XAVIER_FUNC_ROTATE;
          // Adresse carte
          case bitRotation of
            2 : Buf[2]:=1;
            8 : Buf[2]:=2;
            32 : Buf[2]:=3;
            128 : Buf[2]:=4;
          else
            if((bitRotation and 2) = 2)then
            begin
              TwiceBitRot := 2;
              Buf[2] := 1;
            end else begin
              if((bitRotation and 8) = 8)then
              begin
                TwiceBitRot := 8;
                Buf[2] := 2;
              end else begin
                if((bitRotation and 32) = 32)then
                begin
                  TwiceBitRot := 32;
                  Buf[2] := 3;
                end else begin
                  if((bitRotation and 128) = 128)then
                  begin
                    TwiceBitRot := 128;
                    Buf[2] := 4;
                  end;
                end;
              end;
            end;
          end;
          // Paramètre 1 : nombre de pas
          if vitesse >= 0 then
          begin
            Buf[3]:= 0;
            Buf[4]:= 1;
          end else begin
            Buf[3]:= 255;
            Buf[4]:= 255;
          end;
          //Paramètre 2 : vitesse
          Buf[5]:= bitSens;
          Buf[6]:= Abs(vitesse);

          // Appliquer le deuxième pas si nécessaire
          if TwiceBitRot>0 then
          begin
            Buf[7]:= XAVIER_FUNC_ROTATE;
            // Adresse carte
            if (bitRotation - TwiceBitRot = 2) then Buf[8]:=1;
            if (bitRotation - TwiceBitRot = 8) then Buf[8]:=2;
            if (bitRotation - TwiceBitRot = 32) then Buf[8]:=3;
            if (bitRotation - TwiceBitRot = 128) then Buf[8]:=4;
            // Paramètre 1 : nombre de pas
            if vitesse >= 0 then
            begin
              Buf[9]:= 0;
              Buf[10]:= 1;
            end else begin
              Buf[9]:= 255;
              Buf[10]:= 255;
            end;
            //Paramètre 2 : vitesse
            Buf[11]:= bitSens;
            Buf[12]:= Abs(vitesse);
          end;

          // Envoyer le signal
          if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
          begin
            codeRetour := ERROR_ON_SENDING;
          end;
        end;
      end;

      // Construire et envoyer l'ordre de réglage de la chauffe si nécessaire
      if ((ParametreChauffe.chauffeActive)and(ParametreChauffe.chauffeUtilisateur)) then
      begin
        if (decoupeValChauffe <> chauffe) then
        begin
          // Attendre que la machine soit prête
  //        while not bMachineEnAttente do
  //        begin
  //          Sleep(1);
  //          if not bMachineConnected then
  //          begin
  //            codeRetour := ERROR_TIME_OUT;
  //            break;
  //          end;
  //        end;
          if bMachineEnAttente then
          begin
            bMachineEnAttente := false;
            //Buf[0] := 0;
            ToWrite := XavierDevice.Caps.OutputReportByteLength;
            if XavierDevice.Caps.OutputReportByteLength <> 0 then
            begin
              // Initialiser le paquet avec des 0
              for i:=0 to XavierDevice.Caps.OutputReportByteLength do
              begin
                Buf[i] := 0;
              end;
              // Construire le paquet
              // Nom commande : C
              Buf[1] := XAVIER_FUNC_CHAUFFE;
              Buf[2] := Trunc(Int(chauffe));

              decoupeValChauffe := Trunc(Int(chauffe));

              // Envoyer le signal
              if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
              begin
                codeRetour := ERROR_ON_SENDING;
              end;
            end;
          end;
        end;
      end;

    end;
  end else begin
    Sleep(-1 * vitesse);
  end;

  Result := codeRetour;
end;

{-----------------------------------------------------------------}
{ Initialiser les paramètre de la chauffe chauffe }
procedure InitialiserChauffeEtCommunication(portBase : word ; ParamChauffe : TParametreChauffe ; ParamCommunication : TParametreCommunication ; Materiau : TMateriau);
begin
  // Initialisation des composants USB
  if not Assigned(HidEvent) then
    HidEvent := THidEvent.Create;
  if not Assigned(HidCtl) then
  begin
    HidCtl := TJvHidDeviceController.Create(nil);
    //HidCtl.OnEnumerate := HidEvent.HidCtlEnumerate;
    //HidCtl.OnDeviceChange := HidEvent.HidCtlDeviceChange;
    HidCtl.OnArrival := HidEvent.HidCtlArrival;
    HidCtl.OnRemoval := HidEvent.HidCtlRemoval;

    bMachineEnAttente := true; // La machine est en attante de recevoir un paquet
    bEtatMachine := false; // On considère qu'au lancement de la machine les moteurs sont arrêtés
    bMachineConnected := false; // La machine est considérée comme offline
  end;

  // Nécessaire pour autoriser "l'évènement" qui annoncer la présence de la machine
  Application.ProcessMessages;

  MateriauActif := Materiau;
  ParametreChauffe := ParamChauffe;
end;

{-----------------------------------------------------------------}
{ Lire des états de la machine                }
{ - Pour l'instant lecture du mode de chauffe 1 mode manuel, 0 mode PC, 2 non communiqué }
function EtatMachine : byte;
var
  retour : byte;
begin
//  if bMachineChauffePC then retour := 0
//  else  retour := 1;
  retour := 2;
  Result := retour;
end;

{-----------------------------------------------------------------}
{ Adapter les coordonnées en fonction de la machine }
procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur);
var
  i : integer;
  vitesse : smallInt;
begin
  // La vitesse est codée sur 2 octets signés, il faut donc les calculer en reprennant les 2 variables suivantes :
  // - bitSens ne sert plus à indiquer le sens, il va contenir l'octet de poid fort de la vitesse
  // - vitesse (integer) doit contenir l'octet de poids faible de la vitesse
  for i:=0 to Length(ArrayOrdres.ArrayOrdres)-1 do
  begin
    vitesse := ArrayOrdres.ArrayOrdres[i].vitesse;
    // Vitesse de rotation ou pause ?
    if vitesse > 0 then
    begin
      // Extraire l'octet de poid faible
      ArrayOrdres.ArrayOrdres[i].vitesse := vitesse and $FF;
      // On ajoute le signe négatif ici indiquant que le sens négatif
      if(ArrayOrdres.ArrayOrdres[i].bitSens=0)then
      begin
        ArrayOrdres.ArrayOrdres[i].vitesse := -1 * ArrayOrdres.ArrayOrdres[i].vitesse; 
      end;
      // Extraire l'octet de poid fort
      ArrayOrdres.ArrayOrdres[i].bitSens := (vitesse and $FF00) shr 8;
    end else begin
      // cette mise à 0 indique qu'il n'y a pas de rotation des moteurs,
      // et que donc il s'agit d'une pause dans la découpe.
      ArrayOrdres.ArrayOrdres[i].bitRotation := 0;
    end;
  end;
end;

{-----------------------------------------------------------------}
{ Libérer les ressources créées dynamiquement dans la dll }
procedure LibererRessources;
begin
  if Assigned(XavierDevice) then
    XavierDevice.OnData := nil;
  HidEvent.Free;
  HidCtl.Free;
end;

{-----------------------------------------------------------------}
{ Fonction demandant à l'interface USB la valeur de la chauffe }
function LireChauffeMachine : double;
var
  i : Integer;
  Buf: array [0..64] of Byte;
  Written: Cardinal;
  ToWrite: Cardinal;
//  MessageToPrint : string;
begin
  if Assigned(XavierDevice) then
  begin
    // Réactivé la lecture du device
    XavierDevice.OnData := HidEvent.OnHidDeviceRead;

    // Annuler la précédente lecture de chauffe
    machineValChauffe := -1;
    // Attendre que la machine soit prête
//    while not bMachineEnAttente do
//    begin
//      Sleep(1);
//      if not bMachineConnected then
//      begin
//        Result := -3;
//        break;
//      end;
//    end;
//    if bMachineEnAttente then
//    begin
//      bMachineEnAttente := false;
      // Envoyer la lettre I à la machine pour lui demander le mode et la valeur de la chauffe
      ToWrite := XavierDevice.Caps.OutputReportByteLength;
      for i:=0 to ToWrite do
      begin
        Buf[i] := 0;
      end;
      if ToWrite <> 0 then
      begin
        // Nom commande : I
        Buf[1] := XAVIER_FUNC_MODE_CHAUFFE;
        // Envoyer le signal
        if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
        begin
          // MessageToPrint := 'Erreur dans l''envoi des données';
          // MessageDlg(MessageToPrint, mtError, [mbOK], 0);
          Result := -4
        end;
      end;
//    end;
    Result := -99; // Permet de déclencher le timer dans Jedicut qui appelera la fonction GetChauffeMachine
  end else begin
    Result := -2;
  end;
end;

{-----------------------------------------------------------------}
{ Fonction renvoyant la valeur de la chauffe }
function GetChauffeMachine : double;
begin
  Result := machineValChauffe;
end;

{-----------------------------------------------------------------}
{ Méthodes utilisées pour les évènements de TJvHidDevice }
{-----------------------------------------------------------------}

{-----------------------------------------------------------------}
{ Evènement : arrivé d'une donnée à lire }
//procedure THidEvent.HidDeviceRead(HidDev: TJvHidDevice; ReportID: Byte; const Data: Pointer; Size: Word);
//begin
//end;

{-----------------------------------------------------------------}
{ Enumération des périphériques USB pour retrouver la machine }
//function THidEvent.HidCtlEnumerate(HidDev: TJvHidDevice; const Idx: Integer): Boolean;
//begin
//end;

{-----------------------------------------------------------------}
{ Modification de la liste des périphériques USB du PC }
//procedure THidEvent.HidCtlDeviceChange(Sender: TObject);
//begin
//end;

{-----------------------------------------------------------------}
{ Arrivé d'un caractère envoyé par le device USB }
procedure THidEvent.OnHidDeviceRead(HidDev: TJvHidDevice; ReportID: Byte; const Data: Pointer; Size: Word);
var
  Str : string;
  iLu : integer;
begin
  Str := Format('R %.2x  ', [ReportID]);
  iLu := integer(PChar(Data)[0]);
  case iLu of
    83 : begin
      // Lecture du caractère S
      bMachineEnAttente := true;
    end;
    XAVIER_FUNC_CHAUFFE : begin
      // TODO : peut être mettre un contrôle sur le nombre de bit à lire pour couvrir les erreurs de firmware
      if (integer(PChar(Data)[1])=0) then
        bMachineChauffePC := true
      else
        bMachineChauffePC := false;
      machineValChauffe := integer(PChar(Data)[2])/100; // On divise par 100 car Jedicut attend une valeur entre 0 et 1

      // Réactivé la lecture du device
      XavierDevice.OnData := HidEvent.OnHidDeviceRead;
    end;
  end;
end;

{-----------------------------------------------------------------}
{ Connexion d'un périphérique USB }
procedure THidEvent.HidCtlArrival(HidDev: TJvHidDevice);
begin
  // Si XavierDevice est rebranchée
  if not Assigned(XavierDevice) then
  begin
    if ((HidDev.Attributes.VendorID = XAVIER_VID) and (HidDev.Attributes.ProductID = XAVIER_PID)) then
    begin
      HidCtl.CheckOutById(XavierDevice, XAVIER_VID, XAVIER_PID);
      //MessageDlg('Machine Détectée 1', mtError, [mbOK], 0);
      if XavierDevice.HasReadWriteAccess then
      begin
        XavierDevice.OnData := OnHidDeviceRead;
      end else begin
        XavierDevice.OnData := nil;
      end;
      bMachineEnAttente := true; // Pour le premier paquet
      bMachineConnected := true; // La machine est online
    end;
  end;
end;

{-----------------------------------------------------------------}
{ Déconnexion d'un périphérique USB }
procedure THidEvent.HidCtlRemoval(HidDev: TJvHidDevice);
begin
  if ((HidDev.Attributes.VendorID = XAVIER_VID) and (HidDev.Attributes.ProductID = XAVIER_PID)) then
  begin
    XavierDevice.Free;
    //MessageDlg('Machine débranchée 1', mtError, [mbOK], 0);
    bMachineEnAttente := false;
    bMachineConnected := false; // La machine est offline
  end;
end;

end.
