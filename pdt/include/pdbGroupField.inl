/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbGroupField::pdbGroupField(const string& name)
       : pdbTemplateItem(name, -1),
         k(M_NA), typ(0), isBit(false), isMut(false), isSC(false) {}

inline const char *pdbGroupField::desc() const { return "group field"; }
inline const char *pdbGroupField::attrPrefix() const { return "gm"; }

inline pdbItem::mem_t pdbGroupField::kind() const { return k; }
inline const pdbType *pdbGroupField::type() const { return typ; }
inline bool pdbGroupField::isBitField() const { return isBit; }
inline bool pdbGroupField::isMutable() const { return isMut; }
inline bool pdbGroupField::isStaticConst() const { return isSC; }

inline void pdbGroupField::kind(mem_t kd) { k = kd; }
inline void pdbGroupField::type(const pdbType *t) { typ = t; }
inline void pdbGroupField::isBitField(bool b) { isBit = b; }
inline void pdbGroupField::isMutable(bool m) { isMut = m; }
inline void pdbGroupField::isStaticConst(bool c) { isSC = c; }
