/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include <stdarg.h>
#include "apiHeaderAll.h"
#include "objStat.h"
#include "reDataObjOpr.h"
#include "reGlobalsExtern.h"
#include "reEraHelpers.h"

#define BIG_STR 200



/*
 * appendStrToBBuf() - Appends up to size characters to a bytesBuf_t buffer.
 * The buffer is treated as a string buffer.
 * Returns number of bytes written or a negative value upon failure.
 */
int
appendStrToBBuf(bytesBuf_t *dest, size_t size, const char *format, ...)
{
	va_list ap;
	int written;
	size_t index=0;
	char *tmpPtr;


	/* Perform initial sanity checks */
	if ( dest==NULL || dest->buf==NULL || dest->len<0 ) {
		return (-1);
	}

	/* How much has already been written? */
	while (((char *)dest->buf)[index]!='\0' && index<dest->len) {
		index++;
	}
	
	/* Increase buffer size if needed */
	if (index+size > dest->len) {
		dest->len=2*(index+size);
		tmpPtr=(char *)malloc(dest->len);
		strcpy(tmpPtr, dest->buf);
		free(dest->buf);
		dest->buf=tmpPtr;
	}

	/* Append new string to previously written characters */
	va_start(ap, format);
	written=vsnprintf(((char *)dest->buf)+index, size, format, ap);
	va_end(ap);

	return (written);
}



/*
 * Copies metadata AVUs from one iRODS object to another one.
 * Both the source and destination object can be either a file
 * or a collection, independently of each other.
 *
 */
int
copyAVUMetadata(char *destPath, char *srcPath, rsComm_t *rsComm)
{
	char destObjType[NAME_LEN];
	char srcObjType[NAME_LEN];
	modAVUMetadataInp_t modAVUMetadataInp;
	int status;


	/* Get type of source object */
	status = getObjType(rsComm, srcPath, srcObjType);
	
	if (status == INVALID_OBJECT_TYPE) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
			    "copyAVUMetadata: Invalid object type for source object: %s.", srcPath);
		return(status);
	}
	
	
	/* Get type of destination object */
	status = getObjType(rsComm, destPath, destObjType);
	
	if (status == INVALID_OBJECT_TYPE) {
		rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
			    "copyAVUMetadata: Invalid object type for destination object: %s.", destPath);
		return(status);
	}

	
	/* Fill in modAVUMetadataInp */
	modAVUMetadataInp.arg0 = "cp";
	modAVUMetadataInp.arg1 = srcObjType;
	modAVUMetadataInp.arg2 = destObjType;
	modAVUMetadataInp.arg3 = srcPath;
	modAVUMetadataInp.arg4 = destPath;
	modAVUMetadataInp.arg5 = "";
	modAVUMetadataInp.arg6 = "";
	modAVUMetadataInp.arg7 = "";
	modAVUMetadataInp.arg8 = "";
	modAVUMetadataInp.arg9 = "";

	
	/* Call rsModAVUMetadata() */
	status = rsModAVUMetadata(rsComm, &modAVUMetadataInp);
	
	return(status);
	
}



/*
 * Gets pipe separated metadata AVUs for a data object.
 * 
 */
int
getDataObjPSmeta(char *objPath, bytesBuf_t *mybuf, rsComm_t *rsComm)
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[10];
   int i1b[10];
   int i2a[10];
   char *condVal[10];
   char v1[BIG_STR];
   char v2[BIG_STR];
   char fullName[MAX_NAME_LEN];
   char myDirName[MAX_NAME_LEN];
   char myFileName[MAX_NAME_LEN];
   int printCount=0;
   int status;


   if (rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "getDataObjPSmeta: input rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
   }

   
   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_DATA_ATTR_NAME;
   i1b[0]=0; /* currently unused */
   i1a[1]=COL_META_DATA_ATTR_VALUE;
   i1b[1]=0; /* currently unused */
   i1a[2]=COL_META_DATA_ATTR_UNITS;
   i1b[2]=0; /* currently unused */
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 3;

   /* Extract cwd name and object name */
   strncpy(fullName, objPath, MAX_NAME_LEN);
   status = splitPathByKey(fullName, myDirName, myFileName, '/');

   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'",myDirName);
   condVal[0]=v1;

   i2a[1]=COL_DATA_NAME;
   sprintf(v2,"='%s'",myFileName);
   condVal[1]=v2;


   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=2;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;


   /* Actual query happens here */
   status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);


   if (status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_D_DATA_PATH;
      genQueryInp.selectInp.len = 1;
      status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (status==0) {
	 /* printf("None\n"); */
	 return(0);
      }
      if (status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
          "getDataObjPSmeta: DataObject %s not found. status = %d", fullName, status);
	return (status);
      }
      printCount+=extractPSQueryResults(rsComm, status, genQueryOut, mybuf, fullName);
   }
   else {
      printCount+=extractPSQueryResults(rsComm, status, genQueryOut, mybuf, fullName);
   }

   while (status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      printCount+= extractPSQueryResults(rsComm, status, genQueryOut, mybuf, fullName);
   }

  return (status);
}



