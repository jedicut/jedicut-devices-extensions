unit UFInit;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, XPMan, JvComponentBase, JvHidControllerClass, ExtCtrls;

type
  TFInit = class(TForm)
    BtnOk: TButton;
    Label1: TLabel;
    EditStatut: TEdit;
    BtnQuitter: TButton;
    Memo1: TMemo;
    BtnSend: TButton;
    LabeledEdit1: TLabeledEdit;
    BtnMarche: TButton;
    HidCtl: TJvHidDeviceController;
    procedure BtnOkClick(Sender: TObject);

    procedure FormCreate(Sender: TObject);
    function HidCtlEnumerate(HidDev: TJvHidDevice;
      const Idx: Integer): Boolean;
    procedure HidCtlDeviceChange(Sender: TObject);
    procedure HidCtlArrival(HidDev: TJvHidDevice);
    procedure HidCtlRemoval(HidDev: TJvHidDevice);
    procedure BtnQuitterClick(Sender: TObject);
    procedure BtnSendClick(Sender: TObject);
    procedure BtnMarcheClick(Sender: TObject);
  private
    { Déclarations privées }
    XavierDevice : TJvHidDevice;
    bMachineEnAttente : boolean;
    bEtatMachine : boolean;
  public
    { Déclarations publiques }
    procedure OnHidDeviceRead(HidDev: TJvHidDevice; ReportID: Byte; const Data: Pointer; Size: Word);
  end;

const XAVIER_VID = $052B;
const XAVIER_PID = $00AA;

implementation

{$R *.dfm}

procedure TFInit.BtnOkClick(Sender: TObject);
begin
  Close;
end;


// --------------------------------------------------------------------------------------
procedure TFInit.FormCreate(Sender: TObject);
begin
//  XavierDevice := nil;
  bEtatMachine := false; // On considère qu'au lancement de la machine les moteurs sont arrêtés
end;


// Fonction permettant de lister les periphériques USB.
function TFInit.HidCtlEnumerate(HidDev: TJvHidDevice;
  const Idx: Integer): Boolean;
begin
  //if Assigned(XavierDevice) then XavierDevice.Free;
  // Test si le périphérique USB est la machine de Xavier
//  if ((HidDev.Attributes.VendorID = XAVIER_VID) and (HidDev.Attributes.ProductID = XAVIER_PID)) then
//  begin
//    HidCtl.CheckOutByIndex(XavierDevice, Idx);
//    EditStatut.Color := clGreen;
//    EditStatut.Text := 'Online';
//    Memo1.Lines.Add('Lancement de l''appli');
//    if XavierDevice.HasReadWriteAccess then
//    begin
//      XavierDevice.OnData := OnHidDeviceRead;
//      Memo1.Lines.Add('Périphérique avec lecture/écriture');
//    end else begin
//      XavierDevice.OnData := nil;
//      Memo1.Lines.Add('Périphérique sans lecture/écriture');
//    end;
//    bMachineEnAttente := true; // Pour le premier paquet
//  end;
end;

procedure TFInit.HidCtlDeviceChange(Sender: TObject);
var
  MessageToPrint : string;
begin
//  if Assigned(XavierDevice) then
//  begin
//    XavierDevice.Free;
//    Memo1.Lines.Add('Objet.Free');
//  end;
  Memo1.Lines.Add('DeviceChange');
  //HidCtl.Enumerate;
  if (not Assigned(XavierDevice)) then
  begin
    Memo1.Lines.Add('Interface USB non trouvée');
    //MessageToPrint :=Format('L''interface de découpe USB n''a pas été trouvée. Rappel : VID=0x%.4x et PID=0x%.4x', [XAVIER_VID, XAVIER_PID]);
//    MessageDlg(MessageToPrint, mtWarning, [mbOK], 0);
  end;
end;

procedure TFInit.HidCtlArrival(HidDev: TJvHidDevice);
begin
  // Si XavierDevice est rebranchée
  if ((HidDev.Attributes.VendorID = XAVIER_VID) and (HidDev.Attributes.ProductID = XAVIER_PID)) then
  begin
    EditStatut.Color := clGreen;
    EditStatut.Text := 'Online';
    Memo1.Lines.Add('Machine branchée');

    HidCtl.CheckOutById(XavierDevice, XAVIER_VID, XAVIER_PID);
    if XavierDevice.HasReadWriteAccess then
    begin
      XavierDevice.OnData := OnHidDeviceRead;
      Memo1.Lines.Add('Périphérique avec lecture/écriture');
    end else begin
      XavierDevice.OnData := nil;
      Memo1.Lines.Add('Périphérique sans lecture/écriture');
    end;
    bMachineEnAttente := true; // Pour le premier paquet
  end;
end;

