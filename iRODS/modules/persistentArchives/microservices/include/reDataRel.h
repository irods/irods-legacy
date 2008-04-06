
/* reDataRel.h */

#if !defined(NAME_MAX)
#define NAME_MAX	128 /* XXXXX take me out. just to get around compile error */
#endif

#define MAX_NAME_LEN255 255

typedef struct
{
  char attribute[NAME_MAX+1];
  char value[NAME_MAX+1];
  char units[NAME_MAX+1];
} UserDefinedMetadata_t;

typedef struct
{
  char chCollection[NAME_MAX+1];
  char chObjName[NAME_MAX+1];
  char chDataSize[NAME_MAX+1];
  long lDataSize[NAME_MAX+1];
  char chModifyTime[TIME_LEN+1];
  char chRescName[TIME_LEN+1];
  int isData; /* ==1 for data, ==2 for directory */
  int iDataExpiry;
  char chDataExpiry[TIME_LEN+1];
  char chUserName[TIME_LEN+1]; /* ACl #1 */
  char chDataAccessName[TIME_LEN+1]; /* ACl #2 */
} lsInfo_t;

int intGetDataObjChksumsTimeStampsFromAVU (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
int intGetDataObjChksumsTimeStampsFromAVU9 (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
int iFindChkSumDateAvuMetadata(int status, genQueryOut_t *genQueryOut, char *fullName, UserDefinedMetadata_t aAVUarray[], int *countUserDefinedMetadata);
int intAddChkSumDateAvuMetadata (rsComm_t * rsComm, char *objPath, time_t t1, int *iStatus);
int intChkRechkRecompChkSum4DatObj (rsComm_t * rsComm, char* strFullDataPath, time_t t, ruleExecInfo_t * rei);
int msiChkRechkRecompChkSum4DatObj (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei);

int intGetDataObjChksumsTimeStampsFromAVUVol2 (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
int iFindChkSumDateAvuMetadataVol2(int status, genQueryOut_t *genQueryOut, char *fullName, UserDefinedMetadata_t aAVUarray[], int *countUserDefinedMetadata);
int intAddChkSumDateAvuMetadataVol2 (rsComm_t * rsComm, char *objPath, time_t t1, int *iStatus);
int intChkRechkRecompChkSum4DatObjVol2 (rsComm_t * rsComm, char* strFullDataPath, time_t t, ruleExecInfo_t * rei);
int msiChkRechkRecompChkSum4DatObjVol2 (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * inpParam3, msParam_t * outParam1, ruleExecInfo_t * rei);


int intAttr1Lev4                          (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
int intAttr1Lev3              (int status, genQueryOut_t *genQueryOut, char *fullName, UserDefinedMetadata_t aAVUarray[], int *countUserDefinedMetadata);
int intAttr1Lev5                (rsComm_t * rsComm, char *objPath, time_t t1, int *iStatus);
int intAttr1Lev2                   (rsComm_t * rsComm, char* strFullDataPath, time_t t, ruleExecInfo_t * rei);
int msiAttr1Lev2                   (msParam_t * inpParam1, msParam_t * inpParam2, msParam_t * outParam1, ruleExecInfo_t * rei);
