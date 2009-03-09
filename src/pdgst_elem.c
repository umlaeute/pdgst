#include "pdgst.h"
#include <string.h>

static void pdgst_elem__gstout(t_pdgst_elem*x, int argc, t_atom*argv)
{
  outlet_anything(x->x_gstout, s_pdgst__gst, argc, argv);
}
static void pdgst_elem__gstout_mess(t_pdgst_elem*x, t_symbol*s, int argc, t_atom*argv)
{
  int ac=argc+1;
  t_atom*av=(t_atom*)getbytes(sizeof(t_atom)*ac);
  SETSYMBOL(av, s);
  memcpy(av+1, argv, (sizeof(t_atom)*argc));

  pdgst_elem__gstout(x, ac, av);

  freebytes(av, sizeof(t_atom)*(ac));
  av=NULL;
  ac=0;
}

static void pdgst_elem__connect_init(t_pdgst_elem*x) {
  t_atom ap[1];
  SETSYMBOL(ap, x->x_gstname);

  pdgst_elem__gstout_mess(x, gensym("connect"), 1, ap);
}

static void pdgst_elem__connect(t_pdgst_elem*x, t_symbol*s, t_symbol*spad) {
  gboolean res=FALSE;
  //  post("need to connect %s to %s",  s->s_name, x->x_gstname->s_name);
  GstBin*bin=pdgst_get_bin(x);
  gchar*srcpad=NULL;

  GstElement*src = gst_bin_get_by_name (bin, s->s_name);
  if(!src) {
    post("couldn't find element '%s'", s->s_name);
    return;
  }

  if(spad)
    srcpad=spad->s_name;

  //  post("linking '%s' to '%s'", s->s_name, x->x_gstname->s_name);
  res=gst_element_link_pads(src, NULL, x->l_element, NULL);
  //  post("linking %s successfull", (res?"was":"was NOT"));

  gst_object_unref (src);  

  pdgst_elem__connect_init(x);
}

static void pdgst_elem__register(t_pdgst_elem*x) {
 pdgst_bin_add((t_pdgst_elem*)x);
}

static void pdgst_elem__deregister(t_pdgst_elem*x) {
  pdgst_bin_remove((t_pdgst_elem*)x);
}

void pdgst_elem__gstMess(t_pdgst_elem*x, t_symbol*s, int argc, t_atom*argv) {
  t_symbol*selector=NULL;
  if(!argc || !(A_SYMBOL==argv->a_type))
    return;
  selector=atom_getsymbol(argv);
  argv++; argc--;
  if(gensym("register")==selector) {
    pdgst_elem__register(x);
  } else if(gensym("deregister")==selector) {
    pdgst_elem__deregister(x);
  } else if(gensym("connect")==selector) {
    if(argc) {
      t_symbol*source=atom_getsymbol(argv);
      t_symbol*sourcepad=NULL;

      if(argc>1)
        sourcepad=atom_getsymbol(argv+1);

      pdgst_elem__connect(x, source, sourcepad);
    } else {
      pdgst_elem__connect_init(x);
    }
  }
  else
    post("_gst: %s", selector->s_name);
}

static void pdgst_elem__infoout(t_pdgst_elem*x, int argc, t_atom*argv)
{
  outlet_anything(x->x_infout, gensym("info"), argc, argv);
}
static void pdgst_elem__infoout_mess(t_pdgst_elem*x, t_symbol*s, int argc, t_atom*argv)
{
  int ac=argc+1;
  t_atom*av=(t_atom*)getbytes(sizeof(t_atom)*ac);
  SETSYMBOL(av, s);
  memcpy(av+1, argv, (sizeof(t_atom)*argc));

  pdgst_elem__infoout(x, ac, av);

  freebytes(av, sizeof(t_atom)*(ac));
  av=NULL;
  ac=0;
}


static void pdgst_outputparam(t_pdgst_elem*x, t_symbol*name, t_atom*a)
{
  t_atom ap[3];
  SETSYMBOL(ap+0, gensym("property"));
  SETSYMBOL(ap+1, name);

  ap[2].a_type=a->a_type;
  ap[2].a_w=a->a_w;

  pdgst_elem__infoout(x, 3, ap);
}

/* should be "static t_atom*" */
void pdgst_elem__getParam(t_pdgst_elem*x, t_pdgst_property*prop)
{
  GstElement*element=x->l_element;
  GValue value = { 0, };
  GValue *v=&value;
  GType t;
  t_atom a, *ap;

  g_value_init (v, prop->type);

  g_object_get_property(G_OBJECT (element), prop->name->s_name, v);
  ap=pdgst__gvalue2atom(v, &a);

  if(ap) {
    pdgst_outputparam(x, prop->name, ap);
  }
  if(v)
    g_value_unset(v);
}

void pdgst_elem__setParam(t_pdgst_elem*x, t_pdgst_property*prop, t_atom*ap)
{
    GstElement*element=x->l_element;
    GValue v = { 0, };
    g_value_init (&v, prop->type);
    if(pdgst__atom2gvalue(ap, &v)) {
      g_object_set_property(G_OBJECT (element), prop->name->s_name, &v);
    } else {
      pd_error(x, "hmm, couldn't create GValue from atom");
    }

    //    pd_error(x, "setting parameters not yet implemented...");
    return;
}

