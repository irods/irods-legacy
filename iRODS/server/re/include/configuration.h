/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
#include "rules.h"
#include "hashtable.h"
#include "parser.h"
#define RESC_RULE_INDEX 0x1
#define RESC_COND_INDEX 0x2
#define RESC_CORE_RULE_SET 0x4
#define RESC_APP_RULE_SET 0x8
#define RESC_EXT_RULE_SET 0x400
#define RESC_CORE_FUNC_DESC_INDEX 0x10
#define RESC_APP_FUNC_DESC_INDEX 0x20
#define RESC_EXT_FUNC_DESC_INDEX 0x40
#define RESC_REGION_INDEX 0x80
#define RESC_REGION_CORE 0x100
#define RESC_REGION_APP 0x200
#define RESC_CACHE 0x800
typedef enum ruleEngineStatus {
    UNINITIALIZED,
    INITIALIZED,
    SHARED,
    LOCAL,
    COMPRESSED,
    DISABLED
} RuleEngineStatus;
typedef struct {
    unsigned char *address;
    long dataSize;
    long cacheSize;
    RuleEngineStatus coreRuleSetStatus;
    RuleEngineStatus appRuleSetStatus;
    RuleEngineStatus extRuleSetStatus;
    RuleEngineStatus coreFuncDescIndexStatus;
    RuleEngineStatus appFuncDescIndexStatus;
    RuleEngineStatus extFuncDescIndexStatus;
    RuleEngineStatus ruleIndexStatus;
    RuleEngineStatus condIndexStatus;
    RuleEngineStatus ruleEngineStatus;
    RuleEngineStatus cacheStatus;
    RuleEngineStatus regionIndexStatus;
    RuleEngineStatus regionCoreStatus;
    RuleEngineStatus regionAppStatus;
    RuleSet *coreRuleSet;
    RuleSet *appRuleSet;
    RuleSet *extRuleSet;
    Env *coreFuncDescIndex;
    Env *appFuncDescIndex;
    Env *extFuncDescIndex;
	Hashtable *ruleIndex;
    Hashtable *condIndex;
    Region *regionIndex;
    Region *regionCore;
    Region *regionApp;
} Cache;
#define isComponentInitialized(x) ((x)==INITIALIZED || (x)==COMPRESSED || (x)==LOCAL)
#define isComponentAllocated(x) ((x)==INITIALIZED || (x)==LOCAL)
#define clearRegion(u, l) \
		if((resources & RESC_REGION_##u) && isComponentAllocated(ruleEngineConfig.region##l##Status)) { \
			region_free(ruleEngineConfig.region##l); \
			ruleEngineConfig.region##l = NULL; \
			ruleEngineConfig.region##l##Status = UNINITIALIZED; \
		} \

#define delayClearRegion(u, l) \
		if((resources & RESC_REGION_##u) && isComponentAllocated(ruleEngineConfig.region##l##Status)) { \
			listAppendNoRegion(&regionsToClear, ruleEngineConfig.region##l); \
			ruleEngineConfig.region##l = NULL; \
			ruleEngineConfig.region##l##Status = UNINITIALIZED; \
		} \

#define createRegion(u, l) \
		if(ruleEngineConfig.region##l##Status != INITIALIZED) { \
			ruleEngineConfig.region##l = make_region(0, NULL); \
			ruleEngineConfig.region##l##Status = INITIALIZED; \
		} \

#define clearRuleSet(u, l) \
		if((resources & RESC_##u##_RULE_SET) && isComponentAllocated(ruleEngineConfig.l##RuleSetStatus)) { \
			free(ruleEngineConfig.l##RuleSet); \
			ruleEngineConfig.l##RuleSet = NULL; \
			ruleEngineConfig.l##RuleSetStatus = UNINITIALIZED; \
		} else if((resources & RESC_##u##_RULE_SET) && ruleEngineConfig.l##RuleSetStatus == COMPRESSED) { \
			ruleEngineConfig.l##RuleSetStatus = UNINITIALIZED; \
		} \

#define delayClearRuleSet(u, l) \
		if((resources & RESC_##u##_RULE_SET) && isComponentAllocated(ruleEngineConfig.l##RuleSetStatus)) { \
			listAppendNoRegion(&memoryToFree, ruleEngineConfig.l##RuleSet); \
			ruleEngineConfig.l##RuleSet = NULL; \
			ruleEngineConfig.l##RuleSetStatus = UNINITIALIZED; \
		} else if((resources & RESC_##u##_RULE_SET) && ruleEngineConfig.l##RuleSetStatus == COMPRESSED) { \
			ruleEngineConfig.l##RuleSetStatus = UNINITIALIZED; \
		} \

#define delayClearFuncDescIndex(u, l) \
		if((resources & RESC_##u##_FUNC_DESC_INDEX) && isComponentAllocated(ruleEngineConfig.l##FuncDescIndexStatus)) { \
			listAppendNoRegion(&envToClear, ruleEngineConfig.l##FuncDescIndex); \
			ruleEngineConfig.l##FuncDescIndex = NULL; \
			ruleEngineConfig.l##FuncDescIndexStatus = UNINITIALIZED; \
		} else if((resources & RESC_##u##_FUNC_DESC_INDEX) && ruleEngineConfig.l##FuncDescIndexStatus == COMPRESSED) { \
			ruleEngineConfig.l##FuncDescIndexStatus = UNINITIALIZED; \
		} \

#define createRuleSet(u, l) \
		if(!isComponentInitialized(ruleEngineConfig.l##RuleSetStatus)) { \
			ruleEngineConfig.l##RuleSet = (RuleSet *)malloc(sizeof(RuleSet)); \
			ruleEngineConfig.l##RuleSet->len = 0; \
			ruleEngineConfig.l##RuleSetStatus = INITIALIZED; \
		} \

#define clearFuncDescIndex(u, l) \
	if((resources & RESC_##u##_FUNC_DESC_INDEX) && isComponentAllocated(ruleEngineConfig.l##FuncDescIndexStatus)) { \
		deleteEnv(ruleEngineConfig.l##FuncDescIndex, 1); \
		ruleEngineConfig.l##FuncDescIndex = NULL; \
		ruleEngineConfig.l##FuncDescIndexStatus = UNINITIALIZED; \
	} \

#define createFuncDescIndex(u, l) \
	if(!isComponentInitialized(ruleEngineConfig.l##FuncDescIndexStatus)) { \
		ruleEngineConfig.l##FuncDescIndex = newEnv(NULL, NULL, NULL); \
		ruleEngineConfig.l##FuncDescIndex->current = newHashTable(1000); \
		ruleEngineConfig.l##FuncDescIndexStatus = INITIALIZED; \
		getSystemFunctions(ruleEngineConfig.coreFuncDescIndex->current, ruleEngineConfig.regionCore); \
	} \

extern unsigned char *ruleEngineMem;
extern RuleEngineStatus _ruleEngineStatus;
extern int isServer;
extern Cache ruleEngineConfig;
RuleEngineStatus getRuleEngineStatus();
void clearResources(int resources);
int clearRuleSetAndIndex(ruleStruct_t *inRuleStruct);
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct);
int loadRuleFromCacheOrFile(char *irbSet, ruleStruct_t *inRuleStruct);
int availableRules();
void removeRuleFromIndex(char *ruleName, int i);
void appendRuleIntoIndex(RuleDesc *rule, int i, Region *r);
void prependRuleIntoIndex(RuleDesc *rule, int i, Region *r);
int checkPointExtRuleSet();
void appendAppRule(RuleDesc *rd, Region *r);
void prependAppRule(RuleDesc *rd, Region *r);
void popExtRuleSet(int checkPoint);
void clearDelayed();
int generateFunctionDescriptionTables();

#endif /* _CONFIGURATION_H */
