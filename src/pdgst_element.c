#include "pdgst.h"
#include <string.h>

typedef struct _pdgst_element
{
  t_pdgst_elem x_elem;

  t_symbol*x_name;
  t_symbol*x_gstname;

  t_pdgst_property*x_props;

  t_outlet*x_infout;
  t_outlet*x_gstout;
} t_pdgst_element;

static t_symbol*s_gst=NULL;


static void pdgst_element__infoout(t_pdgst_element*x, int argc, t_atom*argv)
{
  outlet_anything(x->x_infout, gensym("info"), argc, argv);
}
static void pdgst_element__infoout_mess(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv)
{
  int ac=argc+1;
  t_atom*av=(t_atom*)getbytes(sizeof(t_atom)*ac);
  SETSYMBOL(av, s);
  memcpy(av+1, argv, (sizeof(t_atom)*argc));

  pdgst_element__infoout(x, ac, av);

  freebytes(av, sizeof(t_atom)*(ac));
  av=NULL;
  ac=0;
}


static void pdgst_outputparam(t_pdgst_element*x, t_symbol*name, t_atom*a)
{
  t_atom ap[3];
  SETSYMBOL(ap+0, gensym("property"));
  SETSYMBOL(ap+1, name);

  ap[2].a_type=a->a_type;
  ap[2].a_w=a->a_w;

  pdgst_element__infoout(x, 3, ap);
}

static t_atom*pdgst_gvalue2atom(const GValue*v, t_atom*a0)
{
  t_atom*a=a0;
  t_symbol*s=NULL;
  t_float f=0;
  gboolean bool_v;
  GValue destval = {0, };
  int success=1;
  if(NULL==a)
    a=(t_atom*)getbytes(sizeof(t_atom));

  g_value_init (&destval, G_TYPE_FLOAT);

  switch (G_VALUE_TYPE (v)) {
  case G_TYPE_STRING:
    {
      const gchar* str=g_value_get_string(v);
      if(NULL==str)
        s=&s_;
      else
        s=gensym(str);
      SETSYMBOL(a, s);
    }
    break;
  case G_TYPE_BOOLEAN:
    bool_v = g_value_get_boolean(v);
    f=(bool_v);
    SETFLOAT(a, f);
    break;
  case G_TYPE_ENUM:
    f = g_value_get_enum(v);
    SETFLOAT(a, f);
    break;
  default:
    if(g_value_transform(v, &destval)) {
      f=g_value_get_float(&destval);
      SETFLOAT(a, f);
    } else {
      if(G_VALUE_HOLDS_ENUM(v)) {
        gint enum_value = g_value_get_enum(v);
        f=enum_value;
        SETFLOAT(a, f);
      } else {
        success=0;
      }
    }
  }

  g_value_unset(&destval);

  if(success)
    return a;

  freebytes(a, sizeof(t_atom));
  return NULL;
}


/* should be "static t_atom*" */
static void pdgst_getparam(t_pdgst_element*x, t_pdgst_property*prop)
{
  GstElement*element=x->x_element;
  GValue value = { 0, };
  GValue *v=&value;
  GType t;
  t_atom a, *ap;

  g_value_init (v, prop->type);

  g_object_get_property(G_OBJECT (element), prop->name->s_name, v);
  ap=pdgst_gvalue2atom(v, &a);

  if(ap) {
    pdgst_outputparam(x, prop->name, ap);
  }
  if(v)
    g_value_unset(v);
}

static void pdgst_setparam(t_pdgst_element*x, t_pdgst_property*prop, t_atom*ap)
{
    GstElement*element=x->x_element;
    pd_error(x, "setting parameters not yet implemented...");
    return;

    switch(ap->a_type) {
    case A_FLOAT:
      g_object_set (G_OBJECT (element), prop->name->s_name, atom_getfloat(ap), NULL);
      break;
    case A_SYMBOL: default:
      g_object_set (G_OBJECT (element), prop->name->s_name, atom_getsymbol(ap)->s_name, NULL);
      break;
    }
}

