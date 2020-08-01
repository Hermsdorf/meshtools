/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_MACRO_H__
#define __PDB_MACRO_H__

#include "pdbItem.h"

class pdbMacro : public pdbItem {
public:
  pdbMacro(int id);
  pdbMacro(const string& name, int id);

  virtual const char *desc() const;
  virtual ostream& print(ostream& ostr) const;
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

  /** the type macro. **/
	macro_t kind() const;
  const string& text() const;

  void kind(macro_t kind);
  void text(const string& text);

private:
  macro_t knd;
  string  txt;
};

#ifndef NO_INLINE
#  include "pdbMacro.inl"
#endif
#endif
