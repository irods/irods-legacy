/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "configuration.h"
#include "utils.h"
#include "rules.h"
#include "index.h"
#include "cache.h"
#include "region.h"
#include "functions.h"
Cache ruleEngineConfig = {
    NULL, // unsigned char *address
    0, // long dataSize
    0, // long cacheSize
    UNINITIALIZED, // RuleEngineStatus coreRuleSetStatus
    UNINITIALIZED, // RuleEngineStatus appRuleSetStatus
    UNINITIALIZED, // RuleEngineStatus extRuleSetStatus
    UNINITIALIZED, // RuleEngineStatus coreFuncDescIndexStatus
    UNINITIALIZED, // RuleEngineStatus appFuncDescIndexStatus
    UNINITIALIZED, // RuleEngineStatus extFuncDescIndexStatus
    UNINITIALIZED, // RuleEngineStatus ruleIndexStatus
    UNINITIALIZED, // RuleEngineStatus condIndexStatus
    UNINITIALIZED, // RuleEngineStatus ruleEngineStatus
    UNINITIALIZED, // RuleEngineStatus cacheStatus
    UNINITIALIZED, // RuleEngineStatus regionIndexStatus
    UNINITIALIZED, // RuleEngineStatus regionCoreStatus
    UNINITIALIZED, // RuleEngineStatus regionAppStatus
    NULL, // RuleSet *coreRuleSet
    NULL, // RuleSet *appRuleSet
    NULL, // RuleSet *extRuleSet
    NULL, // Env *coreFuncDescIndex
    NULL, // Env *appFuncDescIndex
    NULL, // Env *extFuncDescIndex
    NULL, // Hashtable *ruleIndex
    NULL, // Hashtable *condIndex
    NULL, // Region *regionIndex
    NULL, // Region *regionCore
    NULL, // Region *regionApp
};

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

void removeRuleFromIndex(char *ruleName, int i) {
	RuleIndexList *rd = (RuleIndexList *)lookupFromHashTable(ruleEngineConfig.ruleIndex, ruleName);
	removeNodeFromRuleIndexList(rd, i);
	if(rd->head == NULL) {
		deleteFromHashTable(ruleEngineConfig.ruleIndex, ruleName);
	}

}
void appendRuleIntoIndex(RuleDesc *rule, int i, Region *r) {
	RuleIndexList *rd = (RuleIndexList *)lookupFromHashTable(ruleEngineConfig.ruleIndex, RULE_NAME(rule->node));
	if(rd == NULL) {
		rd = newRuleIndexList(RULE_NAME(rule->node), rule->ruleType, i, r);
		insertIntoHashTable(ruleEngineConfig.ruleIndex, RULE_NAME(rule->node), rd);
	} else {
		appendRuleNodeToRuleIndexList(rd, i ,r);
	}
}

