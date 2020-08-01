/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbPragma::pdbPragma(int id)
       : pdbItem(id) {}
inline pdbPragma::pdbPragma(const string& name, int id)
       : pdbItem(name, id) {}

inline const char *pdbPragma::desc() const { return "pragma"; }

inline string pdbPragma::kind() const { return knd; }
inline const pdbLoc& pdbPragma::prBegin() const { return bg; }
inline const pdbLoc& pdbPragma::prEnd() const { return ed; }
inline const string& pdbPragma::text() const { return txt; }

inline void pdbPragma::kind(const string& kind) { knd = kind; }
inline void pdbPragma::prBegin(const pdbLoc& loc) { bg = loc; }
inline void pdbPragma::prEnd(const pdbLoc& loc) { ed = loc; }
inline void pdbPragma::text(const string& text) { txt = text; }
