/* reDataRel.h */

typedef struct
{
  char attribute[NAME_MAX+1];
  char value[NAME_MAX+1];
  char units[NAME_MAX+1];
} UserDefinedMetadata_t;

int iGetDataObjChksumsTimeStampsFromAVU (collInp_t* outCollInp, UserDefinedMetadata_t* aAVUarray, int* countUserDefinedMetadata, char* strOut, ruleExecInfo_t *rei);
