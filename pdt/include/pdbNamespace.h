/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_NAMESPACE_H__
#define __PDB_NAMESPACE_H__

#include "pdbFatItem.h"

/** \brief A class to define the namespace.
	*
	*	This class records the members of each namespace.
	**/
class pdbNamespace : public pdbFatItem {
public:
  
  /** pdbNamespace constructor
  	*
		* \param id unique idenfier.
		**/
  pdbNamespace(int id);
  /** pdbNamespace constructor
  	*
		* \param name this namespace's name.
		* \param id unique idenfier.
		**/
  pdbNamespace(const string& name, int id);

  virtual const char *desc() const;
  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB* p);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);


  typedef vector<pdbItem *> memvec;

	/** A vector of members of this namespace. **/
  const memvec& members() const;
	/** Pointer to alias namespace. **/
  const pdbNamespace* isAlias() const;

  void addMember(pdbItem* item);
  void isAlias(const pdbNamespace* n);

private:
  const pdbNamespace *alias;
  memvec              mems;
};

#ifndef NO_INLINE
#  include "pdbNamespace.inl"
#endif
#endif
