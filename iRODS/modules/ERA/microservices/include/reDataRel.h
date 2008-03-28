
/* reDataRel.h */

#if !defined(NAME_MAX)
#define NAME_MAX	128 /* XXXXX take me out. just to get around compile error */
#endif

typedef struct
{
  char attribute[NAME_MAX+1];
  char value[NAME_MAX+1];
  char units[NAME_MAX+1];
} UserDefinedMetadata_t;

int intGetDataObjChksumsTimeStampsFromAVU (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
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
