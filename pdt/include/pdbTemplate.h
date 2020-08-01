/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_TEMPLATE_H__
#define __PDB_TEMPLATE_H__

#include "pdbFatItem.h"
#include "pdbTemplateArg.h"

/** \brief Template Items class
 *
 * pdbFatItems include pdbTemplates, pdbNamespaces, and pdbTemplateItems.
 * pdbTemplateItems are entities that can be instantiated from templates.
 * Template items are pdbGroups, pdbGroupFields, pdbRoutines.
 **/
class pdbTemplate : public pdbFatItem {
public:
  
  /** pdbTemplate constructor
  	*
	* \param id an unique idenifier.
	**/
  pdbTemplate(int id);
  /** pdbTemplate constructor
  	*
	* \param name the name of the template.
	* \param id an unique idenifier.
	**/
  pdbTemplate(const string& name, int id);

  virtual const char *desc() const;
  virtual ostream& print(ostream& ostr) const;
  virtual void process(PDB *p);
  virtual void adjustPtrs(PDB *p);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

  typedef vector<pdbTemplateArg> targvec;

  /** the type of this template. **/
	templ_t kind() const;
  const string& text() const;
  /** the argument for this template. **/
  const targvec& arguments() const;

  const pdbCRoutine* funcProtoInst() const;
  const pdbClass* classProtoInst() const;
  const pdbType* statMemType() const;
  const pdbTemplate* declaration() const;
  const pdbTemplate* definition() const;
  /** a vector containing the argument for a routine. **/
	const targvec& speclArguments() const;

  void kind(templ_t kind);
  void text(const string& text);
  void addArgument(const pdbTemplateArg& targ);
  void funcProtoInst(const pdbCRoutine* ro);
  void classProtoInst(const pdbClass* cl);
  void statMemType(const pdbType* ty);
  void declaration(const pdbTemplate* t);
  void definition(const pdbTemplate* t);
  void addSpeclArgument(const pdbTemplateArg& targ);

private:
  templ_t knd;
  string  txt;
  targvec args;
  targvec sargs;
  union {
    const pdbCRoutine* rproto;
    const pdbClass*    cproto;
    const pdbType*     stype;
  };
  const pdbTemplate* def;
  const pdbTemplate* decl;
};

#ifndef NO_INLINE
#  include "pdbTemplate.inl"
#endif
#endif
