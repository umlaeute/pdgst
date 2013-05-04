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

/* pdgst_adc~.c
 *    appsrc that drains Pd-audio into a gst-pipeline
 */

#warning add docs

#define MAXPDCHANNELS 1024
#include "pdgst/pdgst.h"
#include <gst/app/gstappsink.h>
#include <gst/base/gstadapter.h>

static t_class *pdgst_adc_class;
typedef struct _pdgst_adc
{
  t_pdgst_base x_elem;
  unsigned int x_channels; /* #channels */
  unsigned int x_n;        /* samples per block */
  t_sample x_f; /* should this be t_sample or t_float? */

  t_pdgst_property*x_props;
  GstAdapter*x_adapter;
} t_pdgst_adc;

static t_int*pdgst_adc_perform(t_int*w){
  unsigned int offset = 2;
  t_pdgst_adc* x = (t_pdgst_adc*) (w[1]);
  unsigned int n = x->x_n;
  t_sample **out = getbytes(sizeof(t_sample)*x->x_channels);
  unsigned int i,j;

  GstBuffer*buf=NULL;
  unsigned int buffersize=sizeof(t_sample)*x->x_channels*n;
  unsigned int availablesize=0;
  GstAppSink*sink=GST_APP_SINK(x->x_element);

  for (i=0;i < x->x_channels;i++) {
    out[i] = (t_float *)(w[offset+i]);
  }

  /* now get the GstBuffer holding the data */
  availablesize=gst_adapter_available(x->x_adapter);
  if( availablesize < buffersize ) {
    /* this might block */

    if( GST_STATE(sink)==GST_STATE_PLAYING && GST_STATE_PENDING(sink)==GST_STATE_VOID_PENDING ) {
      buf= gst_app_sink_pull_buffer(sink);
    }
    if(buf) {
      gst_adapter_push (x->x_adapter, buf);
    }
  }

  /* now copy the data from the GstBuffer */
  if( gst_adapter_available(x->x_adapter) >= buffersize ) {
    t_sample*inbuf=(t_sample*)gst_adapter_take (x->x_adapter, buffersize);
    /* interleaved */
    for(j=0; j<n; j++) {
      for (i=0;i < x->x_channels;i++) {
        t_sample*outbuf=out[i];
        t_sample f=*inbuf++;
        outbuf[j]=f;
      }
    }
  } else {
    for (i=0;i < x->x_channels;i++) {
      t_sample*outbuf=out[i];
      for(j=0; j<n; j++) {
        *outbuf++=0.f;
      }
    }
  }
  return (w+offset+1+i);
}


static void pdgst_adc_dsp(t_pdgst_adc *x, t_signal **sp){
  unsigned int i;
  t_int** myvec = getbytes(sizeof(t_int*)*(x->x_channels + 2));

  myvec[0] = (t_int*)x;
  x->x_n = sp[0]->s_n;

  for (i=0; i<x->x_channels; i++) {
    myvec[1 + i] = (t_int*)sp[i]->s_vec;
  }

  dsp_addv(pdgst_adc_perform, x->x_channels + 2, (t_int*)myvec);
  freebytes(myvec,sizeof(t_int)*(x->x_channels + 2));
}

static void pdgst_adc_any(t_pdgst_adc*x, t_symbol*s, int argc, t_atom*argv) {
  //  startpost("%s: ", s->s_name); postatom(argc, argv);endpost();
  if(!argc) {
    /* get */
    t_pdgst_property*prop=pdgst_getproperty(x->x_props, s);
    if(prop && prop->flags & G_PARAM_READABLE) {
      pdgst_base__getParam(&x->x_elem, prop);
    } else {
      pd_error(x, "[%s] no query method for '%s'", x->x_elem.x_name->s_name, s->s_name);
      return;
    }
  } else {
    /* set */
    t_pdgst_property*prop=pdgst_getproperty(x->x_props, s);
    if(prop && prop->flags & G_PARAM_WRITABLE) {
      verbose(0, "setting property '%s'", s->s_name);
      pdgst_base__setParam(&x->x_elem, prop, argv);
      if(prop->flags & G_PARAM_READABLE)
        pdgst_base__getParam(&x->x_elem, prop);
    } else {
      pd_error(x, "[%s] no set method for '%s'", x->x_elem.x_name->s_name, s->s_name);
      return;
    }
  }
}

static void pdgst_adc_free(t_pdgst_adc*x) {
  if(x->x_adapter)gst_object_unref(x->x_adapter);
}

static void*pdgst_adc_new(t_floatarg f) {
  guint channels=f;
  guint i=0;
  GstAppSink*sink=NULL;
  GstCaps*caps=NULL;
  t_pdgst_adc*x=(t_pdgst_adc*)pd_new(pdgst_adc_class);

  pdgst_pushlocale();

  if(channels<1)
    channels=2;
  if(channels>MAXPDCHANNELS){
    error("max.number of channels is %d", MAXPDCHANNELS);
  }
  x->x_channels=channels;
  x->x_n = 64;

  pdgst_base__new(&x->x_elem, gensym("appsink"), NULL);

  while (channels--) {
    outlet_new(&x->x_obj, gensym("signal"));
  }
  channels=x->x_channels;

  sink=GST_APP_SINK(x->x_element);

  gint samplerate=(gint)sys_getsr();
  caps=gst_caps_new_simple ("audio/x-raw-float",
                            "endianess", G_TYPE_INT, G_BYTE_ORDER,
                            "width", G_TYPE_INT, 8*sizeof(t_sample),
                            "rate", G_TYPE_INT, samplerate,
                            "channels", G_TYPE_INT, channels,
                            NULL);

  gst_app_sink_set_caps(sink, caps);

  /* property handling
   * we build a list of all properties and the expected type of their values 
   */
  x->x_props=NULL;
  guint num_properties=0;
  GParamSpec **property_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (x->x_element), &num_properties);
  for (i = 0; i < num_properties; i++) {
    x->x_props=pdgst_addproperty(x->x_props, property_specs[i]);
  }
  g_free (property_specs);


  /* buffer */
  x->x_adapter=gst_adapter_new();

  pdgst_poplocale();
  return x;
}


void pdgst_adc_tilde_setup(void) {
  pdgst_adc_class = class_new(gensym("pdgst_adc~"),
                              (t_newmethod)pdgst_adc_new, (t_method)pdgst_adc_free,
                              sizeof(t_pdgst_adc), 0, A_DEFFLOAT, 0);
  class_addmethod(pdgst_adc_class, (t_method)pdgst_adc_dsp, gensym("dsp"), 0);

  class_addmethod  (pdgst_adc_class, (t_method)pdgst_base__gstMess, s_pdgst__gst, A_GIMME, 0);
#warning _info hack
  class_addmethod  (pdgst_adc_class, (t_method)pdgst_base__infoMess, gensym("_info"), A_GIMME, 0);


    class_addanything  (pdgst_adc_class, (t_method)pdgst_adc_any);
}