void prependRuleIntoIndex(RuleDesc *rule, int i, Region *r) {
	RuleIndexList *rd = (RuleIndexList *)lookupFromHashTable(ruleEngineConfig.ruleIndex, RULE_NAME(rule->node));
	if(rd == NULL) {
		rd = newRuleIndexList(RULE_NAME(rule->node), rule->ruleType, i, r);
		insertIntoHashTable(ruleEngineConfig.ruleIndex, RULE_NAME(rule->node), rd);
	} else {
		prependRuleNodeToRuleIndexList(rd, i ,r);
	}
}
int checkPointExtRuleSet() {
	ruleEngineConfig.extFuncDescIndex = newEnv(newHashTable(100), ruleEngineConfig.extFuncDescIndex, NULL);
	return ruleEngineConfig.extRuleSet->len;
}
void appendAppRule(RuleDesc *rd, Region *r) {
	int i = ruleEngineConfig.appRuleSet->len++;
	ruleEngineConfig.appRuleSet->rules[i] = rd;
	appendRuleIntoIndex(rd, i, r);
}
void prependAppRule(RuleDesc *rd, Region *r) {
	int i = ruleEngineConfig.appRuleSet->len++;
	ruleEngineConfig.appRuleSet->rules[i] = rd;
	prependRuleIntoIndex(rd, i, r);
}
void popExtRuleSet(int checkPoint) {
	Env *temp = ruleEngineConfig.extFuncDescIndex;
	ruleEngineConfig.extFuncDescIndex = temp->previous;
	deleteEnv(temp, 1);
	int i;
	for(i = checkPoint; i < ruleEngineConfig.extRuleSet->len; i++) {
		removeRuleFromIndex(RULE_NAME(ruleEngineConfig.extRuleSet->rules[i]->node), i);
	}
	ruleEngineConfig.extRuleSet->len = checkPoint;
}
RuleEngineStatus getRuleEngineStatus() {
    return ruleEngineConfig.ruleEngineStatus;
}
/* RuleEngineStatus getRuleEngineMemStatus() {
    return _ruleEngineMemStatus;
}
void setAppRuleSetStatus(RuleEngineStatus s) {
    _ruleEngineStatus = s;
}
RuleEngineStatus getAppRuleSetStatus() {
    return _ruleEngineStatus;
}
void setRuleEngineMemStatus(RuleEngineStatus s) {
	_ruleEngineMemStatus = s;
} */
void clearResources(int resources) {
	if((resources & RESC_RULE_INDEX) && isComponentAllocated(ruleEngineConfig.ruleIndexStatus)) {
		deleteHashTable(ruleEngineConfig.ruleIndex, nop);
		ruleEngineConfig.ruleIndexStatus = UNINITIALIZED;
	}
	if((resources & RESC_COND_INDEX) && isComponentAllocated(ruleEngineConfig.condIndexStatus)) {
		deleteHashTable(ruleEngineConfig.condIndex, (void (*)(void *))deleteCondIndexVal);
		ruleEngineConfig.condIndexStatus = UNINITIALIZED;
	}
	clearFuncDescIndex(APP, app);
	clearFuncDescIndex(CORE, core);
	clearFuncDescIndex(EXT, ext);
	clearRegion(INDEX, Index);
	clearRegion(APP, App);
	clearRegion(CORE, Core);
	clearRuleSet(APP, app);
	clearRuleSet(CORE, core);
	clearRuleSet(EXT, ext);

	if((resources & RESC_CACHE) && isComponentAllocated(ruleEngineConfig.cacheStatus)) {
		free(ruleEngineConfig.address);
		ruleEngineConfig.cacheStatus = UNINITIALIZED;
	}
}
List hashtablesToClear = {NULL, NULL};
List envToClear = {NULL, NULL};
List regionsToClear = {NULL, NULL};
List memoryToFree = {NULL, NULL};

void delayClearResources(int resources) {
	/*if((resources & RESC_RULE_INDEX) && ruleEngineConfig.ruleIndexStatus == INITIALIZED) {
		listAppendNoRegion(hashtablesToClear, ruleEngineConfig.ruleIndex);
		ruleEngineConfig.ruleIndexStatus = UNINITIALIZED;
	}
	if((resources & RESC_COND_INDEX) && ruleEngineConfig.condIndexStatus == INITIALIZED) {
		listAppendNoRegion(hashtableToClear, ruleEngineConfig.condIndex);
		ruleEngineConfig.condIndexStatus = UNINITIALIZED;
	}*/
	delayClearRegion(INDEX, Index);
	delayClearRegion(APP, App);
	delayClearRegion(CORE, Core);
	delayClearRuleSet(APP, app);
	delayClearRuleSet(CORE, core);
	delayClearRuleSet(EXT, ext);
	delayClearFuncDescIndex(APP, app);
	delayClearFuncDescIndex(CORE, core);
	delayClearFuncDescIndex(EXT, ext);

	if((resources & RESC_CACHE) && ruleEngineConfig.cacheStatus == INITIALIZED) {
		listAppendNoRegion(&memoryToFree, ruleEngineConfig.address);
		ruleEngineConfig.cacheStatus = UNINITIALIZED;
	}
}

void clearDelayed() {
	ListNode *n = envToClear.head;
	while(n!=NULL) {
		deleteEnv((Env *) n->value, 1);
		listRemoveNoRegion(&envToClear, n);
		n = envToClear.head;
	}
	n = hashtablesToClear.head;
	while(n!=NULL) {
		deleteHashTable((Hashtable *) n->value, nop);
		listRemoveNoRegion(&hashtablesToClear, n);
		n = hashtablesToClear.head;
	}
	n = regionsToClear.head;
		while(n!=NULL) {
			region_free((Region *) n->value);
			listRemoveNoRegion(&regionsToClear, n);
			n = regionsToClear.head;
		}
		n = memoryToFree.head;
			while(n!=NULL) {
				free(n->value);
				listRemoveNoRegion(&memoryToFree, n);
				n = memoryToFree.head;
			}
}

