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

#ifndef INCLUDE_PDGSTGEM_H_
#define INCLUDE_PDGSTGEM_H_

#include "Base/GemBase.h"
#include "Base/GemPixUtil.h"

#include "pdgst/pdgst.h"


/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pdgstGem
    
    base class for pdgst-based Gem objects
-----------------------------------------------------------------*/

class GEM_EXTERN pdgstGem : public GemBase
{
  CPPEXTERN_HEADER(pdgstGem, GemBase)

    public:

        //////////
        // Constructor
    	pdgstGem(const char*);
    	
    protected:

      bool     init(t_symbol*);
    	
    	//////////
    	// Destructor
    	virtual ~pdgstGem();


      // colorspace
      GstCaps*color2caps(t_symbol*s);

      pixBlock m_pix;
      imageStruct*m_image;

      //////////
      // pdgst-messages
      void     gstMess(t_symbol*,int,t_atom*);
      void     infoMess(t_symbol*,int,t_atom*);

      void setProperty(t_symbol*, int, t_atom*);
      void getProperty(t_symbol*);

      t_pdgst_base*m_base;
      t_pdgst_property*m_props;
      GstElement*getPdGstElement(void);
		
    	//////////
    	// Do the rendering: fill/drain the gst-pipeline
    	virtual void render(GemState *state) = 0;

 private:
      static void     gstMessCallback (void *data, t_symbol*,int,t_atom*);
      static void     infoMessCallback(void *data, t_symbol*,int,t_atom*);
      static void     anyMessCallback (void *data, t_symbol*,int,t_atom*);

      t_pdgst_base*pdgst;

      
};

#endif  // for header file
