
#include "pdgst.h"

t_pdgst_property*pdgst_addproperty(t_pdgst_property*props, GParamSpec*param)
{
  t_pdgst_property*p0=props;
  t_pdgst_property*p=NULL;
  t_symbol*s=gensym(param->name);

  while(props && props->next) {
    if(s==props->name) {      /* already stored property */
      return p0;
    }
    props=props->next;
  }

  p=(t_pdgst_property*)getbytes(sizeof(t_pdgst_property));
  p->next=NULL;
  p->name=s;
  p->flags=param->flags;
  p->type=param->value_type;

  //  startpost("added property '%s'", p->name->s_name);
  if(NULL==p0) {
    /* first entry */
    //    post(" at the beginning");
    return p;
  } else {
    //endpost();
    props->next=p;
  }
  return p0;
}

t_pdgst_property*pdgst_getproperty(t_pdgst_property*props, t_symbol*name)
{
  while(props) {
    if(props->name == name) {
      return props;
    }
    props=props->next;
  }
  return NULL;
}

void pdgst_killproperties(t_pdgst_property*props) {
  while(props) {
    t_pdgst_property*next=props->next;
    props->next=NULL;
    props->name=NULL;
    props->flags=0;
    props->type=0;

    freebytes(props, sizeof(*props));
    props=next;
  }
}