static void pdgst_elem__taglist_foreach(const GstTagList *list, const gchar *tag, gpointer x0)
{
  t_pdgst_elem*x=(t_pdgst_elem*)x0;
  int index=0;
  const GValue*v=NULL;
  while(v=gst_tag_list_get_value_index(list, tag, index)) {
    t_atom ap[3];

    SETSYMBOL(ap+0, gensym("tag"));
    SETSYMBOL(ap+1, gensym(tag));

    if(pdgst__gvalue2atom(v, ap+2)) {
      pdgst_elem__infoout(x, 3, ap);
    } else {
      pdgst_elem__infoout(x, 2, ap);
    }
    index++;
  }
}

#define CALLBACK_UNIMPLEMENTED(x)  pd_error(x, "[%s] unimplemented message in %s:%d", x->x_gstname->s_name, __FILE__, __LINE__);

static void pdgst_elem__bus_callback(t_pdgst_elem*x, GstMessage*message) {
  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_UNKNOWN:
    post("unknown message");
    break;
  case GST_MESSAGE_EOS: {
    /* end-of-stream */
    pdgst_elem__infoout_mess(x, gensym("EOS"), 0, NULL);
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
    pd_error (x, "[%s]: %s", x->x_name->s_name, debug);
    g_error_free (err);
    g_free (debug);

    pdgst_elem__infoout_mess(x, gensym("error"), 2, ap);
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
    error ("[%s]: %s", x->x_name->s_name, debug);
    g_error_free (err);
    g_free (debug);

    pdgst_elem__infoout_mess(x, gensym("warning"), 2, ap);
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
    post ("[%s]: %s", x->x_name->s_name, debug);
    g_error_free (err);
    g_free (debug);

    pdgst_elem__infoout_mess(x, gensym("info"), 2, ap);
  }
    break;
#endif /* gst-0.10.12 */
  case GST_MESSAGE_TAG: {
    GstTagList*tag_list;
    gst_message_parse_tag               (message, &tag_list);
    gst_tag_list_foreach(tag_list, pdgst_elem__taglist_foreach, x);
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
    pdgst_elem__infoout_mess(x, gensym("buffering"), 1, ap);
#if GST_CHECK_VERSION(0, 10, 20)
    SETFLOAT(ap+0, (t_float)mode);
    SETFLOAT(ap+1, (t_float)avg_in);
    SETFLOAT(ap+2, (t_float)avg_out);
    SETFLOAT(ap+3, (t_float)buffering_left);
    pdgst_elem__infoout_mess(x, gensym("buffering_stats"), 4, ap);
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
    pdgst_elem__infoout_mess(x, gensym("state"), 3, ap);
  }
    break;
  case GST_MESSAGE_STATE_DIRTY:
    /* deprecated, so ignore */
    pdgst_elem__infoout_mess(x, gensym("state_dirty"), 0, NULL);
    break;
  case GST_MESSAGE_STEP_DONE:
    pdgst_elem__infoout_mess(x, gensym("step_done"), 0, NULL);
    break;
    /* all the "clock" functions should be parsed */
  case GST_MESSAGE_CLOCK_PROVIDE:
    /* only used internally, should never be here... */
    pdgst_elem__infoout_mess(x, gensym("clock_provide"), 0, NULL);
    break;
  case GST_MESSAGE_CLOCK_LOST:
    pdgst_elem__infoout_mess(x, gensym("clock_lost"), 0, NULL);
    break;
  case GST_MESSAGE_NEW_CLOCK:
    pdgst_elem__infoout_mess(x, gensym("clock_new"), 0, NULL);
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

    pdgst_elem__infoout_mess(x, gensym("structure_change"), 3, ap);
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
      if(pdgst__gvalue2atom(value, ap+2)) {
        pdgst_elem__infoout_mess(x, gensym("application"), 3, ap);
      } else {
        pdgst_elem__infoout_mess(x, gensym("application"), 2, ap);
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
      if(pdgst__gvalue2atom(value, ap+2)) {
        pdgst_elem__infoout_mess(x, gensym("element"), 3, ap);
      } else {
        pdgst_elem__infoout_mess(x, gensym("element"), 2, ap);
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
    pdgst_elem__infoout_mess(x, gensym("segment_start"), 2, ap);
  }
    break;
  case GST_MESSAGE_SEGMENT_DONE: {
    GstFormat format;
    gint64 position;
    t_atom ap[2];

    gst_message_parse_segment_done     (message, &format, &position);

    SETFLOAT(ap+0, (t_float)format);
    SETFLOAT(ap+1, (t_float)position);
    pdgst_elem__infoout_mess(x, gensym("segment_done"), 2, ap);
  }

    break;
  case GST_MESSAGE_DURATION: {
    GstFormat format;
    gint64 position;
    t_atom ap[2];

    gst_message_parse_duration     (message, &format, &position);

    SETFLOAT(ap+0, (t_float)format);
    SETFLOAT(ap+1, (t_float)position);
    pdgst_elem__infoout_mess(x, gensym("duration"), 2, ap);
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
    pdgst_elem__infoout_mess(x, gensym("async_start"), 1, ap);
  }
    break;
  case GST_MESSAGE_ASYNC_DONE:
    pdgst_elem__infoout_mess(x, gensym("async_done"), 0, NULL);
    break;
#endif /* gst-0.10.13 */
  default:
    post("hmm, unknown message of type '%s'", GST_MESSAGE_TYPE_NAME(message));
    break;
  }
}


