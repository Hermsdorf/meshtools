/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_CROUTINE_H__
#define __PDB_CROUTINE_H__

#include "pdbRoutine.h"

class pdbCRoutine : public pdbRoutine {
public:
  pdbCRoutine(int id);
  pdbCRoutine(const string& name, int id);

  virtual ostream& print(ostream& ostr) const;

  virt_t virtuality() const;
  bool covariantReturnVirtualOverride() const;
	/**  Is this routine inlined? **/
	bool isInline() const;
	/** Is this routine compiler generated? **/
  bool isCompilerGenerated() const;
  bool isExplicitCtor() const;
	/** Is this routine static? **/
  bool isStatic() const;
	/** Is this routine virtual? **/
  bool isVirtual() const;

  void virtuality(virt_t v);
  void covariantReturnVirtualOverride(bool c);
  void isInline(bool i);
  void isCompilerGenerated(bool c);
  void isExplicitCtor(bool e);
  void isStatic(bool s);

  virtual void process(PDB *p);

private:
  virt_t             virt;
  bool               crvo;
  bool               inl;
  bool               isCGen;
  bool               isEx;
  bool               isStat;
};

#ifndef NO_INLINE
#  include "pdbCRoutine.inl"
#endif
#endif
