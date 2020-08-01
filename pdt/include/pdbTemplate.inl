/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbTemplate::pdbTemplate(int id)
       : pdbFatItem(id), knd(TE_NA), rproto(0), def(0), decl(0) {}
inline pdbTemplate::pdbTemplate(const string& name, int id)
       : pdbFatItem(name, id), knd(TE_NA), rproto(0), def(0), decl(0) {}

inline const char *pdbTemplate::desc() const { return "template"; }

inline pdbTemplate::templ_t pdbTemplate::kind() const { return knd; }
inline const string& pdbTemplate::text() const { return txt; }
inline const pdbTemplate::targvec& pdbTemplate::arguments() const {
  return args;
}
inline const pdbCRoutine* pdbTemplate::funcProtoInst() const {
  return (kind() == TE_FUNC || kind() == TE_MEMFUNC ? rproto : 0);
}
inline const pdbClass* pdbTemplate::classProtoInst() const {
  return (kind() == TE_CLASS || kind() == TE_MEMCLASS ? cproto : 0);
}
inline const pdbType* pdbTemplate::statMemType() const {
  return (kind() == TE_STATMEM ? stype : 0);
}
inline const pdbTemplate* pdbTemplate::declaration() const { return decl; }
inline const pdbTemplate* pdbTemplate::definition() const { return def; }
inline const pdbTemplate::targvec& pdbTemplate::speclArguments() const {
  return sargs;
}


inline void pdbTemplate::kind(templ_t kind) { knd = kind; }
inline void pdbTemplate::text(const string& text) { txt = text; }
inline void pdbTemplate::addArgument(const pdbTemplateArg& targ) {
  args.push_back(targ);
}
inline void pdbTemplate::funcProtoInst(const pdbCRoutine* ro) { rproto = ro; }
inline void pdbTemplate::classProtoInst(const pdbClass* cl) { cproto = cl; }
inline void pdbTemplate::statMemType(const pdbType* ty) { stype = ty; }
inline void pdbTemplate::declaration(const pdbTemplate* t) { decl = t; }
inline void pdbTemplate::definition(const pdbTemplate* t) { def = t; }
inline void pdbTemplate::addSpeclArgument(const pdbTemplateArg& targ) {
  sargs.push_back(targ);
}
