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

#include "pix_gst2pix.h"

#include "Base/GemState.h"

#include <gst/app/gstappsink.h>
#include <gst/base/gstadapter.h>


CPPEXTERN_NEW_WITH_ONE_ARG(pix_gst2pix, t_symbol*, A_SYMBOL);

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
  GstCaps*caps=color2caps(s);
  if(caps) {
    GstAppSink*sink=GST_APP_SINK(getPdGstElement());
    gst_app_sink_set_caps(sink, caps);

    verbose(1, "gst2pix caps: %s", gst_caps_to_string (caps) );
    gst_caps_unref (caps);
    caps=NULL;
  }
  m_image->reallocate();

  t_atom ap[1];
  SETFLOAT(ap, 1);
  setProperty(gensym("max-buffers"), 1, ap);
  setProperty(gensym("drop"), 1, ap);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_gst2pix :: ~pix_gst2pix()
{
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
    if(24==bpp&&24==depth) {
      m_image->fromRGB(data);
    } else if(32==bpp&&32==depth) {
      m_image->fromRGBA(data);
    } else if(8==bpp&&8==depth)
      m_image->fromGray(data);
    else {
      error("unknown format: %d/%d", bpp, depth);
    }
    break;
  default:
    error("unknown format '%" GST_FOURCC_FORMAT "'", GST_FOURCC_ARGS (fourcc));
    return;
  }
  /* get rid of the GstBuffer */
  data=NULL;
  gst_buffer_unref(buf);

  /* send it downstream */
  m_image->upsidedown=true;

  m_pix.newimage=true;
  // set image
#if (GEM_VERSION_MAJOR > 0) || (GEM_VERSION_MINOR >= 93)
  state->set(GemState::_PIX, &m_pix);
#else
  state->image = &m_pix;
#endif
}

//////////
// get the original state back
void pix_gst2pix :: postrender(GemState *state){
#warning LATER store the original pixblock in render() and restore it here 

#if (GEM_VERSION_MAJOR > 0) || (GEM_VERSION_MINOR >= 93)
  state->set(GemState::_PIX, (pixBlock*)NULL); // orgPixBlock
#else
  state->image = NULL;//orgPixBlock;
#endif
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_gst2pix :: obj_setupCallback(t_class *)
{ }
