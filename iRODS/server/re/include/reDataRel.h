/* reDataRel.h */

typedef struct
{
  char attribute[NAME_MAX+1];
  char value[NAME_MAX+1];
  char units[NAME_MAX+1];
} UserDefinedMetadata_t;

int intGetDataObjChksumsTimeStampsFromAVU (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
int iFindChkSumDateAvuMetadata(int status, genQueryOut_t *genQueryOut, char *fullName, UserDefinedMetadata_t aAVUarray[], int *countUserDefinedMetadata);
int intAddChkSumDateAvuMetadata (rsComm_t * rsComm, char *objPath, time_t t1, int *iStatus);
int mamChksSumDatumNeboHodnotu (rsComm_t * rsComm, char* strFullDataPath, time_t t, ruleExecInfo_t * rei);
int msiChkRechkRecompChkSum4DatObj (msParam_t * inpParam1, msParam_t * outParam1, ruleExecInfo_t * rei);
