/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbCRoutine::pdbCRoutine(int id)
       : pdbRoutine(id),
         virt(VI_NO), crvo(false), inl(false), isCGen(false),
         isEx(false), isStat(false) {}
inline pdbCRoutine::pdbCRoutine(const string& name, int id)
       : pdbRoutine(name, id),
         virt(VI_NO), crvo(false), inl(false), isCGen(false),
         isEx(false), isStat(false) {}

inline pdbItem::virt_t pdbCRoutine::virtuality() const { return virt; }
inline bool pdbCRoutine::covariantReturnVirtualOverride() const { return crvo; }
inline bool pdbCRoutine::isInline() const { return inl; }
inline bool pdbCRoutine::isCompilerGenerated() const { return isCGen; }
inline bool pdbCRoutine::isExplicitCtor() const { return isEx; }
inline bool pdbCRoutine::isStatic() const { return isStat; }
inline bool pdbCRoutine::isVirtual() const {
  return virt == pdbItem::VI_VIRT || virt == pdbItem::VI_PURE;
}

inline void pdbCRoutine::virtuality(virt_t v) { virt = v; }
inline void pdbCRoutine::covariantReturnVirtualOverride(bool c) { crvo = c; }
inline void pdbCRoutine::isInline(bool i) { inl = i; }
inline void pdbCRoutine::isCompilerGenerated(bool c) { isCGen = c; }
inline void pdbCRoutine::isExplicitCtor(bool e) { isEx = e; }
inline void pdbCRoutine::isStatic(bool s) { isStat = s; }
