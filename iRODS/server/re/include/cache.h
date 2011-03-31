/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef CACHE_H
#define CACHE_H
#include "rules.h"
#include "index.h"
#define CACHE_ENABLE 0

#define CONCAT2(a,b) a##b
#define CONCAT(a,b) CONCAT2(a,b)

#define allocate(p, ty, vn, val) \
    ((CacheRecordDesc *)p)->type = CONCAT(ty,_T); \
    ((CacheRecordDesc *)p)->length = 1; \
    p+= sizeof(CacheRecordDesc); \
    ty *vn = ((ty *)p); \
    *vn = val; p+=sizeof(ty)
#define allocateArray(p, elemTy, n, lval, val) \
    ((CacheRecordDesc *)p)->type = CONCAT(elemTy,_T); \
    ((CacheRecordDesc *)p)->length = n; \
    p+= sizeof(CacheRecordDesc); \
    lval = ((elemTy *)p); \
    memcpy(lval, val, sizeof(elemTy)*(n)); \
    p+=sizeof(elemTy) * (n);
#define SHMMAX 30000000
#define SHM_BASE_ADDR ((void *)0x80000000)

typedef struct {
    unsigned char *offset;
    long dataSize;
    RuleSet *coreRuleSet;
    Hashtable *coreRuleIndex;
    Hashtable *condIndex;
} Cache;

enum cacheRecordType {
        Cache_T,
        ExprType_T,
        ExprTypePtr_T,
        Node_T,
        NodePtr_T,
        Bucket_T,
        BucketPtr_T,
        Hashtable_T,
        RuleSet_T,
        CondIndexVal_T,
        TypeConstructor_T,
        char_T,
        int_T,
    };
typedef struct {
    enum cacheRecordType type;
    int length;
} CacheRecordDesc;

extern int isServer;

ExprType *copyExprType(unsigned char **buf, ExprType *type, Hashtable *objectMap);
Node *copyNode(unsigned char **buf, Node *node, Hashtable *objectMap);
Hashtable* copyHashtableCharPrtToIntPtr(unsigned char **buf, Hashtable *h, Hashtable *objectMap);
RuleSet *copyRuleSet(unsigned char **buf, RuleSet *h, Hashtable *objectMap);
CondIndexVal *copyCondIndexVal(unsigned char **buf, CondIndexVal *civ, Hashtable *objectMap);
Cache *copyCache(unsigned char **buf, Cache *c);
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct, Region *r);
int loadRuleFromCacheOrFile(char *irbSet, ruleStruct_t *inRuleStruct);

#endif
