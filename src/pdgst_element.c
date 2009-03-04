#include "pdgst.h"

typedef struct _pdgst_element
{
  t_object x_obj;
  t_symbol*x_name;

  GstElement*x_element;
  t_pdgst_property*x_props;

  t_outlet*x_infout;
} t_pdgst_element;

static void *pdgst_element_new(t_symbol*s, int argc, t_atom* argv);
static void pdgst_element_free(t_pdgst_element*x);

static void pdgst_element_any(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv);


static void pdgst_outputparam(t_pdgst_element*x, t_symbol*name, t_atom*a)
{
  t_atom ap[3];
  SETSYMBOL(ap+0, gensym("property"));
  SETSYMBOL(ap+1, name);

  ap[2].a_type=a->a_type;
  ap[2].a_w=a->a_w;

  outlet_anything(x->x_infout, gensym("info"), 3, ap);
}

/* should be "static t_atom*" */
static void pdgst_getparam(t_pdgst_element*x, t_pdgst_property*prop)
{
  GstElement*element=x->x_element;
  GValue value = { 0, };
  GValue destval = {0, };

  GValue *v=&value;
  GType t;

  t_symbol*s0;
  t_float f;
  t_atom a;
  gboolean bool_v;
  int success=1;

  g_value_init (v, prop->type);
  g_value_init (&destval, G_TYPE_FLOAT);

  g_object_get_property(G_OBJECT (element), prop->name->s_name, v);

  switch (G_VALUE_TYPE (v)) {
  case G_TYPE_STRING:
    {
      const gchar* str=g_value_get_string(v);
      if(NULL==str)
        s0=&s_;
      else
        s0=gensym(str);
      SETSYMBOL(&a, s0);
    }
    break;
  case G_TYPE_BOOLEAN:
    bool_v = g_value_get_boolean(v);
    f=(bool_v);
    SETFLOAT(&a, f);
    break;
  case G_TYPE_ENUM:
    f = g_value_get_enum(v);
    SETFLOAT(&a, f);
    break;
  default:
    if(g_value_transform(v, &destval)) {
      f=g_value_get_float(&destval);
      SETFLOAT(&a, f);
    } else {
      if(G_VALUE_HOLDS_ENUM(v)) {
        gint enum_value = g_value_get_enum(v);
        f=enum_value;
        SETFLOAT(&a, f);
      } else {
        pd_error(x, "[%s] don't know what to do with param '%s': %d", x->x_name->s_name, prop->name->s_name, G_VALUE_TYPE (v));
        success=0;
      }
    }
  }
  if(success) {
    pdgst_outputparam(x, prop->name, &a);
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


static void pdgst_element_gstMess(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv) {
  post("_gst");
}

static void pdgst_element_any(t_pdgst_element*x, t_symbol*s, int argc, t_atom*argv) {
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

static void pdgst_element_free(t_pdgst_element*x) {
  if(x->x_element) {
    gst_object_unref (GST_OBJECT (x->x_element));
  }
  x->x_element=NULL;

  if(x->x_infout)
    outlet_free(x->x_infout);
}

static void *pdgst_element_new(t_symbol*s, int argc, t_atom* argv) {
  t_pdgst_element*x=NULL;
  t_class*c=pdgst_findclass(s);

  if(c) {
    GParamSpec **property_specs;
    guint num_properties, i;

    GstElement*lmn=gst_element_factory_make(s->s_name, NULL);
    if(NULL==lmn) {
      post("factory failed to create element...'%s'", s->s_name);
      return NULL;
    }
    x=(t_pdgst_element*)pd_new(c);
    x->x_infout=outlet_new(&x->x_obj, 0);
    x->x_name=s;

    x->x_element=lmn;
    x->x_props=NULL;

    property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
    for (i = 0; i < num_properties; i++) {
      x->x_props=pdgst_addproperty(x->x_props, property_specs[i]);
    }
    g_free (property_specs);
    
  } else {
    /* should never happen */
  }

  return x;
}

int pdgst_element_setup_class(char*classname) {
  GstElementFactory *fac = gst_element_factory_find (classname);
  GstElement*lmn=NULL;
  GParamSpec **property_specs=NULL;
  guint num_properties=0, i=0;

  t_class*c=NULL;

  if(fac==NULL) {
    return 0;
  }

  lmn=gst_element_factory_create(fac, NULL);
  if(lmn==NULL){
    return 0;
  }

  post("setting up class for '%s'", classname);
  c=pdgst_addclass(gensym(classname));
  post("found class %x", c);
  class_addmethod  (c, (t_method)pdgst_element_gstMess, gensym("_gst"), A_GIMME, 0);

  property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
  for (i = 0; i < num_properties; i++) {
    class_addmethod  (c, (t_method)pdgst_element_any, gensym(property_specs[i]->name), A_GIMME, 0);
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
  post("finding class '%s' in %x", s->s_name, cl);
  while(cl) {
    post("searching class %x", cl);
    post("classes: %x->next=%x", cl, cl->next);
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
  post("class not found, have to really add it...:%x", c);
  if(NULL==c) {
    t_pdgst_classes*cl0=pdgst_classes;
    t_pdgst_classes*cl=(t_pdgst_classes*)getbytes(sizeof(t_pdgst_classes));
    c = class_new(s,
                  (t_newmethod)pdgst_element_new,
                  (t_method)pdgst_element_free,
                  sizeof(t_pdgst_element),
                  0,
                  A_GIMME, 0);

    cl->next=NULL;
    cl->name=s;
    cl->class=c;

    startpost("adding %x", cl);

    /* seeking to the end of our classlist */
    while(cl0 && cl0->next) {
      cl0=cl0->next;
    }
    post(" @ %x", cl0);
    
    if(cl0) {
      cl0->next=cl;
    } else {
      pdgst_classes=cl;
    }
  }
  return c;  
}
