/* This is script-generated code.  */ 
/* See ncInq.h for a description of this API call.*/

#include "ncInq.h"

int
rcNcInq (rcComm_t *conn, ncInqInp_t *ncInqInp, ncInqOut_t **ncInqOut)
{
    int status;
    status = procApiRequest (conn, NC_INQ_AN, ncInqInp, NULL, 
        (void **) ncInqOut, NULL);

    return (status);
}

int
initNcInqOut (int ndims, int nvars, int ngatts, int unlimdimid, int format,
ncInqOut_t **ncInqOut)
{
    *ncInqOut = (ncInqOut_t *) calloc (1, sizeof (ncInqOut_t));
    (*ncInqOut)->ndims = ndims;
    (*ncInqOut)->nvars = nvars;
    (*ncInqOut)->ngatts = ngatts;
    (*ncInqOut)->unlimdimid = unlimdimid;
    (*ncInqOut)->format = format;

    if (ndims > 0)
        (*ncInqOut)->dim = (ncGenDimOut_t *) 
	  calloc (ndims, sizeof (ncGenDimOut_t));
    if (nvars > 0) 
        (*ncInqOut)->var = (ncGenVarOut_t *) 
	  calloc (nvars, sizeof (ncGenVarOut_t));
    if (ngatts > 0)
        (*ncInqOut)->gatt = (ncGenAttOut_t *) 
	  calloc (ngatts, sizeof (ncGenAttOut_t));

    return 0;
}

int
freeNcInqOut (ncInqOut_t **ncInqOut)
{
    int i;

    if (ncInqOut == NULL || *ncInqOut == NULL) return USER__NULL_INPUT_ERR;

    if ((*ncInqOut)->dim != NULL) free ((*ncInqOut)->dim);
    if ((*ncInqOut)->gatt != NULL) free ((*ncInqOut)->gatt);
    if ((*ncInqOut)->var != NULL) {
	for (i = 0; i < (*ncInqOut)->nvars; i++) {
	    if ((*ncInqOut)->var[i].att != NULL) {
#if 0	/* this can cause core dump */
		clearNcGetVarOut (&(*ncInqOut)->var[i].att->value);
		free ((*ncInqOut)->var[i].att);
#endif
	    }
	    if ((*ncInqOut)->var[i].dimId != NULL) 
	        free ((*ncInqOut)->var[i].dimId);
	}
	free ((*ncInqOut)->var);
    }
    free (*ncInqOut);
    *ncInqOut = NULL;

    return 0;
}

