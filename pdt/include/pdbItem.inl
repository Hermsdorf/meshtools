/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbItem::pdbItem(int id) : pdbSimpleItem(id), lc(),
     acs(AC_NA), gptr(0), nptr(0), full("") {}
inline pdbItem::pdbItem(const string& name, int id) : pdbSimpleItem(name, id),
     lc(), acs(AC_NA), gptr(0), nptr(0), full("") {}
inline pdbItem::~pdbItem() {}

inline const string& pdbItem::fullName() const { return full; }
inline pdbItem::access_t pdbItem::access() const { return acs; }
inline const pdbGroup* pdbItem::parentGroup()  const { return gptr; }
inline pdbLoc& pdbItem::location() { return lc; }
inline const pdbLoc& pdbItem::location() const { return lc; }
inline const pdbNamespace* pdbItem::parentNSpace() const { return nptr; }

inline void pdbItem::fullName(const string& name) { full = name; }
inline void pdbItem::access(access_t attr) { acs = attr; }
inline void pdbItem::parentGroup(const pdbGroup* pgroup)  { gptr = pgroup; }
inline void pdbItem::location(const pdbLoc& loc) { lc = loc; }
inline void pdbItem::parentNSpace(const pdbNamespace* nspace) { nptr = nspace; }
