{   Copyright 2009 Jerome

    This file is part of FileTrace.

    FileTrace is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FileTrace is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FileTrace.  If not, see <http://www.gnu.org/licenses/>.

    The software Jedicut is allowed to statically and dynamically link this library.
}

unit UCom;

interface

uses
  Classes, SysUtils, DateUtils, UType;

  // Fonction renvoyant le code famille de la dll, ce code indique le type de la dll
  // Les codes possibles :
  // 0 : Dll de communication - Une seule dll chargée par instance de Jedicut
  // 1 : Dll de lecture SEUL de fichier
  // 2 : Dll de lecture/ecriture de fichier
  function GetDllFamily : byte; export

  procedure GetDescription(Cible : PChar; tailleCible: integer);
  function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt ; export;
  procedure MoteurOnOff(moteurOn : boolean); export;
  procedure InitialiserChauffeEtCommunication(portBase : word ;
                                              ParamChauffe : TParametreChauffe ;
                                              ParamCommunication : TParametreCommunication ;
                                              Materiau : TMateriau); export;
  function EtatMachine : byte; export;

  procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur); export;
  function LireChauffeMachine : double; export;

  // Fonctions privées
  procedure CompresserOrdresMoteur(var ArrayOrdres : TArrayOrdresMoteur);

implementation

const TIME_OUT = 5000000;

var
  ParametreChauffe : TParametreChauffe;
  ParametreCommunication : TParametreCommunication;
  MateriauActif : TMateriau;
  pulseChauffe, periodeChauffe : integer;
  chauffeUn : boolean; // Etat du signal de Chauffe
  portAdresseBase : word; // Adresse de base du port parallèle
  bSignalChauffeHaut : boolean; // Variable permettant de ne positionner le signal de chauffe qu'une seule fois par périodede chauffe

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
  Description := 'Trace datée des bits envoyés dans le fichier C:\MachineVirtuelle.txt. Version 0.7.2';
  StrPLCopy(Cible, Description, tailleCible);
end;

