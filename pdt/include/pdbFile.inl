/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

// class pdbComment inlines
inline pdbComment::pdbComment(int id) : i(id), knd(PDB::LA_NA) {}

inline int pdbComment::id() const { return i; }
inline PDB::lang_t pdbComment::kind() const { return knd; }
inline const pdbLoc& pdbComment::cmtBegin() const { return bg; }
inline const pdbLoc& pdbComment::cmtEnd() const { return ed; }
inline const string& pdbComment::text() const { return txt; }
 
inline void pdbComment::kind(PDB::lang_t kind) { knd = kind; }
inline void pdbComment::cmtBegin(const pdbLoc& loc) { bg = loc; }
inline void pdbComment::cmtEnd(const pdbLoc& loc) { ed = loc; }
inline void pdbComment::text(const string& text) { txt = text; }

// class pdbFile inlines
inline pdbFile::pdbFile(int id)
       : pdbSimpleItem(id), ncall(0), sys(false) {}
inline pdbFile::pdbFile(const string& name, int id)
       : pdbSimpleItem(name, id), ncall(0), sys(false) {}

inline const char *pdbFile::desc() const { return "source file"; }

inline pdbFile::incvec& pdbFile::includes() { return incls; }
inline const pdbFile::incvec& pdbFile::includes() const { return incls; }
inline int pdbFile::numCalled() const { return ncall; }
inline bool pdbFile::isSystemFile() const { return sys; }
inline const pdbFile::cmtvec& pdbFile::comments() const { return cmts; }

inline void pdbFile::addInclude(pdbFile *inc) {
  incls.push_back(inc);
  inc->ncall++;
}
inline void pdbFile::isSystemFile(bool s) { sys = s; }
