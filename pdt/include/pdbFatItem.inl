/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbFatItem::pdbFatItem(int id) : pdbItem(id) {}
inline pdbFatItem::pdbFatItem(const string& name, int id) : pdbItem(name, id) {}
inline pdbFatItem::~pdbFatItem() {}
  
inline void pdbFatItem::headBegin(const pdbLoc& loc) { pos[0] = loc; }
inline void pdbFatItem::headEnd  (const pdbLoc& loc) { pos[1] = loc; }
inline void pdbFatItem::bodyBegin(const pdbLoc& loc) { pos[2] = loc; }
inline void pdbFatItem::bodyEnd  (const pdbLoc& loc) { pos[3] = loc; }

inline const pdbLoc& pdbFatItem::headBegin() const { return pos[0]; }
inline const pdbLoc& pdbFatItem::headEnd  () const { return pos[1]; }
inline const pdbLoc& pdbFatItem::bodyBegin() const { return pos[2]; }
inline const pdbLoc& pdbFatItem::bodyEnd  () const { return pos[3]; }
