/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include "rules.h"
#include "hashtable.h"
#include "parser.h"
#ifdef USE_BOOST
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/creation_tags.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/detail/os_file_functions.hpp>
#include <boost/filesystem.hpp>
#else
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <unistd.h>
#endif

#define RESC_CORE_RULE_SET 0x1
#define RESC_APP_RULE_SET 0x2
#define RESC_EXT_RULE_SET 0x4
#define RESC_CORE_FUNC_DESC_INDEX 0x10
#define RESC_APP_FUNC_DESC_INDEX 0x20
#define RESC_EXT_FUNC_DESC_INDEX 0x40
#define RESC_REGION_CORE 0x100
#define RESC_REGION_APP 0x200
#define RESC_REGION_EXT 0x300
#define RESC_CACHE 0x800
typedef enum ruleEngineStatus {
    UNINITIALIZED,
    INITIALIZED,
    COMPRESSED,
    /*SHARED,
    LOCAL,
    DISABLED*/
} RuleEngineStatus;
#if defined(USE_BOOST)
typedef std::time_t time_type;
#define time_type_gt(mtim, timestamp) \
		(mtim > timestamp)
#define time_type_set(mtim, timestamp) \
		mtim = timestamp;
#define time_type_initializer ((time_type) 0)
#elif defined( _POSIX_VERSION )
typedef struct timespec time_type;
#define time_type_gt(mtim, timestamp) \
		((mtim).tv_sec > (timestamp).tv_sec || \
		((mtim).tv_sec == (timestamp).tv_sec && (mtim).tv_nsec > (timestamp).tv_nsec))
#define time_type_set(mtim, timestamp) \
		(mtim).tv_sec = (timestamp).tv_sec; \
		(mtim).tv_nsec = (timestamp).tv_nsec;
#define time_type_initializer {(__time_t) 0, (long int) 0}
#endif
typedef struct {
    unsigned char *address;
    unsigned char *pointers;
    size_t dataSize;
    size_t cacheSize;
    RuleEngineStatus coreRuleSetStatus;
    RuleEngineStatus appRuleSetStatus;
    RuleEngineStatus extRuleSetStatus;
    RuleEngineStatus coreFuncDescIndexStatus;
    RuleEngineStatus appFuncDescIndexStatus;
    RuleEngineStatus extFuncDescIndexStatus;
    RuleEngineStatus ruleEngineStatus;
    RuleEngineStatus cacheStatus;
    RuleEngineStatus coreRegionStatus;
    RuleEngineStatus appRegionStatus;
    RuleEngineStatus extRegionStatus;
    RuleSet *coreRuleSet;
    RuleSet *appRuleSet;
    RuleSet *extRuleSet;
    Env *coreFuncDescIndex;
    Env *appFuncDescIndex;
    Env *extFuncDescIndex;
    Region *coreRegion;
    Region *appRegion;
    Region *extRegion;
    int clearDelayed;
    time_type timestamp;
    time_type updateTS;
    unsigned int version;
} Cache;
#define isComponentInitialized(x) ((x)==INITIALIZED || (x)==COMPRESSED)
#define isComponentAllocated(x) ((x)==INITIALIZED)
#define clearRegion(u, l) \
		if((resources & RESC_REGION_##u) && isComponentAllocated(ruleEngineConfig.l##Region##Status)) { \
			region_free(ruleEngineConfig.l##Region); \
			ruleEngineConfig.l##Region = NULL; \
			ruleEngineConfig.l##Region##Status = UNINITIALIZED; \
		} \

#define delayClearRegion(u, l) \
		if((resources & RESC_REGION_##u) && isComponentAllocated(ruleEngineConfig.l##Region##Status)) { \
			listAppendNoRegion(&regionsToClear, ruleEngineConfig.l##Region); \
			ruleEngineConfig.l##Region = NULL; \
			ruleEngineConfig.l##Region##Status = UNINITIALIZED; \
		} \

#define createRegion(u, l) \
		if(ruleEngineConfig.l##Region##Status != INITIALIZED) { \
			ruleEngineConfig.l##Region = make_region(0, NULL); \
			ruleEngineConfig.l##Region##Status = INITIALIZED; \
		} \

#define clearRuleSet(u, l) \
		if((resources & RESC_##u##_RULE_SET) && isComponentInitialized(ruleEngineConfig.l##RuleSetStatus)) { \
			ruleEngineConfig.l##RuleSet = NULL; \
			ruleEngineConfig.l##RuleSetStatus = UNINITIALIZED; \
		} \

#define delayClearRuleSet(u, l) \
		if((resources & RESC_##u##_RULE_SET) && isComponentInitialized(ruleEngineConfig.l##RuleSetStatus)) { \
			ruleEngineConfig.l##RuleSet = NULL; \
			ruleEngineConfig.l##RuleSetStatus = UNINITIALIZED; \
		} \

#define delayClearFuncDescIndex(u, l) \
		if((resources & RESC_##u##_FUNC_DESC_INDEX) && isComponentInitialized(ruleEngineConfig.l##FuncDescIndexStatus)) { \
			listAppendNoRegion(&envToClear, ruleEngineConfig.l##FuncDescIndex); \
			ruleEngineConfig.l##FuncDescIndex = NULL; \
			ruleEngineConfig.l##FuncDescIndexStatus = UNINITIALIZED; \
		} else if((resources & RESC_##u##_FUNC_DESC_INDEX) && ruleEngineConfig.l##FuncDescIndexStatus == COMPRESSED) { \
			ruleEngineConfig.l##FuncDescIndexStatus = UNINITIALIZED; \
		} \

#define createRuleSet(u, l) \
		if(!isComponentInitialized(ruleEngineConfig.l##RuleSetStatus)) { \
			ruleEngineConfig.l##RuleSet = (RuleSet *)region_alloc(ruleEngineConfig.l##Region, sizeof(RuleSet)); \
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
		ruleEngineConfig.l##FuncDescIndex->current = newHashTable2(1000, ruleEngineConfig.l##Region); \
		ruleEngineConfig.l##FuncDescIndexStatus = INITIALIZED; \
	} \

extern unsigned char *ruleEngineMem;
extern RuleEngineStatus _ruleEngineStatus;
extern int isServer;
extern Cache ruleEngineConfig;

RuleEngineStatus getRuleEngineStatus();
int unlinkFuncDescIndex();
int clearResources(int resources);
int clearRuleIndex(ruleStruct_t *inRuleStruct);
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct);
int loadRuleFromCacheOrFile(char *irbSet, ruleStruct_t *inRuleStruct);
int createRuleIndex(ruleStruct_t *inRuleStruct);
int availableRules();
void removeRuleFromExtIndex(char *ruleName, int i);
void appendRuleIntoExtIndex(RuleDesc *rule, int i, Region *r);
void prependRuleIntoAppIndex(RuleDesc *rule, int i, Region *r);
int checkPointExtRuleSet();
void appendAppRule(RuleDesc *rd, Region *r);
void prependAppRule(RuleDesc *rd, Region *r);
void popExtRuleSet(int checkPoint);
void clearDelayed();
int generateFunctionDescriptionTables();
int getModifiedTime(char *fn, time_type *timestamp);

#define SEM_NAME "irods_sem_re"
#ifdef USE_BOOST
typedef boost::interprocess::named_mutex mutex_type;
#else
typedef sem_t mutex_type;
#endif

void unlockMutex(mutex_type **mutex);
int lockMutex(mutex_type **mutex);


#endif /* _CONFIGURATION_H */
