/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_ITEM_H__
#define __PDB_ITEM_H__

#include "pdb.h"
#include "pdbSimpleItem.h"
#include "pdbFile.h"  /*???*/

/** \brief An item class with more complex members.
	* 
	* Derived from pdbSimpleItems are pdbFiles and more complex pdbItems, which
	* have a source code location, possibly a parent group or namespace, and an
	* access mode (e.g., public or private) if they are member of a group. The
	* method fullname() returns fully-qualified names (including signatures for
	* routines).
	**/
class pdbItem : public pdbSimpleItem {
public:
  /** defines the types of access modifiers for template items. **/
  enum access_t { AC_NA		/** default **/
		  , AC_PRIV /** private **/
		  , AC_PROT	/** protected **/
		  , AC_PUB 	/** public **/
  };
  /** defines the types of routine signatures **/
  enum routine_t { RO_NA	/** default routine **/
		   , RO_ASM, RO_AUTO
		   , RO_EXT /** external routines, created by the compiler not
				explicitly written in the source code **/
		   , RO_STAT
		   , RO_TPROTO	/** template routine **/
		   , RO_FEXT 	/** Fortran external routine **/
		   , RO_FPROG	/** Fortran program routine **/
		   , RO_FBLDAT		/** Fortran block data **/
		   , RO_FINTRIN	/** Fortran intrinsic **/
		   , RO_FINT			/** Fortran internal **/
		   , RO_FSTFN		/** Fortran statement function **/
		   , RO_FMPROC		/** Fortran module procedure **/
		   , RO_FUNSPEC	/** Fortran unspecified **/
		   , RO_FALIAS 	/** Fortran alias **/
  };


  /** f95 routine prefixes **/
  enum fprefix_t { FP_PURE  /** pure procedure **/
		   , FP_ELEM /** elemental procedure **/
		   , FP_NA /** default **/
  };

  /** the types of special routines. **/
  enum rspec_t { RS_NA, RS_CONV, RS_CTOR, RS_DTOR, RS_OP };
  enum virt_t { VI_NO, VI_PURE, VI_VIRT };
  enum macro_t { MA_NA, MA_DEF, MA_UNDEF };
  enum templ_t { TE_NA
		 , TE_CLASS /** template classes **/
		 , TE_FUNC /** template function **/
		 , TE_MEMCLASS /** classes that are members of a template class **/
		 , TE_MEMFUNC /** functions that are members of a template class */
		 , TE_STATMEM, TE_TPARAM };
  /** defines the types of floating point numbers. **/
  enum float_t { FL_NA 		/** not applicable **/
		 , FL_FLOAT	/** float type **/
		 , FL_DBL		/** double type **/
		 , FL_LONGDBL /** long double type **/
  };
  /** defines the types of integer point numbers. **/
  enum int_t { I_NA			/** not applicable **/
	       , I_CHAR		/** character **/
	       , I_SCHAR		/** signed character **/
	       , I_UCHAR		/** unsigned character **/
	       , I_SHORT		/** short **/
	       , I_USHORT	/** unsigned short **/
	       , I_INT			/** integer **/
	       , I_UINT		/** unsigned integer **/
	       , I_LONG		/** long **/
	       , I_ULONG		/** unsigned long **/
	       , I_LONGLONG	/** long long **/
	       , I_ULONGLONG	/** unsigned long long **/
	       , I_WCHAR 		/** **/
  };
  /** defines other types of primitives. **/
  enum type_t { TY_NA = 0/** not applicable **/
 		, TY_BOOL  =0x0001	/** boolean type **/
		, TY_ENUM =0x0002		/** enumeration type **/
		, TY_ERR  =0x0004		/** error type **/
		, TY_FUNC  =0x0008	/** function type **/
		, TY_VOID  =0x0010	/** void type **/
		, TY_INT  =0x0020		/** primitive type **/
		, TY_FLOAT=0x0040		/** float type **/
		, TY_PTR   =0x0080	/** pointer type **/
		, TY_REF   =0x0100 	/** reference to a class or type**/
		, TY_ARRAY=0x0200		/** array type **/
		, TY_TREF =0x0400		/** reference to a template class or fuction **/
		, TY_PTRMEM=0x0800	/** **/
		, TY_TPARAM=0x1000	/** **/
		, TY_WCHAR=0x2000		/** **/
		, TY_C_TYPE=0xffff	/** **/
		, TY_FERR  =0x00010000	/** Fortran error type **/
		, TY_FVOID =0x00020000	/** Fortran void type **/
		, TY_FINT  =0x00040000	/** Fortran integer type **/
		, TY_FLOGIC=0x00080000	/** Fortran logic type **/
		, TY_FFLOAT=0x00100000	/** Fortran float type **/
		, TY_FFUNC =0x00200000	/** Fortran function type **/
		, TY_FCHAR =0x00400000	/** Fortran character type **/
		, TY_FARRAY=0x00800000	/** Fortran array type **/
		, TY_FCMPLX=0x01000000	/** Fortran complex type **/
		, TY_FUNSPECFUNC=0x02000000	/** **/	
		, TY_FBLDAT=0x04000000	/** **/
		, TY_FMOD  =0x08000000	/** **/
		, TY_FPTR  =0x10000000	/** **/
		, TY_F_TYPE=0x7fff0000	/** **/
		, TY_GROUP=0x70007000		/** **/
  };
  /** defines the types of groups. **/
  enum group_t { GR_NA
		 , GR_CLASS	/** class group **/
		 , GR_STRUCT	/** structure group **/
		 , GR_UNION	/** union group **/
		 , GR_TPROTO	/** template prototype group **/
		 , GR_FDERIVED	/** Fortran derived group **/
		 , GR_FMODULE	/** Fortran module group **/
  };	
  enum link_t { LK_NA				/** link not defined **/
		, LK_INTERNAL	/** internal linke **/
		, LK_CXX			/** c++ link **/
		, LK_C				/** c link **/
		, LK_FINT			/** Fortran link **/
		, LK_F90 			/** Fortran 90 link **/
  };
  enum mem_t { M_NA, M_TYPE, M_STATVAR, M_VAR, M_TEMPL };
  /** defines the different shapes of fortran arrays.
   **/
  enum shape_t { 
    SH_NA					/** initalized value. (for debugging purposes). **/
    ,SH_EXPLICIT  /** set when the rank and extent are defined explicitly. **/
    ,SH_ASIZE 		/** set when the extend of one or more dimension is undefined. **/
    ,SH_ASHAPE 		/** set when the rank of an array is left undefined. **/
    ,SH_DEFERRED 	/** set when an array is allocated but undefined. **/
  };
  enum qual_t { QL_NA 			/** unqualified type **/
		, QL_CONST		/** constant type **/
		, QL_VOLATILE	/** volatile type **/
		, QL_RESTRICT /** restricted type **/
  };
  
