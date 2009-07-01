
typedef struct _pdgst_prop {
  struct _pdgst_prop*next;
  t_symbol*name;
  int flags;
  GType type;
} t_pdgst_property;

t_pdgst_property*pdgst_addproperty(t_pdgst_property*props, GParamSpec*param);
t_pdgst_property*pdgst_getproperty(t_pdgst_property*props, t_symbol*name);
void pdgst_killproperties(t_pdgst_property*props);
