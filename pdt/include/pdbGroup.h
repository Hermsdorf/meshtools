/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_GROUP_H__
#define __PDB_GROUP_H__

#ifdef _OLD_HEADER_
# include <vector.h>
#else
# include <vector>
  using std::vector;
#endif
#include "pdbTemplateItem.h"

/** \brief pdbGroups repersenting abstract data types.
	* 
	* Groups represent abstract data types, i.e. collections of public and private
	* members. Members are divided into data members (described by pdbGroupFields)
	* and member functions/methods (described by pdbRoutines). The different kind
	* of groups are Fortran 90 derived types or modules, or C and C++ structs,
	* unions, or classes. 
	**/
class pdbGroup : public pdbTemplateItem {
public:
  /** \brief pdbGroup constructor.
	  * 
	  * \param id an unique idenifier.
	  **/   
	pdbGroup(int id);
	
  /** \brief pdbGroup constructor.
	  *
	  * \param name the name of the item.
	  * \param id an unique idenifier.
	  **/   													
	pdbGroup(const string& name, int id);
  
	virtual ~pdbGroup();

  virtual const char *desc() const;
  virtual ostream& print(ostream& ostr) const;

  typedef vector<pdbGroupField *> fieldvec;

  /** A list of member of this group. **/
		const fieldvec& dataMembers() const;
  
	/** \brief the type of this group.
		*
		* kind() specifies the abstract type of this group
		* wheather F90 derived types or modules, C/C++ structs, unions, or classes.
		**/
	group_t kind() const;

  void addDataMember(pdbGroupField *field);
  void kind(group_t kd);

protected:
  fieldvec  dmem;
  group_t   knd;
};

#ifndef NO_INLINE
#  include "pdbGroup.inl"
#endif
#endif
