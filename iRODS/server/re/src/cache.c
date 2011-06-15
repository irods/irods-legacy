/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "cache.h"
#include "rules.h"
#include "functions.h"


#define NODE_KEY_SIZE 1024
void envKey(Env *node, char *keyBuf) {
	memset(keyBuf, 0, NODE_KEY_SIZE);
		snprintf(keyBuf, NODE_KEY_SIZE, "%p", node);
}
void nodeKey(Node *node, char *keyBuf) {
	memset(keyBuf, 0, NODE_KEY_SIZE);
	if(node->degree>0) {
		snprintf(keyBuf, NODE_KEY_SIZE, "%p", node);
	} else {
	char *p = keyBuf;
	snprintf(p, NODE_KEY_SIZE, "%s::%d::%p::%ld::%p::%d::%d::%s::%d",
			node->base, node->coerce, node->coercionType, node->expr, node->exprType,
			(int)node->iotype, (int)node->nodeType, node->text, (int)node->vararg
			);
	int len = strlen(keyBuf);
	p += len;
	switch(node->nodeType) {
	case N_FD_C_FUNC:
		snprintf(p, NODE_KEY_SIZE - len, "::%p", node->value.func);
		break;
	case N_FD_DECONSTRUCTOR:
		snprintf(p, NODE_KEY_SIZE - len, "::%d", node->value.proj);
		break;
	case N_ERROR:
		snprintf(p, NODE_KEY_SIZE - len, "::%d", node->value.errcode);
		break;
	case N_FUNC_SYM_LINK:
		snprintf(p, NODE_KEY_SIZE - len, "::%d", node->value.nArgs);
		break;
	case N_PARTIAL_APPLICATION:
		snprintf(p, NODE_KEY_SIZE - len, "::%d", node->value.nArgs);
		break;
	case T_VAR:
		snprintf(p, NODE_KEY_SIZE - len, "::%d", node->value.vid);
		break;
	case N_VAL:
		switch(TYPE(node)) {
		case T_BOOL:
		case T_INT:
		case T_DOUBLE:
			snprintf(p, NODE_KEY_SIZE - len, "::%f", node->value.dval);
			break;
		case T_DATETIME:
			snprintf(p, NODE_KEY_SIZE - len, "::%ld", (long) node->value.tval);
			break;
		case T_ERROR:
			snprintf(p, NODE_KEY_SIZE - len, "::%d", node->value.errcode);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	}
}
Node *copyNode(unsigned char **buf, Node *node, Hashtable *objectMap) {
  unsigned char *p = *buf;
  allocate(p, Node, ncopy, *node);
  ExprType *shared;
  char key[NODE_KEY_SIZE];
  nodeKey(node, key);
  if((shared = (Node *)lookupFromHashTable(objectMap, key)) != NULL) {
/*
      printf("simp type %s is shared\n", typeName_TypeConstructor(type->t));
*/
      return shared;
/*  } else if(node->nodeType == T_VAR && (shared=(ExprType *)lookupFromHashTable(objectMap, typeName_NodeType(node->nodeType)))!=NULL) {

      printf("tvar %s is shared\n", tvarNameBuf);

      return shared;
*/  } else {
      allocate(p, ExprType, ncopy, *node);
      if(ncopy->exprType != NULL) ncopy -> exprType = copyNode(&p, ncopy->exprType, objectMap);
      if(ncopy->coercionType!=NULL) ncopy -> coercionType = copyNode(&p, ncopy->coercionType, objectMap);
      if(ncopy->base!=NULL) {
          allocateArray(p, char, strlen(node->base)+1, ncopy->base, node->base);
      }
      if(ncopy->text!=NULL) {
          allocateArray(p, char, strlen(node->text)+1, ncopy->text, node->text);
      }
      allocateArray(p, NodePtr, ncopy->degree, ncopy->subtrees, node->subtrees);
      int i;
      for(i=0;i<ncopy->degree;i++) {
        ncopy ->subtrees[i] = copyNode(&p, node->subtrees[i], objectMap);
      }
      *buf = p;
/*      printf("inserting %s\n", key); */
	  insertIntoHashTable(objectMap, key, ncopy);
/*
          printf("tvar %s is added to shared objects\n", tvarNameBuf);
*/
      return ncopy;
  }
}
RuleDesc *copyRuleDesc(unsigned char **buf, RuleDesc *h, Hashtable *objectMap) {
  unsigned char *p = *buf;
  allocate(p, RuleDesc, rdcopy, *h);
  rdcopy->node = h->node!=NULL?copyNode(&p, h->node, objectMap):NULL;
  rdcopy->type = h->type!=NULL?copyNode(&p, h->type, objectMap):NULL;
  *buf = p;
  return rdcopy;
}
RuleSet *copyRuleSet(unsigned char **buf, RuleSet *h, Hashtable *objectMap) {
  unsigned char *p = *buf;
  allocate(p, RuleSet, rscopy, *h);
  int i =0;
  for(i=0;i<h->len;i++) {
    rscopy->rules[i] = copyRuleDesc(&p, rscopy->rules[i], objectMap);
  }
  *buf = p;
  return rscopy;
}
char *copyString(unsigned char **buf, char *string) {
    char *ret;
    unsigned char *p = *buf;
    allocateArray(p, char, strlen(string)+1, ret, string);
    *buf = p;
    return ret;
}

RuleIndexListNode *copyRuleIndexListNode(unsigned char **buf, RuleIndexListNode *node) {
    unsigned char *p = *buf;
    allocate(p, RuleIndexListNode, ncopy, *node);
    MAKE_COPY(p, RuleIndexListNode, node->next, ncopy->next);
    *buf = p;
    return ncopy;
}
RuleIndexList *copyRuleIndexList(unsigned char **buf, RuleIndexList *list) {
    unsigned char *p = *buf;
    allocate(p, RuleIndexList, lcopy, *list);
    MAKE_COPY(p, RuleIndexListNode, list->head, lcopy->head);
    MAKE_COPY(p, RuleIndexListNode, list->tail, lcopy->tail);
    MAKE_COPY(p, String, list->ruleName, lcopy->ruleName);
    *buf = p;
    return lcopy;
}
Env* copyEnv(unsigned char **buf, Env *e, void *(*cpfn)(unsigned char **, void *, Hashtable *), Hashtable *objectMap) {
	  Env *shared;
	  char key[NODE_KEY_SIZE];
	  envKey(e, key);
	  if((shared = (Env *)lookupFromHashTable(objectMap, key)) != NULL) {

	      return shared;
	  } else {
		unsigned char *p = *buf;
		allocate(p, Env, ecopy, *e);
		ecopy->previous = e->previous!=NULL? copyEnv(&p, e->previous, cpfn, objectMap) : NULL;
		ecopy->lower = e->lower!=NULL? copyEnv(&p, e->lower, cpfn, objectMap) : NULL;
		ecopy->current = copyHashtable(&p, e->current, cpfn, objectMap);
		*buf = p;
		insertIntoHashTable(objectMap, key, ecopy);
		return ecopy;
	  }
}
Hashtable* copyHashtable(unsigned char **buf, Hashtable *h, void *(*cpfn)(unsigned char **, void *, Hashtable *), Hashtable *objectMap) {
  unsigned char *p = *buf;
  allocate(p, Hashtable, hcopy, *h);
  allocateArray(p, BucketPtr, h->size, hcopy->buckets, h->buckets);
  int i;
  for(i=0;i<hcopy->size;i++) {
    struct bucket *b = hcopy->buckets[i];
    struct bucket *prev = NULL;
    while(b!=NULL) {
      allocate(p, Bucket, bcopy, *b);
      if(prev == NULL) {
        hcopy->buckets[i] = bcopy;
      } else {
        prev->next = bcopy;
      }

      /* copy key */
      allocateArray(p, char, strlen(b->key) + 1, bcopy->key, b->key);
      /* copy value */
      bcopy->value = cpfn(&p, b->value, objectMap);

      prev = bcopy;
      b = b->next;
    }
  }
  *buf = p;
  return hcopy;

}

CondIndexVal *copyCondIndexVal(unsigned char **buf, CondIndexVal *civ, Hashtable *objectMap) {
    unsigned char *p = *buf;
    allocate(p, CondIndexVal, civcopy, *civ);
    civcopy->condExp = copyNode(&p, civcopy->condExp, objectMap);
    civcopy->params = copyNode(&p, civcopy->params, objectMap);
    civcopy->valIndex = copyHashtable(&p, civcopy->valIndex, (Copier)copyRuleIndexListNode, objectMap);
    *buf = p;
    return civcopy;
}

Cache *copyCache(unsigned char **buf, long size, Cache *c) {
    Hashtable *objectMap = newHashTable(100);
    unsigned char *start = *buf;

    unsigned char *p = *buf;
    ((CacheRecordDesc *)p)->type = Cache_T;
    ((CacheRecordDesc *)p)->length = 1;
    p+= sizeof(CacheRecordDesc);
    Cache *ccopy = ((Cache *)p);
    *ccopy = *c;
    p+=sizeof(Cache);
    /*allocate(p, Cache, ccopy, *c); */
    /* shared objects */
    Region *r = make_region(0, NULL);
    region_free(r);

    ccopy->address = *buf;
    ccopy->coreRuleSet = ccopy->coreRuleSet == NULL? NULL:copyRuleSet(&p, ccopy->coreRuleSet, objectMap);
    ccopy->appRuleSet = ccopy->appRuleSet == NULL? NULL:copyRuleSet(&p, ccopy->appRuleSet, objectMap);
    ccopy->extRuleSet = NULL;
    ccopy->extRuleSetStatus = UNINITIALIZED;
    ccopy->ruleIndex = NULL; /* ccopy->ruleIndex == NULL? NULL:copyHashtable(&p, ccopy->ruleIndex, (Copier)copyRuleIndexList, objectMap); */
    ccopy->ruleIndexStatus = UNINITIALIZED;
    ccopy->condIndex = NULL; /* ccopy->condIndex == NULL? NULL:copyHashtable(&p, ccopy->condIndex, (Copier)copyCondIndexVal, objectMap); */
    ccopy->condIndexStatus = UNINITIALIZED;
    ccopy->coreFuncDescIndex = copyEnv(&p, ccopy->coreFuncDescIndex, (Copier)copyNode, objectMap);
    ccopy->appFuncDescIndex = copyEnv(&p, ccopy->appFuncDescIndex, (Copier)copyNode, objectMap);
    ccopy->extFuncDescIndex = NULL;
    ccopy->extFuncDescIndexStatus = UNINITIALIZED;
    ccopy->dataSize = (p - (*buf));

    *buf = p;

    deleteHashTable(objectMap, nop);
    ccopy->address = start;
    ccopy->dataSize = *buf - start;
    ccopy->cacheSize = size;
    return ccopy;
}
Cache *restoreCache(unsigned char *buf) {
    if(((CacheRecordDesc *)buf)->type != Cache_T) {
        /* error */
        return NULL;
    }
    Cache *cache = (Cache *)(buf + sizeof(CacheRecordDesc));
    unsigned char *bufCopy = (unsigned char *)malloc(cache->dataSize);
    if(bufCopy == NULL) {
        return NULL;
    }
    memcpy(bufCopy, buf, cache->dataSize);

    cache = (Cache *)(bufCopy + sizeof(CacheRecordDesc));
    unsigned char *bufOffset = cache->address;
    unsigned char *bufCopyOffset = bufCopy;

    long diff = bufCopyOffset - bufOffset;
    unsigned char *p = bufCopy;
    while(p < bufCopyOffset + cache->dataSize) {
        enum cacheRecordType type = ((CacheRecordDesc *)p)->type;
        int length = ((CacheRecordDesc *)p)->length;
        p+=sizeof(CacheRecordDesc);
        int i, j;
        switch(type) {
            case Cache_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Cache *)p)->condIndex, Hashtable, diff);
                    APPLY_DIFF(((Cache *)p)->ruleIndex, Hashtable, diff);
                    APPLY_DIFF(((Cache *)p)->coreRuleSet, RuleSet, diff);
                    APPLY_DIFF(((Cache *)p)->appRuleSet, RuleSet, diff);
                    APPLY_DIFF(((Cache *)p)->address, unsigned char, diff);
                    p+=sizeof(Cache);
                }
                break;
            case Hashtable_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Hashtable *)p)->buckets, struct bucket *, diff);
                    p+=sizeof(Hashtable);
                }
                break;
            case Bucket_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Bucket *)p)->key, char, diff);
                    APPLY_DIFF(((Bucket *)p)->next, struct bucket, diff);
                    p+=sizeof(Bucket);
                }
                break;
            case BucketPtr_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(*((BucketPtr *)p), struct bucket, diff);
                    p+=sizeof(BucketPtr);
                }
                break;
            case NodeType_T:
                p+= length * sizeof(NodeType);
                break;
            case Env_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Env *)p)->previous, Env, diff);
                    APPLY_DIFF(((Env *)p)->lower, Env, diff);
                    APPLY_DIFF(((Env *)p)->current, Hashtable, diff);
                    p+=sizeof(Env);
                }
                break;
            case char_T:
                p+= length * sizeof(char);
                break;
            case int_T:
                p+=length * sizeof(int);
                break;
            case ExprType_T:
                for(i=0;i<length;i++) {
                    switch(((ExprType *)p)->nodeType) {
                        case T_CONS:
                            for(j=0;j<T_CONS_ARITY((ExprType *)p);j++) {
                                APPLY_DIFF(T_CONS_TYPE_ARG((ExprType *)p, j), ExprType, diff);
                            }
                            APPLY_DIFF(T_CONS_TYPE_NAME((ExprType *)p), char, diff);
                            break;
                        case T_VAR:
                            APPLY_DIFF(T_VAR_DISJUNCTS((ExprType *)p), ExprTypePtr, diff);
                            break;
                        case T_IRODS:
                            APPLY_DIFF(((ExprType *)p)->text, char, diff);
                            break;
                        default:
                            break;
                    }

                    p+=sizeof(ExprType);
                }

                break;
            case ExprTypePtr_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(*((ExprTypePtr *)p), ExprType, diff);
                    p+=sizeof(ExprTypePtr);
                }
                break;
            case Node_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Node *)p)->coercionType, ExprType, diff);
                    APPLY_DIFF(((Node *)p)-> exprType, ExprType, diff);
                    APPLY_DIFF(((Node *)p)->subtrees, NodePtr, diff);
                    p+=sizeof(Node);
                }
                break;
            case NodePtr_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(*((NodePtr *)p), Node, diff);
                    p+=sizeof(NodePtr);
                }
                break;
            case RuleDesc_T:
                APPLY_DIFF(((RuleDesc *)p)->node, Node, diff);
                APPLY_DIFF(((RuleDesc *)p)->type, Node, diff);
            case RuleSet_T:
                for(i=0;i<length;i++) {
                    int j;
                    for(j=0;j<((RuleSet *)p)->len;j++) {
                        APPLY_DIFF(((RuleSet *)p)->rules[j], RuleDesc, diff);
                    }


                    p+=sizeof(RuleSet);
                }
                break;
            case CondIndexVal_T:

                for(i=0;i<length;i++) {
                    APPLY_DIFF(((CondIndexVal *)p)->condExp, Node, diff);
                    APPLY_DIFF(((CondIndexVal *)p)->params, Node, diff);
                    APPLY_DIFF(((CondIndexVal *)p)->valIndex, Hashtable, diff);

                    /*((Cache *)p)->offset += diff; */
                    p+=sizeof(CondIndexVal);
                }
                break;
            case RuleIndexListNode_T:
                APPLY_DIFF(((RuleIndexListNode *)p)->next, RuleIndexListNode, diff);
                break;
            case RuleIndexList_T:
                APPLY_DIFF(((RuleIndexList *)p)->head, RuleIndexListNode, diff);
                APPLY_DIFF(((RuleIndexList *)p)->tail, RuleIndexListNode, diff);
                APPLY_DIFF(((RuleIndexList *)p)->ruleName, char, diff);
                break;
        }
    }
    return cache;


}


int lockMutex(sem_t **mutex) {
  *mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
  if(*mutex == SEM_FAILED)
    {
      perror("unable to create semaphore");
      sem_unlink(SEM_NAME);
      return -1;
    } else {
      int v;
      sem_getvalue(*mutex, &v);
      printf("sem val0: %d\n", v);
      sem_wait(*mutex);
      sem_getvalue(*mutex, &v);
      printf("sem val1: %d\n", v);
      sem_close(*mutex);
      return 0;
    }

}
void unlockMutex(sem_t **mutex) {
  int v;
  *mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
  sem_getvalue(*mutex, &v);
  printf("sem val2: %d\n", v);
  sem_post(*mutex);
  sem_getvalue(*mutex, &v);
  printf("sem val3: %d\n", v);
  sem_close(*mutex);
}

