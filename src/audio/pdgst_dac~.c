/******************************************************
 *
 * pdgst - implementation file
 *
 * copyleft (c) 2009 IOhannes m zmölnig
 *
 *   forum::für::umläute
 *
 *   institute of electronic music and acoustics (iem)
 *   university of music and performing arts
 *
 ******************************************************
 *
 * license: GNU General Public License v.2 or later
 *
 ******************************************************/

/* pdgst_dac~.c
 *    appsrc that drains Pd-audio into a gst-pipeline
 */

#warning add docs

#define MAXPDCHANNELS 1024
#include "pdgst/pdgst.h"
#include <gst/app/gstappsrc.h>

static t_class *pdgst_dac_class;
typedef struct _pdgst_dac
{
  t_pdgst_base x_elem;
  int x_channels; /* #channels */
  t_sample x_f; /* should this be t_sample or t_float? */
} t_pdgst_dac;

static t_int*pdgst_dac_perform(t_int*w){
  t_pdgst_dac* x = (t_pdgst_dac*) (w[1]);
  int n = (int)(w[2]);
  t_sample **in = getbytes(sizeof(t_sample)*x->x_channels);
  int i,j;
  int offset = 3;
  GstBuffer*buf=gst_buffer_new_and_alloc(x->x_channels*sizeof(t_sample)*n);
  GstAppSrc*src=(GstAppSrc*)x->x_element;
  t_sample*outbuf=(t_sample*)GST_BUFFER_DATA (buf);


  for (i=0;i < x->x_channels;i++) {
    in[i] = (t_float *)(w[offset+i]);
  }

  /* now copy the data into the GstBuffer */
  for (i=0;i < x->x_channels;i++) {
    t_sample*inbuf=in[i];
    for(j=0; j<n; j++) {
      *outbuf++=*inbuf++;
    }
  }
  gst_app_src_push_buffer(src,buf);
  return (w+offset+1+i);
}


static void pdgst_dac_dsp(t_pdgst_dac *x, t_signal **sp){
  int i;
  t_int** myvec = getbytes(sizeof(t_int*)*(x->x_channels + 3));
  
  myvec[0] = (t_int*)x;
  myvec[1] = (t_int*)sp[0]->s_n;

  for (i=0;i < x->x_channels;i++) {
    myvec[2 + i] = (t_int*)sp[i]->s_vec;
  }

  dsp_addv(pdgst_dac_perform, x->x_channels + 3, (t_int*)myvec);
  freebytes(myvec,sizeof(t_int)*(x->x_channels + 3));
}


static void pdgst_dac_free(t_pdgst_dac*x) {
}

static void*pdgst_dac_new(t_floatarg f) {
  int channels=f;
  int i=0;
  GstAppSrc*src=NULL;
  GstCaps*caps=NULL;
  t_pdgst_dac*x=(t_pdgst_dac*)pd_new(pdgst_dac_class);

  if(channels<1)
    channels=2;
  if(channels>MAXPDCHANNELS){
    error("max.number of channels is %d", MAXPDCHANNELS);
  }
  x->x_channels=channels;
  while (channels--) {
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  }

  caps=gst_caps_new_simple ("audio/x-raw-float",
			    "endianess", G_TYPE_INT, 1234,
			    "width", G_TYPE_INT, 8*sizeof(t_sample),
			    "rate", G_TYPE_INT, 44100,
			    "channels", G_TYPE_INT, channels,
			    NULL);


  pdgst_pushlocale();

  pdgst_base__new(&x->x_elem, gensym("appsrc"));

  src=(GstAppSrc*)x->x_element;

  gst_app_src_set_caps(src, caps);



  pdgst_poplocale();
  return x;
}


void pdgst_dac_tilde_setup(void) {
  pdgst_dac_class = class_new(gensym("pdgst_dac~"),
			      (t_newmethod)pdgst_dac_new, (t_method)pdgst_dac_free,
			      sizeof(t_pdgst_dac), CLASS_NOINLET, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(pdgst_dac_class, t_pdgst_dac, x_f);
  class_addmethod(pdgst_dac_class, (t_method)pdgst_dac_dsp, gensym("dsp"), 0);

  class_addmethod  (pdgst_dac_class, (t_method)pdgst_base__gstMess, s_pdgst__gst, A_GIMME, 0);
#warning _info hack
  class_addmethod  (pdgst_dac_class, (t_method)pdgst_base__infoMess, gensym("_info"), A_GIMME, 0);



}
