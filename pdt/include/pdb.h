/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_H__
#define __PDB_H__

#ifdef _OLD_HEADER_
# include <iostream.h>
# include <map.h>
# include <vector.h>
# include <mstring.h>
# define STR_NPOS NPOS
#else
# include <iostream>
  using std::ostream;
  using std::cout;
  using std::cerr;
  using std::endl;
# include <map>
  using std::map;
# include <vector>
  using std::vector;
# include <string>
# define STR_NPOS string::npos
  using std::string;
#endif

#include <stdlib.h>
#include <string.h>

class PDB;
class pdbSimpleItem;
class pdbItem;
class pdbType;
class pdbFile;
class pdbGroup;
class pdbClass;
class pdbModule;
class pdbRoutine;
class pdbCRoutine;
class pdbFRoutine;
class pdbTemplate;
class pdbMacro;
class pdbPragma;
class pdbNamespace;
class pdbGroupField;

template<class tag>
struct PDBTraits {
  typedef int item_t;
};

/** \brief A class to control the reading and writing of pdb files.
	*
	* In addition, there is a class PDB that represents an entire PDB file. It
	* provides methods to read, write, and merge PDB files, to get the version of
	* the PDB file format and the programming language it got generated from.
	**/
class PDB {
public:
  // public types and typedefs
  /** the language of the source files. **/
	enum lang_t { LA_NA = 0x00,
	        LA_C  = 0x01, LA_CXX = 0x02, LA_C_or_CXX = 0x03,
		LA_FORTRAN = 0x04,
		LA_JAVA = 0x08,
		LA_MULTI = 0x10,
        LA_UPC = 0x16};
  typedef vector<pdbType*> typevec;
  typedef vector<pdbFile*> filevec;
  typedef vector<pdbClass*> classvec;
  typedef vector<pdbModule*> modulevec;
  typedef vector<pdbCRoutine*> croutinevec;
  typedef vector<pdbFRoutine*> froutinevec;
  typedef vector<pdbTemplate*> templatevec;
  typedef vector<pdbMacro*> macrovec;
  typedef vector<pdbPragma*> pragmavec;
  typedef vector<pdbNamespace*> namespacevec;
  typedef vector<pdbSimpleItem*> itemvec;

  typedef map<int, pdbType*> typemap;
  typedef map<int, pdbFile*> filemap;
  typedef map<int, pdbClass*> classmap;
  typedef map<int, pdbModule*> modulemap;
  typedef map<int, pdbCRoutine*> croutinemap;
  typedef map<int, pdbFRoutine*> froutinemap;
  typedef map<int, pdbTemplate*> templatemap;
  typedef map<int, pdbMacro*> macromap;
  typedef map<int, pdbPragma*> pragmamap;
  typedef map<int, pdbNamespace*> namespacemap;

  // PDB basic interface
  /** A PDB class constructor
		*
		* \param *fname the name of the source file.
		**/
	PDB(char *fname);
  ~PDB();
	bool write(char *fname);
  void write(ostream& ostr);
  bool merge(char *fname);
  operator void *() const;
  int version() const;
  lang_t language() const;

  // PDB item hierachies
  pdbRoutine *callTree();
  pdbFile    *fileTree();
  pdbClass   *classTree();

  // PDB item vectors
	/** a vector of the types within the pdb. **/
  typevec&      getTypeVec();
	/** a vector of the files within the pdb. **/
  filevec&      getFileVec();
	/** a vector of the classes within the pdb. **/
  classvec&     getClassVec();
	/** a vector of the modules within the pdb. **/
  modulevec&    getModuleVec();
	/** a vector of the c/c++ routines within the pdb. **/
  croutinevec&  getCRoutineVec();
	/** a vector of the fortran routines within the pdb. **/
  froutinevec&  getFRoutineVec();
	/** a vector of the templates within the pdb. **/
  templatevec&  getTemplateVec();
	/** a vector of the macros within the pdb. **/
  macrovec&     getMacroVec();
	/** a vector of the pragmas within the pdb. **/
  pragmavec&    getPragmaVec();
	/** a vector of the namespaces within the pdb. **/
  namespacevec& getNamespaceVec();
	/** a vector of the items within the pdb. **/
  itemvec&      getItemVec();

	/** \example vector.cc
		*
		* An example to show how to iterate through the elements of a PDB file.
		* classvec can be replaced with any other vector type.
		**/

