/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_TEMPLATE_ITEM_H__
#define __PDB_TEMPLATE_ITEM_H__

#include "pdbFatItem.h"
#include "pdbTemplateArg.h"

/** \brief A class to record templates.
	*
	* pdbTemplateItems are entities that can be instantiated from templates.
	* Template items are pdbGroups, pdbGroupFields, pdbRoutines.
	**/
class pdbTemplateItem: public pdbFatItem {
public:
  pdbTemplateItem(int id);
  pdbTemplateItem(const string& name, int id);
  virtual ~pdbTemplateItem();

  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB* p);

  static const pdbTemplate* TE_UNKNOWN;

  typedef vector<pdbTemplateArg> targvec;

  /** pointer to the pdbTemplateItem  **/
	const pdbTemplate *isTemplate() const;
  /** Is this item Specialized? **/
	bool isSpecialized() const;
	/** speclized Arguments **/
  const targvec& speclArguments() const;

  void isTemplate(const pdbTemplate *templ);
  void isSpecialized(bool s);
  void addSpeclArgument(const pdbTemplateArg& targ);

protected:
  const pdbTemplate *tmpl;
  bool               isSpecl;
  targvec            sargs;
};

#ifndef NO_INLINE
#  include "pdbTemplateItem.inl"
#endif
#endif
