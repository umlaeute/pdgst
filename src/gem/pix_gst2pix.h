/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    -----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_GST2PIX_H_
#define INCLUDE_PIX_GST2PIX_H_

#include "pdgstGem.h"

#ifdef CLAMP
# undef CLAMP
#endif

#include "Base/GemPixUtil.h"

/*-----------------------------------------------------------------
    
  CLASS
    pix_gst2pix
    
    Loads in a movie with the videoIO framework
    
    KEYWORDS
    pix
    
    DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_gst2pix : public pdgstGem
{
  CPPEXTERN_HEADER(pix_gst2pix, pdgstGem)
    
  public:  
    
    //////////
    // Constructor
    pix_gst2pix(t_symbol*);

  protected:
    
    //////////
    // Destructor
    virtual ~pix_gst2pix(void);

    //////////
    // fetch an image from the gst-pipeline and  output it as pix
    virtual void render(GemState *state);

    // colorspace
    GstCaps*color2caps(t_symbol*s);

    pixBlock m_pix;
    imageStruct*m_image;

    
};

#endif	// for header file
