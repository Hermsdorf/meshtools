/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbFRoutine::pdbFRoutine(int id)
       : pdbRoutine(id),
         isRec(false), isElem(false), eIntface(false), prnt(0), alas(0) {}
inline pdbFRoutine::pdbFRoutine(const string& name, int id)
       : pdbRoutine(name, id),
         isRec(false), isElem(false), eIntface(false), prnt(0), alas(0) {}

inline bool pdbFRoutine::isRecursive() const { return isRec; }
inline bool pdbFRoutine::isElemental() const { return isElem; }
inline bool pdbFRoutine::hasExplicitInterface() const { return eIntface; }
inline const pdbFRoutine* pdbFRoutine::parentRoutine() const { return prnt; }
inline const pdbFRoutine* pdbFRoutine::isAliasFor() const { return alas; }
inline const pdbFRoutine::implvec& pdbFRoutine::implementedBy() const {
  return impls;
}
inline const pdbRoutine::locvec& pdbFRoutine::stopLocations() const {
  return stops;
}

inline void pdbFRoutine::isRecursive(bool r) { isRec = r; }
inline void pdbFRoutine::isElemental(bool e) { isElem = e; }
inline void pdbFRoutine::hasExplicitInterface(bool i) { eIntface = i; }
inline void pdbFRoutine::parentRoutine(const pdbFRoutine* p) { prnt = p; }
inline void pdbFRoutine::isAliasFor(const pdbFRoutine* a) { alas = a; }
inline void pdbFRoutine::addImplementation(pdbFRoutine* i) {
  impls.push_back(i);
}
inline void pdbFRoutine::addStopLocation(pdbFile *file, int line, int col) {
  stops.push_back(new pdbLoc(file, line, col));
}