void setCacheAddress(unsigned char *addr, RuleEngineStatus status, long size) {
    ruleEngineConfig.address = addr;
    ruleEngineConfig.cacheStatus = status;
    ruleEngineConfig.cacheSize = size;

}
#define CASCASE_NON_ZERO(x) {int ret = x; if(ret != 0) { return ret;} }
int generateLocalCache() {
    unsigned char *buf = NULL;
	if(ruleEngineConfig.cacheStatus == INITIALIZED) {
		free(ruleEngineConfig.address);
	}
	buf = (unsigned char *)malloc(SHMMAX);
	if(buf == NULL) {
		return OUT_OF_MEMORY;
	}
	setCacheAddress(buf, INITIALIZED, SHMMAX);
	return 0;
}
int generateFunctionDescriptionTables() {
    createFuncDescIndex(CORE, core);
    createFuncDescIndex(APP, app);
    createFuncDescIndex(EXT, ext);
    ruleEngineConfig.extFuncDescIndex->previous = ruleEngineConfig.appFuncDescIndex;
    ruleEngineConfig.appFuncDescIndex->previous = ruleEngineConfig.coreFuncDescIndex;

	return 0;
}
void generateRuleSets() {
	createRuleSet(APP, app);
	createRuleSet(CORE, core);
	createRuleSet(EXT, ext);
}
void generateRegions() {
	createRegion(INDEX, Index);
	createRegion(APP, App);
	createRegion(CORE, Core);
}
int clearRuleSetAndIndex(ruleStruct_t *inRuleStruct) {
	if(inRuleStruct == &coreRuleStrct) {
		clearResources(RESC_CORE_RULE_SET | RESC_COND_INDEX | RESC_RULE_INDEX | RESC_CORE_FUNC_DESC_INDEX | RESC_REGION_INDEX);
		delayClearResources(RESC_REGION_CORE);
	} else if(inRuleStruct == &appRuleStrct) {
		clearResources(RESC_COND_INDEX | RESC_APP_RULE_SET | RESC_RULE_INDEX | RESC_APP_FUNC_DESC_INDEX | RESC_REGION_INDEX);
		delayClearResources(RESC_REGION_APP);
	}
	return 0;
}
int loadRuleFromCacheOrFile(char *irbSet, ruleStruct_t *inRuleStruct) {
    char r1[NAME_LEN], r2[RULE_SET_DEF_LENGTH], r3[RULE_SET_DEF_LENGTH];
    strcpy(r2,irbSet);
    int res = 0;

    int loadToBuf;
    int loadToShared;
    int unlock_mutex = 0;
    sem_t *mutex = NULL;
    unsigned char *buf = NULL;
	if(ruleEngineConfig.ruleEngineStatus == INITIALIZED) {
		/* Reloading rule set, clear previously generated rule set */
		clearRuleSetAndIndex(inRuleStruct);
		buf = (unsigned char *)malloc(SHMMAX);
		if(ruleEngineConfig.cacheStatus == SHARED) {
			if(lockMutex(&mutex) != 0) {
				res = -1;
				RETURN;
			}
			unlock_mutex = 1;
			loadToShared = 1;
		} else {
			loadToShared = 0;
		}
		loadToBuf = 1;

	} else {
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
            shmid = shmget(key, SHMMAX, IPC_CREAT /*| IPC_EXCL*/ | 0666);
            if(shmid!= -1) {
            	loadToBuf = 1;
                unsigned char *shm = (unsigned char *)shmat(shmid, SHM_BASE_ADDR, 0);
                setCacheAddress(shm, SHARED, SHMMAX);
            } else {
            	CASCASE_NON_ZERO(generateLocalCache());
            	loadToBuf = 1;

            }
        } else {
            shmid = shmget(key, SHMMAX, 0666);
            if(shmid != -1) { /* not server process and shm is successfully allocated */
                loadToBuf = 0;
                buf = (unsigned char *)shmat(shmid, addr, 0);
                setCacheAddress(buf, SHARED, SHMMAX);
            } else {
            	CASCASE_NON_ZERO(generateLocalCache());
            	loadToBuf = 1;
            }
        }
    } else {
    	CASCASE_NON_ZERO(generateLocalCache());
    	loadToBuf = 1;
    }
    buf = ruleEngineConfig.address;
	}
	clearResources(RESC_REGION_INDEX);
    if(loadToBuf) {
    	/* if(inRuleStruct == &coreRuleStrct) {
    		clearResources(RESC_CORE_FUNC_DESC_INDEX);
    	} else if(inRuleStruct == &appRuleStrct) {
    		clearResources(RESC_APP_FUNC_DESC_INDEX);
    	} */
    	generateRegions();
		generateFunctionDescriptionTables();
    	generateRuleSets();
        while (strlen(r2) > 0) {
			int i = rSplitStr(r2,r1,NAME_LEN,r3,RULE_SET_DEF_LENGTH,',');
			if (i == 0)
				i = readRuleStructAndRuleSetFromFile(r1, inRuleStruct);
			if (i != 0) {
			    res = i;
                RETURN;
			}
			strcpy(r2,r3);
        }
        Cache *configNew = copyCache(&buf, SHMMAX, &ruleEngineConfig);
        configNew->regionIndexStatus = UNINITIALIZED;
        configNew->regionAppStatus = UNINITIALIZED;
        configNew->regionCoreStatus = UNINITIALIZED;
        configNew->appRuleSetStatus = COMPRESSED;
        configNew->coreRuleSetStatus = COMPRESSED;
        configNew->coreFuncDescIndexStatus = COMPRESSED;
        configNew->appFuncDescIndexStatus = COMPRESSED;

#ifdef DEBUG
        printf("Buffer usage: %fM\n", ((double)(configNew->dataSize))/(1024*1024));
#endif
        clearResources(RESC_APP_RULE_SET | RESC_CORE_RULE_SET
        		     | RESC_COND_INDEX | RESC_RULE_INDEX
        		     | RESC_CORE_FUNC_DESC_INDEX | RESC_APP_FUNC_DESC_INDEX
        		     | RESC_REGION_APP | RESC_REGION_CORE | RESC_REGION_INDEX );
        if(ruleEngineConfig.ruleEngineStatus == INITIALIZED) {
        	/* previous cache allocated */
        	if(loadToShared) {
        		/* previous cache is in shared memory, this must be a server process */
        		unsigned char *start = ruleEngineConfig.address;
        		configNew = copyCache(&start, SHMMAX, configNew);
        	} else {
        		/* previous cache is not in shared memory, discard and replace with new cache */
        		delayClearResources(RESC_CACHE);
        	}
        }
        /* copy extRuleSet and extFuncDescIndex */
        configNew->extRuleSet = ruleEngineConfig.extRuleSet;
        configNew->extFuncDescIndex = ruleEngineConfig.extFuncDescIndex;
        ruleEngineConfig = *configNew;
        ruleEngineConfig.extRuleSetStatus = LOCAL;
        ruleEngineConfig.extFuncDescIndexStatus = LOCAL;
    	generateFunctionDescriptionTables();
    } else {
        Cache *cache = restoreCache(buf);
        cache->cacheStatus = INITIALIZED;
        ruleEngineConfig = *cache;
        /* generate extRuleSet */
    	generateRuleSets();
    	generateFunctionDescriptionTables();
        ruleEngineConfig.extRuleSetStatus = LOCAL;
        ruleEngineConfig.extFuncDescIndexStatus = LOCAL;
    }
	createCoreAppExtRuleNodeIndex();

