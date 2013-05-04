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

#include <algorithm>

#ifdef x_obj
# undef x_obj
#endif

pdgstGem :: pdgstGem(const char*name)
{
  if(!pdgst_init()) {
    throw(GemException("PdGst failed to initialize"));
  }

  m_image=&m_pix.image;
  m_image->xsize=0;
  m_image->ysize=0;
  m_image->reallocate();

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


/////////////////////////////////////////////////////////
// colorspace
//
// set the colorspace 
/////////////////////////////////////////////////////////
GstCaps*pdgstGem :: color2caps(t_symbol*colorsym) {
  int cs=GL_RGBA_GEM;
  std::string color="rgba";
  if(colorsym && colorsym->s_name)
    color=colorsym->s_name;
  std::transform(color.begin(), color.end(), color.begin(), ::tolower);

  if("rgba"==color) {
    cs=GL_RGBA_GEM;
  } else if("yuv"==color) {
    cs=GL_YUV422_GEM;
  } else if("grey"==color) {
    cs=GL_LUMINANCE;
  } else if("gray"==color) {
    cs=GL_LUMINANCE;
  } else if(""==color) {
  } else {
    error("unknown colorspace '%s'", color.c_str());
    cs=GL_RGBA_GEM;
  }

  if(m_image)
    m_image->setCsizeByFormat(cs);

  switch(cs) {
  case GL_RGBA_GEM:
    return gst_caps_new_simple ("video/x-raw-rgb", 
                              "bpp", G_TYPE_INT, 32,
                              "depth", G_TYPE_INT, 32,
                              "red_mask",   G_TYPE_INT, 0xff000000,
                              "green_mask", G_TYPE_INT, 0x00ff0000,
                              "blue_mask",  G_TYPE_INT, 0x0000ff00,
                              "alpha_mask", G_TYPE_INT, 0x000000ff,
                              //                              "endianess", G_TYPE_INT, G_BIG_ENDIAN,
                              NULL);
    break;
  case GL_RGB:
    return gst_caps_new_simple ("video/x-raw-rgb", 
                              "bpp", G_TYPE_INT, 24,
                              "depth", G_TYPE_INT, 24,
                              "red_mask",   G_TYPE_INT, 0xff000000,
                              "green_mask", G_TYPE_INT, 0x00ff0000,
                              "blue_mask",  G_TYPE_INT, 0x0000ff00,
                              NULL);
    break;
  case GL_LUMINANCE:
    return gst_caps_new_simple ("video/x-raw-gray", 
                              NULL);
    break;
  case GL_YUV422_GEM:
    return gst_caps_new_simple ("video/x-raw-yuv", 
                                "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('U', 'Y', 'V', 'Y'),
                                // "framerate", GST_TYPE_FRACTION, 20, 1,
                                NULL);
    break;
  default: 
    error("no caps for colorspace %d!", cs);
    break;
  }
  return NULL;
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
  if(!pdgst_init())return;
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
