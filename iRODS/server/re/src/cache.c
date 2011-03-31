/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/fcntl.h>
#include "cache.h"
#include "rules.h"
#include "functions.h"

RuleEngineStatus _ruleEngineStatus = UNINITIALIZED;

RuleEngineStatus getRuleEngineStatus() {
    return _ruleEngineStatus;
}

ExprType *copyExprType(unsigned char **buf, ExprType *type, Hashtable *objectMap) {
  unsigned char *p = *buf;
  ExprType *shared;
  char tvarNameBuf[128];
  if((shared = (ExprType *)lookupFromHashTable(objectMap, typeName_TypeConstructor(type->t))) != NULL) {
/*
      printf("simp type %s is shared\n", typeName_TypeConstructor(type->t));
*/
      return shared;
  } else if(type->t == T_VAR && (shared=(ExprType *)lookupFromHashTable(objectMap, typeName_TypeConstructor(type->t)))!=NULL) {
/*
      printf("tvar %s is shared\n", tvarNameBuf);
*/
      return shared;
  } else {
      allocate(p, ExprType, tcopy, *type);
      if(tcopy->t == T_FUNC) {
        tcopy->ext.func.retType = copyExprType(&p, tcopy->ext.func.retType, objectMap);
        allocateArray(p, ExprTypePtr, tcopy->ext.func.arity, tcopy->ext.func.paramsType, type->ext.func.paramsType);
        int i;
        for (i=0;i<tcopy->ext.func.arity;i++) {
            tcopy->ext.func.paramsType[i] = copyExprType(&p, type->ext.func.paramsType[i], objectMap);
        }
      } else if(tcopy->t == T_CONS) {

        allocateArray(p, ExprTypePtr, tcopy->ext.cons.arity, tcopy->ext.cons.typeArgs, type->ext.cons.typeArgs);
        int i;
        for (i=0;i<tcopy->ext.cons.arity;i++) {
            tcopy->ext.cons.typeArgs[i] = copyExprType(&p, type->ext.cons.typeArgs[i], objectMap);
        }
        allocateArray(p, char, strlen(type->ext.cons.typeConsName)+1, tcopy->ext.cons.typeConsName, type->ext.cons.typeConsName);
      } else if(tcopy->t == T_VAR) {
          if(tcopy->ext.tvar.numDisjuncts != 0) {
              allocateArray(p, TypeConstructor, tcopy->ext.tvar.numDisjuncts, tcopy->ext.tvar.disjuncts, type->ext.tvar.disjuncts);
          }
          insertIntoHashTable(objectMap, getTVarName(tcopy->ext.tvar.vid, tvarNameBuf), tcopy);
/*
          printf("tvar %s is added to shared objects\n", tvarNameBuf);
*/
      } else if(tcopy->t == T_IRODS) {
        allocateArray(p, char, strlen(type->ext.irods.name)+1, tcopy->ext.irods.name, type->ext.irods.name);
      }
      *buf = p;
      return tcopy;
  }
}

Node *copyNode(unsigned char **buf, Node *node, Hashtable *objectMap) {
  unsigned char *p = *buf;
  allocate(p, Node, ncopy, *node);
  if(ncopy->exprType != NULL) ncopy -> exprType = copyExprType(&p, ncopy->exprType, objectMap);
  if(ncopy->coercion!=NULL) ncopy -> coercion = copyExprType(&p, ncopy->coercion, objectMap);
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
  return ncopy;
}
RuleSet *copyRuleSet(unsigned char **buf, RuleSet *h, Hashtable *objectMap) {
  unsigned char *p = *buf;
  allocate(p, RuleSet, rscopy, *h);
  int i =0;
  for(i=0;i<h->len;i++) {
    rscopy->rules[i] = copyNode(&p, rscopy->rules[i], objectMap);
  }
  *buf = p;
  return rscopy;
}
Hashtable* copyHashtableCharPtrToIntPtr(unsigned char **buf, Hashtable *h) {
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
      allocateArray(p, int, 1, bcopy->value, b->value);

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
    civcopy->valIndex = copyHashtableCharPtrToIntPtr(&p, civcopy->valIndex);
    *buf = p;
    return civcopy;
}

