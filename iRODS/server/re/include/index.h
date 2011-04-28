/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef INDEX_H
#define INDEX_H
#include "debug.h"
#include "hashtable.h"
#include "rules.h"
#ifndef DEBUG
#include "reGlobalsExtern.h"
#endif

typedef struct condIndexVal CondIndexVal;

struct condIndexVal {
    int startIndex, finishIndex;
    Node *params;
    Node *condExp;
    Hashtable *valIndex; /* char * -> int * */
};

typedef struct ruleIndexListNode {
    int ruleIndex;
    struct ruleIndexListNode *next;
} RuleIndexListNode;

typedef struct ruleIndexList {
    char *ruleName;
    RuleType type;
    RuleIndexListNode *head, *tail;
} RuleIndexList;

char *convertRuleNameArityToKey(char *ruleName, int arity);
RuleIndexList *newRuleIndexList(char *ruleName, RuleType type, int ruleIndex, Region *r);
RuleIndexListNode *newRuleIndexListNode(int ruleIndex, Region *r);
CondIndexVal *newCondIndexVal(Node *condExp, Node *params, Hashtable *groupHashtable, Region *r);

extern Hashtable *coreRuleIndex;
extern Hashtable *appRuleIndex;
extern Hashtable *coreRuleFuncMapDefIndex;
extern Hashtable *appRuleFuncMapDefIndex;
extern Hashtable *microsTableIndex;
/* this is an index of indexed rules */
/* indexed rules are rules such that */
/* 1. all rules has the same rule name */
/* 2. all rules has an empty param list */
/* 3. no other rule has the same rule name */
/* 4. the rule conditions are all of the form exp = "string", where exp is the same and "string" is distinct for all rules */
/* when a subset of rules are indexed rules, the condIndex */
extern Hashtable *condIndex; /* char * -> CondIndexVal * */

void clearIndex(Hashtable **ruleIndex);

int createRuleIndex(ruleStruct_t *inRuleStrct, Hashtable **ruleIndex);
int createRuleNodeIndex(RuleSet *inRuleSet, Hashtable **ruleIndex, Region *r);
int createFuncMapDefIndex(rulefmapdef_t *inFuncStrct1, Hashtable **ruleIndex);

int mapExternalFuncToInternalProc2(char *funcName);
int findNextRuleFromIndex(Hashtable *ruleIndex, char *action, int *index);
int findNextRule2(char *action,  int *ruleInx);
int actionTableLookUp2(char *action);
int createMacorsIndex();
void deleteCondIndexVal(CondIndexVal *h);

#endif
