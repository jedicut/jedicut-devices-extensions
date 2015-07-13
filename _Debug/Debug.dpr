program Debug;

uses
  Forms,
  UMain in 'UMain.pas' {FMain},
  UType in '..\Commun\UType.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TFMain, FMain);
  Application.Run;
end.
