/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbRoutine *PDB::callTree()  { return topRoutine; }
inline pdbFile    *PDB::fileTree()  { return topFile; }
inline pdbClass   *PDB::classTree() { return topClass; }

inline PDB::typevec&      PDB::getTypeVec()      { return typeVec; }
inline PDB::filevec&      PDB::getFileVec()      { return fileVec; }
inline PDB::classvec&     PDB::getClassVec()     { return classVec; }
inline PDB::modulevec&    PDB::getModuleVec()    { return moduleVec; }
inline PDB::croutinevec&  PDB::getCRoutineVec()  { return croutineVec; }
inline PDB::froutinevec&  PDB::getFRoutineVec()  { return froutineVec; }
inline PDB::templatevec&  PDB::getTemplateVec()  { return templateVec; }
inline PDB::macrovec&     PDB::getMacroVec()     { return macroVec; }
inline PDB::pragmavec&    PDB::getPragmaVec()    { return pragmaVec; }
inline PDB::namespacevec& PDB::getNamespaceVec() { return namespaceVec; }
inline PDB::itemvec&      PDB::getItemVec()      { return itemVec; }

inline PDB::typemap&      PDB::getTypeMap()      { return typeMap; }
inline PDB::filemap&      PDB::getFileMap()      { return fileMap; }
inline PDB::classmap&     PDB::getClassMap()     { return classMap; }
inline PDB::modulemap&    PDB::getModuleMap()    { return moduleMap; }
inline PDB::croutinemap&  PDB::getCRoutineMap()  { return croutineMap; }
inline PDB::froutinemap&  PDB::getFRoutineMap()  { return froutineMap; }
inline PDB::templatemap&  PDB::getTemplateMap()  { return templateMap; }
inline PDB::macromap&     PDB::getMacroMap()     { return macroMap; }
inline PDB::pragmamap&    PDB::getPragmaMap()    { return pragmaMap; }
inline PDB::namespacemap& PDB::getNamespaceMap() { return namespaceMap; }

inline PDB::operator void *() const { return status ? (void*)&status : 0; }

inline int PDB::version() const { return versNum; }
inline PDB::lang_t PDB::language() const { return lang; }
