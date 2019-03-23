// Configuration de la machine


#ifndef _CONF_H
#define _CONF_H



#define MACHINE_NAME "  CNC FIL AERODEN"

#define VERSION "v1.4.0"

// Choix de la langue : la langue choisie ne doit pas être en commentaire

#define LANG_FRENCH
//#define LANG_ENGLISH

// millimeter per Step
// Example 1: Stepper Driver setting: Full Step, Stepper Motor: 400 steps per revolution, M6 Lead screw: 1mm thread => (mm per revolution * driver setting) / step per revolution = (1*1)/400 = 0.0025
// Ecxample 2: Stepper Driver Setting: 1/8 Step, Stepper Motor: 200 steps per revolution, Belt: 2mm between teeth, Pulley: 20 teeth => (Pulley teeth * Belt teeth interspace * driver setting) / step per revolution = (20*2*1/8)/200 = 0.025
#define MM_PER_STEP 0.0025


// Limite de puissance Chauffe du fil en %
#define MAX_PERCENTAGE_WIRE 80

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
# define CORRECT_CHAUFFE 1.32 // coefficient de chauffe en fonction vitesse

//==============================================================================
#endif


