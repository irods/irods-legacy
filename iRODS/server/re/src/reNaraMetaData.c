#include "reNaraMetaData.h"
#include "apiHeaderAll.h"

int
msiExtractNaraMetadata (ruleExecInfo_t *rei)
{
  FILE *fp;
  char str[500];
  char *substring;
  int counter;
  int flag;
  char attr[100];
  char value[500];
  char unit[100];
  modAVUMetadataInp_t modAVUMetadataInp;
  int status;
  /* specify the location of the metadata file here */
  char metafile[MAX_NAME_LEN];

  snprintf (metafile, MAX_NAME_LEN, "%-s/reConfigs/%-s", getConfigDir(), 
   NARA_META_DATA_FILE);

  if((fp=fopen(metafile, "r")) == NULL) {
    rodsLog (LOG_ERROR,
     "msiExtractNaraMetadata: Cannot open the metadata file %s.", metafile);
    return (UNIX_FILE_OPEN_ERR);
  }
  
   memset (&modAVUMetadataInp, 0, sizeof (modAVUMetadataInp));
   modAVUMetadataInp.arg0 = "add";

  while(!feof(fp)){
    counter = 0;
    flag = 0;
    if(fgets(str, 500, fp)){
       substring = strtok (str,"|");
       while (substring != NULL){
         if(flag == 0 && strcmp(substring,rei->doi->objPath) == 0){
           flag = 2;
         }

         if(counter == 1){
           strcpy( attr, substring );
         }
         if(flag == 2 && counter == 2){
           strcpy( value, substring );
           /*Call the function to insert metadata here.*/
	   modAVUMetadataInp.arg1 = "-d";
	   modAVUMetadataInp.arg2 = rei->doi->objPath;
	   modAVUMetadataInp.arg3 = attr;
	   modAVUMetadataInp.arg4 = value;
	   modAVUMetadataInp.arg5 = "";
           status = rsModAVUMetadata (rei->rsComm, &modAVUMetadataInp);
           rodsLog (LOG_DEBUG, "msiExtractNaraMetadata: %s:%s",attr, value);
         }
         substring = strtok (NULL, "|");
         counter++;
       }  
    }
  }
  fclose(fp);
  return(0);
}

