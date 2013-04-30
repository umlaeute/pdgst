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

#include "pix_pix2gst.h"
#include "Base/GemState.h"


#include <gst/app/gstappsrc.h>
#include <gst/base/gstadapter.h>


CPPEXTERN_NEW_WITH_FOUR_ARGS(pix_pix2gst, t_symbol*, A_SYMBOL, t_floatarg, A_FLOAT, t_floatarg, A_FLOAT, t_floatarg, A_FLOAT)

/////////////////////////////////////////////////////////
//
// pix_pix2gst
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_pix2gst :: pix_pix2gst( t_symbol*s, t_floatarg w, t_floatarg h, t_floatarg fps)  : pdgstGem("appsrc"),
                                                                       m_width(0), m_height(0), m_fps_numerator(0), m_format(-1)
{
  if(w<0)w=128;
  if(h<0)h=128;
  if(fps<0)fps=20;
  
  m_width=w;
  m_height=h;

  m_fps_numerator = fps;
  m_fps_denominator = 1;

  GstCaps*caps=color2caps(s);
  m_format=m_image->format;
  if(caps) {
    GstAppSrc*src=GST_APP_SRC(getPdGstElement());
    gst_caps_set_simple (caps,
                         "width", G_TYPE_INT, m_width,
                         "height", G_TYPE_INT, m_height,
                         "framerate", GST_TYPE_FRACTION, m_fps_numerator, m_fps_denominator,
                         NULL);
    verbose(1, "pix2gst caps: %s", gst_caps_to_string (caps) );

    gst_app_src_set_caps(src, caps);

    gst_caps_unref (caps);
    caps=NULL;
  }

  m_image->reallocate();
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_pix2gst :: ~pix_pix2gst()
{
}

/////////////////////////////////////////////////////////
// render
//
// fetch an image from the pipeline
// and output it as a pix
/////////////////////////////////////////////////////////

void pix_pix2gst :: render(GemState *state)
{
  if (!state)return;
  pixBlock*pix=NULL;
  imageStruct*img=NULL;

#if (GEM_VERSION_MAJOR > 0) || (GEM_VERSION_MINOR >= 93)
  state->get(GemState::_PIX, pix);
#else
  pix=state->image;
#endif

  if(!pix || !pix->image.data){
    return;
  }

  img=&pix->image;

  if(img->xsize!=m_width || img->ysize!=m_height || img->format!=m_format) {
    error("pix does not match %dx%d", m_width, m_height);
    return;
  }

  int linelength=img->xsize*img->csize;
  int linelengthO=GST_ROUND_UP_4(linelength);

  int size = img->ysize * GST_ROUND_UP_4(img->xsize*img->csize);

  GstAppSrc*source=GST_APP_SRC(getPdGstElement());
  GstBuffer *buf = gst_buffer_new_and_alloc(size);

  unsigned char*rec_data=(unsigned char*)GST_BUFFER_DATA (buf);
  unsigned char*data=img->data;

  int i=0;
#if 1
  bool upsidedown=img->upsidedown;
  for(i=0; i<img->ysize; i++) {
    int j=(upsidedown)?i:(img->ysize-i-1);
    unsigned char*linein =    data +j*linelength;
    unsigned char*lineout=rec_data +i*linelengthO;

    memcpy(lineout, linein, linelength);
  }
#else
  for(i=0; i<img->ysize; i++) {
    unsigned char*linein =    data +i*linelength;
    unsigned char*lineout=rec_data +i*linelength;
    memcpy(lineout, linein, linelength);
    post("offset[%d]=%d", i, i*linelength);
  }
#endif


  gst_app_src_push_buffer (source, buf);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_pix2gst :: obj_setupCallback(t_class *)
{ }
