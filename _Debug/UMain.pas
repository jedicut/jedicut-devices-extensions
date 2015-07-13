unit UMain;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons, Grids, UType, JvExButtons, JvBitBtn, ExtCtrls, iniFiles,
  JvExStdCtrls, JvGroupBox;

type
  TFMain = class(TForm)
    ComboBoxPlugIn: TComboBox;
    BitBtnExit: TBitBtn;
    BitBtnLoad: TBitBtn;
    Memo: TMemo;
    LblEditSample: TLabeledEdit;
    JvBitBtnFile: TJvBitBtn;
    OpenDialog1: TOpenDialog;
    Label1: TLabel;
    BitBtnClearMemo: TBitBtn;
    LblEditNbPas: TLabeledEdit;
    RadioGroupMoteur: TRadioGroup;
    GroupBoxChauffe: TGroupBox;
    Label2: TLabel;
    CheckBoxChauffe: TCheckBox;
    LabeledEditValChauffe: TLabeledEdit;
    JvGroupBoxCom: TJvGroupBox;
    JvGroupBoxFile: TJvGroupBox;
    BitBtnSendStep: TBitBtn;
    BitBtnReadHeating: TBitBtn;
    LabeledEditReadHeatingValue: TLabeledEdit;
    BitBtn1: TBitBtn;
    GroupBoxMotor: TGroupBox;
    procedure FormCreate(Sender: TObject);
    procedure BitBtnLoadClick(Sender: TObject);
    procedure BitBtnExitClick(Sender: TObject);
    procedure JvBitBtnFileClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure ComboBoxPlugInClick(Sender: TObject);
    procedure BitBtnClearMemoClick(Sender: TObject);
    procedure BitBtnSendStepClick(Sender: TObject);
  private
    { Déclarations privées }
    dllLoaded : boolean;
    DllPathName : string;
    DllHandle : THandle;
    TestFile : string;

    // --- General functions for plugin
    GetDllFamily : function : byte;
    GetDescription : procedure(Cible : PChar; tailleCible: integer);
    GetFileExtension : procedure(Format : PChar; tailleCible: integer);
    // --- Specifics functions for plugin of file
    OpenFileDll : function(Src : PChar ; var nbEx, nbIn : integer) : smallInt;
    LoadFileDll : function(var Profil : TCoordonneesProfil) : smallInt;
    GetProfileDescription : procedure(Cible : PChar; tailleCible: integer);
    GetProfileName : procedure(Cible : PChar; tailleCible: integer);
    SaveFileDll : function(Dest : PChar) : smallInt;
    // --- Specifics functions for plugin of communication
    EmettreBit : function (bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt;
    InitialiserChauffeEtCommunication : procedure (portBase : word ; ParametreChauffe : TParametreChauffe ; ParametreCommunication : TParametreCommunication ; MateriauActif : TMateriau);
    AdapterOrdres : procedure (var ArrayOrdres : TArrayOrdresMoteur);
    MoteurOnOff : procedure (moteurOn : boolean);

    procedure Initialiser(var bitSens, bitHorloge, impulsion : integer);
    procedure EnableIHM(bFile, bCom : boolean);
    procedure LoadDll;
    procedure FreeMemory;
    procedure FreeDll;
  public
    { Déclarations publiques }
  end;

var
  FMain: TFMain;

implementation

{$R *.dfm}

// Lister les dll disponibles
procedure TFMain.FormCreate(Sender: TObject);
var
  FichierIni : TIniFile;
  InitPathName, LastPlugIn : string;
  Info   : TSearchRec;
  i : integer;
  plugInFound : boolean;
begin
  dllLoaded := false;
  plugInFound := false;
  InitPathName := ExtractFilePath(Application.ExeName);

  // Charger les préférences utilisateur
  FichierIni := TIniFile.Create(InitPathName + 'Debug.ini');
  LblEditSample.Text := FichierIni.ReadString('General', 'SampleFile', '');
  LastPlugIn := FichierIni.ReadString('General', 'PlugIn', '');
  FreeAndNil(FichierIni);

  // Lister les dll
  ComboBoxPlugIn.Items.Clear;

  // Recherche de la première entrée du répertoire
  if  (FindFirst(InitPathName + 'Dll\*.dll', faAnyFile, Info) = 0) then
  begin
    repeat
      if  not ((Info.Attr and faDirectory) <> 0) then
      begin
        DllPathName := InitPathName + 'Dll\' + Info.FindData.cFileName;
        try
//          // Ouverture de la dll
//          DllHandle:= LoadLibrary(PAnsiChar(DllPathName));
//          if (DllHandle <> 0) then
//          begin
//            @GetDllFamily := GetProcAddress(DllHandle, 'GetDllFamily');
//          end else
//            MessageBoxA(0, PChar('Erreur de chargement de ' + Info.FindData.cFileName),'',MB_ICONSTOP);

          //// On ne s'intéresse qu'aux dll de communication avec la fonction
//          // GetDllFamily implémentée
//          if @GetDllFamily <> nil then
//          begin
//            if GetDllFamily = 0 then
//            begin
//              // Ajouter la dll de communication à la liste déroulante
              ComboBoxPlugIn.Items.Add(Info.FindData.cFileName);
//            end;
//          end;
        finally
//          if DllHandle <> 0 then FreeLibrary(DllHandle);
        end;
      end;

    // Il faut ensuite rechercher l'entrée suivante
    until FindNext(Info) <> 0;

    // Dans le cas ou une entrée au moins est trouvée il faut
    // appeler FindClose pour libérer les ressources de la recherche
    FindClose(Info);
  end;

  // Initialiser IHM
  if (ComboBoxPlugIn.Items.Count>0) then
  begin
    // Sélectionner le premier élément de la liste si elle n'est pas vide,
    // et si LastPlugIn='' ou pas trouvé sinon
    if LastPlugIn<>'' then
    begin
      for i:=0 to ComboBoxPlugIn.Items.Count-1 do
      begin
        if ComboBoxPlugIn.Items[i]=LastPlugIn then
        begin
          plugInFound := true;
          break;
        end;
      end;
    end;
    if plugInFound then
    begin
      ComboBoxPlugIn.ItemIndex := i;
    end else begin
      ComboBoxPlugIn.ItemIndex := 0;
    end;

    // Activer le bouton Load
    BitBtnLoad.Enabled := true;

    // Pré chargement pour activer l'IHM "File" ou "Communication"
    LoadDll;
  end;

end;

procedure TFMain.EnableIHM(bFile, bCom : boolean);
begin
  // IHM pour les fichiers
  JvGroupBoxFile.Enabled := bFile;
  LblEditSample.Enabled := bFile;
  JvBitBtnFile.Enabled := bFile;
  BitBtnLoad.Enabled := bFile;
  // IHM pour la communication
  JvGroupBoxCom.Enabled := bCom;
  LblEditNbPas.Enabled := bCom;
  RadioGroupMoteur.Enabled := bCom;
  GroupBoxChauffe.Enabled := bCom;
  LabeledEditValChauffe.Enabled := bCom;
end;

// Charger la dll sélectionné
procedure TFMain.LoadDll;
var
  Description : array [0..128] of char;
  Extension : array [0..128] of char;
begin
  // On reconstruit le chemin des plugins
  DllPathName := ExtractFilePath(Application.ExeName)+ 'Dll\' + ComboBoxPlugIn.Items[ComboBoxPlugIn.ItemIndex];

  // Ouverture de la dll
  DllHandle:= LoadLibrary(PAnsiChar(DllPathName));
  if (DllHandle <> 0) then
  begin
    Memo.Lines.Add('---');
    Memo.Lines.Add('--- Plugin Loaded : ' + ExtractFileName(DllPathName));
    Memo.Lines.Add('--- Date/Time : '+TimeToStr(Time));

    @GetDllFamily := GetProcAddress(DllHandle, 'GetDllFamily');
    @GetDescription := GetProcAddress(DllHandle, 'GetDescription');
    @GetFileExtension := GetProcAddress(DllHandle, 'GetFileExtension');
    @OpenFileDll := GetProcAddress(DllHandle, 'OpenFileDll');
    @LoadFileDll := GetProcAddress(DllHandle, 'LoadFileDll');
    @GetProfileName := GetProcAddress(DllHandle, 'GetProfileName');
    @GetProfileDescription := GetProcAddress(DllHandle, 'GetProfileDescription');
    @SaveFileDll := GetProcAddress(DllHandle, 'SaveFileDll');
    @EmettreBit := GetProcAddress(DllHandle, 'EmettreBit');
    @InitialiserChauffeEtCommunication := GetProcAddress(DllHandle, 'InitialiserChauffeEtCommunication');
    @AdapterOrdres := GetProcAddress(DllHandle, 'AdapterOrdres');
    @MoteurOnOff := GetProcAddress(DllHandle, 'MoteurOnOff');

  end else begin
    MessageBoxA(0, PChar('Erreur de chargement'),'',MB_ICONSTOP);
    Memo.Lines.Add('Plugin unloaded');
    Exit;
  end;

  if @GetDllFamily<>nil then
  begin
    Memo.Lines.Add('# GetDllFamily loaded :');
    Memo.Lines.Add(IntToStr(GetDllFamily));
  end else begin
    Memo.Lines.Add('! GetDllFamily unloaded');
  end;

  if @GetProfileName<>nil then
  begin
    Memo.Lines.Add('# GetProfileName loaded');
  end else begin
    Memo.Lines.Add('! GetProfileName unloaded ');
  end;

  if @GetProfileDescription<>nil then
  begin
    Memo.Lines.Add('# GetProfileDescription loaded');
  end else begin
    Memo.Lines.Add('! GetProfileDescription unloaded');
  end;

  if @GetDescription<>nil then
  begin
    GetDescription(Description, SizeOf(Description));
    Memo.Lines.Add('# GetDescription loaded : ');
    Memo.Lines.Add(Description);
  end else begin
    Memo.Lines.Add('! GetDescription unloaded');
  end;

  if @GetFileExtension<>nil then
  begin
    GetFileExtension(Extension, SizeOf(Extension));
    Memo.lines.Add('# GetFileExtension loaded : ');
    Memo.lines.Add( Extension);
  end else begin
    Memo.lines.Add('! GetFileExtension unloaded');
  end;

//  if @SaveFileDll<>nil then
//  begin
//  end;

  if @OpenFileDll<>nil then
  begin
    Memo.lines.Add('# OpenFileDll loaded');
  end else begin
    Memo.lines.Add('! OpenFileDll unloaded');
  end;

  if @LoadFileDll<>nil then
  begin
    Memo.lines.Add('# LoadFileDll loaded');
  end else begin
    Memo.lines.Add('! LoadFileDll unloaded');
  end;

  if @EmettreBit<>nil then
  begin
    Memo.lines.Add('# EmettreBit loaded');
  end else begin
    Memo.lines.Add('! EmettreBit unloaded');
  end;

  if @InitialiserChauffeEtCommunication<>nil then
  begin
    Memo.lines.Add('# InitialiserChauffeEtCommunication loaded');
  end else begin
    Memo.lines.Add('! InitialiserChauffeEtCommunication unloaded');
  end;

  if @AdapterOrdres<>nil then
  begin
    Memo.lines.Add('# AdapterOrdres loaded');
  end else begin
    Memo.lines.Add('! AdapterOrdres unloaded');
  end;

  if @MoteurOnOff<>nil then
  begin
    Memo.lines.Add('# MoteurOnOff loaded');
  end else begin
    Memo.lines.Add('! MoteurOnOff unloaded');
  end;

//       TODO Ecrire le test
  if ((GetDllFamily=DLL_FAMILY_FILE_PROFIL)or
      (GetDllFamily=DLL_FAMILY_FILE_PROFIL_READ_ONLY)or
      (GetDllFamily=DLL_FAMILY_FILE_PROFIL_WRITE_ONLY)) then
  begin
    // Plugin Fichier
    EnableIHM(true, false);
  end else begin
// TODO Faire un test sur la famille de plugin comme pour ci-dessous
    // Plugin Communication
    EnableIHM(false, true);
  end;


  dllLoaded := true;
end;

// Charger la dll sélectionné
procedure TFMain.BitBtnLoadClick(Sender: TObject);
var
  ProfileName : array [0..128] of char;
  ProfileDescription : array [0..128] of char;
  Profil : TCoordonneesProfil;
  i, nbEx, nbIn, retour : integer;
begin
  if dllLoaded then
  begin
    // On recupère l'adresse complète du fichier de test quelque soit le plugin
    TestFile := LblEditSample.Text;

    if @GetProfileName<>nil then
    begin
      GetProfileName(ProfileName, SizeOf(ProfileName));
      Memo.Lines.Add('# GetProfileName loaded : ');
      Memo.Lines.Add(ProfileName);
    end else begin
      Memo.Lines.Add('! GetProfileName unloaded ');
    end;

    if @GetProfileDescription<>nil then
    begin
      GetProfileDescription(ProfileDescription, SizeOf(ProfileDescription));
      Memo.Lines.Add('# GetProfileDescription loaded : ');
      Memo.Lines.Add(ProfileDescription);
    end else begin
      Memo.Lines.Add('! GetProfileDescription unloaded');
    end;

    if @OpenFileDll<>nil then
    begin
      retour := OpenFileDll(PChar(TestFile), nbEx, nbIn);
      Memo.Lines.Add('# OpenFileDll : ');
      Memo.Lines.Add(IntToStr(retour));

      SetLength(Profil.coordonneesExDecoupe, nbEx);
      SetLength(Profil.coordonneesInDecoupe, nbIn);

      if @LoadFileDll<>nil then
      begin
        LoadFileDll(Profil);
        Memo.Lines.Add('# LoadFileDll : Executed');
      end else begin
        Memo.Lines.Add('! LoadFileDll unloaded');
      end;


      if (Length(Profil.coordonneesExDecoupe)-1)>=0 then
      begin
        Memo.lines.Add('# Profile loaded :');
        Memo.Lines.Add('# - Coordonnees Extrado');
        for i:=0 to Length(Profil.coordonneesExDecoupe)-1 do
        begin
          Memo.Lines.Add(FloatToStr(Profil.coordonneesExDecoupe[i].X)+';'+
                         FloatToStr(Profil.coordonneesExDecoupe[i].Y));
        end;
        Memo.Lines.Add('');
        Memo.Lines.Add('# - Coordonnees Intrado');
        for i:=0 to Length(Profil.coordonneesInDecoupe)-1 do
        begin
          Memo.Lines.Add(FloatToStr(Profil.coordonneesInDecoupe[i].X)+';'+
                         FloatToStr(Profil.coordonneesInDecoupe[i].Y));
        end;
      end else begin
        Memo.Lines.Add('! No data loaded');
      end;
    end else begin
      Memo.Lines.Add('! OpenFileDll unloaded');
    end;
  end else begin
    Memo.Lines.Add('! No plugin loaded');
  end;
end;

procedure TFMain.FreeMemory;
var
  FichierIni : TIniFile;
  InitPathName : string;
begin
  // Sauver préférences utilisateur
  InitPathName := ExtractFilePath(Application.ExeName);
  FichierIni := TIniFile.Create(InitPathName + 'Debug.ini');
  FichierIni.WriteString('General', 'SampleFile', LblEditSample.Text);
  FichierIni.WriteString('General', 'PlugIn', ComboBoxPlugIn.Items[ComboBoxPlugIn.ItemIndex]);
  FreeAndNil(FichierIni);

  FreeDll;
end;

// Libérer la dll
procedure TFMain.FreeDll;
begin
  // Libérer la mémoire
  if dllLoaded then
  begin
    @GetDllFamily := nil;
    @GetDescription := nil;
    @GetFileExtension := nil;
    @OpenFileDll := nil;
    @LoadFileDll := nil;
    @SaveFileDll := nil;
    @EmettreBit := nil;
    @InitialiserChauffeEtCommunication := nil;
    @AdapterOrdres := nil;
    @MoteurOnOff := nil;
    if DllHandle <> 0 then FreeLibrary(DllHandle);
  end;
end;

procedure TFMain.BitBtnExitClick(Sender: TObject);
begin
  Close;
end;

procedure TFMain.JvBitBtnFileClick(Sender: TObject);
begin
  if OpenDialog1.Execute then
    LblEditSample.Text := OpenDialog1.Files[0];
end;

procedure TFMain.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  FreeMemory;
end;

procedure TFMain.ComboBoxPlugInClick(Sender: TObject);
begin
  // Libérer le précédent plugin si nécessaire
  if dllLoaded then
  begin
    dllLoaded := false;
    EnableIHM(false, false);
    FreeDll;
  end;

  // Charger le plugin
  LoadDll;
end;

procedure TFMain.BitBtnClearMemoClick(Sender: TObject);
begin
  Memo.Clear;
end;

procedure TFMain.BitBtnSendStepClick(Sender: TObject);
var
  i : integer;
  bitSens, bitHorloge : integer;
  nbSteps : integer;
  bChauffe : boolean;
  valChauffe : integer;
  ms1, ms2 : integer;

  // var for plugin of communication
  ParametreChauffe : TParametreChauffe;
  ParametreCommunication : TParametreCommunication;
  MateriauActif : TMateriau;
begin

  BitBtnSendStep.Enabled := false;
  if not TryStrToInt(LblEditNbPas.Text, nbSteps) then
  begin
    nbSteps := 0;
  end;
  Initialiser(bitSens, bitHorloge, nbSteps);

  // Désactivé en attendant l'IHM
//  bChauffe := CheckBoxChauffe.Checked;
//  valChauffe := 100-StrToInt(LabeledEditValChauffe.Text);
  bChauffe := false;
  valChauffe := 0;


// TODO Comment gérer la configuration de ces bits
// Rajouter la config de ces pin dans l'IHM. Ce n'est pas prioritaire
//  ParametreCommunication.BitHorlogeExterne := BitHorlogeExterne;
//  ParametreCommunication.BitModeChauffe := BitModeChauffe;
//  ParametreCommunication.BitEmissionChauffe := BitEmissionChauffe;
//  ParametreCommunication.BitReceptionChauffe := BitReceptionChauffe;
//  ParametreCommunication.BitAlimMoteur := BitAlimMoteur;
//  ParametreCommunication.synchroniserMoteurAvecTimerExterne := ; // TODO true ou false ?
// TODO Permettre la configuration de ces paramètres
//  ParametreChauffe.chauffeActive := ParamChauffe.chauffeActive;
  // On force à false en attendant l'IHM
  ParametreChauffe.chauffeActive := false;
//  ParametreChauffe.chauffeMode := ParamChauffe.chauffeMode;
//  ParametreChauffe.chauffeDynamique := ParamChauffe.chauffeDynamique;
//  ParametreChauffe.chauffeUtilisateur := ; // TODO true ou false ?

  // No material
  //MateriauActif.nom := '';
//  MateriauActif.pourcentage1 := StrToInt(LabeledEditValChauffe.Text);
  // On force la chauffe à 0 tant que IHM non prête
  MateriauActif.pourcentage1 := 0;

  // Initialize heating parameter in the plugin like Jedicut does
  Memo.Lines.Add('# Jedicut will call  InitialiserChauffeEtCommunication');
  Memo.Lines.Add('with $378 as standard parallel port if the plugin dosen''t change it');
  if @InitialiserChauffeEtCommunication <> nil then
    InitialiserChauffeEtCommunication($378, ParametreChauffe, ParametreCommunication, MateriauActif);

  // Motors ON
  Memo.Lines.Add('# Test : Send step orders');
  Memo.Lines.Add('Number of steps : ' + LblEditNbPas.Text);

  if @MoteurOnOff <> nil then
  begin
    Memo.Lines.Add('Motors ON');
    MoteurOnOff(true);
  end else begin
    Memo.Lines.Add('! MotorOnOff unloaded');
  end;

  Memo.Lines.Add('Sending steps and heating orders');
  Memo.Lines.Add('Please wait...');

  // Send orders to motors
  nbSteps := Abs(nbSteps);
  for i := 0 to nbSteps-1 do
  begin
    EmettreBit(bitHorloge, bitSens, impulsion, true, bChauffe, valChauffe);
  end;

//  ms1 := GetTickCount;
//  EmettreBitEtChauffe(bitHorloge, bitSens, impulsion, true, bChauffe, valChauffe, 100, compteur);
//  ms2 := GetTickCount;
//  Memo.Lines.Add('Time for heating : '+IntToStr(ms2-ms1));

  // Motors Off
  MoteurOnOff(false);

  Memo.Lines.Add('Motors OFF');
  Memo.Lines.Add('----------- End of test -----------------');
  BitBtnSendStep.Enabled := true;
end;

procedure TFMain.Initialiser(var bitSens, bitHorloge, impulsion : integer);
begin
//  case RadioGroupMoteur.ItemIndex of
//    0 : begin
//      bitSens := 4;
//      bitHorloge := 8;
//    end;
//    1 : begin
//      bitSens := 1;
//      bitHorloge := 2;
//    end;
//    2 : begin
//      bitSens := 64;
//      bitHorloge := 128;
//    end;
//    3 : begin
//      bitSens := 16;
//      bitHorloge := 32;
//    end;
//  end;
//
//  if (nbSteps<0) then
//    bitSens := 0;

end;


end.
