/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* objInfo.h - header file for general Obj Info
 */

/* rescInfo_t is for info about a resource
rescGrpInfo_t is for info about a resource group
dataObjInfo_t is for info about a data object.
intKeyStrVal_t is a generic (integer keyword)/(string value) pair.
It can be used for many things. For example, for the input "condition",
the condKeywd_t is used for keyword. But it can also be used to 
input parameters for rcat registration.

The routine addIntKeywdStrVal() in rcMisc.c can be used to add a      
keyword/value pair and getValByIntKeywd() can be used to get the
value string based on a keyword.
*/

#ifndef OBJ_INFO_H
#define OBJ_INFO_H

#include "rodsType.h"
#include "rodsUser.h"

/* this defines the "copies" condition */
#define ALL_COPIES	-1	/* "all" */ 

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct RescInfo
{
    char rescName[NAME_LEN];
    rodsLong_t rescId;
    char zoneName[NAME_LEN];
    char rescLoc[NAME_LEN];
    char rescType[NAME_LEN];
    int rescTypeInx;
    char rescClass[NAME_LEN];
    int rescClassInx;
    char rescVaultPath[MAX_NAME_LEN];
    int numOpenPorts;
    int paraOpr;
    char rescInfo[LONG_NAME_LEN];
    char rescComments[LONG_NAME_LEN];
    char gateWayAddr[NAME_LEN];
    rodsLong_t rescMaxObjSize;
    rodsLong_t freeSpace;
    time_t freeSpaceTime;       /* last time freeSpace was checked */
    char freeSpaceTimeStamp[TIME_LEN];
    char rescCreate[TIME_LEN];
    char rescModify[TIME_LEN];
    void *rodsServerHost;
} rescInfo_t;

/* link of resource in resource group */

typedef struct RescGrpInfo
{
    char rescGroupName[NAME_LEN];
    rescInfo_t *rescInfo;
    struct RescGrpInfo *cacheNext; 	/* this is for cached resource grp */
    struct RescGrpInfo *next; 
} rescGrpInfo_t;

typedef struct RescCacheInfo
{
    char inpRescName[NAME_LEN];
    rescGrpInfo_t *rescGrpInfo;
    struct RescCacheInfo *next;
} rescCacheInfo_t;

/* special collection */

typedef enum {         /* class of SpecColl */
    NO_SPEC_COLL,
    STRUCT_FILE_COLL,
    MOUNTED_COLL,
} specCollClass_t;

typedef enum {          /* structFile type */
    HAAW_STRUCT_FILE_T,        /* the UK eScience structFile */
    TAR_STRUCT_FILE_T,	     /* The tar structFile */
} structFileType_t;

typedef enum {          /* specColl operation type */
    NOT_SPEC_COLL_OPR,
    NON_STRUCT_FILE_SPEC_COLL_OPR,
    STRUCT_FILE_SPEC_COLL_OPR,
    NORMAL_OPR_ON_STRUCT_FILE_COLL,
} structFileOprType_t;
    
#define HAAW_STRUCT_FILE_STR		"haawStructFile"
#define TAR_STRUCT_FILE_STR		"tarStructFile"
#define MOUNT_POINT_STR		"mountPoint"
#define INHERIT_PAR_SPEC_COLL_STR	"inheritParentSpecColl"

#define UNMOUNT_STR		"unmount"

typedef struct SpecColl {
    specCollClass_t class;
    structFileType_t type;   
    char collection[MAX_NAME_LEN];  /* structured file or mounted collection */
    char objPath[MAX_NAME_LEN];      /* STRUCT_FILE_COLL-logical path of structFile 
				      * MOUNTED_COLL - NA 
				      */
    char resource[NAME_LEN];	     /* the resource */
    char phyPath[MAX_NAME_LEN];	     /* STRUCT_FILE_COLL-the phyPath of structFile
				      * MOUNTED_COLL-the phyPath od mounted
				      * directory
				      */
    char cacheDir[MAX_NAME_LEN];     /* STRUCT_FILE_COLL-the directory where 
				      * the cache tree is kept
				      */
    int cacheDirty;		     /* Whether the cache has been written */ 
    int replNum;
} specColl_t;

typedef enum {
    UNKNOW_COLL_PERM,
    READ__COLL_PERM,
    WRITE_COLL_PERM,
} specCollPerm_t;

typedef struct SpecCollCache {
    specCollPerm_t perm;
    specColl_t specColl;
    char collId[NAME_LEN];
    char ownerName[NAME_LEN];
    char ownerZone[NAME_LEN];
    char createTime[NAME_LEN];
    char modifyTime[NAME_LEN];
    struct SpecCollCache *next;
} specCollCache_t;

/* definition for replStatus (isDirty) */
#define OLD_COPY        0x0
#define NEWLY_CREATED_COPY      0x1
#define OPEN_EXISTING_COPY	0x10

