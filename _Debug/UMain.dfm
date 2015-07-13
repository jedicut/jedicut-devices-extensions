object FMain: TFMain
  Left = 130
  Top = 31
  BorderStyle = bsDialog
  Caption = 'Debug Jedicut Extension'
  ClientHeight = 526
  ClientWidth = 735
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 16
    Width = 76
    Height = 13
    Caption = 'Choose a plugin'
  end
  object JvGroupBoxCom: TJvGroupBox
    Left = 8
    Top = 180
    Width = 313
    Height = 305
    Caption = 'Test plugin of communication'
    TabOrder = 3
    object LblEditNbPas: TLabeledEdit
      Left = 8
      Top = 36
      Width = 121
      Height = 19
      Ctl3D = False
      EditLabel.Width = 78
      EditLabel.Height = 13
      EditLabel.Caption = 'Number of step :'
      ParentCtl3D = False
      TabOrder = 0
      Text = '10'
    end
    object GroupBoxChauffe: TGroupBox
      Left = 8
      Top = 176
      Width = 165
      Height = 81
      Caption = 'Wire heating'
      Enabled = False
      TabOrder = 1
      object Label2: TLabel
        Left = 80
        Top = 56
        Width = 8
        Height = 13
        Caption = '%'
      end
      object CheckBoxChauffe: TCheckBox
        Left = 8
        Top = 16
        Width = 125
        Height = 17
        Caption = 'Enable heating'
        Checked = True
        Enabled = False
        State = cbChecked
        TabOrder = 0
      end
      object LabeledEditValChauffe: TLabeledEdit
        Left = 8
        Top = 52
        Width = 69
        Height = 21
        EditLabel.Width = 72
        EditLabel.Height = 13
        EditLabel.Caption = 'Heating value :'
        TabOrder = 1
        Text = '50'
      end
    end
    object RadioGroupMoteur: TRadioGroup
      Left = 156
      Top = -4
      Width = 165
      Height = 61
      Caption = 'Motor'
      Columns = 2
      Enabled = False
      ItemIndex = 0
      Items.Strings = (
        'X1'
        'Y1'
        'X2'
        'Y2')
      TabOrder = 2
      Visible = False
    end
    object BitBtnSendStep: TBitBtn
      Left = 200
      Top = 64
      Width = 105
      Height = 69
      Caption = 'Send step orders'
      TabOrder = 3
      OnClick = BitBtnSendStepClick
    end
    object BitBtnReadHeating: TBitBtn
      Left = 200
      Top = 272
      Width = 105
      Height = 25
      Caption = 'Read heating value'
      Enabled = False
      TabOrder = 4
    end
    object LabeledEditReadHeatingValue: TLabeledEdit
      Left = 8
      Top = 276
      Width = 121
      Height = 19
      Ctl3D = False
      EditLabel.Width = 108
      EditLabel.Height = 13
      EditLabel.Caption = 'Heating value readed :'
      Enabled = False
      ParentCtl3D = False
      ReadOnly = True
      TabOrder = 5
    end
    object BitBtn1: TBitBtn
      Left = 200
      Top = 184
      Width = 105
      Height = 73
      Caption = 'Send heating value'
      Enabled = False
      TabOrder = 6
    end
    object GroupBoxMotor: TGroupBox
      Left = 8
      Top = 60
      Width = 165
      Height = 109
      Caption = 'Motor control'
      Enabled = False
      TabOrder = 7
    end
  end
  object BitBtnExit: TBitBtn
    Left = 8
    Top = 492
    Width = 75
    Height = 25
    Caption = 'Exit'
    TabOrder = 0
    Visible = False
    OnClick = BitBtnExitClick
    Glyph.Data = {
      76010000424D7601000000000000760000002800000020000000100000000100
      04000000000000010000120B0000120B00001000000000000000000000000000
      800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00330000000000
      03333377777777777F333301BBBBBBBB033333773F3333337F3333011BBBBBBB
      0333337F73F333337F33330111BBBBBB0333337F373F33337F333301110BBBBB
      0333337F337F33337F333301110BBBBB0333337F337F33337F333301110BBBBB
      0333337F337F33337F333301110BBBBB0333337F337F33337F333301110BBBBB
      0333337F337F33337F333301110BBBBB0333337F337FF3337F33330111B0BBBB
      0333337F337733337F333301110BBBBB0333337F337F33337F333301110BBBBB
      0333337F3F7F33337F333301E10BBBBB0333337F7F7F33337F333301EE0BBBBB
      0333337F777FFFFF7F3333000000000003333377777777777333}
    NumGlyphs = 2
  end
  object Memo: TMemo
    Left = 328
    Top = 8
    Width = 401
    Height = 513
    Color = clBlack
    Ctl3D = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clAqua
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentCtl3D = False
    ParentFont = False
    ReadOnly = True
    TabOrder = 1
  end
  object BitBtnClearMemo: TBitBtn
    Left = 248
    Top = 492
    Width = 75
    Height = 25
    Caption = 'Clear'
    TabOrder = 2
    OnClick = BitBtnClearMemoClick
    Glyph.Data = {
      76010000424D7601000000000000760000002800000020000000100000000100
      04000000000000010000120B0000120B00001000000000000000000000000000
      800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00500005000555
      555557777F777555F55500000000555055557777777755F75555005500055055
      555577F5777F57555555005550055555555577FF577F5FF55555500550050055
      5555577FF77577FF555555005050110555555577F757777FF555555505099910
      555555FF75777777FF555005550999910555577F5F77777775F5500505509990
      3055577F75F77777575F55005055090B030555775755777575755555555550B0
      B03055555F555757575755550555550B0B335555755555757555555555555550
      BBB35555F55555575F555550555555550BBB55575555555575F5555555555555
      50BB555555555555575F555555555555550B5555555555555575}
    NumGlyphs = 2
  end
  object JvGroupBoxFile: TJvGroupBox
    Left = 7
    Top = 68
    Width = 313
    Height = 105
    Caption = 'Test plugin of file'
    TabOrder = 4
    object JvBitBtnFile: TJvBitBtn
      Left = 240
      Top = 32
      Width = 65
      Height = 25
      Caption = 'Select'
      TabOrder = 0
      OnClick = JvBitBtnFileClick
      HotTrackFont.Charset = DEFAULT_CHARSET
      HotTrackFont.Color = clWindowText
      HotTrackFont.Height = -11
      HotTrackFont.Name = 'MS Sans Serif'
      HotTrackFont.Style = []
    end
    object BitBtnLoad: TBitBtn
      Left = 8
      Top = 64
      Width = 297
      Height = 25
      Caption = 'Open'
      TabOrder = 1
      OnClick = BitBtnLoadClick
    end
    object LblEditSample: TLabeledEdit
      Left = 8
      Top = 36
      Width = 225
      Height = 19
      Ctl3D = False
      EditLabel.Width = 162
      EditLabel.Height = 13
      EditLabel.Caption = 'A file to test your plugin if needed :'
      ParentCtl3D = False
      TabOrder = 2
    end
  end
  object ComboBoxPlugIn: TComboBox
    Left = 8
    Top = 32
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 5
    OnClick = ComboBoxPlugInClick
  end
  object OpenDialog1: TOpenDialog
    Left = 292
    Top = 8
  end
end