{-----------------------------------------------------------------}
{ Méthode de la dll gérant l'alimentation des moteurs }
procedure MoteurOnOff(moteurOn : boolean);
begin
  // Méthode non utilisé dans cette dll
  Sleep(1);
  //periodeSignal := CalculerPeriode;
end;

{-----------------------------------------------------------------}
{ Méthode de la dll d'envoi des bits propre à un type de machine }
function EmettreBit(bitRotation, bitSens : byte ; vitesse : integer ; chauffe : double) : smallInt;
var
  Date : TDateTime;
  annee, mois, jour, heure, minute, seconde, milliSeconde : Word;
  Info : string;
  Fichier : TextFile;
  horloge, ancienneHorloge : byte; // Variable pour détecter le signal d'horloge
  timeout : integer;
  i : integer;
  pulse : integer;
  stepOk : boolean;
begin
  Date := Now;
  AssignFile(Fichier, 'E:\MachineVirtuelle.txt');
  try
    Append(Fichier); // On écrit à la suite du fichier s'il existe
  except
    Rewrite(Fichier); // On crée un nouveau fichier si ça n'a pas encore été fait
  end;

  // Ecrire les infos lié au port parallèle
//  Writeln(Fichier, 'ParametreChauffe');
//  Writeln(Fichier, 'BitModeChauffe'+IntToStr(ParametreCommunication.BitModeChauffe.adresseBase)+' '+IntToStr(ParametreCommunication.BitModeChauffe.iRegistre)+' '+IntToStr(ParametreCommunication.BitModeChauffe.iBit));
//  Writeln(Fichier, 'BitHorlogeExterne'+IntToStr(ParametreCommunication.BitHorlogeExterne.adresseBase)+' '+IntToStr(ParametreCommunication.BitHorlogeExterne.iRegistre)+' '+IntToStr(ParametreCommunication.BitHorlogeExterne.iBit));
//  Writeln(Fichier, 'BitEmissionChauffe'+IntToStr(ParametreCommunication.BitEmissionChauffe.adresseBase)+' '+IntToStr(ParametreCommunication.BitEmissionChauffe.iRegistre)+' '+IntToStr(ParametreCommunication.BitEmissionChauffe.iBit));
//  Writeln(Fichier, 'BitReceptionChauffe'+IntToStr(ParametreCommunication.BitReceptionChauffe.adresseBase)+' '+IntToStr(ParametreCommunication.BitReceptionChauffe.iRegistre)+' '+IntToStr(ParametreCommunication.BitReceptionChauffe.iBit));
//  Writeln(Fichier, 'BitAlimMoteur'+IntToStr(ParametreCommunication.BitAlimMoteur.adresseBase)+' '+IntToStr(ParametreCommunication.BitAlimMoteur.iRegistre)+' '+IntToStr(ParametreCommunication.BitAlimMoteur.iBit));
//  Writeln(Fichier, '------------------------');

  // Décoder la date
  DecodeDateTime(Date, annee, mois, jour, heure, minute, seconde, milliSeconde);
  // Formater la date pour le fichier
  Info := IntToStr(jour) +
          '-' + IntToStr(mois) +
          '-' + IntToStr(annee) +
          ' ' + IntToStr(heure) +
          ':' + IntToStr(minute) +
          ':' + IntToStr(seconde) +
          ',' + IntToStr(milliSeconde);

  // Formater l'information succédant à la date
  Info := Info + ' ' + IntToStr(bitRotation) + ' ' + IntToStr(bitSens)
          + ' ' + IntToStr(vitesse) + ' ' + FloatToStr(chauffe);
Writeln(Fichier, 'B;'+IntToStr(bitRotation)+';V;'+IntToStr(vitesse)+';C;'+FloatToStr(chauffe));
  timeOut := 0;

  pulse := 0;
  horloge := 0;

  stepOk := false;
  pulse := 0;

  if (vitesse>=0) then
  begin
    // Si on a choisit l'utilisation du timer externe
    if (ParametreCommunication.synchroniserMoteurAvecTimerExterne) then
    begin
      // Ce if est également appliqué pour MM2001
      if vitesse>1 then vitesse := vitesse-1
      else vitesse := 1;
      while (not stepOk) do  // rajouter la condition que vitesse<>0 pour sortir
      begin
        Inc(horloge);
        Inc(pulseChauffe);
        Inc(pulse);
//Write(Fichier, '.');
        // --------------------------
        // On pourrait mettre ici une gestion du timeOut, inutile pour cette dll
        // --------------------------
        if (true) then
        begin
          if(pulse=vitesse-1)then
          begin
            // Emettre le sens de rotation
            //Writeln(Fichier, '- Sens ' + IntToStr(bitSens));
            // Writeln(Fichier,IntToStr(pulse)+';'+IntToStr(bitSens)+';**V**'+IntToStr(vitesse));
          end;
          if(pulse=vitesse) then
          begin
            // Emettre l'ordre de rotation + sens
            //Writeln(Fichier, '--> Pas ' + IntToStr(bitRotation + bitSens));
            //Writeln(Fichier,IntTostr(pulse)+';'+IntToStr(bitRotation + bitSens)+';'+IntToStr(pulseChauffe));
//            Writeln(Fichier,';B;'+IntToStr(bitRotation)+';V;'+IntToStr(vitesse));
          end;
          if(pulse=vitesse+2)then
          begin
            // Emettre le sens de rotation
            //Writeln(Fichier, '- Sens ' + IntToStr(bitSens));
            stepOk := true; // Pour sortir de la boucle de gestion du pas
          end;
          if ((ParametreChauffe.chauffeActive)and(ParametreChauffe.chauffeUtilisateur)) then
          begin
            // Si Chauffe dynamique
            if ParametreChauffe.chauffeDynamique then
            begin
              if not bSignalChauffeHaut then
              begin
                if (pulseChauffe>=periodeChauffe) then
                begin
                  // Signal de chauffe à 1
                  //Writeln(Fichier, '- Chauffe 1 -----');
                  bSignalChauffeHaut := true;
                end;
              end;
              // On met à jour le nouveau periodeChauffe
              periodeChauffe := Trunc(Int(chauffe));
            end else begin
            // Chauffe PC mais sans chauffe dynamique
              if (pulseChauffe=periodeChauffe) then
                // Signal de chauffe à 1
                //Writeln(Fichier, '- Chauffe 1 -----');
            end;
            if (pulseChauffe=100) then
            begin
              // Signal de chauffe à 0
              //Writeln(Fichier, '- Chauffe 0 -----');
              pulseChauffe:=0;
              bSignalChauffeHaut := false; // Utilisé que pour la chauffe dynamique
            end;
          end else begin
            pulseChauffe := 0; // On force la chauffe à 0. Utile ??
          end;
        end;
        ancienneHorloge := horloge;
      end;
    end else begin
    // Si on n'a pas de chauffe et donc pas de timer externe
      Sleep(1);
      // Emettre l'ordre de rotation + sens
      Writeln(Fichier, '- Pas');
      for i := 0 to vitesse do
      begin
      end;
    end;
  end else begin
    Writeln(Fichier, '--- Pause ---');
  end;

  // Fermer le fichier
  CloseFile(Fichier);

  Result := NO_ERROR;
end;

{-----------------------------------------------------------------}
{ Initialiser les paramètre de la chauffe chauffe }
procedure InitialiserChauffeEtCommunication(portBase : word ; ParamChauffe : TParametreChauffe ; ParamCommunication : TParametreCommunication ; Materiau : TMateriau);
var
  Fichier : TextFile;
begin
  portAdresseBase := portBase;
  ParametreChauffe := ParamChauffe;
  ParametreCommunication := ParamCommunication;
  MateriauActif := Materiau;
  // En fait ce n'est pas la période mais le temps ou le signal est à 1
  periodeChauffe := Trunc(Int(MateriauActif.pourcentage1));
  // Compteur d'impulsion de chauffe

  AssignFile(Fichier, 'E:\MachineVirtuelle.txt');
  try
    Append(Fichier); // On écrit à la suite du fichier s'il existe
  except
    Rewrite(Fichier); // On crée un nouveau fichier si ça n'a pas encore été fait
  end;
  Writeln(Fichier, 'INIT Chauffe');
  CloseFile(Fichier);
  pulseChauffe := 0;
end;

{-----------------------------------------------------------------}
{ Lire des états de la machine                }
{ - Pour l'instant lecture du mode de chauffe : 1 mode PC, 0 mode manuel  }
function EtatMachine : byte;
begin
  Result := 1;
end;

{-----------------------------------------------------------------}
{ Fonction retournant la valeure de la chauffe }
function LireChauffeMachine : double;
begin
  Randomize; // Initialise le random utilisé pour fournir une valeur de chauffe de test
  // Result := -1; // Le signal de chauffe n'arrive pas à être interprété, soit il est absent, soit Jedicut n'arrive pas à le lire
  Result := Random; // Valeur entre 0 et 1, 0 étant la chauffe maxi
end;

{-----------------------------------------------------------------}
{ Adapter les coordonnées en fonction de la machine }
procedure AdapterOrdres(var ArrayOrdres : TArrayOrdresMoteur);
begin
  // Optimiser les vitesses de déplacement
//  CompresserOrdresMoteur(ArrayOrdres);
end;

{-----------------------------------------------------------------}
{ Compresser les ordres de rotation pour optimiser les vitesses }
procedure CompresserOrdresMoteur(var ArrayOrdres : TArrayOrdresMoteur);
type
  TContrainte = record
    byte, compteur : integer;
  end;
var
  Contrainte : array[0..5] of TContrainte;
  i, j, temp : integer;
  bPointOK : boolean;
begin
  // Initialiser les variables temporaires
  for j:=0 to 5 do
  begin
    Contrainte[j].byte := 0;
    Contrainte[j].compteur := 0;
  end;

  // Traiter tous les points
  for i:=0 to Length(ArrayOrdres.ArrayOrdres)-1 do
  begin
    bPointOK := false;

    // Tester si le point est connu
    for j:=0 to 5 do
    begin
      if (Contrainte[j].byte=ArrayOrdres.ArrayOrdres[i].bitRotation) then
      begin
        break;
      end else begin
        if(Contrainte[j].byte=0) then
        begin
          Contrainte[j].byte:=ArrayOrdres.ArrayOrdres[i].bitRotation;
          break;
        end;
      end;
    end;

    // Contrôle pour ne pas sortir de la boucle
    if (i+1 <= Length(ArrayOrdres.ArrayOrdres)-1) then
    begin
      // Si le suivant est du même type que le point courant
      if (ArrayOrdres.ArrayOrdres[i].bitRotation=ArrayOrdres.ArrayOrdres[i+1].bitRotation) then
      begin
        // Mettre à jour les contraintes
        for j:=0 to 5 do
        begin
          if (Contrainte[j].byte=0) then
          begin
            break;
          end;
          if (Contrainte[j].byte = ArrayOrdres.ArrayOrdres[i].bitRotation) then
          begin
            Contrainte[j].compteur := 0;
          end else begin
            Contrainte[j].compteur := Contrainte[j].compteur - ArrayOrdres.ArrayOrdres[i].vitesse - 1;
            if (Contrainte[j].compteur<0) then Contrainte[j].compteur := 0;
          end;
        end;
        bPointOK := true;
      end;
    end;

    // Adapter la vitesse du point courant
    if (not bPointOK) then
    begin
      // On historise la vitesse du point courant
      temp := ArrayOrdres.ArrayOrdres[i].vitesse;
      // On affecte au point courant la contrainte du point suivant quand c'est possible
      // sinon on ne fait rien
      if (i+1 <= Length(ArrayOrdres.ArrayOrdres)-1) then
      begin
        for j:=0 to 5 do
        begin
          // On recherche la contrainte du point suivant
          if (Contrainte[j].byte=ArrayOrdres.ArrayOrdres[i+1].bitRotation) then
          begin
            // Modifier la vitesse
            if (Contrainte[j].compteur - 1 < 0) then
            begin
              ArrayOrdres.ArrayOrdres[i].vitesse := 0;
            end else begin
              ArrayOrdres.ArrayOrdres[i].vitesse := Contrainte[j].compteur - 1;
            end;
            // On sort de la boucle
            break;
          end;
          // Si on arrive là, la contrainte n'était pas initialisée, donc elle est nulle
          // donc la vitesse est nulle
          if (j=5) then
          begin
            ArrayOrdres.ArrayOrdres[i].vitesse := 0;
          end;
        end;
      end;

      // Mettre à jour les contraintes
      for j:=0 to 5 do
      begin
        // Si byte=0 alors les points suivants le son également, donc on sort
        if (Contrainte[j].byte=0) then
        begin
          break;
        end;
        // Traitement particulier pour le point courant
        if (Contrainte[j].byte=ArrayOrdres.ArrayOrdres[i].bitRotation) then
        begin
          if (temp-ArrayOrdres.ArrayOrdres[i].vitesse<0) then
          begin
            Contrainte[j].compteur := 0;
          end else begin
            Contrainte[j].compteur:=temp-ArrayOrdres.ArrayOrdres[i].vitesse;
          end;
        end else begin
          // Pour tous les autres points
          Contrainte[j].compteur := Contrainte[j].compteur - ArrayOrdres.ArrayOrdres[i].vitesse;
          if (Contrainte[j].compteur<0) then Contrainte[j].compteur := 0;
        end;
      end;
    end;
  end;
end;

end.
