/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "index.h"
#include "rules.h"
#include "debug.h"

#ifndef DEBUG
typedef struct {
  char action[MAX_ACTION_SIZE];
  int numberOfStringArgs;
  funcPtr callAction;
} microsdef_t;
extern int NumOfAction;
extern microsdef_t MicrosTable[];
#endif

Hashtable *coreRuleIndex = NULL;
Hashtable *appRuleIndex = NULL;
Hashtable *coreRuleFuncMapDefIndex = NULL;
Hashtable *appRuleFuncMapDefIndex = NULL;
Hashtable *microsTableIndex = NULL;
Hashtable *condIndex = NULL;

void clearIndex(Hashtable **ruleIndex)
{
	if (*ruleIndex!=NULL) {
		deleteHashTable(*ruleIndex, free);
	}
}
/**
 * returns 0 if out of memory
 */
int createRuleIndex(ruleStruct_t *inRuleStrct, Hashtable **ruleIndex)
{
	clearIndex(ruleIndex);
	*ruleIndex = newHashTable(MAX_NUM_OF_RULES*2);
	if (*ruleIndex == NULL)
		return 0;
	int i;
	for (i=0;i<inRuleStrct->MaxNumOfRules;i++) {
		char *key = inRuleStrct->action[i];
		int *value=(int *)malloc(sizeof(int));
		*value = i;

		if (insertIntoHashTable(*ruleIndex, key,value) == 0) {
			deleteHashTable(*ruleIndex, free);
			*ruleIndex=NULL;
			return 0;
		}
	}
	return 1;
}
/**
 * returns 0 if out of memory
 */
int createRuleNodeIndex(RuleSet *inRuleSet, Hashtable **ruleIndex)
{
	clearIndex(ruleIndex);
	*ruleIndex = newHashTable(MAX_NUM_OF_RULES*2);
	if (*ruleIndex == NULL)
		return 0;

    /* generate main index */
	int i;
	for (i=0;i<inRuleSet->len;i++) {
            Node *ruleNode = inRuleSet->rules[i];
            if(ruleNode == NULL)
                continue;
		char *key = ruleNode->subtrees[0]->text;
		int *value=(int *)malloc(sizeof(int));
		*value = i;

		if (insertIntoHashTable(*ruleIndex, key,value) == 0) {
			deleteHashTable(*ruleIndex, free);
			*ruleIndex=NULL;
			return 0;
		}
	}

	/* generate rule condition index */
	condIndex = newHashTable(MAX_NUM_OF_RULES);

    Hashtable *processedRuleNames = newHashTable(MAX_NUM_RULES * 2);
    int ruleGroup[MAX_NUM_OF_RULES];
	char *strGroup[MAX_NUM_OF_RULES];
	for(i=0;i<(*ruleIndex)->size;i++) {
	    struct bucket *b = (*ruleIndex)->buckets[i];

        struct bucket *resumingBucket = b;

        while(resumingBucket!=NULL) {

            if(lookupFromHashTable(processedRuleNames, resumingBucket->key)) {
               /* group already processed, continue */
               resumingBucket = resumingBucket->next;
               continue;
            }

            insertIntoHashTable(processedRuleNames, resumingBucket->key, resumingBucket->key);
            struct bucket *curr = resumingBucket;
            resumingBucket = curr->next;

            int currIndex = *(int *)curr->value;
            Node *ruleNode = inRuleSet->rules[currIndex];
            if(!(ruleNode->subtrees[0]->degree == 0 && /* no params */
               ruleNode->subtrees[1]->type == APPLICATION && strcmp(ruleNode->subtrees[1]->text, "==") == 0 && /* comparison */
               ruleNode->subtrees[1]->degree == 2 && ruleNode->subtrees[1]->subtrees[1]->type == STRING /* with a string */
               )) {
                continue;
            }

            char *ruleName = curr->key;
            Node *condNode = ruleNode->subtrees[1];

            int groupRuleCount = 0;
            Hashtable *processedStrs = newHashTable(MAX_NUM_RULES * 2);

            /* add curr to group */
            ruleGroup[groupRuleCount] = currIndex;
            strGroup[groupRuleCount] = ruleNode->subtrees[1]->subtrees[1]->text;
            insertIntoHashTable(processedStrs, strGroup[groupRuleCount], strGroup[groupRuleCount]);
            groupRuleCount ++;

            int done = 0;
            while(1) {
                do {
                    curr = curr->next;
                    if(curr == NULL) {
                        if(groupRuleCount > 1) {
                            /* generate cond index if there are more than one rules in this group */
                            Hashtable *groupHashtable = newHashTable(groupRuleCount * 2);
                            CondIndexVal *civ = (CondIndexVal *)malloc(sizeof(CondIndexVal));
                            civ->condExp = condNode->subtrees[0];
                            civ->valIndex = groupHashtable;
                            /* todo currently we assume that only core rules are indexed */
                            insertIntoHashTable(condIndex, ruleName, civ);
                            #ifdef DEBUG
                            /* printf("inserting rule group %s into condIndex\n", ruleName); */
                            #endif
                            int k;
                            for(k=0;k<groupRuleCount;k++) {
                                int *index = (int *)malloc(sizeof(int));
                                *index = ruleGroup[k];
                                insertIntoHashTable(groupHashtable, strGroup[k], index);
                                #ifdef DEBUG
                                /* printf("inserting rule cond str %s, index %d into condIndex\n", strGroup[k], ruleGroup[k]); */
                                #endif
                            }
                        }
                        done = 1;

                        break;
                    }
                } while(strcmp(curr->key, ruleName)!=0);
                if(done) {
                    break;
                } else {
                    currIndex = *(int *)curr->value;
                    ruleNode = inRuleSet->rules[currIndex];
                    if(!(ruleNode->subtrees[0]->degree == 0 && /* no params */
                       ruleNode->subtrees[1]->type == APPLICATION && strcmp(ruleNode->subtrees[1]->text, "==") == 0 && /* comparison */
                       ruleNode->subtrees[1]->degree == 2 && ruleNode->subtrees[1]->subtrees[1]->type == STRING && /* with a string */
                       lookupFromHashTable(processedStrs, ruleNode->subtrees[1]->subtrees[1]->text) == NULL && /* string is new */
                       eqExprNodeSyntactic(condNode->subtrees[0], ruleNode->subtrees[1]->subtrees[0]) /* exp is the same */
                       )) {
                       /* criteria failed break out of loop */
                        break;
                    } else {
                        /* add curr to group */
                        ruleGroup[groupRuleCount] = currIndex;
                        strGroup[groupRuleCount] = ruleNode->subtrees[1]->subtrees[1]->text;
                        insertIntoHashTable(processedStrs, strGroup[groupRuleCount], strGroup[groupRuleCount]);
                        groupRuleCount ++;
                    }
                }

            }
            deleteHashTable(processedStrs, nop);
        }

	}
    deleteHashTable(processedRuleNames, nop);

	return 1;
}
/**
 * returns 0 if out of memory
 */
