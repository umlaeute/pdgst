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


/* pdgst_element.c
 *    objectclass for gstreamer-elements
 *
 * derived from pdgst_base
 */


#include "pdgst/pdgst.h"
#include <string.h>


static t_class*pdgst_findclass(t_symbol*s);
static t_class*pdgst_addclass(t_symbol*s);


typedef struct _pdgst_element
{
  t_pdgst_base x_elem;

  t_pdgst_property*x_props;
} t_pdgst_element;


static void pdgst_element__any(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv) {
  if(!argc) {
    /* get */
    t_pdgst_property*prop=pdgst_getproperty(x->x_props, s);
    if(prop && prop->flags & G_PARAM_READABLE) {
      pdgst_base__getParam(&x->x_elem, prop);
    } else {
      pd_error(x, "[%s] no query method for '%s'", x->x_elem.x_name->s_name, s->s_name);
      return;
    }
  } else {
    /* set */
    t_pdgst_property*prop=pdgst_getproperty(x->x_props, s);
    if(prop && prop->flags & G_PARAM_WRITABLE) {
      pdgst_base__setParam(&x->x_elem, prop, argv);
      if(prop->flags & G_PARAM_READABLE)
        pdgst_base__getParam(&x->x_elem, prop);
    } else {
      pd_error(x, "[%s] no set method for '%s'", x->x_elem.x_name->s_name, s->s_name);
      return;
    }
  }
}

static void pdgst_element__seek (t_pdgst_element*x, t_float time)
{
  GstElement *element=x->x_element;
  guint64     time_ns=time*1e6;;
  GstEvent *event;

  event = gst_event_new_seek (1.0, 
			      GST_FORMAT_TIME,
			      GST_SEEK_FLAG_NONE,
			      GST_SEEK_TYPE_SET, time_ns,
			      GST_SEEK_TYPE_NONE, G_GUINT64_CONSTANT (0));
  post("seeking element %s to %f", x->x_elem.x_gstname->s_name, time);
  gst_element_send_event (element, event);
}


/* the destructor for elements/objectclasses */
static void pdgst_element__free(t_pdgst_element*x) {
  pdgst_base__free(&x->x_elem);
}

/* the constructor for elements/objectclasses */
void *pdgst_element__new(t_symbol*s, int argc, t_atom* argv) {
  GParamSpec **property_specs;
  GstElement*lmn=NULL;
  guint num_properties, i;

  t_pdgst_element*x=NULL; t_class*c=pdgst_findclass(s);
  if(!c)return NULL;
  pdgst_pushlocale();

  x=(t_pdgst_element*)pd_new(c);
  pdgst_base__new(&x->x_elem, s);
  lmn=x->x_element;
  if(NULL==lmn) {
    error("pdgst factory failed to create element...'%s'", s->s_name);
    pdgst_poplocale();
    return NULL;
  }

  /* property handling
   * we build a list of all properties and the expected type of their values 
   */
  x->x_props=NULL;
  property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
  for (i = 0; i < num_properties; i++) {
    x->x_props=pdgst_addproperty(x->x_props, property_specs[i]);
  }
  g_free (property_specs);

  pdgst_poplocale();
  return x;
}
















/* =============================================================================== *
 * PdGst - element loader 
 * =============================================================================== */

/* it returns 1 if <classname> appears to be a valid gst-element
 * and 0 otherwise
 */
int pdgst_element_setup_class(char*classname) {
  GstElementFactory *fac = gst_element_factory_find (classname);
  GstElement*lmn=NULL;
  GParamSpec **property_specs=NULL;
  guint num_properties=0, i=0;

  t_class*c=NULL;

  /* do we have a gst-factory for <classname> ? if this fails then <classname> is probably not a gst-element, so we return... */
  if(fac==NULL) {
    return 0;
  }

  /* check whether the factory will actually return a valid element; 
   * if not we won't be able to create objects for this later, so let's get out of here
   * we need this to query the  class-methods;
   */
  lmn=gst_element_factory_create(fac, NULL);
  if(lmn==NULL){
    return 0;
  }

  /* so now we are sure that there is an instantiable element, we cab safely create our objectclass */

  /* create a new objectclass for <classname> */
  c=pdgst_addclass(gensym(classname));
  /* and add the great default method for pdgst-interaction to it */
  class_addmethod  (c, (t_method)pdgst_base__gstMess, s_pdgst__gst, A_GIMME, 0);

#warning _info hack
  class_addmethod  (c, (t_method)pdgst_base__infoMess, gensym("_info"), A_GIMME, 0);


  /* some(?) elements support seek which is not exposed via properties and has to be distributed up/downward through the chain (CHECK) */
  class_addmethod  (c, (t_method)pdgst_element__seek, gensym("_seek"), A_FLOAT, 0);

  /* we want methods to set/get all properties of the element
   * the "get" method is just the property-name, e.g. [pattern(
   * the "set" method is just the property-name + the value(s), e.g. [pattern 1(
   */
  property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
  for (i = 0; i < num_properties; i++) {
    class_addmethod  (c, (t_method)pdgst_element__any, gensym(property_specs[i]->name), A_GIMME, 0);
  }
  g_free (property_specs);
  gst_object_unref (GST_OBJECT (lmn)); /* since we own lmn returned by gst_element_factory_create() */
  return 1;
}

/* class handling */
typedef struct _pdgst_classes {
  struct _pdgst_classes*next;
  t_symbol*name;
  t_class*class;
} t_pdgst_classes;
static t_pdgst_classes*pdgst_classes=NULL;

static t_class*pdgst_findclass(t_symbol*s)
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
static t_class*pdgst_addclass(t_symbol*s)
{
  t_class*c=pdgst_findclass(s);
  if(NULL==c) {
    /* an unheard-of class; create a real objectclass and store it for later use */
    t_pdgst_classes*cl0=pdgst_classes;
    t_pdgst_classes*cl=(t_pdgst_classes*)getbytes(sizeof(t_pdgst_classes));
    c = class_new(s,
                  (t_newmethod)pdgst_element__new,
                  (t_method)pdgst_element__free,
                  sizeof(t_pdgst_element),
                  0,
                  A_GIMME, 0);

    /* append the new class to our list of pdgst-classes */
    cl->next=NULL;
    cl->name=s;
    cl->class=c;
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