Hashtable* copyHashtableCharPtrToCondIndexValPtr(unsigned char **buf, Hashtable *h, Hashtable *objectMap) {
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
      bcopy->value = copyCondIndexVal(&p, (CondIndexVal *)bcopy->value, objectMap);

      prev = bcopy;
      b = b->next;
    }
  }
  *buf = p;
  return hcopy;

}

Cache *copyCache(unsigned char **buf, Cache *c) {
    Hashtable *objectMap = newHashTable(100);


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
    insertIntoHashTable(objectMap, typeName_TypeConstructor(T_INT), copyExprType(&p, newSimpType(T_INT, r), objectMap));
    insertIntoHashTable(objectMap, typeName_TypeConstructor(T_BOOL), copyExprType(&p, newSimpType(T_BOOL, r), objectMap));
    insertIntoHashTable(objectMap, typeName_TypeConstructor(T_DOUBLE), copyExprType(&p, newSimpType(T_DOUBLE, r), objectMap));
    insertIntoHashTable(objectMap, typeName_TypeConstructor(T_DATETIME), copyExprType(&p, newSimpType(T_DATETIME, r), objectMap));
    insertIntoHashTable(objectMap, typeName_TypeConstructor(T_STRING), copyExprType(&p, newSimpType(T_STRING, r), objectMap));
    insertIntoHashTable(objectMap, typeName_TypeConstructor(T_DYNAMIC), copyExprType(&p, newSimpType(T_DYNAMIC, r), objectMap));
    region_free(r);

    ccopy->offset = *buf;
    ccopy->coreRuleSet = ccopy->coreRuleSet == NULL? NULL:copyRuleSet(&p, ccopy->coreRuleSet, objectMap);
    ccopy->coreRuleIndex = ccopy->coreRuleIndex == NULL? NULL:copyHashtableCharPtrToIntPtr(&p, ccopy->coreRuleIndex);
    ccopy->condIndex = ccopy->condIndex == NULL? NULL:copyHashtableCharPtrToCondIndexValPtr(&p, ccopy->condIndex, objectMap);
    ccopy->dataSize = (p - (*buf));

    *buf = p;

    deleteHashTable(objectMap, nop);
    return ccopy;
}
#define APPLY_DIFF(p, t, d) if((p)!=NULL){unsigned char *temp = (unsigned char *)p; temp+=(d); (p)=(t *)temp;}
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
    unsigned char *bufOffset = cache->offset;
    unsigned char *bufCopyOffset = bufCopy;

    long diff = bufCopyOffset - bufOffset;
    unsigned char *p = bufCopy;
    while(p < bufCopyOffset + cache->dataSize) {
        enum cacheRecordType type = ((CacheRecordDesc *)p)->type;
        int length = ((CacheRecordDesc *)p)->length;
        p+=sizeof(CacheRecordDesc);
        int i;
        switch(type) {
            case Cache_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Cache *)p)->condIndex, Hashtable, diff);
                    APPLY_DIFF(((Cache *)p)->coreRuleIndex, Hashtable, diff);
                    APPLY_DIFF(((Cache *)p)->coreRuleSet, RuleSet, diff);
                    APPLY_DIFF(((Cache *)p)->offset, unsigned char, diff);
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
            case TypeConstructor_T:
                p+= length * sizeof(TypeConstructor);
                break;
            case char_T:
                p+= length * sizeof(char);
                break;
            case int_T:
                p+=length * sizeof(int);
                break;
            case ExprType_T:
                for(i=0;i<length;i++) {
                    switch(((ExprType *)p)->t) {
                        case T_FUNC:
                            APPLY_DIFF(((ExprType *)p)->ext.func.retType, ExprType, diff);
                            APPLY_DIFF(((ExprType *)p)->ext.func.paramsType, ExprType *, diff);
                            break;
                        case T_CONS:
                            APPLY_DIFF(((ExprType *)p)->ext.cons.typeArgs, ExprType *, diff);
                            APPLY_DIFF(((ExprType *)p)->ext.cons.typeConsName, char, diff);
                            break;
                        case T_VAR:
                            APPLY_DIFF(((ExprType *)p)->ext.tvar.disjuncts, TypeConstructor, diff);
                            break;
                        case T_IRODS:
                            APPLY_DIFF(((ExprType *)p)->ext.irods.name, char, diff);
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
                    APPLY_DIFF(((Node *)p)->coercion, ExprType, diff);
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
            case RuleSet_T:
                for(i=0;i<length;i++) {
                    int j;
                    for(j=0;j<((RuleSet *)p)->len;j++) {
                        APPLY_DIFF(((RuleSet *)p)->rules[j], Node, diff);
                    }


                    p+=sizeof(RuleSet);
                }
                break;
            case CondIndexVal_T:

                for(i=0;i<length;i++) {
                    APPLY_DIFF(((CondIndexVal *)p)->condExp, Node, diff);
                    APPLY_DIFF(((CondIndexVal *)p)->valIndex, Hashtable, diff);

                    /*((Cache *)p)->offset += diff; */
                    p+=sizeof(CondIndexVal);
                }
                break;
        }
    }
    return cache;


}
#ifdef DEBUG
#define RULE_SET_DEF_LENGTH 1000
int rSplitStr(char *all, char *head, int headLen, char *tail, int tailLen, char sep) {
    char *i = strchr(all, sep);
    if(i==NULL) {
        tail[0] = '\0';
        strcpy(head, all);
    } else {
        strcpy(tail, i+1);
        strncpy(head, all, i-all);
        head[i-all] = '\0';
    }
    return 0;
}
#endif

