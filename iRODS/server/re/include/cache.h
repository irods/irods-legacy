/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef CACHE_H
#define CACHE_H
#include "rules.h"
#include "index.h"
#include "configuration.h"
#include "region.h"
/* #define CACHE_ENABLE 0 */

#define allocateInBuffer(p, ty, vn, val, gd) \
	if(gd) { \
    ((CacheRecordDesc *)p)->type = CONCAT(ty,_T); \
    ((CacheRecordDesc *)p)->length = 1; \
    p+= roundToAlignment(sizeof(CacheRecordDesc)); \
	} \
    ty *vn = ((ty *)p); \
    *vn = val; p+=sizeof(ty)
#define allocateArrayInBuffer(p, elemTy, n, lval, val, gd) \
	if(gd) { \
    ((CacheRecordDesc *)p)->type = CONCAT(elemTy,_T); \
    ((CacheRecordDesc *)p)->length = n; \
    p+= roundToAlignment(sizeof(CacheRecordDesc)); \
	} \
    lval = ((elemTy *)p); \
    memcpy(lval, val, sizeof(elemTy)*(n)); \
    p+=sizeof(elemTy) * (n);
#define SHMMAX 30000000
#define SHM_BASE_ADDR ((void *)0x80000000)
#define shm_rname "SHM"
#define APPLY_DIFF(p, t, d) if((p)!=NULL){unsigned char *temp = (unsigned char *)p; temp+=(d); (p)=(t *)temp;}
#define MAKE_COPY(buf, type, src, tgt, gd) if((src)!=NULL) {tgt=CONCAT(copy, type)(&(buf), src, gd);}
typedef void * (*Copier)(unsigned char **, void *, Hashtable *, int);
enum cacheRecordType {
        Cache_T,
        ExprType_T,
        ExprTypePtr_T,
        Node_T,
        NodePtr_T,
        Bucket_T,
        BucketPtr_T,
        Hashtable_T,
        RuleDesc_T,
        RuleSet_T,
        CondIndexVal_T,
        RuleIndexList_T,
        RuleIndexListNode_T,
        NodeType_T,
        Env_T,
        char_T,
        int_T,
    };

typedef struct {
    enum cacheRecordType type;
    int length;
} CacheRecordDesc;


char *copyString(unsigned char **buf, char *string, int generateDescriptor);
Node *copyNode(unsigned char **buf, Node *node, Hashtable *objectMap, int generateDescriptor);
Hashtable* copyHashtable(unsigned char **buf, Hashtable *h, void *(*cpfn)(unsigned char **, void *, Hashtable *, int), Hashtable *objectMap, int generateDescriptor);
Env *copyEnv(unsigned char **buf, Env *e, void *(*cpfn)(unsigned char **, void *, Hashtable *, int), Hashtable *objectMap, int generateDescriptor);
RuleDesc *copyRuleDesc(unsigned char **buf, RuleDesc *h, Hashtable *objectMap, int generateDescriptor);
Hashtable* copyHashtableCharPrtToIntPtr(unsigned char **buf, Hashtable *h, Hashtable *objectMap, int generateDescriptor);
RuleSet *copyRuleSet(unsigned char **buf, RuleSet *h, Hashtable *objectMap, int generateDescriptor);
CondIndexVal *copyCondIndexVal(unsigned char **buf, CondIndexVal *civ, Hashtable *objectMap, int generateDescriptor);
Cache *copyCache(unsigned char **buf, long size, Cache *c);
Cache *restoreCache(unsigned char *buf);
#endif