ret:
    if(unlock_mutex)
    {
        unlockMutex(&mutex);
    }
    ruleEngineConfig.ruleEngineStatus = INITIALIZED;

    return res;
}
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct)
{
  int i;
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
		if(readRuleSetFromFile(ruleBaseName,ruleEngineConfig.coreRuleSet,ruleEngineConfig.coreFuncDescIndex,&errloc,&errmsgBuf, ruleEngineConfig.regionCore)==0) {
		} else {
			errMsgToString(&errmsgBuf, buf, ERR_MSG_LEN*1024);
			rodsLog(LOG_ERROR, "%s", buf);
			res = -1;
		}
	} else if(inRuleStrct == &appRuleStrct) {
		if(readRuleSetFromFile(ruleBaseName,ruleEngineConfig.appRuleSet, ruleEngineConfig.appFuncDescIndex,&errloc,&errmsgBuf, ruleEngineConfig.regionApp)==0) {
		} else {
			errMsgToString(&errmsgBuf, buf, ERR_MSG_LEN*1024);
			rodsLog(LOG_ERROR, "%s", buf);
			res = -1;
		}
	}
	free(buf);
	freeRErrorContent(&errmsgBuf);
	return res;
}

int availableRules() {
	return (isComponentInitialized(ruleEngineConfig.appRuleSetStatus) ? ruleEngineConfig.coreRuleSet->len : 0) + (isComponentInitialized(ruleEngineConfig.coreRuleSetStatus) == INITIALIZED? ruleEngineConfig.appRuleSet->len : 0);
}
