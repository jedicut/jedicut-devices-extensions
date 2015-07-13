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

library MaxCom;

uses
  SysUtils,
  Classes,
  UCom in 'UCom.pas',
  UType in '..\Commun\UType.pas';

{$R *.res}

{ Liste des fonctions exportées }
exports
  GetDllFamily,
  GetDescription,
  EmettreBit,
  MoteurOnOff,
  InitialiserChauffeEtCommunication,
  EtatMachine;
begin
end.

