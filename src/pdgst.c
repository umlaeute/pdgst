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

#include "pdgst/pdgst.h"

void  pdgst__setup(void);
void  pdgst_dac_tilde_setup(void);


void pdgst_objects_setup(void){
  pdgst__setup();
#ifdef PDGST_CAPSFILTER
  pdgst_capsfilter_setup();
#endif


  pdgst_dac_tilde_setup();
  pdgst_adc_tilde_setup();
}

void pdgst_setup(void)
{
#ifdef VERSION
  post("pdgst ver%s", VERSION);
#else
  post("pdgst");
#endif
  post("\t(copyleft) IOhannes m zmoelnig @ IEM / KUG");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);

  if(pdgst_init()) {
    pdgst_objects_setup();
  }
}


/*
 * interesting stuff:
   gst_update_registry (void);
*/



