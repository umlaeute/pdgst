
/* pdgst_elem.c */
typedef struct _pdgst_elem
{
  t_object l_obj;
  t_canvas  *l_canvas;
  GstElement*l_element;
  t_method l_busCallback;

  t_outlet*x_infout;
  t_outlet*x_gstout;


  t_symbol*x_name;
  t_symbol*x_gstname;
} t_pdgst_elem;

void pdgst_elem__gstMess (t_pdgst_elem*x, t_symbol*s, int argc, t_atom*argv);
void pdgst_elem__setParam(t_pdgst_elem*x, t_pdgst_property*prop, t_atom*ap);
void pdgst_elem__getParam(t_pdgst_elem*x, t_pdgst_property*prop);
void pdgst_elem__free(t_pdgst_elem*x);
void pdgst_elem__new (t_pdgst_elem*x, t_symbol*s);


/* pdgst_element.c */
void pdgst_element_setup(void);
int pdgst_element_setup_class(char*classname);



