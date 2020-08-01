/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbTemplateItem::pdbTemplateItem(int id)
       : pdbFatItem(id), tmpl(0), isSpecl(false) {}
inline pdbTemplateItem::pdbTemplateItem(const string& name, int id)
       : pdbFatItem(name, id), tmpl(0), isSpecl(false) {}
inline pdbTemplateItem::~pdbTemplateItem() {}

inline const pdbTemplate *pdbTemplateItem::isTemplate() const { return tmpl; }
inline bool pdbTemplateItem::isSpecialized() const { return isSpecl; }
inline const pdbTemplateItem::targvec& pdbTemplateItem::speclArguments() const {
  return sargs;
}

inline void pdbTemplateItem::isTemplate(const pdbTemplate *templ) {
  tmpl = templ;
}
inline void pdbTemplateItem::isSpecialized(bool s) { isSpecl = s; }
inline void pdbTemplateItem::addSpeclArgument(const pdbTemplateArg& targ) {
  sargs.push_back(targ);
}
