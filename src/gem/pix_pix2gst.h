/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    -----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_PIX2GST_H_
#define INCLUDE_PIX_PIX2GST_H_

#include "pix_gst2pix.h"


/*-----------------------------------------------------------------
    
  CLASS
    pix_pix2gst
    
    insert a pix into a gst pipeline
    
    KEYWORDS
    pix
    
    DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_pix2gst : public pix_gst2pix
{
  CPPEXTERN_HEADER(pix_pix2gst, pix_gst2pix)
    
  public:  
    
    //////////
    // Constructor
    pix_pix2gst(t_symbol*);

  protected:
    
    //////////
    // Destructor
    virtual ~pix_pix2gst(void);

    //////////
    // fetch an image from the gst-pipeline and  output it as pix
    virtual void render(GemState *state);
    
};

#endif	// for header file
