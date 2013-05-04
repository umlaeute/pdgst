/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    -----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_PIX2GST_H_
#define INCLUDE_PIX_PIX2GST_H_

#include "pdgstGem.h"

/*-----------------------------------------------------------------
    
  CLASS
    pix_pix2gst
    
    Loads in a movie with the videoIO framework
    
    KEYWORDS
    pix
    
    DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_pix2gst : public pdgstGem
{
  CPPEXTERN_HEADER(pix_pix2gst, pdgstGem)
    
  public:  
    
    //////////
    // Constructor
  pix_pix2gst(int, t_atom*);

  protected:
    
    //////////
    // Destructor
    virtual ~pix_pix2gst(void);

    //////////
    // fetch an image from the gst-pipeline and  output it as pix
    virtual void render(GemState *state);

    guint m_width, m_height;
    GLenum m_format;
    guint m_fps_numerator, m_fps_denominator;
};

#endif	// for header file
