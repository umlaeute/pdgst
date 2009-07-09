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
#include <stdio.h>

CPPEXTERN_NEW(pix_gst2pix)

/////////////////////////////////////////////////////////
//
// pix_gst2pix
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_gst2pix :: pix_gst2pix(void) 
{
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
#if 0
  if(!fileReader) return;

  // set Frame Data
  unsigned char *image_ptr = m_image.image.data;

  fileReader->setFrameData(image_ptr, 
                           m_image.image.xsize, m_image.image.ysize, 
                           m_image.image.csize);

  // read frame data into m_image
  int status = fileReader->processFrameData();

  if( status == VideoIO_::VIDEO_STOPPED )
  {
    // output end of video bang in playing mode
    // and stop video
    if(!m_already_banged)
    {
      outlet_bang(m_outEnd);
      m_already_banged = true;
    }
    return;
  }

  if( status == VideoIO_::VIDEO_SIZE_CHANGED )
  {
    // check if image size changed
    if( m_image.image.xsize != fileReader->getWidth() ||
      m_image.image.ysize != fileReader->getHeight() ||
      m_image.image.csize != fileReader->getColorSize() ) 
      reallocate_m_image();

    infoSize();
    // process frame with new size again
    fileReader->processFrameData();
  }

  // set flag if we have a new film
  if(m_newfilm)
  {
    m_image.newfilm = true;
    m_newfilm = false;
  }
  m_image.newimage = true;

  // set image
  state->image = &m_image;
#endif
}

