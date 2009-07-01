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

#warning add docs

#include "pdgst/pdgst.h"


#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <locale.h>

#ifdef x_obj
# undef x_obj
#endif

t_symbol*s_pdgst__gst=NULL;
t_symbol*s_pdgst__gst_source=NULL;
t_symbol*s_pdgst__gst_filter=NULL;
t_symbol*s_pdgst__gst_sink=NULL;



#define PDGST_CAPSFILTER

static GstElement *s_pipeline=NULL;



static t_class*pdgst_class=NULL;
typedef struct _pdgst
{
  t_object x_obj;
  t_outlet*x_infout;
  GstElement*x_pipeline;
} t_pdgst;

static void pdgst_elem__infoout(t_pdgst_elem*x, int argc, t_atom*argv)
{
  if(argc) {
    if(A_SYMBOL==argv->a_type) {
      outlet_anything(x->x_infout, atom_getsymbol(argv), argc-1, argv+1);
    } else {
      outlet_list(x->x_infout, 0, argc, argv);
    }
  } else {
    outlet_bang(x->x_infout);
  }
}



static void pdgst__send_(t_symbol*s, int argc, t_atom*argv)
{
  if(s->s_thing)typedmess(s->s_thing, s_pdgst__gst, argc, argv);
}

static void pdgst__send(int argc, t_atom*argv)
{
  pdgst__send_(s_pdgst__gst, argc, argv);
}

static void pdgst__send_symbol(t_symbol*s)
{
  t_atom ap[1];
  SETSYMBOL(ap, s);
  pdgst__send(1, ap);
}


void pdgst__element_buscallback (GstBus*bus,GstMessage*msg,t_pdgst_elem*x) {
  t_method cb=x->l_busCallback;
  GstElement*src=NULL;

  if(NULL==cb) {
    verbose(0, "no bus callback available");
    return;
  }


  if(GST_IS_ELEMENT(GST_MESSAGE_SRC(msg)))
    src=GST_ELEMENT(GST_MESSAGE_SRC(msg));

  if(!src) {
    error("fixme: message without source");
    return;
  }

  if(0) {
    gchar *name0=NULL, *name1=NULL;
    g_object_get (G_OBJECT (src), "name", &name0, NULL);
    g_object_get (G_OBJECT (x->l_element), "name", &name1, NULL);
    //    post("cb from '%s' for '%s':: '%s'", name0, name1,  GST_MESSAGE_TYPE_NAME(msg));
    g_free (name0);
    g_free (name1);
  }

  if(src==x->l_element) {
    (*(t_gotfn)(*cb))(x, msg);
  } else {
    // hmm, this is a message originating from somebody else
    // how should we do that?
    // LATER make x aware that this is from somebody else...
    // OR shall we output this in the pdgst object??
    if(src==s_pipeline) {
      (*(t_gotfn)(*cb))(x, msg);
    }

  }

}

static GstElement*pdgst__getcontainer(t_pdgst_elem*element)
{
  /* LATER: try to find the responsible [pdgst] object for the given element 
   * and extract the bin/pipeline from there
   */
  return s_pipeline;
}

void pdgst_bin_add(t_pdgst_elem*element)
{
  /* LATER: do not ignore canvas within the element structure0 */
  GstElement*gele=pdgst__getcontainer(element);
  GstBus*bus=gst_pipeline_get_bus (GST_PIPELINE (gele));
  gst_bin_add(GST_BIN(gele), element->l_element);
  g_signal_connect (bus, "message", G_CALLBACK(pdgst__element_buscallback), element);
  gst_object_unref (bus); /* since we own bus returned by gst_pipeline_get_bus() */
}

void pdgst_bin_remove(t_pdgst_elem*element)
{
  GstState state, pending;
  GstElement*gele=pdgst__getcontainer(element);

  if(element->l_element) {
    gst_element_set_state (element->l_element, GST_STATE_NULL);
    if(gst_bin_remove(GST_BIN(gele), element->l_element)) {
      element->l_element=NULL;
    }
  }

}

GstBin*pdgst_get_bin(t_pdgst_elem*element)
{
  /* LATER: try to find the responsible [pdgst] object for the given element 
   * and extract the bin/pipeline from there
   */
  return GST_BIN(s_pipeline);
}




/* ================================================================== */
/* "real" pdgst object for meta-control */

static void pdgst__gstMess(t_pdgst*x, t_symbol*s, int argc, t_atom*argv) {
  post("ignoring message:: __gst: %s", s->s_name);
}

/* rebuild the gst-graph */
static void pdgst__rebuild(t_pdgst*x) {
#if 0
  pdgst__send_symbol(gensym("deregister"));
  pdgst__send_symbol(gensym("register"));
  pdgst__send_symbol(gensym("connect"));
#else
  t_atom ap[1];
  SETSYMBOL(ap,  gensym("connect"));
  pdgst__send_(s_pdgst__gst_source, 1, ap);
#endif
}

static void pdgst__start(t_pdgst*x) 
{
  gst_element_set_state (x->x_pipeline, GST_STATE_PLAYING);


  /* 
   * for testing:: write to XML
   */
  //  gst_xml_write_file (GST_ELEMENT (x->x_pipeline), fopen ("xmlTest.gst", "w"));
}

static void pdgst__stop(t_pdgst*x) {
  gst_element_set_state (x->x_pipeline, GST_STATE_PAUSED);
}

