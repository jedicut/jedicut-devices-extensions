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

library FDC4_XP;

uses
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
  LibererRessources,
  EtatMachine,
  LireChauffeMachine,
  AdapterOrdres;
begin
end.

