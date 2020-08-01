/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_TRAITS_H__
#define __PDB_TRAITS_H__

struct PDBTraitsDefaults {
  static PDB::itemvec& getItems(PDB *p) { return p->getItemVec(); }
};

template<>
struct PDBTraits<PDB::typeTag> : public PDBTraitsDefaults {
  typedef pdbType item_t;
  typedef PDB::typemap map_t;
  typedef PDB::typevec vec_t;
  static map_t& getMap(PDB *p) { return p->getTypeMap(); }
  static vec_t& getVec(PDB *p) { return p->getTypeVec(); }
};

template<>
struct PDBTraits<PDB::fileTag> : public PDBTraitsDefaults {
  typedef pdbFile item_t;
  typedef PDB::filemap map_t;
  typedef PDB::filevec vec_t;
  static map_t& getMap(PDB *p) { return p->getFileMap(); }
  static vec_t& getVec(PDB *p) { return p->getFileVec(); }
};

template<>
struct PDBTraits<PDB::classTag> : public PDBTraitsDefaults {
  typedef pdbClass item_t;
  typedef PDB::classmap map_t;
  typedef PDB::classvec vec_t;
  static map_t& getMap(PDB *p) { return p->getClassMap(); }
  static vec_t& getVec(PDB *p) { return p->getClassVec(); }
};

template<>
struct PDBTraits<PDB::moduleTag> : public PDBTraitsDefaults {
  typedef pdbModule item_t;
  typedef PDB::modulemap map_t;
  typedef PDB::modulevec vec_t;
  static map_t& getMap(PDB *p) { return p->getModuleMap(); }
  static vec_t& getVec(PDB *p) { return p->getModuleVec(); }
};

template<>
struct PDBTraits<PDB::croutineTag> : public PDBTraitsDefaults {
  typedef pdbCRoutine item_t;
  typedef PDB::croutinemap map_t;
  typedef PDB::croutinevec vec_t;
  static map_t& getMap(PDB *p) { return p->getCRoutineMap(); }
  static vec_t& getVec(PDB *p) { return p->getCRoutineVec(); }
};

template<>
struct PDBTraits<PDB::froutineTag> : public PDBTraitsDefaults {
  typedef pdbFRoutine item_t;
  typedef PDB::froutinemap map_t;
  typedef PDB::froutinevec vec_t;
  static map_t& getMap(PDB *p) { return p->getFRoutineMap(); }
  static vec_t& getVec(PDB *p) { return p->getFRoutineVec(); }
};

template<>
struct PDBTraits<PDB::templateTag> : public PDBTraitsDefaults {
  typedef pdbTemplate item_t;
  typedef PDB::templatemap map_t;
  typedef PDB::templatevec vec_t;
  static map_t& getMap(PDB *p) { return p->getTemplateMap(); }
  static vec_t& getVec(PDB *p) { return p->getTemplateVec(); }
};

template<>
struct PDBTraits<PDB::macroTag> : public PDBTraitsDefaults {
  typedef pdbMacro item_t;
  typedef PDB::macromap map_t;
  typedef PDB::macrovec vec_t;
  static map_t& getMap(PDB *p) { return p->getMacroMap(); }
  static vec_t& getVec(PDB *p) { return p->getMacroVec(); }
};

template<>
struct PDBTraits<PDB::pragmaTag> : public PDBTraitsDefaults {
  typedef pdbPragma item_t;
  typedef PDB::pragmamap map_t;
  typedef PDB::pragmavec vec_t;
  static map_t& getMap(PDB *p) { return p->getPragmaMap(); }
  static vec_t& getVec(PDB *p) { return p->getPragmaVec(); }
};

template<>
struct PDBTraits<PDB::namespaceTag> : public PDBTraitsDefaults {
  typedef pdbNamespace item_t;
  typedef PDB::namespacemap map_t;
  typedef PDB::namespacevec vec_t;
  static map_t& getMap(PDB *p) { return p->getNamespaceMap(); }
  static vec_t& getVec(PDB *p) { return p->getNamespaceVec(); }
};

#endif
