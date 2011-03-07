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


ExprType *copyExprType(void **buf, ExprType *type) {
  void *p = *buf;
  allocate(p, ExprType, tcopy, *type);
  if(tcopy->t == T_FUNC) {
    tcopy->ext.func.retType = copyExprType(&p, tcopy->ext.func.retType);
    allocateArray(p, ExprTypePtr, tcopy->ext.func.arity, tcopy->ext.func.paramsType, type->ext.func.paramsType);
    int i;
    for (i=0;i<tcopy->ext.func.arity;i++) {
        tcopy->ext.func.paramsType[i] = copyExprType(&p, type->ext.func.paramsType[i]);
    }
  } else if(tcopy->t == T_CONS) {

    allocateArray(p, ExprTypePtr, tcopy->ext.cons.arity, tcopy->ext.cons.typeArgs, type->ext.cons.typeArgs);
    int i;
    for (i=0;i<tcopy->ext.cons.arity;i++) {
        tcopy->ext.cons.typeArgs[i] = copyExprType(&p, type->ext.cons.typeArgs[i]);
    }
  }
  *buf = p;
  return tcopy;
}

Node *copyNode(void **buf, Node *node) {
  void *p = *buf;
  allocate(p, Node, ncopy, *node);
  if(ncopy->exprType != NULL) ncopy -> exprType = copyExprType(&p, ncopy->exprType);
  if(ncopy->coercion!=NULL) ncopy -> coercion = copyExprType(&p, ncopy->coercion);
  if(ncopy->base!=NULL) {
      allocateArray(p, char, strlen(node->base)+1, ncopy->base, node->base);
  }
  allocateArray(p, NodePtr, ncopy->degree, ncopy->subtrees, node->subtrees);
  int i;
  for(i=0;i<ncopy->degree;i++) {
    ncopy ->subtrees[i] = copyNode(&p, node->subtrees[i]);
  }
  *buf = p;
  return ncopy;
}
RuleSet *copyRuleSet(void **buf, RuleSet *h) {
  void *p = *buf;
  allocate(p, RuleSet, rscopy, *h);
  int i =0;
  for(i=0;i<h->len;i++) {
    rscopy->rules[i] = copyNode(&p, rscopy->rules[i]);
  }
  *buf = p;
  return rscopy;
}
Hashtable* copyHashtableCharPtrToIntPtr(void **buf, Hashtable *h) {
  void *p = *buf;
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

      // copy key
      allocateArray(p, char, strlen(b->key) + 1, bcopy->key, b->key);
      // copy value
      allocateArray(p, int, 1, bcopy->value, b->value);

      prev = bcopy;
      b = b->next;
    }
  }
  *buf = p;
  return hcopy;

}

CondIndexVal *copyCondIndexVal(void **buf, CondIndexVal *civ) {
    void *p = *buf;
    allocate(p, CondIndexVal, civcopy, *civ);
    civcopy->condExp = copyNode(&p, civcopy->condExp);
    civcopy->valIndex = copyHashtableCharPtrToIntPtr(&p, civcopy->valIndex);
    *buf = p;
    return civcopy;
}

Hashtable* copyHashtableCharPtrToCondIndexValPtr(void **buf, Hashtable *h) {
  void *p = *buf;
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

      // copy key
      allocateArray(p, char, strlen(b->key) + 1, bcopy->key, b->key);
      // copy value
      bcopy->value = copyCondIndexVal(&p, bcopy->value);

      prev = bcopy;
      b = b->next;
    }
  }
  *buf = p;
  return hcopy;

}

