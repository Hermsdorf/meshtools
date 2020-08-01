/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

// class pdbStmt inlines
inline pdbStmt::pdbStmt(int id) : i(id), knd(ST_NA),  nxt(0), dwn(0), ex(0), affinity(0) {}

inline void pdbStmt::kind(stmt_t kind) { knd = kind; }
inline void pdbStmt::stmtBegin(const pdbLoc& loc) { bg = loc; }
inline void pdbStmt::stmtEnd(const pdbLoc& loc) { ed = loc; }
inline void pdbStmt::nextStmt(const pdbStmt* s) { nxt = s; }
inline void pdbStmt::downStmt(const pdbStmt* s) { dwn = s; }
inline void pdbStmt::extraStmt(const pdbStmt* s) { ex = s; }
inline void pdbStmt::affinityStmt(const pdbStmt * s) { affinity = s; }

inline int pdbStmt::id() const { return i; }
inline pdbStmt::stmt_t pdbStmt::kind() const { return knd; }
inline const pdbLoc& pdbStmt::stmtBegin() const { return bg; }
inline const pdbLoc& pdbStmt::stmtEnd() const { return ed; }
inline const pdbStmt* pdbStmt::nextStmt() const { return nxt; }
inline const pdbStmt* pdbStmt::downStmt() const { return dwn; }
inline const pdbStmt* pdbStmt::extraStmt() const { return ex; }
inline const pdbStmt* pdbStmt::affinityStmt() const { return affinity; }

// class pdbCallee inlines
inline pdbCallee::pdbCallee ()
       : virt(pdbItem::VI_NO), rptr(0)  {}
inline pdbCallee::pdbCallee (const pdbRoutine *call, pdbItem::virt_t v,
             pdbFile *file, int line, int col)
       : pdbLoc(file, line, col), virt(v), rptr(call) {}
  
inline pdbItem::virt_t pdbCallee::virtuality() const { return virt; }
inline const pdbRoutine *pdbCallee::call() const { return rptr; }
inline bool pdbCallee::isVirtual() const { return virt == pdbItem::VI_VIRT; }

inline void pdbCallee::virtuality(pdbItem::virt_t v) { virt = v; }
inline void pdbCallee::call(const pdbRoutine* call) { rptr = call; }

// class pdbRoutine inlines
inline pdbRoutine::pdbRoutine(int id)
       : pdbTemplateItem(id),
         sig(0), bdy(0), knd(RO_NA), rspec(RS_NA), link(LK_NA), numc(0) , _fprefix(FP_NA) {}
inline pdbRoutine::pdbRoutine(const string& name, int id)
       : pdbTemplateItem(name, id),
         sig(0), bdy(0), knd(RO_NA), rspec(RS_NA), link(LK_NA), numc(0),  _fprefix(FP_NA) {}

inline const char *pdbRoutine::desc() const { return "routine"; }

inline const pdbType *pdbRoutine::signature() const { return sig; }
inline pdbItem::routine_t pdbRoutine::kind() const { return knd; }
inline pdbItem::fprefix_t pdbRoutine::fprefix() const { return _fprefix; }
inline pdbItem::rspec_t pdbRoutine::specialKind() const { return rspec; }
inline pdbRoutine::callvec& pdbRoutine::callees() { return calls; }
inline const pdbRoutine::callvec& pdbRoutine::callees() const { return calls; }
inline int pdbRoutine::numCalled() const { return numc; }
inline pdbItem::link_t pdbRoutine::linkage() const { return link; }
inline const pdbLoc& pdbRoutine::firstExecStmtLocation() const {
  return festmt;
}
inline const pdbRoutine::locvec& pdbRoutine::returnLocations() const {
  return retrns;
}
inline const pdbStmt* pdbRoutine::body() const { return bdy; }
inline const pdbRoutine::stmtvec& pdbRoutine::statements() const {
  return stmts;
}

inline void pdbRoutine::signature(const pdbType *sigtype) { sig = sigtype; }
inline void pdbRoutine::kind(routine_t kind) { knd = kind; }
inline void pdbRoutine::fprefix(fprefix_t fprefix) { _fprefix = fprefix; }
inline void pdbRoutine::specialKind(rspec_t rs) { rspec = rs; }
inline void pdbRoutine::addCallee(const pdbRoutine *callee, pdbItem::virt_t v,
                                  pdbFile *file, int line, int col) {
  calls.push_back(new pdbCallee(callee, v, file, line, col));
  callee->incrCalled();
}
inline void pdbRoutine::linkage(link_t l) { link = l; }
inline void pdbRoutine::firstExecStmtLocation(const pdbLoc& l) {
  festmt = l;
}
inline void pdbRoutine::addReturnLocation(const pdbFile *file, int line,
                                          int col) {
  retrns.push_back(new pdbLoc(file, line, col));
}
inline void pdbRoutine::body(const pdbStmt* b) { bdy = b; }