/*
 * Gets pipe separated metadata AVUs for a collection.
 * 
 */
int
getCollectionPSmeta(char *objPath, bytesBuf_t *mybuf, rsComm_t *rsComm)
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int i1a[10];
   int i1b[10];
   int i2a[10];
   char *condVal[10];
   char v1[BIG_STR];
   char v2[BIG_STR];
   int printCount=0;
   int status;


   if (rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "getCollectionPSmeta: input rsComm is NULL");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
   }


   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   i1a[0]=COL_META_COLL_ATTR_NAME;
   i1b[0]=0; /* currently unused */
   i1a[1]=COL_META_COLL_ATTR_VALUE;
   i1b[1]=0;
   i1a[2]=COL_META_COLL_ATTR_UNITS;
   i1b[2]=0;
   genQueryInp.selectInp.inx = i1a;
   genQueryInp.selectInp.value = i1b;
   genQueryInp.selectInp.len = 3;

   i2a[0]=COL_COLL_NAME;
   sprintf(v1,"='%s'", objPath);
   condVal[0]=v1;

   genQueryInp.sqlCondInp.inx = i2a;
   genQueryInp.sqlCondInp.value = condVal;
   genQueryInp.sqlCondInp.len=1;

   genQueryInp.maxRows=10;
   genQueryInp.continueInx=0;
   genQueryInp.condInput.len=0;


   /* Actual query happens here */
   status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);

   if (status == CAT_NO_ROWS_FOUND) {
      i1a[0]=COL_COLL_COMMENTS;
      genQueryInp.selectInp.len = 1;
      status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      if (status==0) {
	 /* printf("None\n"); */
	 return(0);
      }
      if (status == CAT_NO_ROWS_FOUND) {

	rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, status,
          "getCollectionPSmeta: Collection %s not found. status = %d", objPath, status);
	return (status);
      }
   }

   printCount+=extractPSQueryResults(rsComm, status, genQueryOut, mybuf, objPath);

   while (status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      status = rsGenQuery(rsComm, &genQueryInp, &genQueryOut);
      printCount+= extractPSQueryResults(rsComm, status, genQueryOut, mybuf, objPath);
   }

  return (status);

}



/*
 * myLoadMetadataFromFile() - Looks for an .mdf file in iRODS, parses it
 * and adds metadata to files accordingly
 */
int
loadMetadataFromFile(rsComm_t *rsComm, char *myPath)
{
  FILE *fp;
  char str[500];
  char *substring;
  char path[200];
  char attr[100];
  char value[500];
  char unit[100];
  int counter;
  int status;
  int objStatus;
  dataObjInp_t dataObjInp;
  rodsObjStat_t *rodsObjStatOut = NULL;
  modAVUMetadataInp_t modAVUMetadataInp;

  if (rsComm == NULL) {
    rodsLog (LOG_ERROR, "loadMetadataFromFile: Input rsComm is NULL");
    return (SYS_INTERNAL_NULL_INPUT_ERR);
  }

  if ((fp=fopen(myPath, "r")) == NULL) {
    rodsLog (LOG_ERROR, "loadMetadataFromFile: Cannot open local file %s.", myPath);
    return (UNIX_FILE_OPEN_ERR);
  }

  memset (&modAVUMetadataInp, 0, sizeof(modAVUMetadataInp));
  modAVUMetadataInp.arg0 = "add";

  while(!feof(fp)) {
    counter = 0;
    if (fgets(str, 500, fp)) {
      str[ strlen(str) - 1] = '\0';
      substring = strtok(str,"|");
      while(substring != NULL) {
        if (counter == 0) {
          strncpy(path, substring, 200);
        }
        if (counter == 1) {
          strncpy(attr, substring, 100);
        }
        if (counter == 2) {
          strncpy(value, substring, 500);
        }
        if (counter == 3) {
          strncpy(unit, substring, 100);
        }   

        substring = strtok(NULL, "|");
        if (substring == NULL) {
          memset (&dataObjInp, 0, sizeof (dataObjInp));
          rstrcpy (dataObjInp.objPath, path, MAX_NAME_LEN);
          objStatus = rsObjStat (rsComm, &dataObjInp, &rodsObjStatOut);

          if (objStatus) {


           switch (objStatus) {
               case 1:
                   modAVUMetadataInp.arg1 = "-d";
                   break;
               case 2:
                   modAVUMetadataInp.arg1 = "-c";
                   break;
               default:
		   rodsLog (LOG_ERROR, "loadMetadataFromFile error: Unsupported Object Type");
                   return (INVALID_OBJECT_TYPE);
                   break;
           }

            if (counter == 1) {
              /*Call the function to insert metadata here.*/
              modAVUMetadataInp.arg2 = path;
              modAVUMetadataInp.arg3 = attr;
              modAVUMetadataInp.arg4 = "";
              modAVUMetadataInp.arg5 = "";
              status = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
              rodsLog (LOG_DEBUG, "msiLoadMetadataFromFile: %s:%s:%s",attr, "", "");
            }
            if (counter == 2) {
              /*Call the function to insert metadata here.*/
              modAVUMetadataInp.arg2 = path;
              modAVUMetadataInp.arg3 = attr;
              modAVUMetadataInp.arg4 = value;
              modAVUMetadataInp.arg5 = "";
              status = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
              rodsLog (LOG_DEBUG, "msiLoadMetadataFromFile:%s:%s:%s",attr, value, "");   
            }
            if (counter == 3) {
              /*Call the function to insert metadata here.*/
              modAVUMetadataInp.arg2 = path;
              modAVUMetadataInp.arg3 = attr;
              modAVUMetadataInp.arg4 = value;
              modAVUMetadataInp.arg5 = unit;
              status = rsModAVUMetadata (rsComm, &modAVUMetadataInp);
              rodsLog (LOG_DEBUG, "loadMetadataFromFile:%s:%s:%s",attr, value, unit);   
            }
          }
        }
        counter++;
      }
    }
  }
        
  fclose(fp);
  return(0);
}



