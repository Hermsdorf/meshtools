/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_ROUTINE_H__
#define __PDB_ROUTINE_H__

/* level 1 statements are ST_FDO */
#define PDB_FORTRAN_EXTENDED_STATEMENTS_LEVEL_1 1

/* level 2 statements are ST_FEXIT */
#define PDB_FORTRAN_EXTENDED_STATEMENTS_LEVEL_2 1 

#include "pdbTemplateItem.h"

/** \brief class for storing statements within the source file.
	*
	* This class stores information about the statements (or block of statements)
	* within a source file. This class also keeps track of the next statement
	* within there own context as well as the next statement in childern contexts. 
	*
	* \example stmt.cc
	**/
class pdbStmt {
public:
  /** statements **/
  enum stmt_t { ST_NA, ST_SWITCH, ST_CASE, ST_INIT, ST_RETURN, ST_IF,
                ST_EMPTY, ST_FOR, ST_GOTO, ST_CONTINUE, ST_BREAK,
                ST_LABEL, ST_BLOCK, ST_ASM,  ST_EXPR, ST_ASSIGN, ST_THROW,
                ST_WHILE, ST_DO, ST_TRY, ST_CATCH, ST_DECL,
                ST_SET_VLA_SIZE, ST_VLA_DECL, ST_VLA_DEALLOC, 
                ST_FALLOCATE, ST_FASSIGN, ST_FIO, ST_FDO, ST_FDEALLOCATE, 
                ST_FRETURN, ST_FIF, ST_FGOTO, ST_FSINGLE_IF, ST_FSTOP, 
                ST_FLABEL, ST_FEXIT, ST_FCYCLE, ST_FARITHIF, ST_FENTRY, 
                ST_FPAUSE, ST_FLABELASSIGN, ST_FPOINTERASSIGN, ST_FSELECT, 
                ST_FCASE, ST_FWHERE, ST_FFORALL, ST_FCALL, ST_FCONTINUE, 
                ST_UPC_FORALL, ST_UPC_BARRIER, ST_UPC_FENCE, ST_UPC_NOTIFY, 
                ST_UPC_WAIT, ST_FFLUSH, ST_FWAIT, ST_FLOCK, ST_FUNLOCK, 
                ST_FSYNC_ALL, ST_FSYNC_IMAGES, ST_FSYNC_MEMORY, 
                ST_FSELECT_TYPE, ST_FBLOCK, ST_FASSIGN_CALL, ST_FCOMPCALL, 
                ST_FERROR_STOP, ST_FEND_BLOCK, ST_FCALL_PPC,
		ST_FOMP_PARALLEL, ST_FOMP_PARALLEL_SECTIONS, ST_FOMP_SECTIONS,
		ST_FOMP_ORDERED, ST_FOMP_CRITICAL, ST_FOMP_MASTER, 
		ST_FOMP_SINGLE, ST_FOMP_TASK, ST_FOMP_DO, ST_FOMP_PARALLEL_DO,
		ST_FOMP_BARRIER, ST_FOMP_FLUSH, ST_FOMP_WORKSHARE, 
		ST_FOMP_PARALLEL_WORKSHARE, ST_FOMP_ATOMIC, ST_FOMP_TASKWAIT,
		ST_FOMP_TASKYIELD};

  pdbStmt(int id);

  void kind(stmt_t kind);
  void stmtBegin(const pdbLoc& loc);
  void stmtEnd(const pdbLoc& loc);
  void nextStmt(const pdbStmt* s);
  void downStmt(const pdbStmt* s);  // body, if: then
  void extraStmt(const pdbStmt* s); // if: else, for: init/decl
                                    // case: break, try: catch
                                    // goto, break, continue: target
  void affinityStmt(const pdbStmt * s); // upc_forall: affinity

  int id() const;
  /** the kind of statement. **/
  stmt_t kind() const;
  /** the start of the statement(s). **/
  const pdbLoc& stmtBegin() const;
  /** the end of the statement(s). **/
  const pdbLoc& stmtEnd() const;
  /** pointer to the next statement within the same context. **/
  const pdbStmt* nextStmt() const;
  /** pointer to the next statement within a new context. 
  	* i.e. the next statements in a block or a "case" and the "then" part of
		* "if". 
  **/
  const pdbStmt* downStmt() const;
	/** sometimes an extra statement pointer is needed, in the case of: else
	 * clause, initalization expression in a for loop, try statement pointing to
	 * catch, goto statements, break statement and continue statements. **/
  const pdbStmt* extraStmt() const;
  const pdbStmt* affinityStmt() const;
	
