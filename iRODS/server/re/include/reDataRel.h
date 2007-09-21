/* reDataRel.h */

typedef struct
{
  char attribute[NAME_MAX+1];
  char value[NAME_MAX+1];
  char units[NAME_MAX+1];
} UserDefinedMetadata_t;

int intGetDataObjChksumsTimeStampsFromAVU (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
int iFindChkSumDateAvuMetadata(int status, genQueryOut_t *genQueryOut, char *fullName, UserDefinedMetadata_t aAVUarray[], int *countUserDefinedMetadata);
