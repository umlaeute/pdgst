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

#warning add docs

#include "pdgst/pdgst.h"
#include <string.h>

t_atom*pdgst__gvalue2atom(const GValue*v, t_atom*a0)
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
        g_value_unset(&destval);
        g_value_init (&destval, G_TYPE_STRING);
        if(g_value_transform(v, &destval)) {
          const gchar*str=g_value_get_string(&destval);
          if(NULL==str)
            s=&s_;
          else
            s=gensym(str);
          SETSYMBOL(a, s);
        } else 
          success=0;
      }
    }
  }
  g_value_unset(&destval);

  if(success)
    return a;

  if(a0!=a)
    freebytes(a, sizeof(t_atom));
  return NULL;
}

GValue*pdgst__atom2gvalue(const t_atom*a, GValue*v0)
{
  GValue*v=v0;
  t_symbol*s=atom_getsymbol((t_atom*)a);
  t_float  f=atom_getfloat((t_atom*)a);
  t_int    i=atom_getint((t_atom*)a);

  if(NULL==v) {
    v=(GValue*)getbytes(sizeof(GValue));
    memset(v, 0, sizeof(GValue));
  }
  if( G_TYPE_NONE==G_VALUE_TYPE(v) || G_TYPE_INVALID==G_VALUE_TYPE(v)) {
    if(A_SYMBOL==a->a_type) {
      g_value_init (v, G_TYPE_STRING);
    } else {
      g_value_init (v, G_TYPE_FLOAT);
    }
  }
  post("valuetype: %s", G_VALUE_TYPE_NAME(v));

  switch (G_VALUE_TYPE (v)) {
  case G_TYPE_STRING:
    g_value_set_string(v, s->s_name);
    break;
  case G_TYPE_BOOLEAN:
    g_value_set_boolean(v, i);
    break;
  case G_TYPE_ENUM:
    g_value_set_enum(v, i);
    break;
  case G_TYPE_CHAR: 
    g_value_set_char(v, i);
    break;
  case G_TYPE_UCHAR: 
    g_value_set_uchar(v, i);
    break;
  case G_TYPE_INT:
    g_value_set_int(v, i);
    break;
  case G_TYPE_UINT: 
    g_value_set_uint(v, i);
    break;
  case G_TYPE_LONG: 
    g_value_set_long(v, i);
    break;
  case G_TYPE_ULONG: 
    g_value_set_ulong(v, i);
    break;
  case G_TYPE_INT64: 
    g_value_set_int64(v, i);
    break;
  case G_TYPE_UINT64: 
    g_value_set_uint64(v, i);
    break;
  case G_TYPE_FLOAT:
    g_value_set_float(v, f);
    break;
  case G_TYPE_DOUBLE:
    g_value_set_double(v, f);
    break;
  case G_TYPE_INTERFACE:
  case G_TYPE_INVALID:
  case G_TYPE_NONE:
  case G_TYPE_FLAGS:
  case G_TYPE_POINTER:
  case G_TYPE_BOXED:
  case G_TYPE_PARAM:
  case G_TYPE_OBJECT:
  default:
    if(G_VALUE_HOLDS(v, G_TYPE_ENUM)) {
      /* an enum */
      if(A_SYMBOL==a->a_type) {
      /* LATER: check symbolic values */
        GEnumClass*klass = NULL; 
        int i1=0;
        GEnumValue*value=NULL;
        post("bla:%d", __LINE__);
        klass=(GEnumClass *) g_type_class_ref (G_VALUE_TYPE (v));
        post("bla:%d", __LINE__);
        value=g_enum_get_value_by_name(klass, s->s_name);
        post("bla:%d\t%x", __LINE__, value);
        i1=value->value;
        post("bla:%d", __LINE__);
        post("%s -> %d", s->s_name, i1);
        g_value_set_enum(v, i1);
      } else {
        g_value_set_enum(v, i);
      }
      break;
    } else {
      /* no enum class */
      post("ignoring atom: "); postatom(1, a); endpost();
    }

    if(v!=v0) {
      freebytes((void*)v0, sizeof(GValue));
    }
    return NULL;
    break;
  }
  return v;
}
