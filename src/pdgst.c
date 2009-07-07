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

static void pdgst__send_(t_symbol*s, int argc, t_atom*argv)
{
  //  post("pdgst:send to %s", s->s_name);
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

static GstElement*pdgst__getcontainer(t_pdgst_base*element)
{
  /* LATER: try to find the responsible [pdgst] object for the given element 
   * and extract the bin/pipeline from there
   */
  return s_pipeline;
}

GstBin*pdgst_get_bin(t_pdgst_base*element)
{
  /* LATER: try to find the responsible [pdgst] object for the given element 
   * and extract the bin/pipeline from there
   */
  return GST_BIN(pdgst__getcontainer(element));
}

static void pdgst_buscallback (GstBus*bus,GstMessage*msg,t_pdgst_base*x) {
  //  post("buscallback %x", x);
  /* do some checks whether x is still valid
   * e.g. check whether there is an <x> bound to "__gst"
   */
  //...
  t_symbol*s=pdgst_base__bindsym(x);
  if(s->s_thing) {

    /* if all goes well, we call back the element */
    pdgst_base__buscallback(bus, msg, x);
  }
}

void pdgst_bin_add(t_pdgst_base*element)
{
  /* LATER: do not ignore canvas within the element structure0 */
  GstElement*gele=pdgst__getcontainer(element);
  GstBus*bus=gst_pipeline_get_bus (GST_PIPELINE (gele));
  gulong handler=0;

  gchar*name=gst_element_get_name(element->l_element);
  GstElement*lmn=gst_bin_get_by_name(GST_BIN(gele), name);
  g_free(name);

  post("bin adding element %x [%x]", element, element->l_element);

  if(NULL!=lmn) {
    /* hey, this has already been added to our pipeline... */
    gst_object_unref (lmn); /* since we own bus returned by gst_bin_get_by_name() */
    return;
  }

  if(gst_bin_add(GST_BIN(gele), element->l_element)) {
  } else {
    post("could not add element '%s' [%x] to bus", element->x_name->s_name, element);
    return;
  }
  handler=g_signal_connect (bus, "message", G_CALLBACK(pdgst_buscallback), element);
  post("bin added %x [%x] to bus %x: %d", element, element->l_element, bus, handler);
  gst_object_unref (bus); /* since we own bus returned by gst_pipeline_get_bus() */

  element->l_sighandler_bin=handler;
}

void pdgst_bin_remove(t_pdgst_base*element)
{
  GstState state, pending;
  GstElement*gele=pdgst__getcontainer(element);

  GstBus*bus=gst_pipeline_get_bus (GST_PIPELINE (gele));
  gulong id=element->l_sighandler_bin;

  gchar*name=gst_element_get_name(element->l_element);
  GstElement*lmn=gst_bin_get_by_name(GST_BIN(gele), name);
  g_free(name);

  if(NULL==lmn) {
    /* no element of this name in our pipeline... */
    post("element %x not in bin", element);
    return;
  }
  gst_object_unref (lmn); /* since we own bus returned by gst_bin_get_by_name() */

  if(id) {
    startpost("removing buscallback-handler %d for %x: ", id, element);
    if (g_signal_handler_is_connected (bus, id)) {
      g_signal_handler_disconnect (bus, id);
      post("ok");
    } else post("ko");
    element->l_sighandler_bin=0;
  } else {
    post("no buscallback-handler to remove for element %x", element);
  }
  gst_bus_set_flushing(bus, TRUE) ;

  gst_object_unref (bus); /* since we own bus returned by gst_pipeline_get_bus() */

  GstStateChangeReturn ret=gst_element_set_state (element->l_element, GST_STATE_NULL); // this can halt the system, don't know why yet...
  if(!gst_bin_remove(GST_BIN(gele), element->l_element)) {
    error("could not remove '%s' from pipeline", element->x_gstname->s_name);
  }
  gst_bus_set_flushing(bus, FALSE) ;
}

/* ================================================================== */
/* "real" pdgst object for meta-control */

static void pdgst__gstMess(t_pdgst*x, t_symbol*s, int argc, t_atom*argv) {
  post("ignoring message:: __gst: %s", s->s_name);
}


static void pdgst__remove_all_elements(t_pdgst*x) {
  GstIterator* it=NULL;
  GstIteratorResult res;
  GstElement*element;
  it=gst_bin_iterate_elements(GST_BIN(x->x_pipeline));
  if(NULL==it)return;

  while(GST_ITERATOR_OK==(res=gst_iterator_next(it, (gpointer)&element))) {
    gst_bin_remove(GST_BIN(x->x_pipeline), element);
    gst_object_unref (GST_OBJECT (element)); /* since we own lmn returned by gst_iterator_next() */

  }

  gst_iterator_free(it);
}

/* rebuild the gst-graph */
static void pdgst__rebuild(t_pdgst*x) {
  t_atom ap[1];
  gst_element_set_state (x->x_pipeline, GST_STATE_NULL); // this can halt the system, don't know why yet...
  pdgst__remove_all_elements(x);

  pdgst__send_symbol(gensym("deregister"));
  pdgst__send_symbol(gensym("register"));

  SETSYMBOL(ap,  gensym("connect"));
  pdgst__send_(s_pdgst__gst_source, 1, ap);
}

static void pdgst__start(t_pdgst*x) 
{
  pdgst__rebuild(x);
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

static void pdgst__save(t_pdgst*x, t_symbol*file) {
  /* write the bin to a file */
  if(x->x_pipeline) {
    FILE*f=fopen (file->s_name, "w");
    if(f) {
      gst_xml_write_file (GST_ELEMENT (x->x_pipeline), f);
      fclose(f);
    } else {
      error("[pdgst] failed to write pipeline to '%s'", file->s_name);
    }
  }
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

  /* LATER: acquire a new pipeline for each [pdgst] */
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
  if(s_locale)post("pushing locale '%s'", s_locale);
  s_locale=setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
}

void pdgst_poplocale(void)
{
  if(!s_locale)post("popping empty locale");
  setlocale(LC_NUMERIC, s_locale);
  s_locale=NULL;
}


/* LATER move this into setup.c */
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

  class_addmethod  (pdgst_class, (t_method)pdgst__save, gensym("save"), A_SYMBOL, 0);

#ifdef PDGST_CAPSFILTER
  pdgst_capsfilter_setup();
#endif
  pdgst_objects_setup();
}

t_symbol*pdgst_privatesymbol(void) {
 return gensym("__gst");
}



/*
 * interesting stuff:
   gst_update_registry (void);
*/
