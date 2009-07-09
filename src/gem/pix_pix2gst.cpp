//////////////////////////////////////////////////////////////////////////
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
//   (at your option) any later version.
//
///////////////////////////////////////////////////////////////////////////

#include "pix_pix2gst.h"

CPPEXTERN_NEW(pix_pix2gst)

pix_pix2gst :: pix_pix2gst(void):
{
}

pix_pix2gst :: ~pix_pix2gst()
{
}
void pix_pix2gst :: render(GemState *state)
{
#if 0
  if(!fileWriter ) return;
  if(!m_recording) return;
  if(!state || !state->image)return;
  
  imageStruct *im = &state->image->image;

  if(im)return;
  
  if( m_first_time )
  {
    // get format data from GEM
    int xsize = im->xsize;
    int ysize = im->ysize;
    ///TODO if no movie is loaded to play and you start recording
    /// and create the gemwin it gets segmentation fault here
    int format;

    switch(im->format)
    {
      case GL_LUMINANCE:
        format = VideoIO_::GRAY;
        break;

      case GL_YCBCR_422_GEM:
        format = VideoIO_::YUV422;
        break;
        
      case GL_RGB:
        format = VideoIO_::RGB;
        break;
    
      case GL_RGBA:
      default:
        format = VideoIO_::RGBA;
        break;
    }

    post("writing to video file ...");

    // set frame size
    m_frame.setFrameSize(xsize, ysize, format);

    float framerate = GemMan::getFramerate();
    fileWriter->setFramerate( framerate );

    m_first_time = false;
  }
  
  // set data of frame
  m_frame.setFrameData(im->data, m_frame.getXSize(),
                       m_frame.getYSize(), m_frame.getColorSize());

  fileWriter->pushFrame(m_frame);
#endif
}

