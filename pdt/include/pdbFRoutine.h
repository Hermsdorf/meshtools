/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_FROUTINE_H__
#define __PDB_FROUTINE_H__

#include "pdbRoutine.h"

class pdbFRoutine : public pdbRoutine {
public:
  pdbFRoutine(int id);
  pdbFRoutine(const string& name, int id);

  virtual ~pdbFRoutine();

  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB* p);

  typedef vector<pdbFRoutine *> implvec;

  /** Is this routine recursive? **/
	bool isRecursive() const;
	/** Is this routine elemental? **/
  bool isElemental() const;
	/** Do this routine have explicit interface? **/
  bool hasExplicitInterface() const;
	/** pointer to this routine parent routine. **/
  const pdbFRoutine* parentRoutine() const;
  const pdbFRoutine* isAliasFor() const;
  const implvec& implementedBy() const;
  const locvec& stopLocations() const;

  void isRecursive(bool r);
  void isElemental(bool e);
  void hasExplicitInterface(bool i);
  void parentRoutine(const pdbFRoutine* p);
  void isAliasFor(const pdbFRoutine* a);
  void addImplementation(pdbFRoutine* i);
  void addStopLocation(pdbFile *file, int line, int col);

  virtual void process(PDB *p);

private:
  bool               isRec;
  bool               isElem;
  bool               eIntface;
  const pdbFRoutine* prnt;
  const pdbFRoutine* alas;
  implvec            impls;
  locvec             stops;
};

#ifndef NO_INLINE
#  include "pdbFRoutine.inl"
#endif
#endif
