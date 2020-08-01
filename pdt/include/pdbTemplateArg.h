/*************************************************************************/
/* DUCTAPE Version 2.0                                                   */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef __PDB_TEMPLATE_ARG_H__
#define __PDB_TEMPLATE_ARG_H__

static const string EMPTY;

/** \brief A class to define argument in a template defintions.
	*
	* pdbTemplateArg describes arguments in template definitions and
	* specializations. 
	**/
class pdbTemplateArg {
public:
  /** this enumeration tell the type of template argument Depending on the kind
		* of the template argument, different
	  * methods are applicable.
		**/
	enum targ_t { TA_NA					/** uninitialized **/
								, TA_TYPE			/** types (type() and defaultType()). **/		
								, TA_NONTYPE	/** non types (type(), name() and defaultValue()). **/	
								, TA_TEMPL 		/** templates (templateArg() and defaultTemplateArg()) **/};
  
	/** A constructor
		*
		* \param kind the kind of template argument.
		* \param specialization wheather the arguements are specialized. 
		**/
	pdbTemplateArg(targ_t kind, bool specialization = false);

  void adjustPtrs(PDB* pdb);

  targ_t kind() const;
  /** name of the template arguemnt. **/
	const string& name() const;
	/** the pointer to the type of the arguement. **/
  const pdbType* type() const;

  const pdbTemplate* templateArg() const;
  const string& value() const;
  const pdbType* defaultType() const;
  const pdbTemplate* defaultTemplateArg() const;
  const string& defaultValue() const;
  bool isSpecialization() const;

  void name(const string& name);
  void type(const pdbType* type);
  void templateArg(const pdbTemplate* templ);
  void value(const string& val);
  void defaultType(const pdbType* type);
  void defaultTemplateArg(const pdbTemplate* templ);
  void defaultValue(const string& val);

  static const char *toName(targ_t v);
  static targ_t toTarg(const char* v);

private:
  targ_t               knd;
  string               nam;	
  union {
    const pdbType*     typ;
    const pdbTemplate* tmpl;
  };
  union {
    const pdbType*     deftyp;
    const pdbTemplate* deftmpl;
  };
  string               defval;
  bool                 specl;
};

ostream& operator<<(ostream& ostr, const pdbTemplateArg& ta);

#ifndef NO_INLINE
#  include "pdbTemplateArg.inl"
#endif
#endif