/*
 * genQueryOutToXML() - Extracts AVU results from genQueryOut_t and writes them
 * to a dynamic buffer in an XML syntax.
 * Array length of argument tags must be equal or greater than genQueryOut->attriCnt + 1.
 * tags[0] is used to separate query hits (row count), then each tag[n+1] will wrap
 * the nth attribute.
 */
int
genQueryOutToXML(rsComm_t *Conn, int status, genQueryOut_t *genQueryOut, bytesBuf_t *mybuf, char **tags)
{
   int printCount;
   int i, j;
   size_t size;


   printCount=0;

   if (status!=0) {
      printError(Conn, status, "rsGenQuery");
   }
   else {
      if (status !=CAT_NO_ROWS_FOUND) {
	 for (i=0;i<genQueryOut->rowCnt;i++) {
	 
	    if ( (tags[0] != NULL) && strlen(tags[0]) ) {
		appendStrToBBuf(mybuf, strlen(tags[0])+4, "<%s>\n", tags[0]);
	    }

	    for (j=0;j<genQueryOut->attriCnt;j++) {
	       char *tResult;
	       tResult = genQueryOut->sqlResult[j].value;
	       tResult += i*genQueryOut->sqlResult[j].len;

	       if ( (tags[j+1] != NULL) && strlen(tags[j+1]) ) {
		  size = genQueryOut->sqlResult[j].len + 2*strlen(tags[j+1]) + 10;
	    	  appendStrToBBuf(mybuf, size, "<%s>%s</%s>\n", tags[j+1], tResult, tags[j+1]);
	       }
	       else {
		  size = genQueryOut->sqlResult[j].len + 1;
	    	  appendStrToBBuf(mybuf, size, "%s\n",tResult);
	       }
	       printCount++;
	    }

	    if ( (tags[0] != NULL) && strlen(tags[0]) ) {
		    appendStrToBBuf(mybuf, strlen(tags[0])+5, "</%s>\n", tags[0]);
	    }

	 }
      }
   }

   return (printCount);
}



/*
 * extractPSQueryResults() - Extracts AVU results from a genQueryOut_t 
 * and writes them to a dynamic buffer in a pipe-separated, one AVU per line format.
 * To be used in msiGetDataObjPSmeta().
 */
int
extractPSQueryResults(rsComm_t *Conn, int status, genQueryOut_t *genQueryOut, bytesBuf_t *mybuf, char *fullName)
{
   int printCount;
   int i, j;
   size_t size;


   printCount=0;

   if (status!=0) {
      printError(Conn, status, "rsGenQuery");
   }
   else {
      if (status !=CAT_NO_ROWS_FOUND) {
	 for (i=0;i<genQueryOut->rowCnt;i++) {
	 
	    appendStrToBBuf(mybuf, strlen(fullName)+1, fullName);

	    for (j=0;j<genQueryOut->attriCnt;j++) {
		char *tResult;
		tResult = genQueryOut->sqlResult[j].value;
		tResult += i*genQueryOut->sqlResult[j].len;
		
		/* skip final | if no units were defined */
		if (j<2 || strlen(tResult)) {
			size = genQueryOut->sqlResult[j].len + 2;
			appendStrToBBuf(mybuf, size, "|%s",tResult);
		}
		
		printCount++;
	    }

	    appendStrToBBuf(mybuf, 2, "\n");

	 }
      }
   }

   return (printCount);
}



