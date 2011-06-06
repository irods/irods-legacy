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
#define RESC_FUNC_DESC_INDEX 0x10
#define RESC_REGION_FUNC 0x20
#define RESC_REGION_INDEX 0x80
#define RESC_REGION_CORE 0x100
#define RESC_REGION_APP 0x200
#define RESC_EXT_RULE_SET 0x400
#define RESC_CACHE 0x40
typedef enum ruleEngineStatus {
    UNINITIALIZED,
    INITIALIZED,
    SHARED,
    DISABLED
} RuleEngineStatus;
typedef struct {
    unsigned char *address;
    long dataSize;
    long cacheSize;
    RuleEngineStatus coreRuleSetStatus;
    RuleEngineStatus appRuleSetStatus;
    RuleEngineStatus extRuleSetStatus;
    RuleEngineStatus funcDescIndexStatus;
    RuleEngineStatus ruleIndexStatus;
    RuleEngineStatus condIndexStatus;
    RuleEngineStatus ruleEngineStatus;
    RuleEngineStatus cacheStatus;
    RuleEngineStatus regionFuncStatus;
    RuleEngineStatus regionIndexStatus;
    RuleEngineStatus regionCoreStatus;
    RuleEngineStatus regionAppStatus;
    RuleSet *coreRuleSet;
    RuleSet *appRuleSet;
    RuleSet *extRuleSet;
    Hashtable *funcDescIndex;
    Hashtable *ruleIndex;
    Hashtable *condIndex;
    Region *regionFunc;
    Region *regionIndex;
    Region *regionCore;
    Region *regionApp;
} Cache;

extern unsigned char *ruleEngineMem;
extern RuleEngineStatus _ruleEngineStatus;
extern int isServer;
extern Cache ruleEngineConfig;
RuleEngineStatus getRuleEngineStatus();
void clearResources(int resources);
void clearRuleSetAndIndex(ruleStruct_t *inRuleStruct);
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

#endif /* _CONFIGURATION_H */
