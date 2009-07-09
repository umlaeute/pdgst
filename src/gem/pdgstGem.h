//////////////////////////////////////////////////////////////////////////
//
//   brigde between pixes and pdgst
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 3 of the License, or
//   (at your option) any later version.
//
///////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_PIX_PIX2GST_H_
#define INCLUDE_PIX_PIX2GST_H_

#include "pdgstGem.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_pix2gst
    
    inserts a stream of pixes into a gst-pipeline
-----------------------------------------------------------------*/

class GEM_EXTERN pix_pix2gst : public pdgstGem
{
    CPPEXTERN_HEADER(pix_pix2gst, pdgstGem)

    public:

        //////////
        // Constructor
    	pix_pix2gst();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_pix2gst();
		
    	//////////
    	// Do the rendering
    	virtual void render(GemState *state);
};

#endif  // for header file