static void pdgst_element__connect(t_pdgst_element*x, t_symbol*s) {
  //  post("need to connect %s to %s",  s->s_name, x->x_gstname->s_name);
  GstBin*bin=pdgst_get_bin(&x->x_elem);

  GstElement*src = gst_bin_get_by_name (bin, s->s_name);
  if(!src)return;

  gst_element_link(src,x->x_element);

  gst_object_unref (src);  
}

static void pdgst_element__connect_init(t_pdgst_element*x) {
  t_atom ap[2];
  SETSYMBOL(ap+0, gensym("connect"));
  SETSYMBOL(ap+1, x->x_gstname);

  outlet_anything(x->x_gstout, s_gst, 2, ap);
}







gboolean pdgst_element__message_element_foreach(GQuark field_id, const GValue *value, gpointer x0)
{
  t_pdgst_element*x=(t_pdgst_element*)x0;
  post("element %x has value %x", field_id, value);

  return TRUE;
}









#define CALLBACK_UNIMPLEMENTED(x)  pd_error(x, "[%s] unimplemented message in %s:%d", x->x_gstname->s_name, __FILE__, __LINE__);

static void pdgst_element__bus_callback(t_pdgst_element*x, GstMessage*message) {
  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_UNKNOWN:
    post("unknown message");
    break;
  case GST_MESSAGE_EOS: {
    /* end-of-stream */
    pdgst_element__infoout_mess(x, gensym("EOS"), 0, NULL);
  }
    break;
  case GST_MESSAGE_ERROR: {
    GError *err;
    gchar *debug;
    t_atom ap[2];

    gst_message_parse_error (message, &err, &debug);

    SETSYMBOL(ap+0, gensym(g_quark_to_string(err->domain)));
    SETFLOAT(ap+1, err->code);

    pd_error (x, "[%s]: %s", x->x_name->s_name, err->message);
    pd_error (x, "[%s]: %s", debug);
    g_error_free (err);
    g_free (debug);

    pdgst_element__infoout_mess(x, gensym("error"), 2, ap);
  }
    break;
#if GST_CHECK_VERSION(0, 10, 12)
  case GST_MESSAGE_WARNING: {
    GError *err;
    gchar *debug;
    t_atom ap[2];

    gst_message_parse_warning (message, &err, &debug);

    SETSYMBOL(ap+0, gensym(g_quark_to_string(err->domain)));
    SETFLOAT(ap+1, err->code);

    error ("[%s]: %s", x->x_name->s_name, err->message);
    error ("[%s]: %s", debug);
    g_error_free (err);
    g_free (debug);

    pdgst_element__infoout_mess(x, gensym("warning"), 2, ap);
  }
    break;
  case GST_MESSAGE_INFO: {
    GError *err;
    gchar *debug;
    t_atom ap[2];

    gst_message_parse_info (message, &err, &debug);

    SETSYMBOL(ap+0, gensym(g_quark_to_string(err->domain)));
    SETFLOAT(ap+1, err->code);

    post ("[%s]: %s", x->x_name->s_name, err->message);
    post ("[%s]: %s", debug);
    g_error_free (err);
    g_free (debug);

    pdgst_element__infoout_mess(x, gensym("info"), 2, ap);
  }
    break;
#endif /* gst-0.10.12 */
  case GST_MESSAGE_TAG: {
#if 0
    GstTagList*tag_list;
    gst_message_parse_tag               (message, &tag_list);
    /* LATER: call a help-function for each tag and output that 1-message-per-tag */
#endif
    CALLBACK_UNIMPLEMENTED(x) ;
  }
    break;
#if GST_CHECK_VERSION(0, 10, 11)
  case GST_MESSAGE_BUFFERING: {
    gint percent, avg_in, avg_out;
    gint64 buffering_left;
    GstBufferingMode mode;

    t_atom ap[4];

    gst_message_parse_buffering         (message, &percent);
    gst_message_parse_buffering_stats   (message, &mode, &avg_in, &avg_out, &buffering_left);

    SETFLOAT(ap, (t_float)percent);
    pdgst_element__infoout_mess(x, gensym("buffering"), 1, ap);
#if GST_CHECK_VERSION(0, 10, 20)
    SETFLOAT(ap+0, (t_float)mode);
    SETFLOAT(ap+1, (t_float)avg_in);
    SETFLOAT(ap+2, (t_float)avg_out);
    SETFLOAT(ap+3, (t_float)buffering_left);
    pdgst_element__infoout_mess(x, gensym("buffering_stats"), 4, ap);
  }
#endif /* gst-0.10.20 */
    break;
#endif /* gst-0.10.11 */
  case GST_MESSAGE_STATE_CHANGED: {
    GstState oldstate,  newstate,  pending;
    t_atom ap[3];
    gst_message_parse_state_changed     (message, &oldstate, &newstate, &pending);
    SETFLOAT(ap+0, (t_float)oldstate);
    SETFLOAT(ap+1, (t_float)newstate);
    SETFLOAT(ap+2, (t_float)pending);
    pdgst_element__infoout_mess(x, gensym("state"), 3, ap);
  }
    break;
  case GST_MESSAGE_STATE_DIRTY:
    /* deprecated, so ignore */
    pdgst_element__infoout_mess(x, gensym("state_dirty"), 0, NULL);
    break;
  case GST_MESSAGE_STEP_DONE:
    pdgst_element__infoout_mess(x, gensym("step_done"), 0, NULL);
    break;
    /* all the "clock" functions should be parsed */
  case GST_MESSAGE_CLOCK_PROVIDE:
    /* only used internally, should never be here... */
    pdgst_element__infoout_mess(x, gensym("clock_provide"), 0, NULL);
    break;
  case GST_MESSAGE_CLOCK_LOST:
    pdgst_element__infoout_mess(x, gensym("clock_lost"), 0, NULL);
    break;
  case GST_MESSAGE_NEW_CLOCK:
    pdgst_element__infoout_mess(x, gensym("clock_new"), 0, NULL);
    break;
#if GST_CHECK_VERSION(0, 10, 22)
  case GST_MESSAGE_STRUCTURE_CHANGE: {
    /* used internally by gstreamer; should never be forwarded to use */
    GstStructureChangeType type;
    GstElement *owner;
    gboolean busy;
    t_atom ap[3];
    gchar*name;

    gst_message_parse_structure_change  (message, &type, &owner, &busy);

    SETFLOAT(ap+0, type);

    g_object_get (G_OBJECT (owner), "name", &name, NULL);
    SETSYMBOL(ap+1, gensym(name));
    g_free (name);

    SETFLOAT(ap+2, busy);

    pdgst_element__infoout_mess(x, gensym("structure_change"), 3, ap);
  }
    break;
#endif /* gst-0.10.22 */
  case GST_MESSAGE_STREAM_STATUS:
    /* not implemented upstream */
    CALLBACK_UNIMPLEMENTED(x);
    break;
  case GST_MESSAGE_APPLICATION: {
    const GstStructure * structure = gst_message_get_structure(message);
    int index=0;
    t_atom ap[3];
    const gchar*structname= gst_structure_get_name(structure);
    SETSYMBOL(ap+0, gensym( structname) );

    for(index=0; index<gst_structure_n_fields(structure); index++) {
      const gchar*name=gst_structure_nth_field_name        (structure, index);
      const GValue*value= gst_structure_get_value(structure, name);
      SETSYMBOL(ap+1, gensym(name));
      if(pdgst_gvalue2atom(value, ap+2)) {
        pdgst_element__infoout_mess(x, gensym("application"), 3, ap);
      } else {
        pdgst_element__infoout_mess(x, gensym("application"), 2, ap);
      }
    }
  }
    break;
  case GST_MESSAGE_ELEMENT: {
    const GstStructure * structure = gst_message_get_structure(message);
    int index=0;
    t_atom ap[3];
    const gchar*structname= gst_structure_get_name(structure);
    SETSYMBOL(ap+0, gensym( structname) );

    for(index=0; index<gst_structure_n_fields(structure); index++) {
      const gchar*name=gst_structure_nth_field_name        (structure, index);
      const GValue*value= gst_structure_get_value(structure, name);
      SETSYMBOL(ap+1, gensym(name));
      if(pdgst_gvalue2atom(value, ap+2)) {
        pdgst_element__infoout_mess(x, gensym("element"), 3, ap);
      } else {
        pdgst_element__infoout_mess(x, gensym("element"), 2, ap);
      }
    }
  }
    break;
  case GST_MESSAGE_SEGMENT_START: {
    GstFormat format;
    gint64 position;
    t_atom ap[2];

    gst_message_parse_segment_start     (message, &format, &position);

    SETFLOAT(ap+0, (t_float)format);
    SETFLOAT(ap+1, (t_float)position);
    pdgst_element__infoout_mess(x, gensym("segment_start"), 2, ap);
  }
    break;
  case GST_MESSAGE_SEGMENT_DONE: {
    GstFormat format;
    gint64 position;
    t_atom ap[2];

    gst_message_parse_segment_done     (message, &format, &position);

    SETFLOAT(ap+0, (t_float)format);
    SETFLOAT(ap+1, (t_float)position);
    pdgst_element__infoout_mess(x, gensym("segment_done"), 2, ap);
  }

    break;
  case GST_MESSAGE_DURATION: {
    GstFormat format;
    gint64 position;
    t_atom ap[2];

    gst_message_parse_duration     (message, &format, &position);

    SETFLOAT(ap+0, (t_float)format);
    SETFLOAT(ap+1, (t_float)position);
    pdgst_element__infoout_mess(x, gensym("duration"), 2, ap);
  }
    break;
  case GST_MESSAGE_LATENCY:
    break;
#if GST_CHECK_VERSION(0, 10, 13)
  case GST_MESSAGE_ASYNC_START: {
    gboolean new_base_time;
    t_atom ap[1];

    gst_message_parse_async_start (message, &new_base_time);

    SETFLOAT(ap+0, (new_base_time?1.:0.));
    pdgst_element__infoout_mess(x, gensym("async_start"), 1, ap);
  }
    break;
  case GST_MESSAGE_ASYNC_DONE:
    pdgst_element__infoout_mess(x, gensym("async_done"), 0, NULL);
    break;
#endif /* gst-0.10.13 */
  default:
    post("hmm, unknown message of type '%s'", GST_MESSAGE_TYPE_NAME(message));
    break;
  }




}

