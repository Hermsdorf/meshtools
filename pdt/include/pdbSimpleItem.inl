/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

// class pdbLoc inlines
inline pdbLoc::pdbLoc ()
       : fptr(0), li(0), co(0) {}
inline pdbLoc::pdbLoc (const pdbFile *file, int line, int col)
       : fptr(file), li(line), co(col) {}

inline const pdbFile *pdbLoc::file() const { return fptr; }
inline void pdbLoc::file(const pdbFile* file) { fptr = file; }
inline int pdbLoc::line() const { return li; }
inline int pdbLoc::col() const { return co; }
inline void pdbLoc::setLoc(const pdbFile *file, int line, int col) {
  fptr = file; li = line; co = col;
}

inline bool pdbLoc::operator==(const pdbLoc& rhs) const {
  return file()==rhs.file() && line()==rhs.line() && col()==rhs.col();
}

inline pdbLoc::~pdbLoc () {}

// class pdbSimpleItem inlines
inline pdbSimpleItem::pdbSimpleItem(int id)
       : nam(""), i(id), nId(UNIQUE), flg(0) {}
inline pdbSimpleItem::pdbSimpleItem(const string& name, int id)
       : nam(name), i(id), nId(UNIQUE), flg(0) {}

inline pdbSimpleItem::~pdbSimpleItem() {}

inline const string& pdbSimpleItem::name() const { return nam; }
inline int pdbSimpleItem::id() const { return i; }
inline int pdbSimpleItem::newId() const { return nId; }

inline void pdbSimpleItem::name(const string& name) { nam = name; }
inline void pdbSimpleItem::id(int ii) { i = ii; }
inline void pdbSimpleItem::newId(int id) { nId = id; }

inline void pdbSimpleItem::process(PDB*) { /*DO NOTHING*/ }
inline pdbSimpleItem::dupl_t pdbSimpleItem::findDuplicate(pdbSimpleItem* rhs) {
    return ( name() == rhs->name() ? NEWDUPL : NODUPL );
}

  // simple flag / attribute implementation part i
inline int pdbSimpleItem::flag() const { return flg; }
inline void *pdbSimpleItem::attr() const { return atr; }
inline void pdbSimpleItem::flag(int flag) const { flg = flag; }
inline void pdbSimpleItem::attr(void *attr) const { atr = attr; }