static void pdgst_elem__padcb_added (GstElement *element, GstPad     *pad, t_pdgst_elem*x)
{
  switch (gst_pad_get_direction(pad)) {
 case GST_PAD_SRC: {
    t_atom ap[2];
    gchar*name=gst_pad_get_name(pad);
    SETSYMBOL(ap+0, x->x_gstname);
    SETSYMBOL(ap+1, gensym(name));
    g_free(name);

    pdgst_elem__gstout_mess(x, gensym("connect"), 2, ap);  
 }
   break;
  case GST_PAD_SINK:
    break;
  default:
    pd_error(x, "[%s] added pad with unknown direction...");
  }
}
static void pdgst_elem__padcb_removed (GstElement *element, GstPad     *pad, t_pdgst_elem*x)
{
  switch (gst_pad_get_direction(pad)) {
 case GST_PAD_SRC: {
  t_atom ap[2];
  gchar*name=gst_pad_get_name(pad);
  SETSYMBOL(ap+0, x->x_gstname);
  SETSYMBOL(ap+1, gensym(name));
  g_free(name);

  pdgst_elem__gstout_mess(x, gensym("disconnect"), 2, ap);  
 }
   break;
  case GST_PAD_SINK:
    break;
  default:
    pd_error(x, "[%s] removed pad with unknown direction...");
  }
}
static void pdgst_elem__padcb_nomore(GstElement *element, t_pdgst_elem*x)
{
  //  post("[%s] no more pads", x->x_name->s_name);
}

static void pdgst_elem__add_signals(t_pdgst_elem*x) {
  g_signal_connect (x->l_element, "pad-added", G_CALLBACK(pdgst_elem__padcb_added), x);
  g_signal_connect (x->l_element, "pad-removed", G_CALLBACK(pdgst_elem__padcb_removed), x);
  g_signal_connect (x->l_element, "no-more-pads", G_CALLBACK(pdgst_elem__padcb_nomore), x);
  //  LATER also remove signals(?)
}



void pdgst_elem__free(t_pdgst_elem*x)
{
  GstElement*lmn=x->l_element;
  pd_unbind(&x->l_obj.ob_pd, s_pdgst__gst);

  if(lmn->numsrcpads && lmn->numsinkpads) {
    pd_unbind(&x->l_obj.ob_pd, s_pdgst__gst_filter);
  } else if (lmn->numsrcpads) {
    pd_unbind(&x->l_obj.ob_pd, s_pdgst__gst_source);
  } else if (lmn->numsrcpads) {
    pd_unbind(&x->l_obj.ob_pd, s_pdgst__gst_sink);
  }  

  pdgst_elem__deregister(x);

  if(x->l_element) {
    post("unreffing element");
    gst_object_unref (GST_OBJECT (x->l_element));
    post("unreffed element");
  }
  x->l_element=NULL;

  if(x->x_gstout && x->x_gstout!=x->x_infout)
    outlet_free(x->x_gstout);
  x->x_gstout=NULL;

  if(x->x_infout)
    outlet_free(x->x_infout);
  x->x_infout=NULL;
}


void pdgst_elem__new(t_pdgst_elem*x, t_symbol*s)
{
  gchar *name=NULL;

  GstElement*lmn=gst_element_factory_make(s->s_name, NULL);

  x->l_element=lmn;
  if(NULL==lmn) return;

  x->x_infout=outlet_new(&x->l_obj, 0);
  x->x_gstout=x->x_infout; /* for now, re-use the info-out; LATER create our own outlet */
  x->l_canvas=NULL;

  x->x_name=s;
  /* get name */
  g_object_get (G_OBJECT (lmn), "name", &name, NULL);
  x->x_gstname=gensym(name);
  g_free (name);

  x->l_busCallback=(t_method)pdgst_elem__bus_callback;

  pdgst_bin_add(x);
  pdgst_elem__add_signals(x);

  pd_bind(&x->l_obj.ob_pd, s_pdgst__gst);

  if((lmn->numsrcpads > 0) && (lmn->numsinkpads > 0)) {
    pd_bind(&x->l_obj.ob_pd, s_pdgst__gst_filter);
  } else if (lmn->numsrcpads > 0) {
    pd_bind(&x->l_obj.ob_pd, s_pdgst__gst_source);
  } else if (lmn->numsinkpads > 0) {
    pd_bind(&x->l_obj.ob_pd, s_pdgst__gst_sink);
  } else {
    pd_error(x, "[%s]: hmm, element without pads", x->x_name->s_name);
  }
}