Cache *copyCache(void **buf, Cache *c) {
    void *p = *buf;
    ((CacheRecordDesc *)p)->type = Cache_T;
    ((CacheRecordDesc *)p)->length = 1;
    p+= sizeof(CacheRecordDesc);
    Cache *ccopy = ((Cache *)p);
    *ccopy = *c;
    p+=sizeof(Cache);
    //allocate(p, Cache, ccopy, *c);

    ccopy->offset = *buf;
    ccopy->coreRuleSet = ccopy->coreRuleSet == NULL? NULL:copyRuleSet(&p, ccopy->coreRuleSet);
    ccopy->coreRuleIndex = ccopy->coreRuleIndex == NULL? NULL:copyHashtableCharPtrToIntPtr(&p, ccopy->coreRuleIndex);
    ccopy->condIndex = ccopy->condIndex == NULL? NULL:copyHashtableCharPtrToCondIndexValPtr(&p, ccopy->condIndex);
    ccopy->dataSize = (p - (*buf))/sizeof(void);

    *buf = p;
    return ccopy;
}
#define APPLY_DIFF(p, d) if((p)!=NULL){void *temp = p; temp+=(d); (p)=temp;}
Cache *restoreCache(void *buf) {
    if(((CacheRecordDesc *)buf)->type != Cache_T) {
        // error
        return NULL;
    }
    Cache *cache = (Cache *)(buf + sizeof(CacheRecordDesc)/sizeof(void));
    void *bufCopy = malloc(cache->dataSize);
    if(bufCopy == NULL) {
        return NULL;
    }
    memcpy(bufCopy, buf, cache->dataSize);

    cache = (Cache *)(bufCopy + sizeof(CacheRecordDesc)/sizeof(void));
    void *bufOffset = cache->offset;
    void *bufCopyOffset = bufCopy;

    long diff = bufCopyOffset - bufOffset;
    void *p = bufCopy;
    while(p < bufCopyOffset + cache->dataSize) {
        enum cacheRecordType type = ((CacheRecordDesc *)p)->type;
        int length = ((CacheRecordDesc *)p)->length;
        p+=sizeof(CacheRecordDesc)/sizeof(void);
        int i;
        switch(type) {
            case Cache_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Cache *)p)->condIndex, diff);
                    APPLY_DIFF(((Cache *)p)->coreRuleIndex, diff);
                    APPLY_DIFF(((Cache *)p)->coreRuleSet, diff);
                    APPLY_DIFF(((Cache *)p)->offset, diff);
                    p+=sizeof(Cache)/sizeof(void);
                }
                break;
            case Hashtable_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Hashtable *)p)->buckets, diff);
                    p+=sizeof(Hashtable)/sizeof(void);
                }
                break;
            case Bucket_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Bucket *)p)->key, diff);
                    APPLY_DIFF(((Bucket *)p)->next, diff);
                    p+=sizeof(Bucket)/sizeof(void);
                }
                break;
            case BucketPtr_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(*((BucketPtr *)p), diff);
                    p+=sizeof(BucketPtr)/sizeof(void);
                }
                break;
            case char_T:
                p+= length * (sizeof(char)/sizeof(void));
                break;
            case int_T:
                p+=length * (sizeof(int)/sizeof(void));
                break;
            case ExprType_T:
                for(i=0;i<length;i++) {
                    switch(((ExprType *)p)->t) {
                        case T_FUNC:
                            APPLY_DIFF(((ExprType *)p)->ext.func.retType, diff);
                            APPLY_DIFF(((ExprType *)p)->ext.func.paramsType, diff);
                            break;
                        case T_CONS:
                            APPLY_DIFF(((ExprType *)p)->ext.cons.typeArgs, diff);
                            break;
                        default:
                            break;
                    }

                    p+=sizeof(ExprType)/sizeof(void);
                }

                break;
            case ExprTypePtr_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(*((ExprTypePtr *)p), diff);
                    p+=sizeof(ExprTypePtr)/sizeof(void);
                }
                break;
            case Node_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(((Node *)p)->coercion, diff);
                    APPLY_DIFF(((Node *)p)-> exprType, diff);
                    APPLY_DIFF(((Node *)p)->subtrees, diff);
                    p+=sizeof(Node)/sizeof(void);
                }
                break;
            case NodePtr_T:
                for(i=0;i<length;i++) {
                    APPLY_DIFF(*((NodePtr *)p), diff);
                    p+=sizeof(NodePtr)/sizeof(void);
                }
                break;
            case RuleSet_T:
                for(i=0;i<length;i++) {
                    int j;
                    for(j=0;j<((RuleSet *)p)->len;j++) {
                        APPLY_DIFF(((RuleSet *)p)->rules[j], diff);
                    }


                    p+=sizeof(RuleSet)/sizeof(void);
                }
                break;
            case CondIndexVal_T:

                for(i=0;i<length;i++) {
                    APPLY_DIFF(((CondIndexVal *)p)->condExp, diff);
                    APPLY_DIFF(((CondIndexVal *)p)->valIndex, diff);

                    //((Cache *)p)->offset += diff;
                    p+=sizeof(CondIndexVal)/sizeof(void);
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
      return 0;
    }

}
void unlockMutex(sem_t **mutex) {
  int v;
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
    if(CACHE_ENABLE) {
        sem_t *mutex;

        if(lockMutex(&mutex) != 0) {
            res = -1;
            RETURN;
        }
        int shmid = - 1;
        int key = 1200;
        void *addr = SHM_BASE_ADDR;
        if(isServer) {
          shmid = shmget(key, SHMMAX, IPC_CREAT /*| IPC_EXCL*/ | 0666);
        } else {
          shmid = shmget(key, SHMMAX, 0666);
          if(shmid != -1) { // not server process and shm is successfully allocated
            void *shm = shmat(shmid, addr, 0);
            Cache *cache = restoreCache(shm);
            coreRuleIndex = cache->coreRuleIndex;
            coreRules = *(cache->coreRuleSet);
            condIndex = cache->condIndex;
          }
        }

        if(isServer || shmid == -1) {
            while (strlen(r2) > 0) {
                    int i = rSplitStr(r2,r1,NAME_LEN,r3,RULE_SET_DEF_LENGTH,',');
                    if (i == 0)
                      i = readRuleStructAndRuleSetFromFile(r1, inRuleStruct, r);
                    if (i != 0) {
                      unlockMutex(&mutex);
                      res = i;
                      RETURN;
                    }
                    strcpy(r2,r3);
            }
            if(isServer && shmid!= -1) {
                void *shm = shmat(shmid, SHM_BASE_ADDR, 0);
                void *buf = shm;
                Cache cacheBuf;
                cacheBuf.coreRuleIndex = coreRuleIndex;
                cacheBuf.coreRuleSet = &coreRules;
                cacheBuf.condIndex = condIndex;
                copyCache(&buf, &cacheBuf);
            }
        }
        unlockMutex(&mutex);
    } else {
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
        void *buf = malloc(SHMMAX);
        Cache cacheBuf;
        cacheBuf.coreRuleIndex = coreRuleIndex;
        cacheBuf.coreRuleSet = &coreRules;
        cacheBuf.condIndex = condIndex;
        Cache *cacheNew = copyCache(&buf, &cacheBuf);
        deleteHashTable(coreRuleIndex, free);
        deleteHashTable(condIndex, (void (*)(void *))deleteCondIndexVal);
        coreRuleIndex = cacheNew->coreRuleIndex;
        coreRules = *cacheNew->coreRuleSet;
        condIndex = cacheNew->condIndex;
    }

ret:
    region_free(r);
    return res;
}
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct, Region *r)
{
  int i = 0;
//  char l0[MAX_RULE_LENGTH];
//  char l1[MAX_RULE_LENGTH];
//  char l2[MAX_RULE_LENGTH];
//  char l3[MAX_RULE_LENGTH];
   char rulesFileName[MAX_NAME_LEN];
//   FILE *file;
//   char buf[MAX_RULE_LENGTH];
   char *configDir;
//   char *t;
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
