/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef CACHE_H
#define CACHE_H
#include "rules.h"
#include "index.h"
#include "configuration.h"
#include "region.h"
/* #define CACHE_ENABLE 1 */

#define allocateInBuffer(ty, vn, val) \
    ty *vn = ((ty *)(*p)); \
    memcpy(vn, val, sizeof(ty)); (*p)+=CACHE_SIZE(ty, 1); \
    if(*p > *pointers) return NULL;
#define allocateArrayInBuffer(elemTy, n, lval, val) \
    lval = ((elemTy *)(*p)); \
    memcpy(lval, val, CACHE_SIZE(elemTy, n)); \
    (*p)+=CACHE_SIZE(elemTy, n); \
    if(*p > *pointers) return NULL;
#define SHMMAX 30000000
#define SHM_BASE_ADDR ((void *)0x80000000)
#define shm_rname "SHM"
#define CASCADE_NULL(x) if((x)==NULL) return NULL;
#define MK_POINTER(ptr) \
			if(generatePtrDesc) { \
				(*pointers)-=CACHE_SIZE(unsigned char *, 1); \
				*(unsigned char **)(*pointers) = (unsigned char *)(ptr); \
			} \
		    if(*p > *pointers) return NULL;

#define MK_COPY(type, src, tgt) \
		if((src)!=NULL) { \
			CASCADE_NULL(tgt=CONCAT(copy, type)(buf, p, pointers, src, generatePtrDesc));\
			MK_POINTER(&(tgt)); \
		}
#define MK_COPY_OBJ_MAP(type, src, tgt) \
		if((src)!=NULL) { \
			CASCADE_NULL(tgt=CONCAT(copy, type)(buf, p, pointers, src, objectMap, generatePtrDesc)); \
			MK_POINTER(&(tgt)); \
		}
#define MK_COPY_COPIER(type, src, tgt, cpr) \
		if((src)!=NULL) { \
			CASCADE_NULL(tgt=CONCAT(copy, type)(buf, p, pointers, src, cpr, objectMap, generatePtrDesc)); \
			MK_POINTER(&(tgt)); \
		}
#define COPY_FUNC_PROTO_OBJ_MAP(T) \
		T* copy##T(unsigned char *buf, unsigned char **p, unsigned char **pointers, T *e, Hashtable *objectMap, int generatePtrDesc)

#define COPY_FUNC_OBJ_MAP_BEGIN(T) \
	 COPY_FUNC_PROTO_OBJ_MAP(T) {  \
		  allocateInBuffer(T, ecopy, e);

#define COPY_FUNC_PROTO_COPIER(T) \
		T* copy##T(unsigned char *buf, unsigned char **p, unsigned char **pointers, T *e, Copier cpfn, Hashtable *objectMap, int generatePtrDesc)

#define COPY_FUNC_COPIER_BEGIN(T) \
	 COPY_FUNC_PROTO_COPIER(T) {	\
		allocateInBuffer(T, ecopy, e);

#define COPY_FUNC_OBJ_MAP_SHARED_NO_COPY_BEGIN(T) \
	 COPY_FUNC_PROTO_OBJ_MAP(T) {  \
		  T *shared; \
		  char key[KEY_SIZE]; \
		  key##T(e, key); \
		  if((shared = (T *)lookupFromHashTable(objectMap, key)) != NULL) { \
			  return shared; \
		  }

#define COPY_FUNC_OBJ_MAP_SHARED_BEGIN(T) \
	COPY_FUNC_OBJ_MAP_SHARED_NO_COPY_BEGIN(T) \
		  allocateInBuffer(T, ecopy, e);

#define COPY_FUNC_COPIER_SHARED_BEGIN(T) \
	 COPY_FUNC_PROTO_COPIER(T) {	\
		  T *shared; \
		  char key[KEY_SIZE]; \
		  key##T(e, key); \
		  if((shared = (T *)lookupFromHashTable(objectMap, key)) != NULL) { \
			  return shared; \
		  } \
		  allocateInBuffer(T, ecopy, e);
#define COPY_FUNC_PROTO(T) \
		T* copy##T(unsigned char *buf, unsigned char **p, unsigned char **pointers, T *e, int generatePtrDesc)

#define COPY_FUNC_BEGIN(T) \
		COPY_FUNC_NO_COPY_BEGIN(T) \
		allocateInBuffer(T, ecopy, e);

#define COPY_FUNC_NO_COPY_BEGIN(T) \
		COPY_FUNC_PROTO(T) {

#define COPY_FUNC_END \
	return ecopy; \
		}
#define COPIER(e) \
		cpfn(buf, p, pointers, e, objectMap, generatePtrDesc)
typedef void * (*Copier)(unsigned char *, unsigned char **, unsigned char **, void *, Hashtable *, int);

COPY_FUNC_PROTO_OBJ_MAP(char);
COPY_FUNC_PROTO_OBJ_MAP(Node);
COPY_FUNC_PROTO_COPIER(Bucket);
COPY_FUNC_PROTO_COPIER(Hashtable);
COPY_FUNC_PROTO_COPIER(Env);
COPY_FUNC_PROTO_OBJ_MAP(RuleDesc);
COPY_FUNC_PROTO_OBJ_MAP(RuleSet);
COPY_FUNC_PROTO_OBJ_MAP(RuleIndexList);
COPY_FUNC_PROTO_OBJ_MAP(RuleIndexListNode);
COPY_FUNC_PROTO_OBJ_MAP(CondIndexVal);
Cache *copyCache(unsigned char **buf, size_t size, Cache *c);
Cache *restoreCache(unsigned char *buf);
void applyDiff(unsigned char *pointers, long pointersSize, long diff, long pointerDiff);
void applyDiffToPointers(unsigned char *pointers, long pointersSize, long pointerDiff);
void updateCache(unsigned char *shared, size_t size, Cache *cache, int forceReload);
#endif
