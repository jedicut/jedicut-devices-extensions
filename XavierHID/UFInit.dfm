object FInit: TFInit
  Left = 1203
  Top = 269
  Width = 237
  Height = 454
  Caption = 'XavierHID - Configuration de la machine'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 12
    Top = 12
    Width = 138
    Height = 13
    Caption = 'Statut du p'#233'riph'#233'rique USB : '
  end
  object BtnOk: TButton
    Left = 96
    Top = 348
    Width = 75
    Height = 25
    Caption = 'Ok'
    TabOrder = 0
    Visible = False
    OnClick = BtnOkClick
  end
  object EditStatut: TEdit
    Left = 160
    Top = 9
    Width = 45
    Height = 19
    Color = clRed
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentCtl3D = False
    ParentFont = False
    ReadOnly = True
    TabOrder = 1
    Text = 'Offline'
  end
  object BtnQuitter: TButton
    Left = 144
    Top = 388
    Width = 75
    Height = 25
    Caption = 'Quitter'
    TabOrder = 2
    OnClick = BtnQuitterClick
  end
  object Memo1: TMemo
    Left = 8
    Top = 36
    Width = 209
    Height = 269
    Ctl3D = False
    ParentCtl3D = False
    ReadOnly = True
    TabOrder = 3
  end
  object BtnSend: TButton
    Left = 8
    Top = 388
    Width = 75
    Height = 25
    Caption = 'Envoyer'
    TabOrder = 4
  end
  object LabeledEdit1: TLabeledEdit
    Left = 12
    Top = 328
    Width = 41
    Height = 19
    Ctl3D = False
    EditLabel.Width = 85
    EditLabel.Height = 13
    EditLabel.Caption = 'Nombre d'#39'envois :'
    ParentCtl3D = False
    TabOrder = 5
    Text = '2'
  end
  object BtnMarche: TButton
    Left = 8
    Top = 356
    Width = 75
    Height = 25
    Caption = 'Marche/Arr'#234't'
    TabOrder = 6
  end
  object HidCtl: TJvHidDeviceController
    Left = 156
    Top = 308
  end
end
