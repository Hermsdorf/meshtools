/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_MODULE_H__
#define __PDB_MODULE_H__

#include "pdbGroup.h"
#include "pdbFRoutine.h"

class pdbModFunc : public pdbLoc {
public:
  pdbModFunc();
  pdbModFunc(const pdbFRoutine *f, pdbFile *file, int line, int col);

  const pdbFRoutine *func() const;
  void func(const pdbFRoutine* f);

private:
  const pdbFRoutine *fptr;
};

ostream& operator<<(ostream& ostr, const pdbModFunc& f);

/** \brief A class to define modules.
	*
	* pdbModule hold information about module and their functions.
	**/
class pdbModule : public pdbGroup {
public:
  
	/** pdbModule constructor.
		*
		* \param id unique idenfier.
		**/
	pdbModule(int id);
	
	/** pdbModule constructor.
		*
		* \param name the name of this module.
		* \param id unique idenfier.
		**/
  pdbModule(const string& name, int id);
  virtual ~pdbModule();

  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB* p);
  virtual void process(PDB* p);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

  typedef vector<pdbModFunc *> modfuncvec;

  const modfuncvec& funcMembers() const;

  void addFuncMember(pdbFRoutine *func, pdbFile *file, int line, int col);

private:
  modfuncvec fmem;
};

#ifndef NO_INLINE
#  include "pdbModule.inl"
#endif
#endif
