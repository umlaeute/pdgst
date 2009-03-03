#include "pdgst.h"


#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <locale.h>




/* the pdgst base-class */
typedef struct _pdgst_core
{
  GstElement*element;
} t_pdgst_core;



static t_class*pdgst_class;
typedef struct _pdgst
{
  t_object x_obj;
  t_pdgst_core*x_gst;

  t_outlet*x_infout;
} t_pdgst;


/* should be "static t_atom*" */
static void pdgst_getparam(t_pdgst*x, t_symbol*s)
{
  if(x->x_gst&&x->x_gst->element) {
    GstElement*element=x->x_gst->element;
    GValue v;
    GType t;

    t_symbol*s0;
    t_float f;
    t_atom a;
    int success=0;

    g_object_get_property(G_OBJECT (element), s->s_name, &v);

    if(G_VALUE_HOLDS(&v,G_TYPE_STRING)) {
         success=1;
         s0=gensym(g_value_get_string(&v));
         SETSYMBOL(&a, s0);
    } else if (G_VALUE_HOLDS(&v,G_TYPE_INT)) {
      success=1;
      f=g_value_get_int(&v);
      SETFLOAT(&a, f);
    } else if  (G_VALUE_HOLDS(&v,G_TYPE_FLOAT)) {
      success=1;
      f=g_value_get_float(&v);
      SETFLOAT(&a, f);
    } else {
      pd_error(x, "don't know what to do...");
      success=0;
    }
    
    if(success) {
      outlet_list(x->x_infout, 0, 1, &a);
    }
    g_value_unset(&v);
  } else {
    // no element
  }
}

static void pdgst_setparam(t_pdgst*x, t_symbol*s, t_atom*ap)
{
  if(x->x_gst&&x->x_gst->element) {
    GstElement*element=x->x_gst->element;
    switch(ap->a_type) {
    case A_FLOAT:
      g_object_set (G_OBJECT (element), s->s_name, atom_getfloat(ap), NULL);
      break;
    case A_SYMBOL: default:
      g_object_set (G_OBJECT (element), s->s_name, atom_getsymbol(ap)->s_name, NULL);
      break;
    }
  } else {
    // no element
  }
}

static void pdgst_core_free(t_pdgst_core*gstcore)
{
  if(!gstcore)return;
  if(gstcore->element) {
    gst_object_unref (GST_OBJECT (gstcore->element));
  }
}

static t_pdgst_core*pdgst_core_new(GstElement*lmn, t_object*obj)
{
  t_pdgst_core*ret=(t_pdgst_core*)getbytes(sizeof(t_pdgst_core));
  ret->element=lmn;
  return ret;
}


static void pdgst_free(t_pdgst*x) {
  if(x->x_gst)
    pdgst_core_free(x->x_gst);
  x->x_gst=NULL;
  outlet_free(x->x_infout);
}

static void *pdgst_new(t_symbol*s, int argc, t_atom* argv) {
  t_pdgst*x=(t_pdgst*)pd_new(pdgst_class);
  post("pdgst_new: %s", s->s_name);
  x->x_gst=NULL;
  x->x_infout=NULL;

  if(gensym("pdgst")==s) {
    /* LATER make a controller.... */

  } else {
    GstElement*lmn=gst_element_factory_make(s->s_name, NULL);
    if(NULL==lmn) {
      post("factory failed to create element...'%s'", s->s_name);
      return NULL;
    }
    x->x_infout=outlet_new(&x->x_obj, 0);
    x->x_gst=pdgst_core_new(lmn, &x->x_obj);
    post("created gstelement");
  }


  return x;
}

static void pdgst_class_setup(char*classname) {
  class_addcreator((t_newmethod)pdgst_new, gensym(classname), A_GIMME, 0);
}




static int pdgst_loader(t_canvas *canvas, char *classname)
{
  GstElementFactory *factory = NULL;
  factory=gst_element_factory_find (classname);
  if(factory){
    pdgst_class_setup(classname);
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
}

/*
 * interesting stuff:
   gst_update_registry (void);
*/
