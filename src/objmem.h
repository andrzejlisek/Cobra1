#define OBJMEMDEBUG0

#define OBJMEMSTR std::cout
#define OBJMEMSIZETYPE size_t
#ifdef OBJMEMDEBUG
 #define NEW(T, C)              ([](T * __ptr) -> T*   { OBJMEMSTR << "NEW_OBJ_" << (void*)__ptr << "_" << #T << std::endl;   return __ptr; })(new C)
 #define NEWARR(T, C)           ([](T * __ptr) -> T*   { OBJMEMSTR << "NEW_ARR_" << (void*)__ptr << "_" << #T << std::endl;   return __ptr; })(new C)
 #define NEWARRINIT(T, C, N, V) ([](T * __ptr, OBJMEMSIZETYPE __N, T __V) -> T*   { OBJMEMSTR << "NEW_ARR_" << (void*)__ptr << "_" << #T << std::endl; for (OBJMEMSIZETYPE __it = 0; __it < __N; __it++) { __ptr[__it] = __V; } return __ptr; })(new C, N, V)
 #define DEL(T, P)              ([](T * __ptr) -> void { OBJMEMSTR << "DEL_OBJ_" << (void*)__ptr << "_" << #T << std::endl;   delete __ptr; })(P)
 #define DELARR(T, P)           ([](T * __ptr) -> void { OBJMEMSTR << "DEL_ARR_" << (void*)__ptr << "_" << #T << std::endl; delete[] __ptr; })(P)

 #define MALLOC(S)              ([](OBJMEMSIZETYPE __S)                     -> void* { void * __ptr = malloc(__S);      OBJMEMSTR << "NEW_MEM_" << __ptr << "_void" << std::endl; return __ptr; })(S)
 #define CALLOC(N, S)           ([](OBJMEMSIZETYPE __N, OBJMEMSIZETYPE __S) -> void* { void * __ptr = calloc(__N, __S); OBJMEMSTR << "NEW_MEM_" << __ptr << "_void" << std::endl; return __ptr; })(N, S)
 #define FREE(P)                ([](void * __ptr)                           -> void  {                                  OBJMEMSTR << "DEL_MEM_" << __ptr << "_void" << std::endl;  free(__ptr); })(P)
 #define REALLOC(P, S)          ([](void * __ptr, OBJMEMSIZETYPE __S)       -> void* { void * __ptr0; OBJMEMSTR << "DEL_MEM_" << __ptr << "_void" << std::endl; __ptr0 = realloc(__ptr, __S); OBJMEMSTR << "NEW_MEM_" << __ptr0 << "_void" << std::endl; return __ptr0; })(P, S)
#else
 #define NEW(T, C)              new C
 #define NEWARR(T, C)           new C
 #define NEWARRINIT(T, C, N, V) ([](T * __ptr, OBJMEMSIZETYPE __N, T __V) -> T*   { for (OBJMEMSIZETYPE __it = 0; __it < __N; __it++) { __ptr[__it] = __V; } return __ptr; })(new C, N, V)
 #define DEL(T, P)              delete P
 #define DELARR(T, P)           delete[] P

 #define MALLOC(S)              malloc(S)
 #define CALLOC(N, S)           calloc(N, S)
 #define FREE(P)                free(P)
 #define REALLOC(P, S)          realloc(P, S)
#endif

#define DELNULL(T, P)    if (P != NULL) { DEL(T, P); P = NULL; }
#define DELARRNULL(T, P) if (P != NULL) { DELARR(T, P); P = NULL; }
#define FREENULL(P)      if (P != 0) { free(P); P = 0; }
