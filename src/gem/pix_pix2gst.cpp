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
#include "pix_pix2gst.h"

#include <gst/app/gstappsrc.h>


CPPEXTERN_NEW_WITH_ONE_ARG(pix_pix2gst, t_symbol*, A_DEFSYM)

/////////////////////////////////////////////////////////
//
// pix_pix2gst
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_pix2gst :: pix_pix2gst(t_symbol*s)  : pdgstGem("appsrc")
{
  post("pix2gst::::::::::::::");

  m_image=&m_pix.image;
  m_image->xsize=0;
  m_image->ysize=0;

  GstCaps*caps=color2caps(s);
  if(caps) {
    GstAppSrc*src=GST_APP_SRC(getPdGstElement());
    gst_app_src_set_caps(src, caps);

    post("pix2gst caps: %s", gst_caps_to_string (caps) );
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
// insert a pix into the gst-pipeline
/////////////////////////////////////////////////////////
void pix_pix2gst :: render(GemState *state)
{

}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_pix2gst :: obj_setupCallback(t_class *)
{ }
