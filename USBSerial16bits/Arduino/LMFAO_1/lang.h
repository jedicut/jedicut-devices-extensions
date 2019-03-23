/* ========================================================================== */
/*                                                                            */
/*   lang.h                                                                   */
/*   (c) 2018/12 Author     Alain DENIS       alain@aeroden.fr                */
/*                                                                            */
/*   Description                                                              */
/*                    Fichier de langage  
   Dans le sketch, les textes affiches sont sous la forme de variable "TEXT" 
   avec un numéro . Pour creer une autre langue il faut recopier la definition 
   d'une langue et de la recopier avant le dernier #endif. ensuite changer le 
   nom apres " #ifdef LANG_****** "  puis traduire le texte qui se trouve entre 
   "" respecter la longueur la longueur entre "" espace compris.
   Dans la  fichier conf.h ajouter une ligne comme "//#define LANG_FRENCH " en 
   remplacant FRENCH par votre langue ******.                                 */
/*                                                                            */
/* ========================================================================== */
 /*Le 2018/12/26   Complété par Bryan  pour ENGLISH
 
 */
 /*===========================================================================*/
// Configuration du langage des affichagess la machine

#ifndef _LANG_H
#define _LANG_H
#include "conf.h"

#ifdef LANG_FRENCH
  #define TEXT1 "Lim Chauffe = "
  #define TEXT2 "Moteur asservi      "
  #define TEXT3 "Moteurs toujours ON "
  #define TEXT4 "Chauffe asservie    "
  #define TEXT5 "Chauffe fixe        "
  #define TEXT6 " Vitesse    Chauffe "
  #define TEXT7 "Mettre Mode en manu "
  #define TEXT8 "Mettre Mot. sur OFF "
  #define TEXT9 "Mettre Chauf sur OFF"
  #define TEXT10 "Mettre Cut sur OFF  "
  #define TEXT11 " Etat fin de course"
  #define TEXT12 " --> BP Homing"
  #define TEXT13 "  Homing en cours  "
  #define TEXT14 "Mode  Mot  Chau  Cut"
  #define TEXT15 "Manu"
  #define TEXT16 " PC "
  #define TEXT17 " Etat fin de course"
  #define TEXT18 "Fin -> Pot Ch < 10%"
  #define TEXT19 "   Attente Homing   "
  #define TEXT20 " Homing Termine     "
#endif        

// I suggest the following as they read better in English
#ifdef LANG_ENGLISH
  #define TEXT1 "Heat Limit = "
  #define TEXT2 "Piloted engines     "
  #define TEXT3 "Engines always ON   "
  #define TEXT4 "Corrected heating   "
  #define TEXT5 "Fixed heating       "
  #define TEXT6 "   Speed    Heating "
  #define TEXT7 "Move Mode to manual "
  #define TEXT8 "Move Steppers to OFF"
  #define TEXT9 "Move Heater to OFF  "
  #define TEXT10 "Move Cutter to OFF  " 
  #define TEXT11 "Limit switch states "
  #define TEXT12 "Push button to home "
  #define TEXT13 "Homing in progress  "
  #define TEXT14 "Mode  Eng  Heat  Cut"
  #define TEXT15 "Manu"
  #define TEXT16 " PC "
  #define TEXT17 "Limit switch states "  // Duplicate to #define 11?
  #define TEXT18 "Exit-> Heat < 10%   "
  #define TEXT19 "Waiting for Homing  "
  #define TEXT20 "  Homing Finished   "
#endif

#endif                 
