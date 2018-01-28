// Configuration de la machine

#ifndef _CONF_H
#define _CONF _H



// Pas par step : 400steps par tour, vis M6 au pas de 1mm --> 1/400 = 0.0025
#define PAS_STEP 0.0025 
// Limite de puissance Cutter: cutter 6V alimenté en 12v PWM à 50%
# define LIM_P_CUTTER 50
// Limite de puissance Chauffe du fil en %
# define LIM_P_FIL 80
// Choix entre Potentiomètre  et encodeur pour le chauffe du fil en manuel
// Mettre POT_CHAUF "1" pour le potentiomètre, Mettre POT_CHAUF 0 pour l'encodeur
# define POT_CHAUF 1
// Mettre ON_BUZZER "1" pour Alarme sonore fin de course,"0" pas d'alarme sonore
# define ON_BUZZER 1
// vitesse USB
# define BAUDRATE 115200 // 110,300,600,1200,2400,4800,9600,14400
                         //19200, 38400,56000,57600,115200,128000,256000


#endif
