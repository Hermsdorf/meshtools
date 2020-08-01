/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_TYPE_H__
#define __PDB_TYPE_H__

#include "pdbItem.h"

/** \brief A class to represent C/C++ enumuations. 
	*
	* pdbEnum describes one element of an C / C++ enumeration type by its name
	* (identifier) and the corresponding integer value.
	**/
class pdbEnum {
public:
  /** A constructor 
		*
		* \param *id unique idenifier.
		* \param val integer value of the enumuation. **/
	pdbEnum(const char* id, int val);
  const string& id() const;
  int val() const;
private:
  string i;
  int    v;
};

/** \brief A class to represent Fortran bounds.
	*
	* pdbBounds is used to describe the bounds of one dimension of a Fortran
	* array. 
	**/
class pdbBounds {
public:
  /** A constructor
		*
		* \param low lower bound.
		* \param upp upper bound.
		**/
	pdbBounds(int low, int upp);
  /** lower bound. **/
	int lower() const;
	/** upper bound. **/
  int upper() const;
  static const int UNKNOWN;
private:
  int lo;
  int up;
};

/** \brief A class for arguments for routines.
	*
	* This class describes arguments given to pbdRoutines. It hold information
	* about these arguments and how they are given to Routines as parameters.
	**/
	
class pdbArg : public pdbLoc {
public:
  enum arg_t { ARG_NA=0, ARG_IN=1, ARG_OUT=2, ARG_INOUT=3,
	       ARG_OPT=4, ARG_DEF=8 };

  pdbArg(const pdbType *type, const string& name,
         pdbFile *file, int line, int col, int flags = ARG_NA);
  
	/** the abstract type of the argument. **/
	const pdbType* type() const;
	/** the name of the argument. **/
  const string& name() const;
  bool intentIn() const;        /*f90*/
  bool intentOut() const;       /*f90*/
  bool isOptional() const;      /*f90*/
  /** if the argument of this routine has a default parameter for this argument.
	 **/
	bool hasDefault() const;
  int flags() const;

  void type(const pdbType *t);
  void name(const string& n);
  void flags(int f);

  static const char *toName(arg_t v);
  static arg_t toArg(const char* v);

private:
  const pdbType *typ;
  string         nam;
  int            flg;
};

/** \brief A class to contain the abstract Type information.
	*
	*	This class hold the information about Fortran and C/C++ types both abstract
	*	and basic types.
	**/
class pdbType : public pdbItem {
public:
  pdbType(int id);
  pdbType(const string& name, int id);

  virtual const char *desc() const;
  virtual const char *attrPrefix() const;
  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB *p);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);
  virtual void process(PDB *p);

  typedef vector<qual_t> qualvec;
  typedef vector<pdbArg> argvec;
  typedef vector<const pdbType *> typevec;
  typedef vector<pdbEnum> enumvec;
  typedef vector<pdbBounds> boundvec;

  static const pdbType* NULLEXCEP;

	/** the abstract type of this argument. **/
  const pdbType* elementType() const;
  /** the float type of this argument. **/
	float_t floatType() const;
	/** the integer type of this argument. **/
  int_t integerType() const;
  /** the kind of the type.**/
	type_t kind() const;
  const qualvec& qualifiers() const;
  const pdbType* pointedToType() const;
  const pdbType* referencedType() const;
  bool isSigned() const;
  const pdbType* baseType() const;
  const pdbGroup* isGroup() const;
  const pdbType* returnType() const;
  const argvec& arguments() const;
  bool hasEllipsis() const;
  bool oldStyleDecl() const;  /*C*/
  const pdbType* memberPointerType() const;
  const pdbClass* memberPointerClass() const;
  bool isConst() const;
  int numElements() const;  /** for C arrays and f90 characters **/
  const typevec& exceptionSpec() const;
  const enumvec& enumeration() const;
  shape_t shape() const;              /*f90*/
  int rank() const;                   /*f90*/
  const boundvec& bounds() const;     /*f90*/
  bool isStaticArray() const;  /*C99*/

  void elementType(const pdbType* elem);
  void floatType(float_t ftype);
  void integerType(int_t itype);
  void kind(type_t type);
  void addQualifier(qual_t qual);
  void pointedToType(const pdbType* ptr);
  void referencedType(const pdbType* ptr);
  void isSigned(bool s);
  void baseType(const pdbType* base);
  void isGroup(const pdbGroup* gr);
  void returnType(const pdbType* type);
  void addArgument(const pdbType* type, const string& name, 
                   pdbFile *file, int line, int col, int flags);
  void hasEllipsis(bool e);
  void oldStyleDecl(bool o);  /*C*/
  void memberPointerType(const pdbType* mptype);
  void memberPointerClass(const pdbClass* mpclass);
  void isConst(bool c);
  void numElements(int n);
  void addExceptionSpec(const pdbType* c);
  void addEnumeration(const pdbEnum& e);
  void shape(shape_t s);             /*f90*/
  void rank(int r);                  /*f90*/
  void addBound(int low, int high);  /*f90*/
  void isStaticArray(bool s);  /*C99*/

  // UPC
  bool isShared() const;
  int  blockSize() const;
  bool isRelaxed() const;
  bool isStrict() const;

  void isShared(bool s);
  void blockSize(int b);
  void isRelaxed(bool r);
  void isStrict(bool s);

private:
  const pdbType  *elemTy;
  float_t         fltTy;
  int_t           intTy;
  type_t          knd;
  shape_t         shp;
  bool            snd;
  bool            ell;
  bool            cst;
  bool            sta;
  bool            oldSD;
  int             numElem;
  const pdbType  *ptrTy;
  const pdbType  *baseTy;
  const pdbGroup *groupTy;
  const pdbType  *retTy;
  const pdbType  *memPtrTy;
  const pdbClass *memPtrCl;
  argvec          args;
  qualvec         quals;
  typevec         exSpec;
  enumvec         enums;
  boundvec        bnds;
  bool            shared;
  int             blocksize;
  bool            relaxed;
  bool            strict;
};

#ifndef NO_INLINE
#  include "pdbType.inl"
#endif
#endif
