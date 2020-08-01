/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbTemplateArg::pdbTemplateArg(targ_t kind, bool specialization)
       : knd(kind), typ(0), deftyp(0), specl(specialization) {}

inline pdbTemplateArg::targ_t pdbTemplateArg::kind() const { return knd; }
inline const string& pdbTemplateArg::name() const { return nam; }
inline const pdbType* pdbTemplateArg::type() const {
  return (knd!=TA_TEMPL ? typ : 0);
}
inline const string& pdbTemplateArg::value() const {
  return (specl ? defval : EMPTY);
}
inline const pdbTemplate* pdbTemplateArg::templateArg() const {
  return (knd==TA_TEMPL ? tmpl : 0);
}
inline const pdbType* pdbTemplateArg::defaultType() const {
  return (knd==TA_TYPE ? deftyp : 0);
}
inline const pdbTemplate* pdbTemplateArg::defaultTemplateArg() const {
  return (knd==TA_TEMPL ? deftmpl : 0);
}
inline const string& pdbTemplateArg::defaultValue() const {
  return (specl ? EMPTY : defval);
}
inline bool pdbTemplateArg::isSpecialization() const { return specl; }

inline void pdbTemplateArg::name(const string& name) { nam = name; }
inline void pdbTemplateArg::type(const pdbType* type) { typ = type; }
inline void pdbTemplateArg::templateArg(const pdbTemplate* templ) {
  tmpl = templ;
}
inline void pdbTemplateArg::value(const string& val) { defval = val; }
inline void pdbTemplateArg::defaultType(const pdbType* type) { deftyp = type; }
inline void pdbTemplateArg::defaultTemplateArg(const pdbTemplate* templ) {
  deftmpl = templ;
}
inline void pdbTemplateArg::defaultValue(const string& val) { defval = val; }