/* dumpVarLen 0 mean dump all. > 0 means dump the specified len */
int
dumpNcInqOut (rcComm_t *conn, char *fileName, int ncid, int dumpVarLen, 
ncInqOut_t *ncInqOut)
{
    int i, j, dimId, status;
    char tempStr[NAME_LEN];
    rodsLong_t start[NC_MAX_DIMS], stride[NC_MAX_DIMS], count[NC_MAX_DIMS];
    ncGetVarInp_t ncGetVarInp;
    ncGetVarOut_t *ncGetVarOut = NULL;
    void *bufPtr;
    char myDir[MAX_NAME_LEN], myFile[MAX_NAME_LEN];

    if (fileName == NULL || splitPathByKey (fileName, myDir, myFile, '/') < 0) {
	printf ("netcdf UNKNOWN_FILE {\n");
    } else {
	int len = strlen (myFile);
	char *myptr = myFile + len - 3;
	if (strcmp (myptr, ".nc") == 0) *myptr = '\0'; 
	printf ("netcdf %s {\n", myFile);
    }

    /* attrbutes */
    for (i = 0; i < ncInqOut->ngatts; i++) {
	bufPtr = ncInqOut->gatt[i].value.dataArray->buf;
        printf ("%s = ", ncInqOut->gatt[i].name);
        ncValueToStr (ncInqOut->gatt[i].dataType, &bufPtr, tempStr);
        printf ("%s ;\n", tempStr);
    }

    /* dimensions */
    if (ncInqOut->ndims <= 0 || ncInqOut->dim == NULL) 
	return USER__NULL_INPUT_ERR;
    printf ("dimensions:\n");
    for (i = 0; i < ncInqOut->ndims; i++) {
	if (ncInqOut->unlimdimid == ncInqOut->dim[i].id) {
	    /* unlimited */
	    printf ("        %s = UNLIMITED ; // (%lld currently)\n", 
	      ncInqOut->dim[i].name, ncInqOut->dim[i].arrayLen);
	} else { 
	    printf ("        %s = %lld ;\n", 
	      ncInqOut->dim[i].name, ncInqOut->dim[i].arrayLen);
	}
    }
    /* variables */
    if (ncInqOut->nvars <= 0 || ncInqOut->var == NULL)
        return USER__NULL_INPUT_ERR;
    printf ("variables:\n");
    for (i = 0; i < ncInqOut->nvars; i++) {
	status = getNcTypeStr (ncInqOut->var[i].dataType, tempStr);
	if (status < 0) return status;
        printf ("        %s %s(", tempStr, ncInqOut->var[i].name);
	for (j = 0; j < ncInqOut->var[i].nvdims - 1; j++) {
	    dimId = ncInqOut->var[i].dimId[j];
	    printf ("%s, ",  ncInqOut->dim[dimId].name);
	}
	/* last one */
         dimId = ncInqOut->var[i].dimId[j];
         printf ("%s) ;\n", ncInqOut->dim[dimId].name);
	/* print the attributes */
	for (j = 0; j < ncInqOut->var[i].natts; j++) {
	    ncGenAttOut_t *att = &ncInqOut->var[i].att[j];
            printf ("                 %s:%s = ",   
	      ncInqOut->var[i].name, att->name);
	    bufPtr = att->value.dataArray->buf;
	    ncValueToStr (att->dataType, &bufPtr, tempStr);
	    printf ("%s ;\n", tempStr);
        }
    }

    /* data */
    printf ("data:\n\n");
    for (i = 0; i < ncInqOut->nvars; i++) {
	printf (" %s = ", ncInqOut->var[i].name);
	if (ncInqOut->var[i].nvdims > 1) printf ("\n  ");
	for (j = 0; j < ncInqOut->var[i].nvdims; j++) {
	    int dimId = ncInqOut->var[i].dimId[j];
	    start[j] = 0;
            if (dumpVarLen > 0 && ncInqOut->dim[dimId].arrayLen > dumpVarLen) {
                /* If it is NC_CHAR, it could be a str */
                if (ncInqOut->var[i].dataType == NC_CHAR && 
                  j == ncInqOut->nvars -1) {
	            count[j] = ncInqOut->dim[dimId].arrayLen;
                } else {
                    count[j] = dumpVarLen;
                }
            } else {
	        count[j] = ncInqOut->dim[dimId].arrayLen;
            }
	    stride[j] = 1;
	}
        bzero (&ncGetVarInp, sizeof (ncGetVarInp));
        ncGetVarInp.dataType = ncInqOut->var[i].dataType;
        ncGetVarInp.ncid = ncid;
        ncGetVarInp.varid =  ncInqOut->var[i].id;
        ncGetVarInp.ndim =  ncInqOut->var[i].nvdims;
        ncGetVarInp.start = start;
        ncGetVarInp.count = count;
        ncGetVarInp.stride = stride;

        status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);

        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "dumpNcInqOut: rcNcGetVarsByType error for %s", 
	      ncInqOut->var[i].name);
            return status;
        } else {
	    /* print it */
	    int outCnt = 0;
	    int lastDimLen = count[ncInqOut->var[i].nvdims - 1];
	    bufPtr = ncGetVarOut->dataArray->buf;
            bzero (tempStr, sizeof (tempStr));
            if (ncInqOut->var[i].dataType == NC_CHAR) {
                int nextLastDimLen;
                if (ncInqOut->var[i].nvdims >= 2) {
                    nextLastDimLen = count[ncInqOut->var[i].nvdims - 2];
                } else {
                    nextLastDimLen = 0;
                }
	        for (j = 0; j < ncGetVarOut->dataArray->len; j+=lastDimLen) {
                    /* treat it as strings */
                    if (j + lastDimLen >= ncGetVarOut->dataArray->len - 1) {
                        printf ("%s ;\n", (char *) bufPtr);
                    } else if (outCnt >= nextLastDimLen) {
                        /* reset */
                        printf ("%s,\n  ", (char *) bufPtr);
                        outCnt = 0;
                    } else {
                        printf ("%s, ", (char *) bufPtr);
                    }
                }
            } else {
                for (j = 0; j < ncGetVarOut->dataArray->len; j++) {
                    ncValueToStr (ncInqOut->var[i].dataType, &bufPtr, tempStr);
		    outCnt++;
		    if (j >= ncGetVarOut->dataArray->len - 1) {
		        printf ("%s ;\n", tempStr);
		    } else if (outCnt >= lastDimLen) {
		        /* reset */
                        printf ("%s,\n  ", tempStr);
		        outCnt = 0;
		    } else {
                        printf ("%s, ", tempStr);
		    }
	        }
            }
	    freeNcGetVarOut (&ncGetVarOut);
	}
    }
    printf ("}\n");
    return 0;
}

