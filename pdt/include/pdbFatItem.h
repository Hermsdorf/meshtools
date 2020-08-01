/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_FAT_ITEM_H__
#define __PDB_FAT_ITEM_H__

#include "pdbItem.h"

/** \brief A class for items spanning serval lines of code.
	*
	* pdbItems are pdbPragmas, pdbMacros, pdbTypes, or so-called fat items.
	* pdbFatItems have a header and a body, and attributes describing the source
	* location of these parts.
	**/
class pdbFatItem: public pdbItem {
public:

	/** pdbFatItem constructor
		*
		* \param id an unique idenifier.
		**/
	pdbFatItem(int id);

	/** pdbFatItem constructor
		*
		* \param name the name of the item.
		* \param id an unique idenifier.
		**/
  pdbFatItem(const string& name, int id);
  virtual ~pdbFatItem();
  
  virtual ostream& print(ostream& ostr) const;
  
  void headBegin(const pdbLoc& loc);
  void headEnd  (const pdbLoc& loc);
  void bodyBegin(const pdbLoc& loc);
  void bodyEnd  (const pdbLoc& loc);

	/** the line number in the source code that begins the header. **/
  const pdbLoc& headBegin() const;
	/** the line number in the source code that ends the header. **/
  const pdbLoc& headEnd  () const;
	/** the line number in the source code that begins the body. **/
  const pdbLoc& bodyBegin() const;
	/** the line number in the source code that ends the body. **/
  const pdbLoc& bodyEnd  () const;

  virtual void adjustPtrs(PDB*);

protected:
  pdbLoc pos[4];
};

#ifndef NO_INLINE
#  include "pdbFatItem.inl"
#endif
#endif
