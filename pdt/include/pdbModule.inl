/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

// class pdbModFunc inlines
inline pdbModFunc::pdbModFunc()
       : fptr(0) {}
inline pdbModFunc::pdbModFunc(const pdbFRoutine *f,
                              pdbFile *file, int line, int col)
       : pdbLoc(file, line, col), fptr(f) {}

inline const pdbFRoutine *pdbModFunc::func() const { return fptr; }
inline void pdbModFunc::func(const pdbFRoutine* f) { fptr = f; }

// class pdbModule inlines
inline pdbModule::pdbModule(int id)
       : pdbGroup(id) {}
inline pdbModule::pdbModule(const string& name, int id)
       : pdbGroup(name, id) {}

inline const pdbModule::modfuncvec& pdbModule::funcMembers() const {
  return fmem;
}

inline void pdbModule::addFuncMember(pdbFRoutine *func,
                                     pdbFile *file, int line, int col) {
  fmem.push_back(new pdbModFunc(func, file, line, col));
}
