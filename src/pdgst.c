#include "pdgst.h"


#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <locale.h>


typedef struct _pdgst_prop {
  struct _pdgst_prop*next;
  t_symbol*name;
  int flags;
  GType type;
} t_pdgst_property;

/* the pdgst base-class */
typedef struct _pdgst_core
{
  GstElement*element;
  t_pdgst_property*props;
} t_pdgst_core;


static t_pdgst_property*pdgst_addproperty(t_pdgst_property*props, GParamSpec*param)
{
  t_pdgst_property*p0=props;
  t_pdgst_property*p=NULL;
  t_symbol*s=gensym(param->name);

  while(props && props->next) {
    if(s==props->name) {      /* already stored property */
      return p0;
    }
    props=props->next;
  }

  p=(t_pdgst_property*)getbytes(sizeof(t_pdgst_property));
  p->next=NULL;
  p->name=s;
  p->flags=param->flags;
  p->type=param->value_type;

  startpost("added property '%s'", p->name->s_name);
  if(NULL==p0) {
    /* first entry */
    post(" at the beginning");
    return p;
  } else {
    endpost();
    props->next=p;
  }
  return p0;
}
static t_pdgst_property*pdgst_getproperty(t_pdgst_property*props, t_symbol*name)
{
  while(props) {
    if(props->name == name) {
      return props;
    }
    props=props->next;
  }
  return NULL;
}

static void pdgst_killproperties(t_pdgst_property*props) {
  while(props) {
    t_pdgst_property*next=props->next;
    props->next=NULL;
    props->name=NULL;
    props->flags=0;
    props->type=0;

    freebytes(props, sizeof(*props));
    props=next;
  }
}




static t_class*pdgst_class=NULL;
typedef struct _pdgst
{
  t_object x_obj;
  t_symbol*x_name;
  t_pdgst_core*x_gst;

  t_outlet*x_infout;
} t_pdgst;


static void *pdgst_new(t_symbol*s, int argc, t_atom* argv);
static void pdgst_free(t_pdgst*x);

static void pdgst_any(t_pdgst*x, t_symbol*s, int argc, t_atom*argv);

typedef struct _pdgst_classes {
  struct _pdgst_classes*next;
  t_symbol*name;
  t_class*class;
} t_pdgst_classes;

static t_pdgst_classes*pdgst_classes=NULL;

