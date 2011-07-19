/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include <limits.h>
#include "cache.h"
#include "rules.h"
#include "functions.h"


#define KEY_SIZE 1024
void keychar(char *node, char *keyBuf) {
	int len = snprintf(keyBuf, KEY_SIZE, "string::%s", node);
	if(len >= KEY_SIZE) {
		snprintf(keyBuf, KEY_SIZE, "pointer::%p", node);
	}

}
void keyRuleIndexListNode(RuleIndexListNode *node, char *keyBuf) {
	memset(keyBuf, 0, KEY_SIZE);
	snprintf(keyBuf, KEY_SIZE, "pointer::%p", node);
}
void keyEnv(Env *node, char *keyBuf) {
	memset(keyBuf, 0, KEY_SIZE);
	snprintf(keyBuf, KEY_SIZE, "pointer::%p", node);
}
void keyNode(Node *node, char *keyBuf) {
	memset(keyBuf, 0, KEY_SIZE);
	if(node->degree>0) {
		snprintf(keyBuf, KEY_SIZE, "%p", node);
	} else {
		char *p = keyBuf;
		int len = snprintf(p, KEY_SIZE, "node::%d::%p::%lld::%p::%d::%s::%s::",
				node->option, node->coercionType, node->expr, node->exprType,
				(int)node->nodeType, node->base, node->text
				);
		if(len + sizeof(union node_ext) * 3 >= KEY_SIZE) {
			snprintf(keyBuf, KEY_SIZE, "pointer::%p", node);
			return;
		}
		unsigned int i;
		for(i=0;i<sizeof(union node_ext);i++) {
			len += sprintf(keyBuf + len, "%02X ", ((unsigned char *)&(node->value))[i]);
		}
	}
}
COPY_FUNC_OBJ_MAP_SHARED_BEGIN(Node)
      MK_COPY_OBJ_MAP(Node, e->exprType, ecopy->exprType);
      MK_COPY_OBJ_MAP(Node, e->coercionType, ecopy->coercionType);
      MK_COPY_OBJ_MAP(char, e->base, ecopy->base);
      MK_COPY_OBJ_MAP(char, e->text, ecopy->text);
      allocateArrayInBuffer(NodePtr, e->degree, ecopy->subtrees, e->subtrees);
      MK_POINTER(&(ecopy->subtrees));
      int i;
      for(i=0;i<e->degree;i++) {
    	  MK_COPY_OBJ_MAP(Node, e->subtrees[i], ecopy->subtrees[i]);
      }
      if(e->nodeType == N_FD_RULE_INDEX_LIST) {
    	  MK_COPY_OBJ_MAP(RuleIndexList,FD_RULE_INDEX_LIST(e), FD_RULE_INDEX_LIST_LVAL(ecopy));
      }
/*      printf("inserting %s\n", key); */
	  insertIntoHashTable(objectMap, key, ecopy);
/*
          printf("tvar %s is added to shared objects\n", tvarNameBuf);
*/
COPY_FUNC_END

COPY_FUNC_OBJ_MAP_BEGIN(RuleDesc)
  MK_COPY_OBJ_MAP(Node, e->node, ecopy->node);
  MK_COPY_OBJ_MAP(Node, e->type, ecopy->type);
COPY_FUNC_END
COPY_FUNC_OBJ_MAP_BEGIN(RuleSet)
  int i =0;
  for(i=0;i<e->len;i++) {
    MK_COPY_OBJ_MAP(RuleDesc, e->rules[i], ecopy->rules[i]);
  }
COPY_FUNC_END
COPY_FUNC_OBJ_MAP_SHARED_NO_COPY_BEGIN(char)
	char *ecopy;
    allocateArrayInBuffer(char, strlen(e)+1, ecopy, e);
COPY_FUNC_END

COPY_FUNC_OBJ_MAP_SHARED_BEGIN(RuleIndexListNode)
    MK_COPY_OBJ_MAP(RuleIndexListNode, e->next, ecopy->next);
    MK_COPY_OBJ_MAP(CondIndexVal, e->condIndex, ecopy->condIndex);
COPY_FUNC_END
COPY_FUNC_OBJ_MAP_BEGIN(RuleIndexList)
    MK_COPY_OBJ_MAP(RuleIndexListNode, e->head, ecopy->head);
    MK_COPY_OBJ_MAP(RuleIndexListNode, e->tail, ecopy->tail);
    MK_COPY_OBJ_MAP(char, e->ruleName, ecopy->ruleName);
