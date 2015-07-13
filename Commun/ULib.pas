unit ULib;

interface

function Power2(Base, Exponent : Double) : Double;

implementation

{** A power function from Jack Lyle. Said to be more powerful than the
    Pow function that comes with Delphi.
    http://edn.embarcadero.com/article/20026 }
{ Better function for negative base }
function Power2(Base, Exponent : Double) : Double;
{ raises the base to the exponent }
  CONST
    cTiny = 1e-15;

  VAR
    Power : Double; { Value before sign correction }

  BEGIN
    Power := 0;
    { Deal with the near zero special cases }
    IF (Abs(Base) < cTiny) THEN BEGIN
      Base := 0.0;
    END; { IF }
    IF (Abs(Exponent) < cTiny) THEN BEGIN
      Exponent := 0.0;
    END; { IF }

    { Deal with the exactly zero cases }
    IF (Base = 0.0) THEN BEGIN
      Power := 0.0;
    END; { IF }
    IF (Exponent = 0.0) THEN BEGIN
      Power := 1.0;
    END; { IF }

    { Cover everything else }
    IF ((Base < 0) AND (Exponent < 0)) THEN
        Power := 1/Exp(-Exponent*Ln(-Base))
    ELSE IF ((Base < 0) AND (Exponent >= 0)) THEN
        Power := Exp(Exponent*Ln(-Base))
    ELSE IF ((Base > 0) AND (Exponent < 0)) THEN
        Power := 1/Exp(-Exponent*Ln(Base))
    ELSE IF ((Base > 0) AND (Exponent >= 0)) THEN
        Power := Exp(Exponent*Ln(Base));

    { Correct the sign }
    IF ((Base < 0) AND (Frac(Exponent/2.0) <> 0.0)) THEN
      Result := -Power
    ELSE
      Result := Power;
  END; { FUNCTION Pow }


end.