int
getNcTypeStr (int dataType, char *outString)
{
    switch (dataType) {
	case NC_CHAR:
	    rstrcpy (outString, "char", NAME_LEN);
	    break;
	case NC_BYTE:
	    rstrcpy (outString, "byte", NAME_LEN);
	    break;
	case NC_UBYTE:
	    rstrcpy (outString, "ubyte", NAME_LEN);
	    break;
	case NC_STRING:
	    rstrcpy (outString, "string", NAME_LEN);
	    break;
	case NC_INT:
	    rstrcpy (outString, "int", NAME_LEN);
	    break;
	case NC_UINT:
	    rstrcpy (outString, "uint", NAME_LEN);
	    break;
	case NC_INT64:
	    rstrcpy (outString, "longlong", NAME_LEN);
	    break;
	case NC_UINT64:
	    rstrcpy (outString, "ulonglong", NAME_LEN);
	    break;
	case NC_FLOAT:
	    rstrcpy (outString, "float", NAME_LEN);
	    break;
	case NC_DOUBLE:
	    rstrcpy (outString, "double", NAME_LEN);
	    break;
      default:
        rodsLog (LOG_ERROR,
          "getNcTypeStr: Unknow dataType %d", dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }
    return 0;
}

int
ncValueToStr (int dataType, void **invalue, char *outString)
{
    void *value = *invalue;
    char **ptr = (char **) invalue;

    switch (dataType) {
	case NC_CHAR:
            *outString = *(char *) value;
             *ptr+= sizeof (char);
            break;
	case NC_STRING:
	    snprintf (outString, NAME_LEN, "\"%s\"", (char *) value);
	    break;
        case NC_BYTE:
        case NC_UBYTE:
	    snprintf (outString, NAME_LEN, "%x", *(*ptr));
	    *ptr+= sizeof (char);	/* advance pointer */
	    break;
	case NC_INT:
	    snprintf (outString, NAME_LEN, "%d", *(int *) value);
	    *ptr+= sizeof (int);	/* advance pointer */
	    break;
	case NC_UINT:
	    snprintf (outString, NAME_LEN, "%d", *(unsigned int *) value);
	    *ptr+= sizeof (int);	/* advance pointer */
	    break;
	case NC_INT64:
	    snprintf (outString, NAME_LEN, "%lld", *(rodsLong_t *) value);
	    *ptr+= sizeof (rodsLong_t);	/* advance pointer */
	    break;
	case NC_UINT64:
	    snprintf (outString, NAME_LEN, "%lld", *(rodsULong_t *) value);
	    *ptr+= sizeof (rodsULong_t);	/* advance pointer */
	    break;
	case NC_FLOAT:
	    snprintf (outString, NAME_LEN, "%.2f", *(float *) value);
	    *ptr+= sizeof (float);	/* advance pointer */
	    break;
	case NC_DOUBLE:
	    snprintf (outString, NAME_LEN, "%lf", *(double *) value);
	    *ptr+= sizeof (double);	/* advance pointer */
	    break;
      default:
        rodsLog (LOG_ERROR,
          "getNcTypeStr: Unknow dataType %d", dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }
    return 0;
}
