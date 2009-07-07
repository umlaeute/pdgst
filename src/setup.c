/******************************************************
 *
 * pdgst - implementation file
 *
 * copyleft (c) 2009 IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *   university of music and performing arts
 *
 ******************************************************
 *
 * license: GNU General Public License v.2 or later
 *
 ******************************************************/

/* setup.c
 *    call the setup-functions of the various pdgst objectclasses
 */


void  pdgst_dac_tilde_setup(void);

void pdgst_objects_setup(void){
  pdgst_dac_tilde_setup();
}