#define SEM_NAME "irods_sem_re"
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

int loadRuleFromCacheOrFile(char *irbSet, ruleStruct_t *inRuleStruct) {
    char r1[NAME_LEN], r2[RULE_SET_DEF_LENGTH], r3[RULE_SET_DEF_LENGTH];
    strcpy(r2,irbSet);
    int res = 0;
    Region *r = make_region(0, NULL);
    unsigned char *buf = NULL;
    int loadToBuf;
    int unlock_mutex = 0;
    sem_t *mutex = NULL;
    if(CACHE_ENABLE) {

        if(lockMutex(&mutex) != 0) {
            res = -1;
            RETURN;
        }
        unlock_mutex = 1;
        
        int shmid = - 1;
        int key = 1200;
        void *addr = SHM_BASE_ADDR;
        if(isServer) {
            loadToBuf = 1;
            shmid = shmget(key, SHMMAX, IPC_CREAT /*| IPC_EXCL*/ | 0666);
            if(shmid!= -1) {
                unsigned char *shm = (unsigned char *)shmat(shmid, SHM_BASE_ADDR, 0);
                buf = shm;
            } else {
                buf = (unsigned char *)malloc(SHMMAX);
            }
        } else {
            shmid = shmget(key, SHMMAX, 0666);
            if(shmid != -1) { /* not server process and shm is successfully allocated */
                loadToBuf = 0;
                buf = (unsigned char *)shmat(shmid, addr, 0);
            } else {
                loadToBuf = 1;
                buf = (unsigned char *)malloc(SHMMAX);
            }
        }
    } else {
        loadToBuf = 1;
        buf = (unsigned char *)malloc(SHMMAX);
    }
    if(loadToBuf) {
        while (strlen(r2) > 0) {
                int i = rSplitStr(r2,r1,NAME_LEN,r3,RULE_SET_DEF_LENGTH,',');
                if (i == 0)
                  i = readRuleStructAndRuleSetFromFile(r1, inRuleStruct, r);
                if (i != 0) {
                  res = i;
                  RETURN;
                }
                strcpy(r2,r3);
        }
        Cache cacheBuf;
        cacheBuf.coreRuleIndex = coreRuleIndex;
        cacheBuf.coreRuleSet = &coreRules;
        cacheBuf.condIndex = condIndex;
#ifdef DEBUG
        unsigned char *bufStart = buf;
#endif
        Cache *cacheNew = copyCache(&buf, &cacheBuf);
#ifdef DEBUG
        printf("Buffer usage: %fM\n", ((double)(buf-bufStart))/(1024*1024));
#endif
        deleteHashTable(coreRuleIndex, free);
        deleteHashTable(condIndex, (void (*)(void *))deleteCondIndexVal);
        coreRuleIndex = cacheNew->coreRuleIndex;
        coreRules = *cacheNew->coreRuleSet;
        condIndex = cacheNew->condIndex;
    } else {
        Cache *cache = restoreCache(buf);
        coreRuleIndex = cache->coreRuleIndex;
        coreRules = *(cache->coreRuleSet);
        condIndex = cache->condIndex;
    }

ret:
    if(unlock_mutex)
    {
        unlockMutex(&mutex);
    }
    region_free(r);
    _ruleEngineStatus = INITIALIZED;

    return res;
}
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct, Region *r)
{
  int i = 0;
/*  char l0[MAX_RULE_LENGTH]; */
/*  char l1[MAX_RULE_LENGTH]; */
/*  char l2[MAX_RULE_LENGTH]; */
/*  char l3[MAX_RULE_LENGTH]; */
   char rulesFileName[MAX_NAME_LEN];
/*   FILE *file; */
/*   char buf[MAX_RULE_LENGTH]; */
   char *configDir;
/*   char *t; */
   i = inRuleStrct->MaxNumOfRules;

   if (ruleBaseName[0] == '/' || ruleBaseName[0] == '\\' ||
       ruleBaseName[1] == ':') {
     snprintf (rulesFileName,MAX_NAME_LEN, "%s",ruleBaseName);
   }
   else {
     configDir = getConfigDir ();
     snprintf (rulesFileName,MAX_NAME_LEN, "%s/reConfigs/%s.re", configDir,ruleBaseName);
   }
   /*file = fopen(rulesFileName, "r");
   if (file == NULL) {
#ifndef DEBUG
       rodsLog(LOG_NOTICE,
	     "readRuleStructFromFile() could not open rules file %s\n",
	     rulesFileName);
#endif
     return(RULES_FILE_READ_ERROR);
   }
   buf[MAX_RULE_LENGTH-1]='\0';
   while (fgets (buf, MAX_RULE_LENGTH-1, file) != NULL) {
     if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
     if (buf[0] == '#' || strlen(buf) < 4)
       continue;
		char *l0, *l2, *l3;
		// rSplitStr(buf, l1, MAX_RULE_LENGTH, l0, MAX_RULE_LENGTH, '|');
		l0 = nextRuleSection(buf, l1);
     inRuleStrct->action[i] = strdup(l1);
     inRuleStrct->ruleHead[i] = strdup(l1);
     if ((t = strstr(inRuleStrct->action[i],"(")) != NULL) {
       *t = '\0';
     }
     inRuleStrct->ruleBase[i] = strdup(ruleBaseName);
		// rSplitStr(l0, l1, MAX_RULE_LENGTH, l3, MAX_RULE_LENGTH,'|');
		l3 = nextRuleSection(l0, l1);
     inRuleStrct->ruleCondition[i] = strdup(l1);
		// rSplitStr(l3, l1, MAX_RULE_LENGTH, l2, MAX_RULE_LENGTH, '|');
		l2 = nextRuleSection(l3, l1);
     inRuleStrct->ruleAction[i] = strdup(l1);
     inRuleStrct->ruleRecovery[i] = strdup(l2);
     i++;
   }
   fclose (file);
   inRuleStrct->MaxNumOfRules = i;*/
	int errloc;
	rError_t errmsgBuf;
        errmsgBuf.errMsg = NULL;
        errmsgBuf.len = 0;

        char *buf = (char *) malloc(ERR_MSG_LEN*1024*sizeof(char));
        int res = 0;
	if(inRuleStrct == &coreRuleStrct) {
		if(readRuleSetFromFile(ruleBaseName,&coreRules,&errloc,&errmsgBuf, r)==0) {
                    createRuleNodeIndex(&coreRules, &coreRuleIndex);
                } else {
                    errMsgToString(&errmsgBuf, buf, ERR_MSG_LEN*1024);
                    rodsLog(LOG_ERROR, "%s", buf);
                    res = -1;
                }
	} else if(inRuleStrct == &appRuleStrct) {
		if(readRuleSetFromFile(ruleBaseName,&appRules,&errloc,&errmsgBuf, r)==0) {
                    createRuleNodeIndex(&appRules, &appRuleIndex);
                } else {
                    errMsgToString(&errmsgBuf, buf, ERR_MSG_LEN*1024);
                    rodsLog(LOG_ERROR, "%s", buf);
                    res = -1;
                }
	}
        free(buf);
        return res;
}
