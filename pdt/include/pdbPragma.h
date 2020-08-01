/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_PRAGMA_H__
#define __PDB_PRAGMA_H__

#include "pdbItem.h"

class pdbPragma : public pdbItem {
public:
  pdbPragma(int id);
  pdbPragma(const string& name, int id);

  virtual const char *desc() const;
  virtual ostream& print(ostream& ostr) const;
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

	string kind() const;
  const pdbLoc& prBegin() const;
  const pdbLoc& prEnd() const;
  const string& text() const;

  void kind(const string& kind);
  void prBegin(const pdbLoc& loc);
  void prEnd(const pdbLoc& loc);
  void text(const string& text);

private:
  string  knd;
  pdbLoc  bg;
  pdbLoc  ed;
  string  txt;
};

#ifndef NO_INLINE
#  include "pdbPragma.inl"
#endif
#endif