typedef struct DataObjInfo {
    char objPath[MAX_NAME_LEN];
    char rescName[NAME_LEN];       /* This could be resource group */
    char rescGroupName[NAME_LEN];       /* This could be resource group */
    char dataType[NAME_LEN];
    rodsLong_t dataSize;
    char chksum[NAME_LEN];
    char version[NAME_LEN];
    char filePath[MAX_NAME_LEN];
    rescInfo_t *rescInfo;
    char dataOwnerName[NAME_LEN];
    char dataOwnerZone[NAME_LEN];
    int  replNum;
    int  replStatus;     /* isDirty flag */
    char statusString[NAME_LEN];     
    rodsLong_t  dataId;
    rodsLong_t  collId;
    int  dataMapId;
    char dataComments[LONG_NAME_LEN];
    char dataExpiry[TIME_LEN];
    char dataCreate[TIME_LEN];
    char dataModify[TIME_LEN];
    char dataAccess[NAME_LEN];
    int  dataAccessInx;
    char destRescName[NAME_LEN];
    char backupRescName[NAME_LEN];
    char subPath[MAX_NAME_LEN];
    specColl_t *specColl;
    struct DataObjInfo *next;
} dataObjInfo_t;

typedef struct CollInfo {
  rodsLong_t collId;
  char collName[MAX_NAME_LEN];
  char collParentName[MAX_NAME_LEN];
  char collOwnerName[NAME_LEN];
  char collOwnerZone[NAME_LEN];
  int  collMapId;
  char collComments[LONG_NAME_LEN];
  char collInheritance[LONG_NAME_LEN];
  char collExpiry[TIME_LEN];
  char collCreate[TIME_LEN];
  char collModify[TIME_LEN];
  char collAccess[NAME_LEN];
  int  collAccessInx;
  char collType[NAME_LEN];
  char collInfo1[MAX_NAME_LEN];
  char collInfo2[MAX_NAME_LEN];
  
  struct CollInfo *next;
} collInfo_t;

typedef struct RuleInfo {
   int TDB;
} ruleInfo_t;

/* keyValPair_t - str key, str value pair */
typedef struct KeyValPair {
    int len;
    char **keyWord;     /* array of keyword */
    char **value;       /* pointer to an array of values */
} keyValPair_t;

/* inxIvalPair_t - int index, int value pair */

typedef struct InxIvalPair {
    int len;
    int *inx;     	/* pointer to an array of int index */
    int *value;       /* pointer to an array of int value values */
} inxIvalPair_t;

/* inxValPair_t - int index, str value pair */

typedef struct InxValPair {
    int len;
    int *inx;          /* pointer to an array of int index */
    char **value;       /* pointer to an array of str value values */
} inxValPair_t;

/* strArray_t - just a single array */
typedef struct StrArray {
    int len;
    int size;
    char *value;	/* char aray of [len][size] */
} strArray_t;

/* intArray_t - just a single array */
typedef struct IntArray {
    int len;
    int *value;        /* int aray of [len] */
} intArray_t;


/* definition for RescTypeDef */

typedef enum {	/* resource category */
    FILE_CAT,
} rescCat_t;

typedef enum {
    UNIX_FILE_TYPE,
} fileDriverType_t;

#define DEFAULT_FILE_MODE	0600
#define DEFAULT_DIR_MODE	0750

/* definition for chkPathPerm */

#define NO_CHK_PATH_PERM	0
#define DO_CHK_PATH_PERM	1

/* definition for trash policy */

#define DO_TRASH_CAN	0
#define NO_TRASH_CAN	1

typedef struct RescTypeDef {
    char *typeName;
    rescCat_t rescCat;
    int driverType;
    int chkPathPerm;	/* whether to check path permission */
} rescTypeDef_t;

/* definition for classType */

#define CACHE_CL	0
#define ARCHIVAL_CL	1

#define PRIMARY_FLAG	0x8000		/* primary class when this bit is set */
typedef struct RescClass {
    char *className;
    int classType;
} rescClass_t;

typedef struct {
    int numThreads;
    rodsLong_t bytesWritten;
} transStat_t;

/* tagStruct_t - tagged keyword structure
   preTag  defines the reg exp to find beginning of value
   postTag defines the reg exp to be checked to find end of value
   the value found between the tags is associated with the KeyWord*/
typedef struct TagStruct {
    int len;
    char **preTag;     /* array of prestring tag */
    char **postTag;     /* array of poststring tag */
    char **keyWord;       /* pointer to an array of KeyWords */
} tagStruct_t;

#if 0
typedef struct SpecCollMeta {
    char objType[NAME_LEN];     /* structFile subfile - structFile type,
                                 * mounted obj */
    char collection[MAX_NAME_LEN];	/* path of the special collection.
                                         * structFile subfile - structured
					 * file coll
                                         * mounted obj - mounted coll */
    char collInfo1[MAX_NAME_LEN];       /* structFile subfile - structFile file path
                                         * mounted obj - mounted dir path */
    char collInfo2[MAX_NAME_LEN];       /* structFile subfile - NA
                                         * mounted obj - resource */
} specCollMeta_t;
#endif

typedef struct Subfile {
    rodsHostAddr_t addr;
    char subFilePath[MAX_NAME_LEN];
    int mode;
    int flags;
    rodsLong_t offset;
    specColl_t *specColl;
} subFile_t;

typedef struct StructFileTypeDef {
    char *typeName;
    structFileType_t type;
} structFileTypeDef_t;

#ifdef  __cplusplus
}
#endif

#endif	/* OBJ_INFO_H */