int createFuncMapDefIndex(rulefmapdef_t *inFuncStrct, Hashtable **ruleIndex)
{
	clearIndex(ruleIndex);
	*ruleIndex = newHashTable(MAX_NUM_OF_DVARS*2);
	if (*ruleIndex == NULL)
		return 0;
	int i;
	for (i=0;i<inFuncStrct->MaxNumOfFMaps;i++) {
		char *key = inFuncStrct->funcName[i];
		int *value=(int *)malloc(sizeof(int));
		*value = i;

		if (insertIntoHashTable(*ruleIndex, key,value) == 0) {
			deleteHashTable(*ruleIndex, free);
			*ruleIndex=NULL;
			return 0;
		}
	}
	return 1;
}
/**
 * returns 0 if out of memory
 */
int createMacorsIndex()
{
	clearIndex(&microsTableIndex);
	microsTableIndex = newHashTable(NumOfAction*2);
	if (microsTableIndex == NULL)
		return 0;
	int i;
	for (i=0;i<NumOfAction;i++) {
		char *key = MicrosTable[i].action;
		int *value=(int *)malloc(sizeof(int));
		*value = i;

		if (insertIntoHashTable(microsTableIndex, key,value) == 0) {
			deleteHashTable(microsTableIndex, free);
			microsTableIndex=NULL;
			return 0;
		}
	}
	return 1;
}

int findNextRuleFromIndex(Hashtable *ruleIndex, char *action, int *index)
{
	int  i=*index;

	if (ruleIndex!=NULL) {
/*        dumpHashtableKeys(coreRuleIndex); */
		struct bucket *b=lookupBucketFromHashTable(ruleIndex, action);

		while (b!=NULL && *((int *)(b->value))<i) {
			b= nextBucket(b, action);
		}
		if (b!=NULL) {
			*index = *((int *)(b->value));
			return(0);
		}
	}

	return NO_MORE_RULES_ERR;
}
/**
 * adapted from original code
 */
int findNextRule2(char *action,  int *ruleInx)
{
	int i;
	i = *ruleInx;
	i++;

	if (i < 0)
		i = 0;
	if (i < 1000) {
		if (appRuleIndex != NULL) {
			int ii = findNextRuleFromIndex(appRuleIndex, action, ruleInx);
			if (ii!=NO_MORE_RULES_ERR) {
				return 0;
			}
		} /*else {
			for ( ; i < appRuleStrct.MaxNumOfRules; i++) {
				if (!strcmp( appRuleStrct.action[i],action)) {
					*ruleInx = i;
					return(0);
				}
			}
		}*/
		i = 1000;
	}
	i  = i - 1000;
	if (coreRuleIndex != NULL) {
		int ii = findNextRuleFromIndex(coreRuleIndex, action, &i);
/*		dumpHashtableKeys(coreRuleIndex); */
		*ruleInx = i + 1000;
		return ii;
	} /*else {
		for ( ; i < coreRuleStrct.MaxNumOfRules; i++) {
			if (!strcmp( coreRuleStrct.action[i],action)) {
				*ruleInx = i + 1000;
				return(0);
			}
		}*/
		return(NO_MORE_RULES_ERR);
	/*}*/
}

int mapExternalFuncToInternalProc2(char *funcName)
{
	int *i;

	if (appRuleFuncMapDefIndex!=NULL && (i=(int *)lookupFromHashTable(appRuleFuncMapDefIndex, funcName))!=NULL) {
		strcpy(funcName, appRuleFuncMapDef.func2CMap[*i]);
		return(1);
	}
	if (coreRuleFuncMapDefIndex!=NULL && (i=(int *)lookupFromHashTable(coreRuleFuncMapDefIndex, funcName))!=NULL) {
		strcpy(funcName, coreRuleFuncMapDef.func2CMap[*i]);
		return(1);
	}
	return(0);
}
int actionTableLookUp2(char *action)
{
	int *i;

	if ((i=(int *)lookupFromHashTable(microsTableIndex, action))!=NULL) {
		return (*i);
	}

	return (UNMATCHED_ACTION_ERR);
}
void deleteCondIndexVal(CondIndexVal *h) {
    deleteHashTable(h->valIndex, free);
    free(h);
}