static void pdgst_element__register(t_pdgst_element*x) {
 pdgst_bin_add((t_pdgst_elem*)x);
}

static void pdgst_element__deregister(t_pdgst_element*x) {
  pdgst_bin_remove((t_pdgst_elem*)x);
}


static void pdgst_element__gstMess(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv) {
  t_symbol*selector=NULL;
  if(!argc || !(A_SYMBOL==argv->a_type))
    return;
  selector=atom_getsymbol(argv);
  argv++; argc--;
  if(gensym("register")==selector) {
    pdgst_element__register(x);
  } else if(gensym("deregister")==selector) {
    pdgst_element__deregister(x);
  } else if(gensym("connect")==selector) {
    if(argc) {
      t_symbol*source=atom_getsymbol(argv);
      pdgst_element__connect(x, source);
    } else {
      pdgst_element__connect_init(x);
    }
  }
  else
    post("_gst: %s", selector->s_name);


}

static void pdgst_element__any(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv) {
  if(!argc) {
    /* get */
    t_pdgst_property*prop=pdgst_getproperty(x->x_props, s);
    if(prop && prop->flags & G_PARAM_READABLE) {
      pdgst_getparam(x, prop);
    } else {
      pd_error(x, "[%s] no query method for '%s'", x->x_name->s_name, s->s_name);
      return;
    }
  } else {
    /* set */
    t_pdgst_property*prop=pdgst_getproperty(x->x_props, s);
    if(prop && prop->flags & G_PARAM_WRITABLE) {
      pdgst_setparam(x, prop, argv);
    } else {
      pd_error(x, "[%s] no set method for '%s'", x->x_name->s_name, s->s_name);
      return;
    }
  }
}

