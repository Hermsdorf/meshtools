/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbNamespace::pdbNamespace(int id)
       : pdbFatItem(id), alias(0) {}
inline pdbNamespace::pdbNamespace(const string& name, int id)
       : pdbFatItem(name, id), alias(0) {}

inline const char *pdbNamespace::desc() const { return "namespace"; }

inline const pdbNamespace::memvec& pdbNamespace::members() const { return mems; }
inline const pdbNamespace* pdbNamespace::isAlias() const { return alias; }

inline void pdbNamespace::addMember(pdbItem* item) { mems.push_back(item); }
inline void pdbNamespace::isAlias(const pdbNamespace* n) { alias = n; }