  static const char *toName(stmt_t v);
  static stmt_t toStmt(const char* v);

  friend class pdbRoutine;

private:
  int            i;
  stmt_t         knd;
  pdbLoc         bg;
  pdbLoc         ed;
  const pdbStmt* nxt;
  const pdbStmt* dwn;
  const pdbStmt* ex;
  const pdbStmt* affinity;
};

ostream& operator<<(ostream& ostr, const pdbStmt& s);

/** \brief A class to represent a routine call.
	*
	* pdbCallee is used to represent a routine call (i.e., a call site).
	* Attributes are the routine called, whether it is was called virtually, and
	* the location of the call site.
	**/
class pdbCallee : public pdbLoc {
public:
  pdbCallee ();
  pdbCallee (const pdbRoutine *call, pdbItem::virt_t v,
             pdbFile *file, int line, int col);
  
  pdbItem::virt_t virtuality() const;
	/** A pointer to the routine called. **/
  const pdbRoutine *call() const;
  /** Is this a virtual call. **/
	bool isVirtual() const;

  void virtuality(pdbItem::virt_t v);
  void call(const pdbRoutine* call);

private:
  pdbItem::virt_t virt;
  const pdbRoutine *rptr;
};

ostream& operator<<(ostream& ostr, const pdbCallee& c);

/** \brief pdbRoutine includes members common to function/methods.
	*
	* Routines describes the common part of global functions, Fortran 90 local and
	* module functions, and C++ class methods. The common atrributes are
	* signature, kind (e.g., extern or intrinsic), specialKind (e.g., constructor
	* or operator), a list of routines called from this routine, how often it gets
	* called from other routines, linkage, for C and C++ the statement
	* representing the body and a list of all statements, and for C and Fortran
	* routines the location of the first executable statement and of all return
	* statements. 
	**/	
class pdbRoutine : public pdbTemplateItem {
public:
  pdbRoutine(int id);
  pdbRoutine(const string& name, int id);
  virtual ~pdbRoutine();

  virtual ostream& print(ostream& ostr) const;
  virtual const char *desc() const;
  virtual void adjustPtrs(PDB* p);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

  typedef vector<pdbCallee *> callvec;
  typedef vector<pdbLoc *> locvec;
  typedef vector<pdbStmt *> stmtvec;

  /** the complete signature of this routine. **/
  const pdbType *signature() const;
  /** the type of this routine. **/
  routine_t kind() const;

  /** the f95 prefix, if any **/
  fprefix_t fprefix() const;

  /** the special type of this routine. **/
  rspec_t specialKind() const;
  
  /** a vector listing the routines this routine references. **/
  callvec& callees();
  
  /** the above vector in a const */
  const callvec& callees() const;
  
  /** the number of times this routine was referenced with the source code (not
   * of number of time it was called during execution.) 1 if this routine is
   * not referenced elsewhere with the source code. **/
  int numCalled() const;
  
  link_t linkage() const;
  
  /** the location of the first statement (excluding declaration) within a
   * routine. **/
  const pdbLoc& firstExecStmtLocation() const;
  
  /** a location vector listing all the return statements with a routine. **/
  const locvec& returnLocations() const;
  
  /** the location of the first statement with the body of a routine. **/
  const pdbStmt* body() const;
  
  /** statements within a routine. **/
  const stmtvec& statements() const;
  
  void signature(const pdbType *sigtype);
  
  void kind(routine_t kind);
  void fprefix(fprefix_t prefix);
  void specialKind(rspec_t rs);
  void addCallee(const pdbRoutine *callee, pdbItem::virt_t v,
                 pdbFile *file, int line, int col);
  void linkage(link_t l);
  void firstExecStmtLocation(const pdbLoc& l);
  void addReturnLocation(const pdbFile *file, int line, int col);
  void body(const pdbStmt* b);
  pdbStmt* addStmt(int id);

  virtual void process(PDB *p);

protected:
  void incrCalled() const;

  const pdbType     *sig;
  const pdbStmt     *bdy;
  routine_t          knd;
  rspec_t            rspec;
  link_t             link;
  int                numc;
  callvec            calls;
  locvec             retrns;
  pdbLoc             festmt;
  stmtvec            stmts;
  fprefix_t          _fprefix;
};

#ifndef NO_INLINE
#  include "pdbRoutine.inl"
#endif
#endif
