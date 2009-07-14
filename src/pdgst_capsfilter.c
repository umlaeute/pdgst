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

/* pdgst_capsfilter.c
 *    objectclass for capsfilter-elements
 *
 * derived from pdgst_elem
 *
 * doesn't work yet: how do we find-out whether a certain class is actually a capsfilter?
 *
 *
 * note on inheritance:
 *   pdgst_element is quite complex, as it creates a real objectclass for each pdgst-element
 *   this is not necessary with capsfilters: they all belong to the same base-objectclass [pdgst_capsfilter]
 */


#warning add docs

#include "pdgst/pdgst.h"
#include <string.h>


static t_class*pdgst_capsfilter_class=NULL;
typedef struct _pdgst_capsfilter
{
  t_pdgst_base x_elem;
  GstCaps*x_caps;
} t_pdgst_capsfilter;


/* ================================================================== */
/* capsfilter, e.g. [audio/x-raw-float channels=10] */


static void pdgst_capsfilter__bang(t_pdgst_capsfilter*x) {
  GValue value = { 0, };
  gchar *caps_str=NULL;
  g_value_init (&value, GST_TYPE_CAPS);
  g_object_get_property(G_OBJECT (x->x_element), "caps", &value); 
  caps_str= gst_caps_to_string ( gst_value_get_caps ( &value));
  post("caps: %s", caps_str);
  g_free (caps_str);

}

static void pdgst_capsfilter__free(t_pdgst_capsfilter*x) {
  pdgst_base__free(&x->x_elem);
}

static void *pdgst_capsfilter__new(t_symbol*s, int argc, t_atom* argv) {
  t_pdgst_capsfilter*x=(t_pdgst_capsfilter*)pd_new(pdgst_capsfilter_class);
  GstStructure *struc= gst_structure_empty_new(s->s_name);

  pdgst_base__new(&x->x_elem, gensym("capsfilter"), NULL);
  if(NULL==x->x_element) {
    return NULL;
  }

  while(argc--) {
    const char*cap=atom_getsymbol(argv++)->s_name;

    char*value=index(cap, '='); 

    if(value) {
      int len=value-cap;
      char field[MAXPDSTRING];
      t_binbuf*bb=binbuf_new();
    
      if(len>=MAXPDSTRING-1)
        len=MAXPDSTRING-2;

      value++;
      snprintf(field, len+1, "%s", cap);

      binbuf_text(bb, value, strlen(value));
      if(binbuf_getnatom(bb)) {
        t_atom*a=binbuf_getvec(bb);
        GValue v={0, };
        
        if(pdgst__atom2gvalue(a, &v)) {
          gst_structure_set_value (struc, field, &v);
        } else {
          post("couldn't convert '%s' to GValue", value);
        }
        g_value_unset(&v);
      }
    } else {
      //      post("not setting: '%s'", cap);
    }    
    //    gst_caps_set_simple(x->x_caps, 
  }

  x->x_caps=gst_caps_new_full(struc, NULL);
  if(x->x_caps) {
    GValue v={0, };
    g_value_init (&v, GST_TYPE_CAPS);
    gst_value_set_caps(&v, x->x_caps);
    g_object_set_property(G_OBJECT (x->x_element), "caps", &v);
  }

  return x;
}


int pdgst_capsfilter_setup_class(char*classname)
{
  GstElement*lmn=NULL;
  GError*gerror=NULL;

  char dummypipeline[MAXPDSTRING];


  
  //  GstCaps*caps = gst_caps_new_simple (classname, NULL); //gst_caps_from_string (classname);


  return 0;

  /* could be a capsfilter:
   * "capsfilter caps=audio/x-raw-float" == "audio/x-raw-float"
   */

  /* this is always true, so not such a good idea... */
  snprintf(dummypipeline, MAXPDSTRING-1, "fakesrc ! capsfilter caps=%s ! fakesink", classname);

  lmn=gst_parse_launch(dummypipeline, &gerror);
  if(lmn){
    gst_object_unref (GST_OBJECT (lmn)); /* since we own lmn created by gst_parse_launch */

    if(gerror)return 0;

    class_addcreator((t_newmethod)pdgst_capsfilter__new, gensym(classname), A_GIMME, 0);
    return 1;
  } 
  return 0;
}


void pdgst_capsfilter_setup(void)
{
  pdgst_capsfilter_class=class_new(gensym("pdgst-capsfilter"), /* NULL is not a good classname; Pd will crash on generating error messages */
                                   (t_newmethod)pdgst_capsfilter__new,
                                   (t_method)pdgst_capsfilter__free,
                                   sizeof(t_pdgst_capsfilter),
                                   0 /* CLASS_NOINLET */,
                                   A_GIMME, 0);
  class_addmethod  (pdgst_capsfilter_class, (t_method)pdgst_base__gstMess, s_pdgst__gst, A_GIMME, 0);
  class_addbang  (pdgst_capsfilter_class, (t_method)pdgst_capsfilter__bang);
}
