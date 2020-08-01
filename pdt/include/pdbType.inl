// class pdbEnum inlines
inline pdbEnum::pdbEnum(const char* id, int val) : i(id), v(val) {}
inline const string& pdbEnum::id() const { return i; }
inline int pdbEnum::val() const { return v; }

// class pdbBounds inlines
inline pdbBounds::pdbBounds(int low, int upp) : lo(low), up(upp) {}
inline int pdbBounds::lower() const { return lo; }
inline int pdbBounds::upper() const { return up; }

// class pdbArg inlines
inline pdbArg::pdbArg(const pdbType *type, const string& name,
                      pdbFile *file, int line, int col, int flags)
       : pdbLoc(file, line, col), typ(type), nam(name), flg(flags) {}
inline const pdbType* pdbArg::type() const { return typ; }
inline const string& pdbArg::name() const { return nam; }
inline bool pdbArg::intentIn() const { return flg & ARG_IN; }
inline bool pdbArg::intentOut() const { return flg & ARG_OUT; }
inline bool pdbArg::isOptional() const { return flg & ARG_OPT; }
inline bool pdbArg::hasDefault() const { return flg & ARG_DEF; }
inline int pdbArg::flags() const { return flg; }

inline void pdbArg::type(const pdbType *t) { typ = t; }
inline void pdbArg::name(const string& n) { nam = n; }
inline void pdbArg::flags(int f) { flg = f; }


//class pdbType inlines
inline pdbType::pdbType(int id)
       : pdbItem(id), elemTy(0), fltTy(FL_NA), intTy(I_NA), knd(TY_NA),
         shp(SH_NA), snd(false), ell(false), cst(false), sta(false),
         oldSD(false), numElem(0), ptrTy(0), baseTy(0), groupTy(0), retTy(0),
         memPtrTy(0), memPtrCl(0), shared(false), blocksize(-1), relaxed(false),
         strict(false) {}
inline pdbType::pdbType(const string& name, int id)
       : pdbItem(name, id), elemTy(0), fltTy(FL_NA), intTy(I_NA), knd(TY_NA),
         shp(SH_NA), snd(false), ell(false), cst(false), sta(false),
         oldSD(false), numElem(0), ptrTy(0), baseTy(0), groupTy(0), retTy(0),
         memPtrTy(0), memPtrCl(0), shared(false), blocksize(-1), relaxed(false),
         strict(false) {}

inline const char *pdbType::desc() const { return "type"; }
inline const char *pdbType::attrPrefix() const { return "y"; }

inline const pdbType* pdbType::elementType() const { return elemTy; }
inline pdbItem::float_t pdbType::floatType() const { return fltTy; }
inline pdbItem::int_t pdbType::integerType() const { return intTy; }
inline pdbItem::type_t pdbType::kind() const { return knd; }
inline const pdbType::qualvec& pdbType::qualifiers() const  { return quals; }
inline const pdbType* pdbType::pointedToType() const {
  return ( knd==TY_PTR || knd==TY_FPTR ) ? ptrTy : 0;
}
inline const pdbType* pdbType::referencedType() const {
  return ( knd==TY_REF ) ? ptrTy : 0;
}
inline bool pdbType::isSigned() const { return snd; }
inline const pdbType* pdbType::baseType() const { return baseTy; }
inline const pdbGroup* pdbType::isGroup() const { return groupTy; }
inline const pdbType* pdbType::returnType() const { return retTy; }
inline const pdbType::argvec& pdbType::arguments() const { return args; }
inline bool pdbType::hasEllipsis() const { return ell; }
inline bool pdbType::oldStyleDecl() const { return oldSD; }
inline const pdbType* pdbType::memberPointerType() const { return memPtrTy; }
inline const pdbClass* pdbType::memberPointerClass() const { return memPtrCl; }
inline bool pdbType::isConst() const { return cst; }
inline int pdbType::numElements() const { return numElem; }
inline const pdbType::typevec& pdbType::exceptionSpec() const {
  return exSpec;
}
inline const pdbType::enumvec& pdbType::enumeration() const { return enums; }
inline pdbItem::shape_t pdbType::shape() const { return shp; }
inline int pdbType::rank() const { return numElem; }
inline const pdbType::boundvec& pdbType::bounds() const { return bnds; }
inline bool pdbType::isStaticArray() const { return sta; }


inline void pdbType::elementType(const pdbType* elem) { elemTy = elem; }
inline void pdbType::floatType(float_t ftype) { fltTy = ftype; }
inline void pdbType::integerType(int_t itype) { intTy = itype; }
inline void pdbType::kind(type_t type) { knd = type; }
inline void pdbType::addQualifier(qual_t qual) { quals.push_back(qual); }
inline void pdbType::pointedToType(const pdbType* ptr) { ptrTy = ptr; }
inline void pdbType::referencedType(const pdbType* ptr) { ptrTy = ptr; }
inline void pdbType::isSigned(bool s) { snd = s; }
inline void pdbType::baseType(const pdbType* base) { baseTy = base; }
inline void pdbType::isGroup(const pdbGroup* gr) { groupTy = gr; }
inline void pdbType::returnType(const pdbType* type) { retTy = type; }
inline void pdbType::addArgument(const pdbType* type, const string& name, 
                                 pdbFile *file, int line, int col, int flags) {
  args.push_back(pdbArg(type, name, file, line, col, flags));
}
inline void pdbType::hasEllipsis(bool e) { ell = e; }
inline void pdbType::oldStyleDecl(bool o)  { oldSD = o; }
inline void pdbType::memberPointerType(const pdbType* mptype) {
  memPtrTy = mptype;
}
inline void pdbType::memberPointerClass(const pdbClass* mpclass) {
  memPtrCl = mpclass;
}
inline void pdbType::isConst(bool c) { cst = c; }
inline void pdbType::numElements(int n) { numElem = n; }
inline void pdbType::addExceptionSpec(const pdbType* c) {
  exSpec.push_back(c);
}
inline void pdbType::addEnumeration(const pdbEnum& e) { enums.push_back(e); }
inline void pdbType::shape(shape_t s) { shp = s; }
inline void pdbType::rank(int r) { numElem = r; }
inline void pdbType::addBound(int low, int high) {
  bnds.push_back(pdbBounds(low, high));
}
inline void pdbType::isStaticArray(bool s) { sta = s; }

inline bool pdbType::isShared() const { return shared; }
inline int  pdbType::blockSize() const { return blocksize; }
inline bool pdbType::isRelaxed() const { return relaxed; }
inline bool pdbType::isStrict() const { return strict; }

inline void pdbType::isShared(bool s) { shared = s; }
inline void pdbType::blockSize(int b) { blocksize = b; }
inline void pdbType::isRelaxed(bool r) { relaxed = r; }
inline void pdbType::isStrict(bool s) { strict = s; }
