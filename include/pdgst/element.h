
/* ****************************************************** */
/* the Pd GST external - GStreamer bindings for Pure data */
/******************************************************** */
/*
 * copyleft (c) 2009 IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *   university of music and performing arts
 *
 *
 ********************************************************
 *
 * license: GNU General Public License v.2 or later
 *
 ********************************************************/

/* all objectclasses are child-classes of pdgst_base
 *
 * child classes:
 *   pdgst_element: objectclass for a normal gstreamer-element
 *   pdgst_capsfilter: objectclass for a gstreamer-capsfilter
 */


/* ======================================================== *
 * base class: pdgst_base
 * ======================================================== */

/* pdgst_base.c */
typedef struct _pdgst_base
{
  t_object l_obj;
  t_canvas  *l_canvas;
  GstElement*l_element;

  gulong   l_sighandler_bin; /* bus callback for bin */
  gulong   l_sighandler_pad_add;  /* pad-added */
  gulong   l_sighandler_pad_del;  /* pad-removed */
  gulong   l_sighandler_pad_done; /* pad-nomore */

  t_outlet*x_infout;
  t_outlet*x_gstout;

  t_symbol*x_name;
  t_symbol*x_gstname;

  t_pd*x_bindobject;
} t_pdgst_base;

t_symbol*pdgst_base__bindsym(t_pdgst_base*x);

/* messages */
void pdgst_base__gstMess (t_pdgst_base*x, t_symbol*s, int argc, t_atom*argv);
void pdgst_base__setParam(t_pdgst_base*x, t_pdgst_property*prop, t_atom*ap);
void pdgst_base__getParam(t_pdgst_base*x, t_pdgst_property*prop);
void pdgst_base__infoMess (t_pdgst_base*x, t_symbol*s, int argc, t_atom*argv); /* hack to ignore "_info" */

void pdgst_base__buscallback(GstBus*bus,GstMessage*msg,t_pdgst_base*x);

/* constructor/destructor */
void pdgst_base__free(t_pdgst_base*x);
void pdgst_base__new (t_pdgst_base*x, t_symbol*s, t_pd*bindobject);

/* -------------------------------------------------------- *
 * child class: pdgst_element
 * -------------------------------------------------------- */
/* pdgst_element.c */
void pdgst_element_setup(void);
int pdgst_element_setup_class(char*classname);

/* -------------------------------------------------------- *
 * child class: pdgst_capsfilter
 * -------------------------------------------------------- */
/* pdgst_capsfilter.c */
void pdgst_capsfilter_setup(void);
int pdgst_capsfilter_setup_class(char*classname);





