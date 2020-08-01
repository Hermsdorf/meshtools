/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_FILE_H__
#define __PDB_FILE_H__

#include "pdbSimpleItem.h"

/** A class to represent comment in the source file.
	*
	* pdbComment represents a comment in a source file. Comments are numbered 0 to
	* N inside one file. The kind, its exact location, and the comment text is
	* available.
	**/
class pdbComment {
public:
 	/** A constructor
 		*
		* \param id unique identifier.
		**/
 pdbComment(int id);

  int id() const;
  /** returns the language of this source file.**/
	PDB::lang_t kind() const;
  /** location in the source file where the comment begans. **/
	const pdbLoc& cmtBegin() const;
	/** location in the source file where the comment ends. **/
  const pdbLoc& cmtEnd() const;
	/** the contents of the comment. **/
	const string& text() const;

  void kind(PDB::lang_t kind);
  void cmtBegin(const pdbLoc& loc);
  void cmtEnd(const pdbLoc& loc);
  void text(const string& text);

  friend class pdbFile;

private:
  int         i;
  PDB::lang_t knd;
  pdbLoc      bg;
  pdbLoc      ed;
  string      txt;
};

ostream& operator<<(ostream& ostr, const pdbComment& c);

class pdbFile : public pdbSimpleItem {
public:

	/** A pdbFile constructor
		*
		* \param id unique identifier.
		**/
  pdbFile(int id);
	/** A pdbFile constructor
		*
		* \param name the name of the pdb file.
		* \param id unique identifier.
  	**/
	pdbFile(const string& name, int id);
  virtual ~pdbFile();

  typedef vector<pdbFile *> incvec;
  typedef vector<pdbComment *> cmtvec;

  virtual const char *desc() const;
  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB* p);
  virtual void process(PDB* p);

  incvec& includes();
	/** a vector of files that are included in the source file. **/
  const incvec& includes() const;
  int numCalled() const;
  bool isSystemFile() const;

	/** a vector containing the comments within the source files. **/
	const cmtvec& comments() const;

  void addInclude(pdbFile *inc);
  void isSystemFile(bool s);
  pdbComment* addComment(int id);

private:
  incvec incls;
  cmtvec cmts;
  int    ncall;
  bool   sys;
};

#ifndef NO_INLINE
#  include "pdbFile.inl"
#endif
#endif