  // PDB item maps
  typemap&      getTypeMap();
  filemap&      getFileMap();
  classmap&     getClassMap();
  modulemap&    getModuleMap();
  croutinemap&  getCRoutineMap();
  froutinemap&  getFRoutineMap();
  templatemap&  getTemplateMap();
  macromap&     getMacroMap();
  pragmamap&    getPragmaMap();
  namespacemap& getNamespaceMap();

  static const char *toName(lang_t v);
  static lang_t toLang(const char *v);

  class typeTag {};
  class fileTag {};
  class classTag {};
  class moduleTag {};
  class croutineTag {};
  class froutineTag {};
  class templateTag {};
  class macroTag {};
  class pragmaTag {};
  class namespaceTag {};

  template<class tag>
  typename PDBTraits<tag>::item_t *findItem(tag t, int id);

  template<class tag>
  typename PDBTraits<tag>::item_t *findItem(tag t, const string& name, int id);

  template<class tag>
  void finalCheck(tag t);

private:
  struct ltstr {
    bool operator()(const char* s1, const char* s2) const {
      return strcmp(s1, s2) < 0;
    }
  };

  enum attr_t { NOATTR,
       // Common Item Attributes
       ACS, CLASS, GROUP, LOC, NSPACE, POS,

       // Common Template Item Attributes
       TEMPL, SPECL, GSPARAM,

       // Item Specific Attributes
       // -- classes (V1.0)
       CBASE, CFRCLASS, CFRFUNC, CFUNC, CMEM, CMCONST, CMISBIT,
       CMKIND, CMMUT, CMTYPE, CKIND,
       // -- groups (V2.0)
       GBASE, GFRGROUP, GFRFUNC, GFUNC, GMEM, GMCONST, GMISBIT,
       GMKIND, GMMUT, GMTYPE, GKIND,
       // -- language
       LANG,
       // -- macros
       MKIND, MTEXT,
       // -- namespaces
       NALIAS, NMEM,
       // -- pragmas
       PKIND, PPOS, PTEXT,
       // -- routines
       RALIAS, RARGINFO, RBODY, RCALL, RCATCH, RCGEN, RCRVO, REXPL, RIMPL,
       RINLINE, RISELEM, RKIND, RFPREFIX, RLINK, RREC, RRET, RROUT, RSIG, RSKIND,
       RSTAT, RSTMT, RSTART, RSTOP, RSTORE, RVIRT,
       // -- source files
       SCOM, SINC, SSYS,
       // -- templates
       TDECL, TDEF, TKIND, TPARAM, TPROTO, TSPARAM, TTEXT, TTYPE,
       // -- types
       YARGT, YCLEN, YDIM, YELEM, YELLIP, YENUM, YEXCEP, YFKIND,
       YIKIND, YKIND, YMPGROUP, YMPTYPE, YNELEM, YOLD, YPTR, YQUAL, YRANK,
       YREF, YRETT, YSHAPE, YSIGNED, YSTAT, YTREF, YSHARED, YBLOCKSIZE,
       YRELAXED, YSTRICT
  };

  typedef map<const char*, attr_t, ltstr> attrmap;
  typedef map<attr_t, const char*> namemap;

  itemvec      itemVec;
  typevec      typeVec;
  filevec      fileVec;
  classvec     classVec;
  modulevec    moduleVec;
  croutinevec  croutineVec;
  froutinevec  froutineVec;
  templatevec  templateVec;
  macrovec     macroVec;
  pragmavec    pragmaVec;
  namespacevec namespaceVec;

  static attr_t toAttr(const char *v);
  static void   initAttrs();
  static bool   attrsAreInit;

  static attrmap attr;
  static namemap name;

  typemap      typeMap;
  filemap      fileMap;
  classmap     classMap;
  modulemap    moduleMap;
  croutinemap  croutineMap;
  froutinemap  froutineMap;
  templatemap  templateMap;
  macromap     macroMap;
  pragmamap    pragmaMap;
  namespacemap namespaceMap;

  pdbRoutine *topRoutine;
  pdbClass *topClass;
  pdbFile *topFile;
  bool status;
  int versNum;
  lang_t lang;

  bool read(char *fname);
  pdbType *getTypeOrGroup(const char *value);
  bool getLocation(const char *value, char **ptr,
                   pdbFile*& file, int& line, int& col);
  bool getStmt(const char *value, char **ptr, int& st);

  template<class tag>
  void markDuplicates(tag t, PDB& other);
};

#ifndef NO_INLINE
#  include "pdb.inl"
#endif
#endif
