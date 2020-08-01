/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

// class pdbBase inlines
inline pdbBase::pdbBase()
       : virt(pdbItem::VI_NO), acs(pdbItem::AC_NA), bptr(0) {}
inline pdbBase::pdbBase(pdbItem::virt_t v, pdbItem::access_t a,
                        const pdbClass *base,
                        pdbFile *file, int line, int col)
       : pdbLoc(file, line, col), virt(v), acs(a), bptr(base) {}
 
inline pdbItem::virt_t pdbBase::virtuality() const { return virt; }
inline pdbItem::access_t pdbBase::access() const { return acs; }
inline const pdbClass *pdbBase::base() const { return bptr; }
inline bool pdbBase::isVirtual() const { return virt == pdbItem::VI_VIRT; }

inline void pdbBase::virtuality(pdbItem::virt_t v) { virt = v; }
inline void pdbBase::access(pdbItem::access_t a) { acs = a; }
inline void pdbBase::base(const pdbClass* base) { bptr = base; }

// class pdbFriendFunc inlines
inline pdbFriendFunc::pdbFriendFunc()
       : fuptr(0) {}
inline pdbFriendFunc::pdbFriendFunc(const pdbCRoutine *f,
                                    pdbFile *file, int line, int col)
       : pdbLoc(file, line, col), fuptr(f) {}

inline const pdbCRoutine *pdbFriendFunc::friendFunc() const { return fuptr; }
inline void pdbFriendFunc::friendFunc(const pdbCRoutine* f) { fuptr = f; }

// class pdbFriendClass inlines
inline pdbFriendClass::pdbFriendClass()
       : cptr(0) {}
inline pdbFriendClass::pdbFriendClass(const pdbClass *c,
                                      pdbFile *file, int line, int col)
       : pdbLoc(file, line, col), cptr(c) {}

inline const pdbClass *pdbFriendClass::friendClass() const { return cptr; }
inline void pdbFriendClass::friendClass(const pdbClass* c) { cptr = c; }

// class pdbMethod inlines
inline pdbMethod::pdbMethod()
       : fptr(0) {}
inline pdbMethod::pdbMethod(const pdbCRoutine *f,
                            pdbFile *file, int line, int col)
       : pdbLoc(file, line, col), fptr(f) {}

inline const pdbCRoutine *pdbMethod::func() const { return fptr; }
inline void pdbMethod::func(const pdbCRoutine* f) { fptr = f; }

// class pdbClass inlines
inline pdbClass::pdbClass(int id)
       : pdbGroup(id), abc(false) {}
inline pdbClass::pdbClass(const string& name, int id)
       : pdbGroup(name, id), abc(false) {}

inline const pdbClass::basevec& pdbClass::baseClasses() const { return base; }
inline pdbClass::classvec& pdbClass::derivedClasses() { return drvd; }
inline const pdbClass::classvec& pdbClass::derivedClasses() const {
  return drvd;
}
inline const pdbClass::methodvec& pdbClass::methods() const { return fmem; }
inline bool pdbClass::isABC() const { return abc; }
inline const pdbClass::friendclassvec& pdbClass::friendClasses() const {
  return fcl;
}
inline const pdbClass::friendfuncvec& pdbClass::friendRoutines() const {
  return fro;
}

inline void pdbClass::addBaseClass(pdbItem::virt_t virt, pdbItem::access_t acs,
                     const pdbClass *bclass, pdbFile *file, int line, int col) {
  base.push_back(new pdbBase(virt, acs, bclass, file, line, col));
}
inline void pdbClass::addDerivedClass(pdbClass *dclass) {
  drvd.push_back(dclass);
}
inline void pdbClass::addMethod(pdbCRoutine *func,
                                pdbFile *file, int line, int col) {
  fmem.push_back(new pdbMethod(func, file, line, col));
}
inline void pdbClass::addFriendClass(pdbClass *f,
                                     pdbFile *file, int line, int col) {
  fcl.push_back(new pdbFriendClass(f, file, line, col));
}
inline void pdbClass::addFriendRoutine(pdbCRoutine *f,
                                       pdbFile *file, int line, int col) { 
  fro.push_back(new pdbFriendFunc(f, file, line, col));
}
