#include "pdgst.h"
#include <strings.h>


static t_symbol*s_gst=NULL;

static t_class*pdgst_capsfilter_class=NULL;
typedef struct _pdgst_capsfilter
{
  t_pdgst_elem x_elem;
  GstCaps*x_caps;
} t_pdgst_capsfilter;


/* ================================================================== */
/* capsfilter, e.g. [audio/x-raw-float channels=10] */

static void pdgst_capsfilter_gstMess(t_pdgst_capsfilter*x, t_symbol*s, int argc, t_atom*argv) {

}

static void pdgst_capsfilter_free(t_pdgst_capsfilter*x) {

}

void *pdgst_element__new(t_symbol*s, int argc, t_atom* argv);
static void *pdgst_capsfilter_new(t_symbol*s, int argc, t_atom* argv) {
  t_pdgst_capsfilter*x=(t_pdgst_capsfilter*)pd_new(pdgst_capsfilter_class);
  
  x->x_caps=gst_caps_new_simple(s->s_name, NULL);

  while(argc--) {
    const char*cap=atom_getsymbol(argv++)->s_name;

    char*value=index(cap, '='); 

    if(value) {
      int len=value-cap;
      char field[MAXPDSTRING];
    
      if(len>=MAXPDSTRING-1)
        len=MAXPDSTRING-2;

      value++;
      snprintf(field, len+1, "%s", cap);

      gst_caps_set_simple(x->x_caps, field, G_TYPE_STRING, value);


      //      post("'%s' setting: '%s'[%d]: '%s'", cap, field, len, value);

    } else {
      //      post("not setting: '%s'", cap);
    }

    
    
    //    gst_caps_set_simple(x->x_caps, 

  }


  return x;
}


int pdgst_capsfilter_setup_class(char*classname)
{
  GstElement*lmn=NULL;
  GError*gerror=NULL;

  char dummypipeline[MAXPDSTRING];
  /* could be a capsfilter:
   * "capsfilter caps=audio/x-raw-float" == "audio/x-raw-float"
   */
  //    lmn=gst_parse_launch(classname, &gerror);

  snprintf(dummypipeline, MAXPDSTRING-1, "fakesrc ! %s ! fakesink", classname);
  lmn=gst_parse_launch(dummypipeline, &gerror);
  if(lmn){
    gst_object_unref (GST_OBJECT (lmn));

    if(gerror)return 0;

    class_addcreator((t_newmethod)pdgst_capsfilter_new, gensym(classname), A_GIMME, 0);
    return 1;
  } 
  return 0;
}


void pdgst_capsfilter_setup(void)
{
  pdgst_capsfilter_class=class_new(NULL, 
                                   (t_newmethod)pdgst_capsfilter_new,
                                   (t_method)pdgst_capsfilter_free,
                                   sizeof(t_pdgst_capsfilter),
                                   0 /* CLASS_NOINLET */,
                                   A_GIMME, 0);
  class_addmethod  (pdgst_capsfilter_class, (t_method)pdgst_capsfilter_gstMess, s_gst, A_GIMME, 0);
  s_gst=pdgst_privatesymbol();
}
