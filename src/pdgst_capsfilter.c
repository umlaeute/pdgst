#include "pdgst.h"

static t_class*pdgst_capsfilter_class=NULL;
typedef struct _pdgst_capsfilter
{
  t_object x_obj;
  GstElement*element;
} t_pdgst_capsfilter;


/* ================================================================== */
/* capsfilter, e.g. [audio/x-raw-float channels=10] */

static void pdgst_capsfilter_gstMess(t_pdgst_capsfilter*x, t_symbol*s, int argc, t_atom*argv) {

}

static void pdgst_capsfilter_free(t_pdgst_capsfilter*x) {

}

static void *pdgst_capsfilter_new(t_symbol*s, int argc, t_atom* argv) {
  t_pdgst_capsfilter*x=(t_pdgst_capsfilter*)pd_new(pdgst_capsfilter_class);
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
    class_addcreator((t_newmethod)pdgst_capsfilter_new, gensym(classname), A_GIMME, 0);
    gst_object_unref (GST_OBJECT (lmn));
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
  class_addmethod  (pdgst_capsfilter_class, (t_method)pdgst_capsfilter_gstMess, gensym("_gst"), A_GIMME, 0);
}
