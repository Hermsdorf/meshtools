/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

inline pdbGroup::pdbGroup(int id)
       : pdbTemplateItem(id), knd(GR_NA) {}
inline pdbGroup::pdbGroup(const string& name, int id)
       : pdbTemplateItem(name, id), knd(GR_NA) {}

inline const char *pdbGroup::desc() const { return "group"; }

inline const pdbGroup::fieldvec& pdbGroup::dataMembers() const { return dmem; }
inline  pdbItem::group_t pdbGroup::kind() const { return knd; }

inline void pdbGroup::addDataMember(pdbGroupField *field) {
  dmem.push_back(field);
}
inline void pdbGroup::kind(group_t kd) { knd = kd; }
