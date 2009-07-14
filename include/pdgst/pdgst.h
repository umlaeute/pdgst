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
 ********************************************************
 *
 * relevant links:
 *  Pd: http://puredata.info/
 *  gstreamer: http://www.gstreamer.net/
 *  IEM: http://iem.at
 */

#ifndef INCLUDE_PDGST_H__
#define INCLUDE_PDGST_H__

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif



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


#include "pdgst/properties.h"

#define x_element x_elem.l_element
#define x_obj x_elem.l_obj
#define x_canvas x_elem.l_canvas

typedef void (*pdgst_buscallback_fun_t)(t_object*x, GstMessage*msg);

/* gvalue.c */
t_atom*pdgst__gvalue2atom(const GValue*v, t_atom*a);
GValue*pdgst__atom2gvalue(const t_atom*a, GValue*v);

#include "pdgst/element.h"

/* pdgst.c */
t_symbol*pdgst_privatesymbol(void);
void pdgst_bin_add(t_pdgst_base*element);
void pdgst_bin_remove(t_pdgst_base*element);
GstBin*pdgst_get_bin(t_pdgst_base*element);

void pdgst_pushlocale(void);
void pdgst_poplocale(void);
 

/* from loop.c */
void pdgst_loop_setup(void);
void pdgst_loop_flush(void);


/* from nowhere */
static const char *pdgst_version = "$Revision: 0.0 $";


extern t_symbol*s_pdgst__gst;
extern t_symbol*s_pdgst__gst_source;
extern t_symbol*s_pdgst__gst_filter;
extern t_symbol*s_pdgst__gst_sink;

/* objectclasses setup */
void pdgst_objects_setup(void);



#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif
#endif /* INCLUDE_PDGST_H__ */
