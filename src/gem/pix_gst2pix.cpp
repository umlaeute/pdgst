////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2007, Thomas Holzmann, Georg Holzmann
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

//#include "pdgstGem.cpp"
#include "pix_gst2pix.h"

#include <gst/app/gstappsink.h>
#include <gst/base/gstadapter.h>


CPPEXTERN_NEW_WITH_ONE_ARG(pix_gst2pix, t_symbol*, A_DEFSYM)

/////////////////////////////////////////////////////////
//
// pix_gst2pix
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_gst2pix :: pix_gst2pix(t_symbol*s)  : pdgstGem("appsink")
{
  post("gst2pix::::::::::::::");

  m_image=&m_pix.image;
  m_image->xsize=0;
  m_image->ysize=0;

  GstCaps*caps=color2caps(s);
  if(caps) {
    GstAppSink*sink=GST_APP_SINK(getPdGstElement());
    gst_app_sink_set_caps(sink, caps);

    post("gst2pix caps: %s", gst_caps_to_string (caps) );
    gst_caps_unref (caps);
    caps=NULL;
  }

  m_image->reallocate();
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_gst2pix :: ~pix_gst2pix()
{
}

/////////////////////////////////////////////////////////
// colorspace
//
// set the colorspace 
/////////////////////////////////////////////////////////
GstCaps*pix_gst2pix :: color2caps(t_symbol*color) {
  int cs=GL_RGBA_GEM;

  if(gensym("rgba")==color) {
    cs=GL_RGBA_GEM;
  } else if(gensym("yuv")==color) {
    cs=GL_YUV422_GEM;
  } else if(gensym("grey")==color) {
    cs=GL_LUMINANCE;
  } else if(gensym("gray")==color) {
    cs=GL_LUMINANCE;
  } else if(color&&gensym("")==color) {
  } else {
    error("unknown colorspace '%s'", color->s_name);
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
                              "framerate", GST_TYPE_FRACTION, 1, 20,
                              NULL);
    break;
  default: 
    error("no caps for colorspace %d!", cs);
    break;
  }
  return NULL;
}

/////////////////////////////////////////////////////////
// render
//
// fetch an image from the pipeline
// and output it as a pix
/////////////////////////////////////////////////////////
void pix_gst2pix :: render(GemState *state)
{
  GstAppSink*sink=GST_APP_SINK(getPdGstElement());
  if( !sink || gst_app_sink_is_eos(GST_APP_SINK (sink)) ) {
    return;
  }
  GstBuffer *buf = 0;
  if( GST_STATE(sink)==GST_STATE_PLAYING && GST_STATE_PENDING(sink)==GST_STATE_VOID_PENDING )
    {
      buf = gst_app_sink_pull_buffer(sink);
    }
  if(!buf) return;

  guint8 *data = GST_BUFFER_DATA( buf );
  guint32 fourcc=0;
  int x_size, y_size, bpp, depth;


  m_pix.newfilm=false;

  if(0==fourcc) {
    GstCaps *caps = gst_buffer_get_caps (buf);
    GstStructure *str = gst_caps_get_structure (caps, 0);

    g_assert( gst_structure_get_int(str, "width", &x_size) );
    g_assert( gst_structure_get_int(str, "height", &y_size) );


    gst_structure_get_int(str, "bpp", &bpp);
    gst_structure_get_int(str, "depth", &depth);

    gst_structure_get_fourcc(str, "format", &fourcc);

    if(m_image->xsize!=x_size || m_image->ysize!=y_size) {
      m_pix.newfilm=true;
    }

    m_image->xsize=x_size;
    m_image->ysize=y_size;

    m_image->reallocate();

    gst_caps_unref(caps);
    //gst_structure_free (str);


  }

  switch(fourcc) {
  case GST_MAKE_FOURCC('U','Y','V','Y'): 
    m_image->fromUYVY(data); 
    break;
  case GST_MAKE_FOURCC('Y','U','Y','2'): 
    m_image->fromYUY2(data); 
    break;
  case 0:
    if(24==bpp&&24==depth)
      m_image->fromRGB(data);
    else if(32==bpp&&32==depth)
      m_image->fromRGBA(data);
    else if(8==bpp&&8==depth)
      m_image->fromGray(data);
    else
      error("unknown format: %d/%d", bpp, depth);
    break;
  default:
    error("unknown format '%" GST_FOURCC_FORMAT "'", GST_FOURCC_ARGS (fourcc));
    return;
  }
  m_image->upsidedown=true;

  m_pix.newimage=true;
  // set image
  state->image = &m_pix;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_gst2pix :: obj_setupCallback(t_class *)
{ }
