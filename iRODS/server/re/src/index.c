/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "index.h"
#include "rules.h"
#include "debug.h"
#include "configuration.h"
#define ERROR(x) if(x) { goto error; }
#ifndef DEBUG
typedef struct {
  char action[MAX_ACTION_SIZE];
  int numberOfStringArgs;
  funcPtr callAction;
} microsdef_t;
extern int NumOfAction;
extern microsdef_t MicrosTable[];
#endif

Hashtable *coreRuleFuncMapDefIndex = NULL;
Hashtable *appRuleFuncMapDefIndex = NULL;
Hashtable *microsTableIndex = NULL;

void clearIndex(Hashtable *ruleIndex)
{
	if (ruleIndex!=NULL) {
		deleteHashTable(ruleIndex, free);
	}
}
/**
 * returns 0 if out of memory
 */
int createRuleStructIndex(ruleStruct_t *inRuleStrct, Hashtable *ruleIndex)
{
	if (ruleIndex == NULL)
		return 0;
	int i;
	for (i=0;i<inRuleStrct->MaxNumOfRules;i++) {
		char *key = inRuleStrct->action[i];
		int *value=(int *)malloc(sizeof(int));
		*value = i;

		if (insertIntoHashTable(ruleIndex, key,value) == 0) {
			return 0;
		}
	}
	return 1;
}
int createCoreAppExtRuleNodeIndex() {
	clearResources(RESC_RULE_INDEX);
	clearResources(RESC_COND_INDEX);
	createRegion(INDEX, Index);
	ruleEngineConfig.ruleIndex = newHashTable(MAX_NUM_RULES * 2);
    if (ruleEngineConfig.ruleIndex == NULL)
        return OUT_OF_MEMORY;


	if(isComponentInitialized(ruleEngineConfig.extRuleSetStatus)) {
		ERROR(createRuleNodeIndex(ruleEngineConfig.extRuleSet, ruleEngineConfig.ruleIndex, 0, ruleEngineConfig.regionIndex) == 0);
	}
	if(isComponentInitialized(ruleEngineConfig.appRuleSetStatus)) {
		ERROR(createRuleNodeIndex(ruleEngineConfig.appRuleSet, ruleEngineConfig.ruleIndex, APP_RULE_INDEX_OFF, ruleEngineConfig.regionIndex) == 0);
	}
	if(isComponentInitialized(ruleEngineConfig.coreRuleSetStatus)) {
		ERROR(createRuleNodeIndex(ruleEngineConfig.coreRuleSet, ruleEngineConfig.ruleIndex, CORE_RULE_INDEX_OFF, ruleEngineConfig.regionIndex) == 0);
	}

	createCondIndex(ruleEngineConfig.regionIndex);
	ruleEngineConfig.ruleIndexStatus = LOCAL;
	return 0;
error:
	deleteHashTable(ruleEngineConfig.ruleIndex, nop);
	ruleEngineConfig.ruleIndex=NULL;
	return -1;

}
int createCondIndex(Region *r) {
    /* generate rule condition index */
	int i;
    ruleEngineConfig.condIndex = newHashTable(MAX_NUM_RULES);
    for(i=0;i<ruleEngineConfig.ruleIndex->size;i++) {
        struct bucket *b = ruleEngineConfig.ruleIndex->buckets[i];

        struct bucket *resumingBucket = b;

        while(resumingBucket!=NULL) {

            struct bucket *curr = resumingBucket;
            resumingBucket = curr->next;

            RuleIndexList *currIndex = (RuleIndexList *)curr->value;
            Hashtable *processedStrs = newHashTable(MAX_NUM_RULES * 2);

            RuleIndexListNode *currIndexNode = currIndex->head;
            int createIndex = 1;
            int groupCount = 0;
            Node *condExp = NULL;
            Node *params = NULL;

            while(createIndex && currIndexNode != NULL) {
                Node *ruleNode = getRuleNode(currIndexNode->ruleIndex);
                if(ruleNode->subtrees[1]->nodeType == N_APPLICATION &&
				   ruleNode->subtrees[1]->subtrees[0]->nodeType == TK_TEXT &&
				   strcmp(ruleNode->subtrees[1]->subtrees[0]->text, "==") == 0 && /* comparison */
                   ruleNode->subtrees[1]->subtrees[1]->nodeType == N_TUPLE &&
                   ruleNode->subtrees[1]->subtrees[1]->degree == 2 &&
                   ruleNode->subtrees[1]->subtrees[1]->subtrees[1]->nodeType == TK_STRING && /* with a string */
                   lookupFromHashTable(processedStrs, ruleNode->subtrees[1]->subtrees[1]->subtrees[1]->text)==NULL /* no repeated string */
                        ) {
                    int i;
                    insertIntoHashTable(processedStrs, ruleNode->subtrees[1]->subtrees[1]->text, ruleNode->subtrees[1]->subtrees[1]->text);
                    groupCount ++;
                    if(condExp == NULL) {
                        condExp = ruleNode->subtrees[1]->subtrees[1]->subtrees[0];
                        params = ruleNode->subtrees[0]->subtrees[0];

                    } else if(RULE_NODE_NUM_PARAMS(ruleNode) == params->degree) {
                    	Hashtable *varMapping = newHashTable(100);
                        for(i=0;i<params->degree;i++) {
                            updateInHashTable(varMapping, params->subtrees[i]->text, ruleNode->subtrees[0]->subtrees[0]->subtrees[i]->text);
                        }
                        if(!eqExprNodeSyntacticVarMapping(condExp, ruleNode->subtrees[1]->subtrees[0], varMapping)) {
                            createIndex = 0;
                        }
                        deleteHashTable(varMapping, nop);
                    } else {
                        createIndex = 0;
                    }

                }
                currIndexNode = currIndexNode ->next;
            }

            deleteHashTable(processedStrs, nop);
            /* generate cond index if there are more than one rules in this group */
            if(!createIndex || groupCount <= 1) {
                continue;
            }

            #ifdef DEBUG_INDEX
            printf("inserting rule group %s into ruleEngineConfig.condIndex\n", currIndex->ruleName);
            #endif
            Hashtable *groupHashtable = newHashTable(groupCount * 2);
            CondIndexVal *civ = newCondIndexVal(condExp, params, groupHashtable, r);
            char *ruleName = currIndex->ruleName;
            insertIntoHashTable(ruleEngineConfig.condIndex, ruleName, civ);

            currIndexNode = currIndex->head;
            while(currIndexNode != NULL) {
                Node *ruleNode = getRuleNode(currIndexNode->ruleIndex);
                Node *condNode = ruleNode->subtrees[1];
                #ifdef DEBUG_INDEX
                printf("inserting rule cond str %s, index %d into ruleEngineConfig.condIndex\n", condNode->subtrees[1]->text, currIndexNode->ruleIndex);
                #endif
                insertIntoHashTable(groupHashtable, condNode->subtrees[1]->text, currIndexNode);
                currIndexNode = currIndexNode->next;
            }

        }

    }
    ruleEngineConfig.condIndexStatus = LOCAL;
    return 1;

}
void removeNodeFromRuleIndexList(RuleIndexList *rd, int i) {
	if(rd->head->ruleIndex == i) {
		rd->head = rd->head->next;

	} else {
		RuleIndexListNode *n = rd->head;
		while(n->next->ruleIndex != i) {
			n=n->next;
		}
		n->next = n->next->next;
		if(n->next == NULL) {
			rd->tail = n;
		}
	}
}
void appendRuleNodeToRuleIndexList(RuleIndexList *list, int i, Region *r) {
    RuleIndexListNode *listNode = newRuleIndexListNode(i, r);
    list->tail ->next = listNode;
    list->tail = listNode;

}
void prependRuleNodeToRuleIndexList(RuleIndexList *list, int i, Region *r) {
    RuleIndexListNode *listNode = newRuleIndexListNode(i, r);
    listNode->next = list->head;
    list->head = listNode;

}
/**
 * returns 0 if out of memory
 */
