/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_GROUP_FIELD_H__
#define __PDB_GROUP_FIELD_H__

#ifdef _OLD_HEADER_
# include <vector.h>
#else
# include <vector>
  using std::vector;
#endif
#include "pdbTemplateItem.h"

/** \brief A class to define field within a group. **/
class pdbGroupField : public pdbTemplateItem {
public:
  pdbGroupField(const string& name);

  virtual const char *desc() const;
  virtual const char *attrPrefix() const;
  virtual ostream& print(ostream& ostr) const;

  /** the memory type of this field **/
	mem_t kind() const;
	/** the type of this field **/
	const pdbType *type() const;
	/** Is this a bit field? **/
  bool isBitField() const;
	/** Is this field mutable? **/
  bool isMutable() const;
	/** Is this a static constant **/
  bool isStaticConst() const;

  void kind(mem_t kd);
  void type(const pdbType *t);
  void isBitField(bool b);
  void isMutable(bool m);
  void isStaticConst(bool c);

private:
  mem_t          k;
  const pdbType *typ;
  bool           isBit;
  bool           isMut;
  bool           isSC;
};

#ifndef NO_INLINE
#  include "pdbGroupField.inl"
#endif
#endif