  /** \brief pdbItem constructor.
   * 
   * \param id an unique idenifier.
   **/	
  pdbItem(int id);
  /** \brief pdbItem constructor.
   *
   * \param name the name of the item.
   * \param id an unique idenifier.
   **/	
  pdbItem(const string& name, int id);
  virtual ~pdbItem();

  /** \brief the full name of the item.
   *
   * The full name contains the full signatures for templates.
   **/
  const string& fullName() const;
  /** \brief access mode for this item. **/
  access_t access() const;
  /** \brief the groups this item is a member of. **/
  const pdbGroup* parentGroup() const;
  pdbLoc& location();
  /** \brief the location of this item in the source file. **/	
  const pdbLoc& location() const;
  /** \brief the name space this item is in. **/
  const pdbNamespace* parentNSpace() const;

  void fullName(const string& name);
  void access(access_t attr);
  void parentGroup(const pdbGroup* pgroup);
  void location(const pdbLoc& loc);
  void parentNSpace(const pdbNamespace* nspace);
  
  virtual ostream& print(ostream& ostr) const;
  virtual void adjustPtrs(PDB*);
  virtual void process(PDB*);
  virtual dupl_t findDuplicate(pdbSimpleItem* r);

  static const char *toName(access_t v);
  static const char *toName(routine_t v);
  static const char *toName(rspec_t v);
  static const char *toName(virt_t v);
  static const char *toName(macro_t v);
  static const char *toName(templ_t v);
  static const char *toName(float_t v);
  static const char *toName(int_t v);
  static const char *toName(type_t v);
  static const char *toName(qual_t v);
  static const char *toName(group_t v);
  static const char *toName(link_t v);
  static const char *toName(mem_t v);
  static const char *toName(shape_t v);

  static access_t  toAccess(const char *v);
  static fprefix_t toFPrefix(const char *v);
  static routine_t toRoutine(const char *v);
  static rspec_t   toRSpecial(const char *v);
  static virt_t    toVirt(const char *v);
  static macro_t   toMacro(const char *v);
  static templ_t   toTempl(const char *v);
  static float_t   toFloat(const char* v);
  static int_t     toInt(const char* v);
  static type_t    toType(const char* v);
  static qual_t    toQual(const char* v);
  static group_t   toGroup(const char* v);
  static link_t    toLink(const char* v);
  static mem_t     toMem(const char* v);
  static shape_t   toShape(const char* v);

private:
  pdbLoc              lc;
  access_t            acs;
  const pdbGroup     *gptr;
  const pdbNamespace *nptr;
  string              full;
};

#ifndef NO_INLINE
#  include "pdbItem.inl"
#endif
#endif