procedure TFInit.HidCtlRemoval(HidDev: TJvHidDevice);
begin
  // Si XavierDevice est déconnectée
  if ((HidDev.Attributes.VendorID = XAVIER_VID) and (HidDev.Attributes.ProductID = XAVIER_PID)) then
  begin
    EditStatut.Color := clRed;
    EditStatut.Text := 'Offline';
    Memo1.Lines.Add('Machine enlevée');
    XavierDevice.Free;
    Memo1.Lines.Add('Objet.Free');
  end;
end;

procedure TFInit.BtnQuitterClick(Sender: TObject);
begin
  Close;
end;

procedure TFInit.BtnSendClick(Sender: TObject);
var
  I, j : Integer;
  Buf: array [0..64] of Byte;
  Written: Cardinal;
  ToWrite: Cardinal;
  MessageToPrint : string;
  paquetToSend : integer;
begin
  paquetToSend := StrToIntDef(LabeledEdit1.Text, 1);
  bMachineEnAttente := true;
  while paquetToSend > 0 do
  begin
    if bMachineEnAttente then
    begin
      bMachineEnAttente := false;
      if Assigned(XavierDevice) then
      begin
        Buf[0] := 0;
        ToWrite := XavierDevice.Caps.OutputReportByteLength;
        Memo1.Lines.Add('--- ToWrite');
        Memo1.Lines.Add(IntToStr(ToWrite));
        Memo1.Lines.Add('--- Paquet');
        Memo1.Lines.Add(' 010011010000000100000000000000010000011111010000');
        Memo1.Lines.Add('---');
        if XavierDevice.Caps.OutputReportByteLength <> 0 then
        begin
          // Construire le paquet
          {
          // Test carte CNC USB
          // M
          Buf[1]:= 0;
          Buf[2]:= 1;
          Buf[3]:= 0;
          Buf[4]:= 0;
          Buf[5]:= 1;
          Buf[6]:= 1;
          Buf[7]:= 0;
          Buf[8]:= 1;
          // Adresse carte : 1
          Buf[9]:= 0;
          Buf[10]:= 0;
          Buf[11]:= 0;
          Buf[12]:= 0;
          Buf[13]:= 0;
          Buf[14]:= 0;
          Buf[15]:= 0;
          Buf[16]:= 1;
          // Param 1 : 1 pas
          Buf[17]:= 0;
          Buf[18]:= 0;
          Buf[19]:= 0;
          Buf[20]:= 0;
          Buf[21]:= 0;
          Buf[22]:= 0;
          Buf[23]:= 0;
          Buf[24]:= 0;
          Buf[25]:= 0;
          Buf[26]:= 0;
          Buf[27]:= 0;
          Buf[28]:= 0;
          Buf[29]:= 0;
          Buf[30]:= 0;
          Buf[31]:= 0;
          Buf[32]:= 1;
          // Param 2
          Buf[33]:= 0;
          Buf[34]:= 0;
          Buf[35]:= 0;
          Buf[36]:= 0;
          Buf[37]:= 0;
          Buf[38]:= 1;
          Buf[39]:= 1;
          Buf[40]:= 1;
          Buf[41]:= 1;
          Buf[42]:= 1;
          Buf[43]:= 0;
          Buf[44]:= 1;
          Buf[45]:= 0;
          Buf[46]:= 0;
          Buf[47]:= 0;
          Buf[48]:= 0;
          }
          // Test interface USB \ //
          // Attention, ce sont des bytes !
          // Nom commande : M
          Buf[1]:= 77;
          // Adresse carte : 1
          Buf[2]:= 1;
          // Paramètre 1 : 10 pas
          Buf[3]:= 0;
          Buf[4]:= 10;
          //Paramètre 2 :
          Buf[5]:= 31;
          Buf[6]:= 255;
          // Et on recommence... :)
          Buf[7]:= 0;
          Buf[8]:= 0;
          Buf[9]:= 0;
          Buf[10]:= 0;
          Buf[11]:= 0;
          Buf[12]:= 0;
          Buf[13]:= 0;
          Buf[14]:= 0;
          Buf[15]:= 0;
          Buf[16]:= 0;
          Buf[17]:= 0;
          Buf[18]:= 0;
          Buf[19]:= 0;
          Buf[20]:= 0;
          Buf[21]:= 0;
          Buf[22]:= 0;
          Buf[23]:= 0;
          Buf[24]:= 0;
          Buf[25]:= 0;
          Buf[26]:= 0;
          Buf[27]:= 0;
          Buf[28]:= 0;
          Buf[29]:= 0;
          Buf[30]:= 0;
          Buf[31]:= 0;
          Buf[32]:= 0;
          Buf[33]:= 0;
          Buf[34]:= 0;
          Buf[35]:= 0;
          Buf[36]:= 0;
          Buf[37]:= 0;
          Buf[38]:= 0;
          Buf[39]:= 0;
          Buf[40]:= 0;
          Buf[41]:= 0;
          Buf[42]:= 0;
          Buf[43]:= 0;
          Buf[44]:= 0;
          Buf[45]:= 0;
          Buf[46]:= 0;
          Buf[47]:= 0;
          Buf[48]:= 0;

          {for I := 1 to ToWrite-1 do
          begin
            Buf[I] := 1;
          end;}
          if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
          begin
            MessageToPrint := 'Erreur dans l''envoi des données';
            MessageDlg(MessageToPrint, mtError, [mbOK], 0);
          end else begin
            paquetToSend := paquetToSend - 1;
            Memo1.Lines.Add('Paquet envoyé');
          end;
        end;
      end;
    end else begin
      //Sleep(1);
      Application.ProcessMessages
    end;
  end;