static t_class*pdgst_findclass(t_symbol*s)
{
  t_pdgst_classes*cl=pdgst_classes;
  post("search class %s in %x", s->s_name, cl);
  while(cl) {
    post("searching for classes in %x: %x=%s ?= %s", pdgst_classes, cl, cl->name->s_name, s->s_name);
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
    t_pdgst_classes*cl0=pdgst_classes;
    t_pdgst_classes*cl=(t_pdgst_classes*)getbytes(sizeof(t_pdgst_classes));
    c = class_new(s,
                  (t_newmethod)pdgst_new,
                  (t_method)pdgst_free,
                  sizeof(t_pdgst),
                  0,
                  A_GIMME, 0);

    post("created new class %x for '%s'", c, s->s_name);

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
  post("classlist=%x", pdgst_classes);
  return c;  
}

static void pdgst_outputparam(t_pdgst*x, t_symbol*name, t_atom*a)
{
  t_atom ap[3];
  SETSYMBOL(ap+0, gensym("property"));
  SETSYMBOL(ap+1, name);

  ap[2].a_type=a->a_type;
  ap[2].a_w=a->a_w;

  outlet_anything(x->x_infout, gensym("info"), 3, ap);
}

/* should be "static t_atom*" */
static void pdgst_getparam(t_pdgst*x, t_pdgst_property*prop)
{
  GstElement*element=x->x_gst->element;
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

static void pdgst_setparam(t_pdgst*x, t_pdgst_property*prop, t_atom*ap)
{
    GstElement*element=x->x_gst->element;
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


static void pdgst_gstMess(t_pdgst*x, t_symbol*s, int argc, t_atom*argv) {
  post("_gst");


}

static void pdgst_any(t_pdgst*x, t_symbol*s, int argc, t_atom*argv) {
  if(!argc) {
    /* get */
    t_pdgst_property*prop=pdgst_getproperty(x->x_gst->props, s);
    if(prop && prop->flags & G_PARAM_READABLE) {
      pdgst_getparam(x, prop);
    } else {
      pd_error(x, "[%s] no query method for '%s'", x->x_name->s_name, s->s_name);
      return;
    }
  } else {
    /* set */
    t_pdgst_property*prop=pdgst_getproperty(x->x_gst->props, s);
    if(prop && prop->flags & G_PARAM_WRITABLE) {
      pdgst_setparam(x, prop, argv);
    } else {
      pd_error(x, "[%s] no set method for '%s'", x->x_name->s_name, s->s_name);
      return;
    }
  }
}


static void pdgst_core_free(t_pdgst_core*gstcore)
{
  if(!gstcore)return;
  if(gstcore->element) {
    gst_object_unref (GST_OBJECT (gstcore->element));
  }
}

static t_pdgst_core*pdgst_core_new(GstElement*lmn, t_object*obj)
{
  t_pdgst_core*ret=(t_pdgst_core*)getbytes(sizeof(t_pdgst_core));
  ret->element=lmn;
  ret->props=NULL;
  if(lmn) {
    GParamSpec **property_specs;
    guint num_properties, i;
    property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
    for (i = 0; i < num_properties; i++) {
      ret->props=pdgst_addproperty(ret->props, property_specs[i]);
    }
    g_free (property_specs);
  }

  return ret;
}


static void pdgst_free(t_pdgst*x) {
  if(x->x_gst)
    pdgst_core_free(x->x_gst);

  x->x_gst=NULL;
  if(x->x_infout)
    outlet_free(x->x_infout);
}

static void *pdgst_new(t_symbol*s, int argc, t_atom* argv) {
  t_pdgst*x=NULL;
  
  if(gensym("pdgst")==s) {
    /* LATER make a controller.... */
    x=(t_pdgst*)pd_new(pdgst_class);
    x->x_name=s;
    x->x_gst=NULL;    
    
    x->x_infout=outlet_new(&x->x_obj, 0);
    
  } else {
    t_class*c=pdgst_findclass(s);
    post("class for '%s' = %x", s->s_name, c);

    if(c) {
      GstElement*lmn=gst_element_factory_make(s->s_name, NULL);
      if(NULL==lmn) {
        post("factory failed to create element...'%s'", s->s_name);
        return NULL;
      }
      x=(t_pdgst*)pd_new(c);
      x->x_infout=outlet_new(&x->x_obj, 0);
      x->x_name=s;
      
      x->x_gst=pdgst_core_new(lmn, &x->x_obj);
      
    } else {
      /* should never happen */
    }
  }

  return x;
}


static void pdgst_class_setup(char*classname, GstElementFactory*fac) {
  GstElement*lmn=gst_element_factory_create(fac, NULL);
  t_class*c=NULL;

  if(lmn==NULL)return;

  c=pdgst_addclass(gensym(classname));
  class_addmethod  (c, (t_method)pdgst_gstMess, gensym("_gst"), A_GIMME, 0);

  if(lmn) {
    GParamSpec **property_specs;
    guint num_properties, i;
    property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (lmn), &num_properties);
    for (i = 0; i < num_properties; i++) {
      class_addmethod  (c, (t_method)pdgst_any, gensym(property_specs[i]->name), A_GIMME, 0);
    }
    g_free (property_specs);
  }

  gst_object_unref (GST_OBJECT (lmn));
}


static int pdgst_loader(t_canvas *canvas, char *classname)
{
  GstElementFactory *factory = NULL;
  factory=gst_element_factory_find (classname);
  if(factory){
    pdgst_class_setup(classname, factory);
    return 1;
  }

  /* fallback to not-our-business */
  return (0);
}

static int pdgst_loader_setup(void)
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

  sys_register_loader(pdgst_loader);

  return 1;
}


void pdgst_setup(void)
{
  char*locale=NULL;

  post("pdgst %s",pdgst_version);  
  post("\t(copyleft) IOhannes m zmoelnig @ IEM / KUG");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);


  locale=setlocale(LC_NUMERIC, NULL);
  setlocale(LC_NUMERIC, "C");
  pdgst_loader_setup();
  setlocale(LC_NUMERIC, locale);

  pdgst_class=class_new(gensym("pdgst"), 
                        (t_newmethod)pdgst_new,
                        (t_method)pdgst_free,
                        sizeof(t_pdgst),
                        0 /* CLASS_NOINLET */,
                        A_GIMME, 0);
  class_addmethod  (pdgst_class, (t_method)pdgst_gstMess, gensym("_gst"), A_GIMME, 0);
  class_addanything(pdgst_class, pdgst_any);
}

/*
 * interesting stuff:
   gst_update_registry (void);
*/
