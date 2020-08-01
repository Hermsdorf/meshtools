/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_TDEFS_H__
#define __PDB_TDEFS_H__

template<class tag>
typename PDBTraits<tag>::item_t *PDB::findItem(tag, int id) {
  typedef typename PDBTraits<tag>::item_t item_t;

  typename PDBTraits<tag>::map_t& map = PDBTraits<tag>::getMap(this);
  typename PDBTraits<tag>::map_t::iterator it = map.find(id);
  if ( it != map.end() )
    return (*it).second;
  else {
    item_t* item = new item_t(id);
    map[id] = item;
    PDBTraits<tag>::getItems(this).push_back(item);
    return item;
  }
}

template<class tag>
typename PDBTraits<tag>::item_t* PDB::findItem(tag, const string& name, int id) {
  typedef typename PDBTraits<tag>::item_t item_t;

  typename PDBTraits<tag>::map_t& map = PDBTraits<tag>::getMap(this);
  typename PDBTraits<tag>::map_t::iterator it = map.find(id);
  if ( it != map.end() ) {
    item_t *i = (*it).second;
    if ( i->name()[0] ) pdb_ERROR("Duplicate", i->desc(), id);
    i->name(name);
    return i;
  } else {
    item_t* item = new item_t(name, id);
    map[id] = item;
    PDBTraits<tag>::getItems(this).push_back(item);
    return item;
  }
}

template<class tag>
void PDB::finalCheck(tag) {
  typename PDBTraits<tag>::map_t& map = PDBTraits<tag>::getMap(this);
  typename PDBTraits<tag>::vec_t& vec = PDBTraits<tag>::getVec(this);
  for (typename PDBTraits<tag>::map_t::const_iterator mit = map.begin();
       mit != map.end(); ++mit) {
    if (! (*mit).second->name()[0] )
      pdb_ERROR("Undefined", (*mit).second->desc(), (*mit).second->id());
    (*mit).second->process(this);
    vec.push_back((*mit).second);
  }
}

template<class tag>
void PDB::markDuplicates(tag, PDB& other) {
  typename PDBTraits<tag>::vec_t& vecOld = PDBTraits<tag>::getVec(this);
  typename PDBTraits<tag>::vec_t& vecNew = PDBTraits<tag>::getVec(&other);
  typename PDBTraits<tag>::map_t& mapOld = PDBTraits<tag>::getMap(this);
  typename PDBTraits<tag>::map_t& mapNew = PDBTraits<tag>::getMap(&other);
  int maxId = 0;
  bool oldDupls = false;

  // look for duplicate item in "other" PDB
  // use "findDuplicate" to check for duplicates
  for (typename PDBTraits<tag>::vec_t::const_iterator oit = vecOld.begin();
       oit != vecOld.end(); ++oit) {
    for (typename PDBTraits<tag>::vec_t::const_iterator nit = vecNew.begin();
         nit != vecNew.end(); ++nit) {
      // only compare Items if not marked as duplicate yet
      if ( (*nit)->newId() == pdbSimpleItem::UNIQUE ) {
        pdbSimpleItem::dupl_t res = (*oit)->findDuplicate(*nit);
        if ( res == pdbSimpleItem::OLDDUPL ) {
          // duplicate in "old" PDB 
          (*oit)->newId((*nit)->id());  // "old" id, needs to be fixed later
          oldDupls = true;
          break;
        } else if ( res == pdbSimpleItem::NEWDUPL ) {
          // duplicate in "other" PDB
          (*nit)->newId((*oit)->id());
          break;
        }
      }
    }
    // determine highest used ID
    if ( (*oit)->id() > maxId ) maxId = (*oit)->id();
  }

  // assign new items (non duplicates) from "other" PDB new ID
  // and insert them in "old" PDB (map AND vecs!)
  for (typename PDBTraits<tag>::vec_t::const_iterator nit = vecNew.begin();
       nit != vecNew.end(); ++nit) {
    if ( (*nit)->newId() == pdbSimpleItem::UNIQUE ) {
      ++maxId;
      mapOld[maxId] = (*nit);
      vecOld.push_back(*nit);
      PDBTraits<tag>::getItems(this).push_back(*nit);
      (*nit)->id(maxId);

      // mark them, so they are not deleted if "other" PDB is destructed
      (*nit)->newId(pdbSimpleItem::NOTDEL);
    }
  }

  // if duplicates in "old" PDB, fix the references to the new id's
  if ( oldDupls ) {
    for (typename PDBTraits<tag>::vec_t::const_iterator oit = vecOld.begin();
         oit != vecOld.end(); ++oit) {
      if ( (*oit)->newId() > pdbSimpleItem::UNIQUE ) {
         (*oit)->newId((mapNew[(*oit)->newId()])->id());
      }
    }
  }
}

#endif
