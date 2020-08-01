/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_SIMPLE_ITEM_H__
#define __PDB_SIMPLE_ITEM_H__

#include "pdb.h"
#include <limits.h>

/** \brief class that store the location of pdbItems in the source code. 
	*
	* pdbLoc describes source code locations which are characterized by a source
	* file, a line number (starting with 1), and a character position within this
	* line (starting with 0).
	**/
class pdbLoc {
public:
  /** A constructor without any argument **/
	pdbLoc ();
	/** A constructor 
		*
		* \param *file pointer to the pdbFile.
		*	\param line the line number of this location in the source file.
		* \param col the column number of this location in the source file.
		**/
  pdbLoc (const pdbFile *file, int line, int col);

	/** pointer of a pdbFile. **/
  const pdbFile *file() const;
  void file(const pdbFile* file);
	/** line number in source code. **/
  int line() const;
	/** line number in source code. **/
  int col() const;

  void setLoc(const pdbFile *file, int line, int col);
  bool operator==(const pdbLoc& rhs) const;
  ostream& printLoc(ostream& ostr) const;

  virtual ~pdbLoc ();

private:
  const pdbFile *fptr;
  int li;
  int co;
};

ostream& operator<<(ostream& ostr, const pdbLoc& l);

/** \brief The Root class is the pdb hierarchy
	* 
	* The root class of the hierarchy is pdbSimpleItem. pdbSimpleItems, and
	* therefore all items derived from it, have two attributes, their name and PDB
	* ID.
	**/
class pdbSimpleItem {
public:
  enum dupl_t { NODUPL, OLDDUPL, NEWDUPL };

  static const int UNIQUE = INT_MIN+1;
  static const int NOTDEL = INT_MIN;

/** \brief pdbSimpleItem constructor.
	*
	* \param id an unique identifier.
	**/
	pdbSimpleItem(int id);

/** \brief pdbSimpleItem constructor.
	*
	* \param name the name of this item. 
	* \param id an unique identifier.
	**/
	pdbSimpleItem(const string& name, int id);

  virtual ~pdbSimpleItem();

/** \brief Item's Name.
	*
	* String to hold the Item's name.
	**/
  const string& name() const;

/** \brief Unique ID.
  * 
	* Every pdb Item has a unqiue identifier.
	**/
  int id() const;

	int newId() const;

  void name(const string& name);
  void id(int ii);
  void newId(int id);

  virtual const char *desc() const = 0;
  virtual const char *attrPrefix() const;
  virtual ostream& print(ostream& ostr) const = 0;
  virtual void adjustPtrs(PDB*) = 0;

  virtual void process(PDB*);
  virtual dupl_t findDuplicate(pdbSimpleItem* rhs);

  // simple flag / attribute implementation part i
  int flag() const;
  void *attr() const;
  void flag(int flag) const;
  void attr(void *attr) const;

private:
  string nam;
  int    i;
  int    nId;

  // simple flag / attribute implementation part ii
  union {
    mutable int      flg;
    mutable void    *atr;
  };

};

ostream& operator<<(ostream& ostr, const pdbSimpleItem& i);

#ifndef NO_INLINE
#  include "pdbSimpleItem.inl"
#endif
#endif
