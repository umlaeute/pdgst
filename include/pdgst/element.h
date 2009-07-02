
/* ****************************************************** */
/* the Pd GST external - GStreamer bindings for Pure data */
/******************************************************** */
/*
 * copyleft (c) 2009 IOhannes m zm�lnig
 *
 *   forum::f�r::uml�ute
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

/* all objectclasses are of pdgst_elem
 *
 * child classes:
 *   pdgst_element: objectclass for a normal gstreamer-element
 *   pdgst_capsfilter: objectclass for a gstreamer-capsfilter
 */


/* pdgst_elem.c */
typedef struct _pdgst_base
{
  t_object l_obj;
  t_canvas  *l_canvas;
  GstElement*l_element;
  t_method l_busCallback;

  t_outlet*x_infout;
  t_outlet*x_gstout;


  t_symbol*x_name;
  t_symbol*x_gstname;
} t_pdgst_base;

void pdgst_base__gstMess (t_pdgst_base*x, t_symbol*s, int argc, t_atom*argv);
void pdgst_base__setParam(t_pdgst_base*x, t_pdgst_property*prop, t_atom*ap);
void pdgst_base__getParam(t_pdgst_base*x, t_pdgst_property*prop);
void pdgst_base__free(t_pdgst_base*x);
void pdgst_base__new (t_pdgst_base*x, t_symbol*s);


/* pdgst_element.c */
void pdgst_element_setup(void);
int pdgst_element_setup_class(char*classname);

/* pdgst_capsfilter.c */
void pdgst_capsfilter_setup(void);
int pdgst_capsfilter_setup_class(char*classname);





