// Configuration de la machine
#ifndef _CONF_H
#define _CONF_H

//#define DEBUG

#define MACHINE_NAME "  CNC FIL AERODEN"

#define VERSION "v4.7.0"

// Choix de la langue : la langue choisie ne doit pas être en commentaire
#define LANG_FRENCH
//#define LANG_ENGLISH
//#define LANG_DEUTSCH                            // Dirk: deutsche Texte eingefügt

// millimeter per Step
// Example 1: Stepper Driver setting: Full Step, Stepper Motor: 400 steps per revolution, M6 Lead screw: 1mm thread => (mm per revolution * driver setting) / step per revolution = (1*1)/400 = 0.0025
// Ecxample 2: Stepper Driver Setting: 1/8 Step, Stepper Motor: 200 steps per revolution, Belt: 2mm between teeth, Pulley: 20 teeth => (Pulley teeth * Belt teeth interspace * driver setting) / step per revolution = (20*2*1/8)/200 = 0.025
#define MM_PER_STEP 0.0025

// Limite de puissance Cutter: cutter 6V alimenté en 12v PWM à 50%
#define MAX_PERCENTAGE_CUTTER 75

// Limite de puissance Chauffe du fil en %
#define MAX_PERCENTAGE_WIRE 100

// Choix entre Potentiomètre  et encodeur pour le chauffe du fil en manuel
// Decommenter la ligne de choix
//#define HEAT_CONSIGN_ROTARY_ENCODER
#define HEAT_CONSIGN_POTENTIOMETER
//Valeur d'initialisation de l'encodeur en %
#define VALEUR_INIT_ENCODEUR 25
//Decommenter la ligne suivante pour inverser le sens de l'encodeur
//#define ROT_ENC_INVERS

// Mettre ON_BUZZER "1" pour Alarme sonore fin de course,"0" pas d'alarme sonore
#define BUZZER_ON


/*=============================================================================
Paramètre permettant de définir si les moteurs restent alimentés à l'arrêt,
pour les équipement mecaniques à vis ce n'est pas nécessaire, par contre
pour les équipements à courroies il est préférable de les laisser sous
tension. Il est possible de mettre les moteurs hors tension par l'interrupteur 
"ON/OFF". "MOTEUR_ON_ASSERVI" mis à "1" determine que la mise sous tension est
asservie aux ordres du sketch Arduino et Jedicut. A "0" les moteurs sont
toujours sous tension.
*/
#define MOTEUR_ON_ASSERVI 0 // "0" Les moteurs sont toujours sous tension

/*=============================================================================
Impératif : Ecran de "configuration machine" dans Jedicut : toutes les cases
" Inverser " sont décochées. L'inversion des sens si nécessaire doit se faire
 par le mask ci-dessous pour obtenir un fonctionnement correct des séquences
 de mise à Zéro.
 Inversion du sens de déplacement mask "1" pour inversion. (bits de poids fort)
 Y2, Y1, X2, X1, 0,0,0,0 ; "0b" annonce une valeur binaire
*/ 
#define INV_DIR_MASK 0b00000000 // "1" inversion des 4 axes -> 0b11110000

/*==============================================================================
Suivant les fins de courses utilisés il est nécessaire parfois de les inverser.
Le sens normal: le fin de course non sollicité on a "0" sur l'entrée de la ramps
Le sens inverse : le fin de course non sollicité on a "1" sur l'entrée de la ramps
*/
#define INV_FDC_X1 0 // "0" Non inversion, "1" Inversion
#define INV_FDC_Y1 0 // "0" Non inversion, "1" Inversion
#define INV_FDC_X2 0 // "0" Non inversion, "1" Inversion
#define INV_FDC_Y2 0 // "0" Non inversion, "1" Inversion

/*=============================================================================
Paramètres pour permettre le calcul des données nécessaires à la mise à zéro
de la machine
Pas par step : 400steps par tour, vis M6 au pas de 1mm --> 1/400 = 0.0025
Mettre la même valeur que pour Jedicut.
*/
//#define PAS_STEP 0.0025 
#define VIT_RECH_FDC 4.50 // Vitesse de recherche fdc en mm/s Format XX.XX
#define VIT_AJUST_FDC 1.00  // Vitesse d'ajustement fdc en mm/s Format XX.XX

/*=============================================================================
Ces paramètres déterminent les différents types de séquence Homing
*/
#define SEQ_HOMING 0 // "1" sequence Homing active, "0"  Pas de sequence Homing
#define POS_SECU_Y 1  // "1" exécute une remonté des Y avant le homing
#define MM_POS_SECU_Y 2 // valeur de remonté des Y de x mm avant le homing
#define PREPOS 1 // "1" Pré-positionnemnt après homing permis, "0" pas de 
                 // pré-positionnement
#define MM_PREPOS_Y 1 // Valeur en mm de la distance du pré-positionnement Y
#define MM_PREPOS_X 1 // Valeur en mm de la distance du pré-positionnement X

//=============================================================================
// USB Serial communication speed, choose amongst following value:
// 9600, 14400, 38400, 57600, 115200, 250000
// JediCut comport.ini file must have the exact same value
#define BAUDRATE 115200

//==============================================================================
// If the Stepper Driver has an inverted enable pin level, uncomment this 
//#define INVERT_STEPPER_DRIVER_ENABLE_PIN

#ifdef INVERT_STEPPER_DRIVER_ENABLE_PIN
	#define	STEPPER_DRIVER_ENABLE_HIGH_LEVEL 1
	#define	STEPPER_DRIVER_ENABLE_LOW_LEVEL 0
#else
	#define	STEPPER_DRIVER_ENABLE_HIGH_LEVEL 0
	#define	STEPPER_DRIVER_ENABLE_LOW_LEVEL 1
#endif

/*==============================================================================
Attention : Si vous utilisez cette option, il ne faut pas utiliser la chauffe 
dynamique de Jedicut. cela ferait double emploi.
--------------------------------------------------------------------------------
Par le jeux des steps, il est possible de déterminer la vitesse sur trajectoire
et asservir la chauffe en entrant une correction de chauffe . La vitesse sur 
trajectoire est la plus grande lorsque le fil parcourt un segment à 45°.
 Si vous avez une vitesse de 2mm/s en X et Y la vitesse du fil est 2 x 1.414 =
 2.828 mm/s. A partir de votre abaque de vos matériaux pour un arc donné, vous 
 notez la valeur en % pour 2 mm/s --> 30% puis la valeur pour la vitesse de 
 2.828 --> 39.5% maintenant vous faites 39.5 / 30 =  1.3166, ce coefficient de 
 correction sera donc de 1.32. Le sketch se débrouille pour les calculs de
 vitesse intermédaire du fil.
*/                         
# define CHAUFFE_ASSERV 0 // "0" pas de chauffe asservie, "1" chauffe asservie
# define CORRECT_CHAUFFE 1.326 // coefficient de chauffe en fonction vitesse

//==============================================================================

#endif
