#include "pdgst.h"


#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <locale.h>

static t_class*pdgst_class=NULL;
typedef struct _pdgst
{
  t_object x_obj;
  t_outlet*x_infout;
} t_pdgst;



/* ================================================================== */
/* "real" pdgst object for meta-control */

static void pdgst_gstMess(t_pdgst*x, t_symbol*s, int argc, t_atom*argv) {
  post("_gst");
}

static void pdgst_free(t_pdgst*x) {
  if(x->x_infout)
    outlet_free(x->x_infout);
}

static void *pdgst_new(t_symbol*s, int argc, t_atom* argv) {
  /* LATER make a controller.... */
  t_pdgst*x=(t_pdgst*)pd_new(pdgst_class);
  x->x_infout=outlet_new(&x->x_obj, 0);
  return x;
}


/* ================================================================== */
/* class setup */


static int pdgst_loader(t_canvas *canvas, char *classname)
{
  post("pdgst: trying to load '%s'", classname);
  if(pdgst_element_setup_class(classname)) {
    return 1;
  }
  post("pdgst: trying to capsload '%s'", classname);
  if(pdgst_capsfilter_setup_class(classname)) {
    return 1;
  }

  /* fallback to not-our-business */
  return (0);
}

static int pdgst_loader_setup(void)
{
  GError *err = NULL;
  guint major=0, minor=0, micro=0, nano=0;

  /* initialize gstreamer */
  if (!gst_init_check(NULL, NULL, &err)) {
    if(err) {
      error ("gstreamer failed to initialize: %s", err->message);
      g_error_free (err);
    } else {
      error ("gstreamer failed to initialize");
    }
    return 0;
  }

  /* version check */
  gst_version(&major, &minor, &micro, &nano);
  if(GST_VERSION_MAJOR == major && GST_VERSION_MINOR == minor) {
    /* fine */
  } else {
    error("pdgst has been compiled against gst-%d.%d.%d.%d while you are using gst-%d.%d.%d.%d",
          GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO, GST_VERSION_NANO,
          major, minor, micro, nano);
    return 0;
  }

  sys_register_loader(pdgst_loader);

  return 1;
}


void pdgst_setup(void)
{
  char*locale=NULL;

  post("pdgst %s",pdgst_version);  
  post("\t(copyleft) IOhannes m zmoelnig @ IEM / KUG");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);


  locale=setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  pdgst_loader_setup();
  setlocale(LC_NUMERIC, locale);

  pdgst_class=class_new(gensym("pdgst"), 
                        (t_newmethod)pdgst_new,
                        (t_method)pdgst_free,
                        sizeof(t_pdgst),
                        0 /* CLASS_NOINLET */,
                        A_GIMME, 0);
  class_addmethod  (pdgst_class, (t_method)pdgst_gstMess, gensym("_gst"), A_GIMME, 0);

  pdgst_capsfilter_setup();
}

/*
 * interesting stuff:
   gst_update_registry (void);
*/