end;

procedure TFInit.OnHidDeviceRead(HidDev: TJvHidDevice; ReportID: Byte;
  const Data: Pointer; Size: Word);
var
  I: Integer;
  Str: string;
  iLu : integer;
begin
  Str := Format('R %.2x  ', [ReportID]);
  for I := 0 to Size - 1 do
  begin
    iLu := integer(PChar(Data)[I]);
    Str := Str + Format('%.2x ', [iLu]);
    if (iLu = 83) then
    begin
      Memo1.Lines.Add('Caractère S reçu');          
      bMachineEnAttente := true;
    end;
  end;
  Memo1.Lines.Add(Str);
end;


procedure TFInit.BtnMarcheClick(Sender: TObject);
var
  I, j : Integer;
  Buf: array [0..64] of Byte;
  Written: Cardinal;
  ToWrite: Cardinal;
  MessageToPrint : string;
  paquetToSend : integer;
begin
  //paquetToSend := StrToIntDef(LabeledEdit1.Text, 1);
  bMachineEnAttente := true;
  //while paquetToSend > 0 do
  //begin
    if bMachineEnAttente then
    begin
      bMachineEnAttente := false;
      if Assigned(XavierDevice) then
      begin
        Buf[0] := 0;
        ToWrite := XavierDevice.Caps.OutputReportByteLength;
        Memo1.Lines.Add('--- ToWrite');
        Memo1.Lines.Add(IntToStr(ToWrite));
        Memo1.Lines.Add('--- Paquet');
        Memo1.Lines.Add('D0000... ou F0000');
        Memo1.Lines.Add('---');
        if XavierDevice.Caps.OutputReportByteLength <> 0 then
        begin
          // Test interface USB \ //
          // Attention, ce sont des bytes !
          // Nom commande : D ou F
          if not bEtatMachine then
            Buf[1]:= 68
          else
            Buf[1]:= 70;
          bEtatMachine:= not bEtatMachine;
          
          Buf[2]:= 0;
          Buf[3]:= 0;
          Buf[4]:= 0;
          Buf[5]:= 0;
          Buf[6]:= 0;
          Buf[7]:= 0;
          Buf[8]:= 0;
          Buf[9]:= 0;
          Buf[10]:= 0;
          Buf[11]:= 0;
          Buf[12]:= 0;
          Buf[13]:= 0;
          Buf[14]:= 0;
          Buf[15]:= 0;
          Buf[16]:= 0;
          Buf[17]:= 0;
          Buf[18]:= 0;
          Buf[19]:= 0;
          Buf[20]:= 0;
          Buf[21]:= 0;
          Buf[22]:= 0;
          Buf[23]:= 0;
          Buf[24]:= 0;
          Buf[25]:= 0;
          Buf[26]:= 0;
          Buf[27]:= 0;
          Buf[28]:= 0;
          Buf[29]:= 0;
          Buf[30]:= 0;
          Buf[31]:= 0;
          Buf[32]:= 0;
          Buf[33]:= 0;
          Buf[34]:= 0;
          Buf[35]:= 0;
          Buf[36]:= 0;
          Buf[37]:= 0;
          Buf[38]:= 0;
          Buf[39]:= 0;
          Buf[40]:= 0;
          Buf[41]:= 0;
          Buf[42]:= 0;
          Buf[43]:= 0;
          Buf[44]:= 0;
          Buf[45]:= 0;
          Buf[46]:= 0;
          Buf[47]:= 0;
          Buf[48]:= 0;

          {for I := 1 to ToWrite-1 do
          begin
            Buf[I] := 1;
          end;}
          if not XavierDevice.WriteFile(Buf, ToWrite, Written) then
          begin
            MessageToPrint := 'Erreur dans l''envoi des données';
            MessageDlg(MessageToPrint, mtError, [mbOK], 0);
          end else begin
            paquetToSend := paquetToSend - 1;
            Memo1.Lines.Add('Paquet envoyé');
          end;
        end;
      end;
    end else begin
      //Sleep(1);
      Application.ProcessMessages
    end;
  //end;
end;



end.
