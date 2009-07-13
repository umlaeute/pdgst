//////////////////////////////////////////////////////////////////////////
//
//   gstreamer bindings for Gem (using pdgst)

//   copyright            : (C) 2009 IOhannes m zmölnig
//                              Institute of Electronic Music and Acoustics,
//                              University of Music and Performing Arts
//                              forum::für::umläute
//   email                : zmoelnig@iem.at
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
//   (at your option) any later version.
//
///////////////////////////////////////////////////////////////////////////

#include "pdgstGem.h"

#ifdef x_obj
# undef x_obj
#endif

pdgstGem :: pdgstGem(const char*name)
{
  m_base = (t_pdgst_base*)getbytes(sizeof(t_pdgst_base));
  m_base->l_obj=*(this->x_obj);
  m_base->l_canvas=NULL;
  m_base->l_element=NULL;
  m_base->l_sighandler_bin=0;
  m_base->l_sighandler_pad_add=0;
  m_base->l_sighandler_pad_del=0;
  m_base->l_sighandler_pad_done=0;
  m_base->x_infout=m_base->x_gstout=NULL;
  m_base->x_name=gensym(name);
  m_base->x_gstname=gensym(name);

  pdgst_base__new(m_base, gensym(name), &this->x_obj->ob_pd);

  m_props=NULL;
  guint num_properties=0, i=0;
  GParamSpec **property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (m_base->l_element), &num_properties);
  for (i = 0; i < num_properties; i++) {
    m_props=pdgst_addproperty(m_props, property_specs[i]);
  }
  g_free (property_specs);

}

pdgstGem :: ~pdgstGem()
{
  pdgst_base__free(m_base);
}

void pdgstGem :: gstMess(t_symbol*s, int argc, t_atom*argv)
{
  pdgst_base__gstMess(m_base, s, argc, argv);
}
void pdgstGem :: infoMess(t_symbol*s, int argc, t_atom*argv)
{
  // dummy method to handle info's of upstream pdgst objects
}

void pdgstGem :: setProperty(t_symbol*s, int argc, t_atom*argv)
{
  t_pdgst_property*prop=pdgst_getproperty(m_props, s);
  if(prop && prop->flags & G_PARAM_WRITABLE) {
    verbose(1, "setting property '%s'", s->s_name);
    pdgst_base__setParam(m_base, prop, argv);
    if(prop->flags & G_PARAM_READABLE)
      pdgst_base__getParam(m_base, prop);
  } else {
    error("no set method for '%s'", s->s_name);
    return;
  }
}

void pdgstGem :: getProperty(t_symbol*s)
{
  t_pdgst_property*prop=pdgst_getproperty(m_props, s);
  if(prop && prop->flags & G_PARAM_READABLE) {
    pdgst_base__getParam(m_base, prop);
  } else {
    error("no query method for '%s'", s->s_name);
  }
}


GstElement*pdgstGem :: getPdGstElement()
{
  return m_base->l_element;
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////

void pdgstGem :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pdgstGem::gstMessCallback,
                   s_pdgst__gst, A_GIMME, A_NULL);
  class_addmethod(classPtr, (t_method)&pdgstGem::infoMessCallback,
                  gensym("_info"), A_GIMME, A_NULL);

  class_addanything(classPtr, (t_method)&pdgstGem::anyMessCallback);
}

void pdgstGem :: gstMessCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  GetMyClass(data)->gstMess(s, argc, argv);
}

void pdgstGem :: infoMessCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  GetMyClass(data)->infoMess(s, argc, argv);
}


void pdgstGem :: anyMessCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  if(0==argc) {
     GetMyClass(data)->getProperty(s);
  } else {
     GetMyClass(data)->setProperty(s, argc, argv);
  }
}
