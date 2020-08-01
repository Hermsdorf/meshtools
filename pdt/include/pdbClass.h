/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_CLASS_H__
#define __PDB_CLASS_H__

#include "pdbGroup.h"
#include "pdbCRoutine.h"

/** \brief A class to define a super class.
	*
	* pdbBase describes a base class (super class) of a pdbClass. It provides
	* methods to ask for the base class, its access mode (e.g., public or
	* private), and whether the derivation was virtual.
	**/
class pdbBase : public pdbLoc {
public:
  
	/** A constructor with no arguments **/
	pdbBase();
	/** A constructor
		*
		* \param v the virtual type of this class. 
		* \param a the access type of this class.
		* \param *base a pointer to the base class.
		* \param *file a pointer to the source file.
		* \param line the line number where this class begans.
		* \param col the column number where this class begans.
		**/
  pdbBase(pdbItem::virt_t v, pdbItem::access_t a, const pdbClass *base,
          pdbFile *file, int line, int col);
 
  /** the virtual type of this class **/ 
	pdbItem::virt_t virtuality() const;
	/** the access type of this class **/
  pdbItem::access_t access() const;
	/** a pointer to the base class **/
  const pdbClass *base() const;
	/** is this class derivation virtual? **/
  bool isVirtual() const;

  void virtuality(pdbItem::virt_t v);
  void access(pdbItem::access_t a);
  void base(const pdbClass* base);

private:
  pdbItem::virt_t   virt;
  pdbItem::access_t acs;
  const pdbClass   *bptr;
};

class pdbFriendFunc : public pdbLoc {
public:
  pdbFriendFunc();
  pdbFriendFunc(const pdbCRoutine *f, pdbFile *file, int line, int col);

  const pdbCRoutine *friendFunc() const;
  void friendFunc(const pdbCRoutine* f);

private:
  const pdbCRoutine *fuptr;
};

class pdbFriendClass : public pdbLoc {
public:
  pdbFriendClass();
  pdbFriendClass(const pdbClass *c, pdbFile *file, int line, int col);

  const pdbClass *friendClass() const;
  void friendClass(const pdbClass* c);

private:
  const pdbClass *cptr;
};

class pdbMethod : public pdbLoc {
public:
  pdbMethod();
  pdbMethod(const pdbCRoutine *f, pdbFile *file, int line, int col);

  const pdbCRoutine *func() const;
  void func(const pdbCRoutine* f);

private:
  const pdbCRoutine *fptr;
};

ostream& operator<<(ostream& ostr, const pdbBase& b);
ostream& operator<<(ostream& ostr, const pdbFriendFunc& f);
ostream& operator<<(ostream& ostr, const pdbFriendClass& c);
ostream& operator<<(ostream& ostr, const pdbMethod& f);

/** \brief This class defines the attributes for a class.
	*
	*	This class defines the genric class within a class hierarchy, and its associated
	*	friend classes.
	**/
class pdbClass : public pdbGroup {
public:
  pdbClass(int id);
  pdbClass(const string& name, int id);
  virtual ~pdbClass();

  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB* p);
  virtual void process(PDB* p);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

  typedef vector<pdbBase *> basevec;
  typedef vector<pdbClass *> classvec;
  typedef vector<pdbMethod *> methodvec;
  typedef vector<pdbFriendClass *> friendclassvec;
  typedef vector<pdbFriendFunc *> friendfuncvec;

  const basevec& baseClasses() const;
  classvec& derivedClasses();
  /** a vector listing the classes derived from this one. **/
  const classvec& derivedClasses() const;
  /** a vector of methods. **/
  const methodvec& methods() const;
  bool isABC() const;
  /** a vector of friendly classes. **/
  const friendclassvec& friendClasses() const;
  /** a vector of friendly Routines. **/
  const friendfuncvec& friendRoutines() const;

  void addBaseClass(pdbItem::virt_t virt, pdbItem::access_t acs,
                    const pdbClass *bclass, pdbFile *file, int line, int col);
  void addDerivedClass(pdbClass *dclass);
  void addMethod(pdbCRoutine *func, pdbFile *file, int line, int col);
  void addFriendClass(pdbClass *f, pdbFile *file, int line, int col);
  void addFriendRoutine(pdbCRoutine *f, pdbFile *file, int line, int col);

private:
  basevec            base;
  classvec           drvd;
  methodvec          fmem;
  bool               abc;
  friendclassvec     fcl;
  friendfuncvec      fro;
};

#ifndef NO_INLINE
#  include "pdbClass.inl"
#endif
#endif