COPY_FUNC_END
COPY_FUNC_COPIER_SHARED_BEGIN(Env)
		MK_COPY_COPIER(Env, e->previous, ecopy->previous, cpfn);
		MK_COPY_COPIER(Env, e->lower, ecopy->lower, cpfn);
		MK_COPY_COPIER(Hashtable, e->current, ecopy->current, cpfn);
		insertIntoHashTable(objectMap, key, ecopy);
COPY_FUNC_END
COPY_FUNC_COPIER_BEGIN(Bucket)
	MK_COPY_OBJ_MAP(char, e->key, ecopy->key);
	ecopy->value = COPIER(e->value);
	MK_POINTER(&(ecopy->value));
    MK_COPY_COPIER(Bucket, e->next, ecopy->next, cpfn);
COPY_FUNC_END
COPY_FUNC_COPIER_BEGIN(Hashtable)
	allocateArrayInBuffer(BucketPtr, e->size, ecopy->buckets, e->buckets);
	MK_POINTER(&(ecopy->buckets));
	int i;
	for(i=0;i<e->size;i++) {
		MK_COPY_COPIER(Bucket, e->buckets[i], ecopy->buckets[i], cpfn);
	}

COPY_FUNC_END

COPY_FUNC_OBJ_MAP_BEGIN(CondIndexVal)
    MK_COPY_OBJ_MAP(Node, e->condExp, ecopy->condExp);
    MK_COPY_OBJ_MAP(Node, e->params, ecopy->params);
    MK_COPY_COPIER(Hashtable, e->valIndex, ecopy->valIndex, (Copier)copyRuleIndexListNode);
COPY_FUNC_END

Cache *copyCache(unsigned char **p, size_t size, Cache *c) {
	if(size%ALIGNMENT != 0) { /* size should be divisible by ALIGNMENT */
		return NULL;
	}

	unsigned char *buf = *p;
	unsigned char *pointers0 = buf + size;

    /* shared objects */
	Hashtable *objectMap = newHashTable(100);
    unsigned char **pointers = &pointers0;
    int generatePtrDesc = 1;

    allocateInBuffer(Cache, ccopy, c);

    MK_POINTER(&(ccopy->address));
    MK_POINTER(&(ccopy->pointers));
    MK_COPY_OBJ_MAP(RuleSet, c->coreRuleSet, ccopy->coreRuleSet);
    ccopy->coreRuleSetStatus = COMPRESSED;
    ccopy->appRuleSet = NULL;
    ccopy->appRuleSetStatus = UNINITIALIZED;
    ccopy->extRuleSet = NULL;
    ccopy->extRuleSetStatus = UNINITIALIZED;
    MK_COPY_COPIER(Env, c->coreFuncDescIndex, ccopy->coreFuncDescIndex, (Copier)copyNode);
    ccopy->coreFuncDescIndexStatus = COMPRESSED;
    ccopy->appFuncDescIndex = NULL;
    ccopy->appFuncDescIndexStatus = UNINITIALIZED;
    ccopy->extFuncDescIndex = NULL;
    ccopy->extFuncDescIndexStatus = UNINITIALIZED;
    ccopy->dataSize = (*p - buf);
    ccopy->address = buf;
    ccopy->pointers = pointers0;
	ccopy->cacheSize = size;
    ccopy->cacheStatus = INITIALIZED;
	ccopy->appRegion = NULL;
	ccopy->appRegionStatus = UNINITIALIZED;
	ccopy->coreRegion = NULL;
	ccopy->coreRegionStatus = UNINITIALIZED;
	ccopy->extRegion = NULL;
	ccopy->extRegionStatus = UNINITIALIZED;

    deleteHashTable(objectMap, nop);

    return ccopy;
}
Cache *restoreCache(unsigned char *buf) {
	mutex_type *mutex;
    Cache *cache = (Cache *) buf;
    unsigned char *bufCopy;
    unsigned char *pointers;
    size_t pointersSize;
    unsigned char *bufMapped;

    unsigned char *pointersCopy;
    unsigned char *pointersMapped;
    size_t dataSize;
    unsigned int version, version2;
    int success = 0;
    do {
		lockMutex(&mutex);
    	version = cache->version;
        unlockMutex(&mutex);
    	dataSize = cache->dataSize;
    	pointersMapped = cache->pointers;
    	bufMapped = cache->address;
    	if(pointersMapped<bufMapped || pointersMapped -bufMapped > SHMMAX || dataSize > SHMMAX) {
    		sleep(1);
    		continue;
    	}
    	bufCopy = (unsigned char *)malloc(dataSize);
    	if(bufCopy == NULL) {
			return NULL;
		}
		memcpy(bufCopy, buf, cache->dataSize);
    	pointersSize = bufMapped + SHMMAX - pointersMapped;
		pointersCopy = (unsigned char *)malloc(pointersSize);
    	if(pointersCopy == NULL) {
    		free(bufCopy);
			return NULL;
		}
		memcpy(pointersCopy, pointersMapped+(buf - bufMapped), pointersSize);
		lockMutex(&mutex);
    	version2 = cache->version;
        unlockMutex(&mutex);
		if(version2 != version) {
			free(bufCopy);
			free(pointersCopy);
			sleep(1);
		} else {
			success = 1;
		}
    } while(!success);
    pointers = pointersCopy;

/*    bufCopy = (unsigned char *)malloc(cache->dataSize);

    if(bufCopy == NULL) {
        return NULL;
    }
    memcpy(bufCopy, buf, cache->dataSize);

    bufMapped = cache->address;

    long diffBuf = buf - bufMapped;
    pointers = cache->pointers + diffBuf;
    pointersSize = pointers - buf; */
    long diff = bufCopy - bufMapped;
    long pointerDiff = diff;
    applyDiff(pointers, pointersSize, diff, pointerDiff);
    free(pointersCopy);
    cache = (Cache *) bufCopy;
    return cache;
}
void applyDiff(unsigned char *pointers, long pointersSize, long diff, long pointerDiff) {
    unsigned char *p;
#ifdef DEBUG_VERBOSE
    printf("storing cache from buf = %p, pointers = %p\n", buf, pointers);
#endif
    for (p = pointers; p - pointers < pointersSize; p+=CACHE_SIZE(unsigned char *, 1)) {
#ifdef DEBUG_VERBOSE
    	printf("p = %p\n", p);
#endif
    	unsigned char *pointer = *((unsigned char **)p) + pointerDiff;
#ifdef DEBUG_VERBOSE
    	printf("applying diff to pointer at %p\n", pointer);
#endif
    	*((unsigned char **)pointer) += diff;
    }
}