static void pdgst__float(t_pdgst*x, t_floatarg f) 
{
  if(f>0.f)
    pdgst__start(x);
  else
    pdgst__stop(x);
}


static gboolean pdgst__bus_callback (GstBus     *bus,
                                    GstMessage *message,
                                    gpointer    data)
{
  t_pdgst*x=(t_pdgst*)data;
  // post("bus %x, got message '%s'", x, GST_MESSAGE_TYPE_NAME (message));

  GstElement*src=NULL;
  if(GST_IS_ELEMENT(GST_MESSAGE_SRC(message)))
    src=GST_ELEMENT(GST_MESSAGE_SRC(message));

  if(src) {
    gchar *name;
    g_object_get (G_OBJECT (src), "name", &name, NULL);
    post("got message from %s", name);
    g_free (name);
  }

  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_UNKNOWN:
    break;
  case GST_MESSAGE_EOS:
    /* end-of-stream */
    post("EOS");
    break;
  case GST_MESSAGE_ERROR: {
    GError *err;
    gchar *debug;

    gst_message_parse_error (message, &err, &debug);
    pd_error (x, "%s", err->message);
    g_error_free (err);
    g_free (debug);

    break;
  }

  case GST_MESSAGE_WARNING:
    break;
  case GST_MESSAGE_INFO:
    break;
  case GST_MESSAGE_TAG:
    break;
  case GST_MESSAGE_BUFFERING:
    break;
  case GST_MESSAGE_STATE_CHANGED:
    break;
  case GST_MESSAGE_STATE_DIRTY:
    break;
  case GST_MESSAGE_STEP_DONE:
    break;
  case GST_MESSAGE_CLOCK_PROVIDE:
    break;
  case GST_MESSAGE_CLOCK_LOST:
    break;
  case GST_MESSAGE_NEW_CLOCK:
    break;
  case GST_MESSAGE_STRUCTURE_CHANGE:
    break;
  case GST_MESSAGE_STREAM_STATUS:
    break;
  case GST_MESSAGE_APPLICATION:
    break;
  case GST_MESSAGE_ELEMENT:
    break;
  case GST_MESSAGE_SEGMENT_START:
    break;
  case GST_MESSAGE_SEGMENT_DONE:
    break;
  case GST_MESSAGE_DURATION:
    break;
  case GST_MESSAGE_LATENCY:
    break;
  case GST_MESSAGE_ASYNC_START:
    break;
  case GST_MESSAGE_ASYNC_DONE:
    break;
  default:
    post("hmm, unknown message of type '%s'", GST_MESSAGE_TYPE_NAME(message));
    break;
  }


  return TRUE;
}


static void pdgst__free(t_pdgst*x) {
  if(x->x_infout) outlet_free(x->x_infout);
  x->x_infout=NULL;
}

void cb_message_error (GstBus*bus,GstMessage*msg,t_pdgst*x) {
  post("%x is a message!", msg);
  post("message type is '%s'", GST_MESSAGE_TYPE_NAME (msg));
}

static void *pdgst__new(t_symbol*s, int argc, t_atom* argv) {
  /* LATER make a controller.... */
  t_pdgst*x=(t_pdgst*)pd_new(pdgst_class);
  GstBus*bus=NULL;
  
  x->x_infout=outlet_new(&x->x_obj, 0);

  x->x_pipeline=s_pipeline;

  /* set up the bus watch */
  bus = gst_pipeline_get_bus (GST_PIPELINE (x->x_pipeline));
  //gst_bus_add_watch (bus, pdgst__bus_callback, x);
  gst_bus_add_signal_watch (bus);
  gst_object_unref (bus); /* since we own bus returned by gst_pipeline_get_bus() */

  return x;
}


/* ================================================================== */
/* class setup */


static int pdgst_loader(t_canvas *canvas, char *classname)
{
  if(pdgst_element_setup_class(classname)) {
    return 1;
  }
#ifdef PDGST_CAPSFILTER
  if(pdgst_capsfilter_setup_class(classname)) {
    return 1;
  }
#endif

  /* fallback to not-our-business */
  return (0);
}

static int pdgst_loader_init(void)
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

  s_pipeline=gst_pipeline_new(NULL);


  return 1;
}


static char*s_locale=NULL;
void pdgst_pushlocale(void)
{
  s_locale=setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
}

void pdgst_poplocale(void)
{
  setlocale(LC_NUMERIC, s_locale);
  s_locale=NULL;
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

  sys_register_loader(pdgst_loader);

  pdgst_class=class_new(gensym("pdgst"), 
                        (t_newmethod)pdgst__new,
                        (t_method)pdgst__free,
                        sizeof(t_pdgst),
                        0 /* CLASS_NOINLET */,
                        A_GIMME, 0);
  class_addmethod  (pdgst_class, (t_method)pdgst__gstMess, s_pdgst__gst, A_GIMME, 0);
  class_addbang  (pdgst_class, (t_method)pdgst__rebuild);
  class_addfloat  (pdgst_class, (t_method)pdgst__float);
  class_addmethod  (pdgst_class, (t_method)pdgst__start, gensym("start"), 0);
  class_addmethod  (pdgst_class, (t_method)pdgst__stop, gensym("stop"), 0);

#ifdef PDGST_CAPSFILTER
  pdgst_capsfilter_setup();
#endif

}

t_symbol*pdgst_privatesymbol(void) {
 return gensym("__gst");
}



/*
 * interesting stuff:
   gst_update_registry (void);
*/