static void pdgst_element__free(t_pdgst_element*x) {
  pdgst_element__deregister(x);

  if(x->x_element) {
    post("unreffing element");
    gst_object_unref (GST_OBJECT (x->x_element));
    post("unreffed element");
  }
  x->x_element=NULL;

  if(x->x_gstout && x->x_gstout!=x->x_infout)
    outlet_free(x->x_gstout);

  if(x->x_infout)
    outlet_free(x->x_infout);

  pd_unbind(&x->x_elem.l_obj.ob_pd, s_gst);
  
}

static void *pdgst_element__new(t_symbol*s, int argc, t_atom* argv) {
  GParamSpec **property_specs;
  GstElement*lmn=NULL;
  guint num_properties, i;
  gchar *name;


  t_pdgst_element*x=NULL;
  t_class*c=pdgst_findclass(s);

  if(!c)return NULL;

  lmn=gst_element_factory_make(s->s_name, NULL);
  if(NULL==lmn) {
    post("gst factory failed to create element...'%s'", s->s_name);
    return NULL;
  }

  x=(t_pdgst_element*)pd_new(c);
  x->x_infout=outlet_new(&x->x_obj, 0);

  x->x_gstout=x->x_infout; /* for now, re-use the info-out; LATER create our own outlet */

  x->x_name=s;  
  x->x_element=lmn;
  x->x_canvas=NULL;
  x->x_props=NULL;
  x->x_elem.l_busCallback=(t_method)pdgst_element__bus_callback;

  /* get name */
  g_object_get (G_OBJECT (lmn), "name", &name, NULL);
  x->x_gstname=gensym(name);
  g_free (name);

  property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
  for (i = 0; i < num_properties; i++) {
    x->x_props=pdgst_addproperty(x->x_props, property_specs[i]);
  }
  g_free (property_specs);

  pdgst_bin_add(&x->x_elem);

  pd_bind(&x->x_elem.l_obj.ob_pd, s_gst);
  
  return x;
}