void applyDiffToPointers(unsigned char *pointers, long pointersSize, long pointerDiff) {
        unsigned char *p;
    #ifdef DEBUG_VERBOSE
        printf("storing cache from buf = %p, pointers = %p\n", buf, pointers);
    #endif
        for (p = pointers; p - pointers < pointersSize; p+=CACHE_SIZE(unsigned char *, 1)) {
    #ifdef DEBUG_VERBOSE
        	printf("p = %p\n", p);
    #endif
        	*((unsigned char **)p) += pointerDiff;
    #ifdef DEBUG_VERBOSE
        	printf("applying diff to pointer at %p\n", pointer);
    #endif
        }

}
void updateCache(unsigned char *shared, size_t size, Cache *cache, int forceReload) {
	mutex_type *mutex;
	time_type timestamp;
	time_type_set(timestamp, cache->timestamp);

	lockMutex(&mutex);
	if(forceReload || time_type_gt(timestamp, ((Cache *)shared)->updateTS)) {
		time_type_set(((Cache *)shared)->updateTS, timestamp);
		unlockMutex(&mutex);

		unsigned char *buf = (unsigned char *) malloc(size);
		if(buf != NULL) {
			unsigned char *cacheBuf = buf;
			Cache *cacheCopy = copyCache(&cacheBuf, size, cache);
			if(cacheCopy != NULL) {
#ifdef DEBUG
				printf("Buffer usage: %fM\n", ((double)(cacheCopy->dataSize))/(1024*1024));
#endif
				size_t pointersSize = (cacheCopy->address + cacheCopy->cacheSize) - cacheCopy->pointers;
				long diff = shared - cacheCopy->address;
				unsigned char *pointers = cacheCopy->pointers;

				applyDiff(pointers, pointersSize, diff, 0);
				applyDiffToPointers(pointers, pointersSize, diff);

				lockMutex(&mutex);
				if(forceReload || !time_type_gt(((Cache *)shared)->updateTS, timestamp)) {
					/* copy data */
					memcpy(shared, buf, cacheCopy->dataSize);
					/* copy pointers */
					memcpy(cacheCopy->pointers, pointers, pointersSize);
				}
				unlockMutex(&mutex);
			} else {
				rodsLog(LOG_ERROR, "Error updating the cache\n");
			}
			free(buf);
		} else {
			rodsLog(LOG_ERROR, "Cannot update the cache for out of memory error, let some other process update it later when memory is available\n");
		}
	} else {
		unlockMutex(&mutex);
		rodsLog(LOG_DEBUG, "Cache has been updated by some other process\n");
	}

}



