// Configuration de la machine

#ifndef _CONF_H
#define _CONF _H




// Limite de puissance Chauffe du fil
# define LIM_P_FIL 100
// vitesse USB
# define BAUDRATE 115200 // 110,300,600,1200,2400,4800,9600,14400
                         //19200, 38400,56000,57600,115200,128000,256000
/*==============================================================================
Attention : Si vous utilisez cette option, il ne faut pas utiliser la chauffe 
dynamique de Jedicut. cela ferait double emploi.
--------------------------------------------------------------------------------
Par le jeux des steps, il est possible de d�terminer la vitesse sur trajectoire
et asservir la chauffe en entrant une correction de chauffe . La vitesse sur 
trajectoire est la plus grande lorsque le fil parcourt un segment � 45�.
 Si vous avez une vitesse de 2mm/s en X et Y la vitesse du fil est 2 x 1.414 =
 2.828 mm/s. A partir de votre abaque de vos mat�riaux pour un arc donn�, vous 
 notez la valeur en % pour 2 mm/s --> 30% puis la valeur pour la vitesse de 
 2.828 --> 39.5% maintenant vous faites 39.5 / 30 =  1.3166, ce coefficient de 
 correction sera donc de 1.32. Le sketch se d�brouille pour les calculs de
 vitesse interm�daire du fil.
*/                         
# define CHAUFFE_ASSERV 1 // "0" pas de chauffe asservie, "1" chauffe asservie
# define CORRECT_CHAUFFE 1.32 // coefficient de chauffe en fonction vitesse

//==============================================================================







#endif