int pdgst_element_setup_class(char*classname) {
  GstElementFactory *fac = gst_element_factory_find (classname);
  GstElement*lmn=NULL;
  GParamSpec **property_specs=NULL;
  guint num_properties=0, i=0;

  t_class*c=NULL;

  if(!s_gst)
    s_gst=pdgst_privatesymbol();

  if(fac==NULL) {
    return 0;
  }

  lmn=gst_element_factory_create(fac, NULL);
  if(lmn==NULL){
    return 0;
  }

  c=pdgst_addclass(gensym(classname));
  class_addmethod  (c, (t_method)pdgst_element__gstMess, s_gst, A_GIMME, 0);

  property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
  for (i = 0; i < num_properties; i++) {
    class_addmethod  (c, (t_method)pdgst_element__any, gensym(property_specs[i]->name), A_GIMME, 0);
  }
  g_free (property_specs);
  gst_object_unref (GST_OBJECT (lmn));
  return 1;
}












/* =============================================================================== */
/* class handling */


typedef struct _pdgst_classes {
  struct _pdgst_classes*next;
  t_symbol*name;
  t_class*class;
} t_pdgst_classes;
static t_pdgst_classes*pdgst_classes=NULL;

t_class*pdgst_findclass(t_symbol*s)
{
  t_pdgst_classes*cl=pdgst_classes;
  while(cl) {
    if(s==cl->name) {
      return cl->class; 
    }
    cl=cl->next;
  }
  return NULL;
}
t_class*pdgst_addclass(t_symbol*s)
{
  t_class*c=pdgst_findclass(s);
  if(NULL==c) {
    t_pdgst_classes*cl0=pdgst_classes;
    t_pdgst_classes*cl=(t_pdgst_classes*)getbytes(sizeof(t_pdgst_classes));
    c = class_new(s,
                  (t_newmethod)pdgst_element__new,
                  (t_method)pdgst_element__free,
                  sizeof(t_pdgst_element),
                  0,
                  A_GIMME, 0);

    cl->next=NULL;
    cl->name=s;
    cl->class=c;

    /* seeking to the end of our classlist */
    while(cl0 && cl0->next) {
      cl0=cl0->next;
    }
    if(cl0) {
      cl0->next=cl;
    } else {
      pdgst_classes=cl;
    }
  }
  return c;  
}