int createRuleNodeIndex(RuleSet *inRuleSet, Hashtable *ruleIndex, int offset, Region *r)
{

    /* generate main index */
    int i;
    for (i=0;i<inRuleSet->len;i++) {
        Node *ruleNode = inRuleSet->rules[i]->node;
        if(ruleNode == NULL)
            continue;
        char *key = ruleNode->subtrees[0]->text;
        RuleIndexList *list = (RuleIndexList *)lookupFromHashTable(ruleIndex, key);
        if(list != NULL) {
        	appendRuleNodeToRuleIndexList(list, i + offset, r);
        } else {
            RuleType ruleType = ruleNode->subtrees[2]->nodeType == N_ACTIONS? RK_REL : RK_FUNC;
            if (insertIntoHashTable(ruleIndex, key, newRuleIndexList(key, ruleType, i + offset, r)) == 0) {

                return 0;
            }
        }
    }

    return 1;
}
/**
 * returns 0 if out of memory
 */
int createFuncMapDefIndex(rulefmapdef_t *inFuncStrct, Hashtable **ruleIndex)
{
	clearIndex(*ruleIndex);
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
	clearIndex(microsTableIndex);
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
/* find the next rule to *index, if *index is -1, return the index of the first rule found */
int findNextRuleFromIndex(Hashtable *ruleIndex, char *action, int *index)
{
	int  i=*index;

	if (ruleIndex!=NULL) {
/*        dumpHashtableKeys(coreRuleIndex); */
		RuleIndexList *l=(RuleIndexList *)lookupFromHashTable(ruleIndex, action);
		if (l == NULL) {
			return NO_MORE_RULES_ERR;
		}
		RuleIndexListNode *b = l->head;
		if(i == -1) {
			if (b!=NULL) {
				*index = b->ruleIndex;
				return(0);
			}

		} else {
			while (b!=NULL && b->ruleIndex!=i) {
				b= b->next;
			}
			if (b!=NULL && b->next!=NULL) {
				*index = b->next->ruleIndex;
				return(0);
			}
		}
	}

	return NO_MORE_RULES_ERR;
}
/**
 * adapted from original code
 */
int findNextRule2(char *action,  int *ruleInx) {
	if (isComponentInitialized(ruleEngineConfig.ruleIndexStatus)) {
		int ii = findNextRuleFromIndex(ruleEngineConfig.ruleIndex, action, ruleInx);
		if (ii!=NO_MORE_RULES_ERR) {
			return 0;
		} else {
			return NO_MORE_RULES_ERR;
		}
	} else {
		return NO_MORE_RULES_ERR;
	}
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
    deleteHashTable(h->valIndex, nop);
}

char *convertRuleNameArityToKey(char *ruleName, int arity) {
    // assume that arity < 100
    char *key = (char *)malloc(strlen(ruleName) + 3);
    sprintf(key, "%02d%s", arity, ruleName);
    return key;
}

RuleIndexList *newRuleIndexList(char *ruleName, RuleType type, int ruleIndex, Region *r) {
    RuleIndexList *list = (RuleIndexList *)region_alloc(r, sizeof(RuleIndexList));
    list->ruleName = cpStringExt(ruleName, r);
    list->type = type;
    list->head = list->tail = newRuleIndexListNode(ruleIndex, r);
    return list;
}

RuleIndexListNode *newRuleIndexListNode(int ruleIndex, Region *r) {
    RuleIndexListNode *node = (RuleIndexListNode *)region_alloc(r, sizeof(RuleIndexListNode));
    node->ruleIndex = ruleIndex;
    node->next = NULL;
    return node;

}

CondIndexVal *newCondIndexVal(Node *condExp, Node *params, Hashtable *groupHashtable, Region *r) {
            CondIndexVal *civ = (CondIndexVal *)region_alloc(r, sizeof(CondIndexVal));
            civ->startIndex = 0;
            civ->finishIndex = MAX_NUM_RULES;
            civ->condExp = condExp;
            civ->params = params;
            civ->valIndex = groupHashtable;
            return civ;
}
