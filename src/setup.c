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

t_symbol*s_pdgst__gst=NULL;
t_symbol*s_pdgst__gst_source=NULL;
t_symbol*s_pdgst__gst_filter=NULL;
t_symbol*s_pdgst__gst_sink=NULL;



void  pdgst__setup(void);
void  pdgst_dac_tilde_setup(void);


void pdgst_objects_setup(void){
  pdgst__setup();
#ifdef PDGST_CAPSFILTER
  pdgst_capsfilter_setup();
#endif


  pdgst_dac_tilde_setup();
}

void pdgst_setup(void)
{
  char*locale=NULL;
  int err=0;

  if(!s_pdgst__gst) {
    const char*_gst_="__gst";
    char buf[MAXPDSTRING];
    s_pdgst__gst=gensym(_gst_);;
    snprintf(buf, MAXPDSTRING-1, "%s_source", _gst_); buf[MAXPDSTRING-1]=0;
    s_pdgst__gst_source=gensym(buf);
    snprintf(buf, MAXPDSTRING-1, "%s_filter", _gst_); buf[MAXPDSTRING-1]=0;
    s_pdgst__gst_filter=gensym(buf);
    snprintf(buf, MAXPDSTRING-1, "%s_sink", _gst_); buf[MAXPDSTRING-1]=0;
    s_pdgst__gst_sink=gensym(buf);
  }

  post("pdgst %s",pdgst_version);  
  post("\t(copyleft) IOhannes m zmoelnig @ IEM / KUG");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);

  pdgst_pushlocale();
  err=pdgst_loader_init();
  if(err)pdgst_loop_setup();
  pdgst_poplocale();
  if(!err)return;

  pdgst_objects_setup();
}

t_symbol*pdgst_privatesymbol(void) {
 return gensym("__gst");
}



/*
 * interesting stuff:
   gst_update_registry (void);
*/



