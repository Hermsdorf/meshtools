/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbMacro::pdbMacro(int id)
       : pdbItem(id), knd(MA_NA) {}
inline pdbMacro::pdbMacro(const string& name, int id)
       : pdbItem(name, id), knd(MA_NA) {}

inline const char *pdbMacro::desc() const { return "macro"; }

inline pdbItem::macro_t pdbMacro::kind() const { return knd; }
inline const string& pdbMacro::text() const { return txt; }

inline void pdbMacro::kind(macro_t kind) { knd = kind; }
inline void pdbMacro::text(const string& text) { txt = text; }
