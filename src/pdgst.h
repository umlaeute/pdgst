/* ********************************************** */
/* the Pd GST external                            */
/* ********************************************** */
/*                          forum::fuer::umlaeute */
/* ********************************************** */

/* Pd GST is a gstreamer-binding for Pure data
 *
 * relevant links:
 * Pd: http://puredata.info/
 * gstreamer: http://www.gstreamer.net/
 * IEM: http://iem.at
 */



#ifndef INCLUDE_PDGST_H__
#define INCLUDE_PDGST_H__

#ifdef __WIN32__
# ifndef NT
#  define NT
# endif
# ifndef MSW
#  define MSW
# endif
#endif

#ifdef _MSC_VER
# pragma warning( disable : 4018 )
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
# pragma warning( disable : 4996)  /* deprecated functions */
#endif

/*
 * to use the config.h compile-time configurations, you have to set HAVE_CONFIG_H
 * usually this is done in Make.config by configure
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "m_pd.h"
#include <gst/gst.h>

#if PD_MAJOR_VERSION == 0 && PD_MINOR_VERSION > 41
/* fine */
#else
# error need at least Pd 0.41
#endif /* PD_VERSION_CHECK */



typedef struct _pdgst_prop {
  struct _pdgst_prop*next;
  t_symbol*name;
  int flags;
  GType type;
} t_pdgst_property;

t_pdgst_property*pdgst_addproperty(t_pdgst_property*props, GParamSpec*param);
t_pdgst_property*pdgst_getproperty(t_pdgst_property*props, t_symbol*name);
void pdgst_killproperties(t_pdgst_property*props);

t_class*pdgst_findclass(t_symbol*s);
t_class*pdgst_addclass(t_symbol*s);


#define x_element x_elem.l_element
#define x_obj x_elem.l_obj
#define x_canvas x_elem.l_canvas

typedef void (*pdgst_buscallback_fun_t)(t_object*x, GstMessage*msg);

typedef struct _pdgst_elem
{
  t_object l_obj;
  t_canvas  *l_canvas;
  GstElement*l_element;
  t_method l_busCallback;

  t_outlet*x_infout;
  t_outlet*x_gstout;


  t_symbol*x_name;
  t_symbol*x_gstname;
} t_pdgst_elem;

void pdgst_elem__gstMess (t_pdgst_elem*x, t_symbol*s, int argc, t_atom*argv);
void pdgst_elem__setParam(t_pdgst_elem*x, t_pdgst_property*prop, t_atom*ap);
void pdgst_elem__getParam(t_pdgst_elem*x, t_pdgst_property*prop);
void pdgst_elem__free(t_pdgst_elem*x);
void pdgst_elem__new (t_pdgst_elem*x, t_symbol*s);

/* from gvalue.c */
t_atom*pdgst__gvalue2atom(const GValue*v, t_atom*a);
GValue*pdgst__atom2gvalue(const t_atom*a, GValue*v);


/* from pdgst_element.c */
void pdgst_element_setup(void);
int pdgst_element_setup_class(char*classname);

/* from pdgst_capsfilter.c */
void pdgst_capsfilter_setup(void);
int pdgst_capsfilter_setup_class(char*classname);

/* from pdgst.c */
t_symbol*pdgst_privatesymbol(void);
void pdgst_bin_add(t_pdgst_elem*element);
void pdgst_bin_remove(t_pdgst_elem*element);
GstBin*pdgst_get_bin(t_pdgst_elem*element);

/* from loop.c */
void pdgst_loop_setup(void);


/* from nowhere */
static char *pdgst_version = "$Revision: 0.0 $";


extern t_symbol*s_pdgst__gst;
extern t_symbol*s_pdgst__gst_source;
extern t_symbol*s_pdgst__gst_filter;
extern t_symbol*s_pdgst__gst_sink;

#endif /* INCLUDE_PDGST_H__ */
