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

int
prNcHeader (rcComm_t *conn, char *fileName, int ncid, int noattr,
ncInqOut_t *ncInqOut)
{
    int i, j, dimId, status;
    char tempStr[NAME_LEN];
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
    if (noattr == False) {
        for (i = 0; i < ncInqOut->ngatts; i++) {
            bufPtr = ncInqOut->gatt[i].value.dataArray->buf;
            printf ("   %s = \n", ncInqOut->gatt[i].name);
	    if (ncInqOut->gatt[i].dataType == NC_CHAR) {
                /* assume it is a string */
                /* printf ("%s ;\n", (char *) bufPtr); */
                if (printNice ((char *) bufPtr, "      ", 72) < 0) 
                    printf ("     %s", (char *) bufPtr);
            } else {
                ncValueToStr (ncInqOut->gatt[i].dataType, &bufPtr, tempStr);
                if (printNice (tempStr, "      ", 72) < 0)
                    printf ("     %s", (char *) bufPtr);
            }
            printf (";\n");
        }
    }

    /* dimensions */
    if (ncInqOut->ndims <= 0 || ncInqOut->dim == NULL) 
	return USER__NULL_INPUT_ERR;
    printf ("dimensions:\n");
    for (i = 0; i < ncInqOut->ndims; i++) {
	if (ncInqOut->unlimdimid == ncInqOut->dim[i].id) {
	    /* unlimited */
	    printf ("   %s = UNLIMITED ; // (%lld currently)\n", 
	      ncInqOut->dim[i].name, ncInqOut->dim[i].arrayLen);
	} else { 
	    printf ("   %s = %lld ;\n", 
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
        printf ("   %s %s(", tempStr, ncInqOut->var[i].name);
	for (j = 0; j < ncInqOut->var[i].nvdims - 1; j++) {
	    dimId = ncInqOut->var[i].dimId[j];
	    printf ("%s, ",  ncInqOut->dim[dimId].name);
	}
	/* last one */
         dimId = ncInqOut->var[i].dimId[j];
         printf ("%s) ;\n", ncInqOut->dim[dimId].name);
	/* print the attributes */
        if (noattr == False) {
	    for (j = 0; j < ncInqOut->var[i].natts; j++) {
	        ncGenAttOut_t *att = &ncInqOut->var[i].att[j];
                  printf ("     %s:%s =\n",   
	          ncInqOut->var[i].name, att->name);
	        bufPtr = att->value.dataArray->buf;
                if (att->dataType == NC_CHAR) {
                    /* assume it is a string */
                    /* printf ("%s ;\n", (char *) bufPtr); */
                    if (printNice ((char *) bufPtr, "         ", 70) < 0)
                        printf ("     %s", (char *) bufPtr);
                } else {
	            ncValueToStr (att->dataType, &bufPtr, tempStr);
	            /* printf ("%s ;\n", tempStr); */
                    if (printNice (tempStr, "         ", 70) < 0) 
                        printf ("     %s", (char *) bufPtr);
                }
                printf (";\n");
            }
        }
    }
    return 0;
}

int
prNcDimVar (rcComm_t *conn, char *fileName, int ncid, int printAsciTime,
ncInqOut_t *ncInqOut)
{
    int i, j, status = 0;

    /* dimensions */
    if (ncInqOut->ndims <= 0 || ncInqOut->dim == NULL)
        return USER__NULL_INPUT_ERR;
    printf ("dimensions:\n");
    for (i = 0; i < ncInqOut->ndims; i++) {
        if (ncInqOut->unlimdimid == ncInqOut->dim[i].id) {
            /* unlimited */
            printf ("    %s = UNLIMITED ; // (%lld currently)\n",
              ncInqOut->dim[i].name, ncInqOut->dim[i].arrayLen);
        } else {
            printf ("    %s = %lld ;\n",
              ncInqOut->dim[i].name, ncInqOut->dim[i].arrayLen);
        }
        /* get the dim variables */
        for (j = 0; j < ncInqOut->nvars; j++) {
            if (strcmp (ncInqOut->dim[i].name, ncInqOut->var[j].name) == 0) {
                break;
            }
        }
        if (j >= ncInqOut->nvars) {
            /* not found. tabledap allows this */
            continue;
#if 0
            rodsLogError (LOG_ERROR, status,
              "prNcDimVar: unmatched dim var  %s", ncInqOut->dim[i].id);
            return NETCDF_DIM_MISMATCH_ERR;
#endif
        }
        status = prSingleDimVar (conn, ncid, j, 10, printAsciTime, ncInqOut);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "prNcDimVar: prSingleDimVar error for %s",
              ncInqOut->var[j].name);
            return status;
        }
    }
    return status;
}

int
prSingleDimVar (rcComm_t *conn, int ncid, int varInx, 
int itemsPerLine, int printAsciTime, ncInqOut_t *ncInqOut)
{
    int j, status;
    rodsLong_t start[NC_MAX_DIMS], stride[NC_MAX_DIMS], count[NC_MAX_DIMS];
    ncGetVarOut_t *ncGetVarOut = NULL;
    int lastDimLen;
    char tempStr[NAME_LEN];
    void *bufPtr;
    int outCnt = 0;
    int itemsInLine = 0;

    status = getSingleNcVarData (conn, ncid, varInx, ncInqOut, NULL, 
      &ncGetVarOut, start, stride, count);
#if 0
    for (j = 0; j < ncInqOut->var[varInx].nvdims; j++) {
        int dimId = ncInqOut->var[varInx].dimId[j];
        start[j] = 0;
        if (dumpVarLen > 0 && ncInqOut->dim[dimId].arrayLen > dumpVarLen) {
            /* If it is NC_CHAR, it could be a str */
            if (ncInqOut->var[varInx].dataType == NC_CHAR &&
              j ==  ncInqOut->var[varInx].nvdims - 1) {
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
    ncGetVarInp.dataType = ncInqOut->var[varInx].dataType;
    ncGetVarInp.ncid = ncid;
    ncGetVarInp.varid =  ncInqOut->var[varInx].id;
    ncGetVarInp.ndim =  ncInqOut->var[varInx].nvdims;
    ncGetVarInp.start = start;
    ncGetVarInp.count = count;
    ncGetVarInp.stride = stride;

    status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);
#endif

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "dumpNcInqOut: rcNcGetVarsByType error for %s",
          ncInqOut->var[varInx].name);
          return status;
    }
    /* print it */
    lastDimLen = count[ncInqOut->var[varInx].nvdims - 1];
    bufPtr = ncGetVarOut->dataArray->buf;
    bzero (tempStr, sizeof (tempStr));
    if (ncInqOut->var[varInx].dataType == NC_CHAR) {
        int nextLastDimLen;
        if (ncInqOut->var[varInx].nvdims >= 2) {
            nextLastDimLen = count[ncInqOut->var[varInx].nvdims - 2];
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
            ncValueToStr (ncInqOut->var[varInx].dataType, &bufPtr, tempStr);
            outCnt++;
           if (printAsciTime == True && 
              strcasecmp (ncInqOut->var[varInx].name, "time") == 0) {
                /* asci time */
                time_t mytime =atoi (tempStr);
                struct tm *mytm = gmtime (&mytime);
                if (mytm != NULL) {
                    snprintf (tempStr, NAME_LEN, 
                      "%04d-%02d-%02dT%02d:%02d:%02dZ",
                      1900+mytm->tm_year, mytm->tm_mon + 1, mytm->tm_mday,
                      mytm->tm_hour, mytm->tm_min, mytm->tm_sec);
                }
            }
            if (j >= ncGetVarOut->dataArray->len - 1) {
                printf ("%s ;\n", tempStr);
            } else if (itemsPerLine > 0) {
                int numbLine = outCnt / itemsPerLine;
                if (itemsInLine == 0) {
                    printf ("(%d - %d)  ", numbLine * itemsPerLine, 
                      numbLine * itemsPerLine + itemsPerLine -1);
                }
                itemsInLine++;
                if (itemsInLine >= itemsPerLine) {
                    printf ("%s,\n", tempStr);
                    itemsInLine = 0;
                } else {
                    printf ("%s, ", tempStr);
                }
            } else if (outCnt >= lastDimLen) {
                /* reset */
                printf ("%s,\n  ", tempStr);
                outCnt = 0;
            } else {
                printf ("%s, ", tempStr);
            }
        }
    }

    return status;
}

/* dumpVarLen 0 mean dump all. > 0 means dump the specified len */
int
dumpNcInqOut (rcComm_t *conn, char *fileName, int ncid, int dumpVarLen,
ncInqOut_t *ncInqOut)
{
    int status;

    status = prNcHeader (conn, fileName, ncid, False, ncInqOut);
    if (status < 0) return status;

    if (dumpVarLen > 0) {
        int i;
        ncVarSubset_t ncVarSubset;

        bzero (&ncVarSubset, sizeof (ncVarSubset));
        ncVarSubset.numSubset = ncInqOut->ndims;
        for (i = 0; i < ncInqOut->ndims; i++) {
            rstrcpy (ncVarSubset.ncSubset[i].subsetVarName, 
              ncInqOut->dim[i].name, LONG_NAME_LEN);
            ncVarSubset.ncSubset[i].start = 0;
            ncVarSubset.ncSubset[i].stride = 1;
            if (dumpVarLen > ncInqOut->dim[i].arrayLen) {
                ncVarSubset.ncSubset[i].end = ncInqOut->dim[i].arrayLen - 1;
            } else {
                ncVarSubset.ncSubset[i].end = dumpVarLen -1;
            }
        }
        status = prNcVarData (conn, fileName, ncid, False, ncInqOut, &ncVarSubset);
    } else {
        status = prNcVarData (conn, fileName, ncid, False, ncInqOut, NULL);
    }
    return status;
}

int
prNcVarData (rcComm_t *conn, char *fileName, int ncid, int printAsciTime,
ncInqOut_t *ncInqOut, ncVarSubset_t *ncVarSubset)
{
    int i, j, status;
    char tempStr[NAME_LEN];
    rodsLong_t start[NC_MAX_DIMS], stride[NC_MAX_DIMS], count[NC_MAX_DIMS];
    ncGetVarOut_t *ncGetVarOut = NULL;
    void *bufPtr;

    /* data */
    printf ("data:\n\n");
    for (i = 0; i < ncInqOut->nvars; i++) {
        if (ncVarSubset->numVar > 1 || (ncVarSubset->numVar == 1 && 
          strcmp (&ncVarSubset->varName[0][LONG_NAME_LEN], "all") != 0)) {
            /* only do those that match */
            for (j = 0; j < ncVarSubset->numVar; j++) {
                if (strcmp (&ncVarSubset->varName[j][LONG_NAME_LEN], 
                  ncInqOut->var[i].name) == 0) break;
            }
            if (j >= ncVarSubset->numVar) continue;    /* no match */
        }
	printf (" %s = ", ncInqOut->var[i].name);
	if (ncInqOut->var[i].nvdims > 1) printf ("\n  ");
        status = getSingleNcVarData (conn, ncid, i, ncInqOut, ncVarSubset,
          &ncGetVarOut, start, stride, count);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "dumpNcInqOut: rcNcGetVarsByType error for %s", 
	      ncInqOut->var[i].name);
              /* don't exit yet. This could be caused by tabledap not having
               * the variable */
              printf (" ;\n");
              continue;
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
                    if (printAsciTime == True &&
                      strcasecmp (ncInqOut->var[i].name, "time") == 0) { 
                        /* asci time */
                        time_t mytime =atoi (tempStr);
                        struct tm *mytm = gmtime (&mytime);
                        if (mytm != NULL) {
                            snprintf (tempStr, NAME_LEN,
                              "%04d-%02d-%02dT%02d:%02d:%02dZ",
                              1900+mytm->tm_year, mytm->tm_mon + 1, 
                              mytm->tm_mday,
                              mytm->tm_hour, mytm->tm_min, mytm->tm_sec);
                        }
                    }
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
getSingleNcVarData (rcComm_t *conn, int ncid, int varInx, ncInqOut_t *ncInqOut,
ncVarSubset_t *ncVarSubset, ncGetVarOut_t **ncGetVarOut, rodsLong_t *start, 
rodsLong_t *stride, rodsLong_t *count)
{
    int j, k, status;
    ncGetVarInp_t ncGetVarInp;

    for (j = 0; j < ncInqOut->var[varInx].nvdims; j++) {
        int dimId = ncInqOut->var[varInx].dimId[j];
        int doSubset = False;
        if (ncVarSubset != NULL && ncVarSubset->numSubset > 0) {
            for (k = 0; k < ncVarSubset->numSubset; k++) {
                if (strcmp (ncInqOut->dim[dimId].name,
                  ncVarSubset->ncSubset[k].subsetVarName) == 0) {
                    doSubset = True;
                    break;
                }
            }
        }
        if (doSubset == True) {
            if (ncVarSubset->ncSubset[k].start >=
              ncInqOut->dim[dimId].arrayLen ||
              ncVarSubset->ncSubset[k].end >=
              ncInqOut->dim[dimId].arrayLen ||
              ncVarSubset->ncSubset[k].start >
              ncVarSubset->ncSubset[k].end) {
                rodsLog (LOG_ERROR, 
                 "getSingleNcVarData:start %d or end %d for %s outOfRange %lld",
                 ncVarSubset->ncSubset[k].start,
                 ncVarSubset->ncSubset[k].end,
                 ncVarSubset->ncSubset[k].subsetVarName,
                 ncInqOut->dim[dimId].arrayLen);
                return NETCDF_DIM_MISMATCH_ERR;
            }
            start[j] = ncVarSubset->ncSubset[k].start;
            stride[j] = ncVarSubset->ncSubset[k].stride;
            count[j] = ncVarSubset->ncSubset[k].end -
              ncVarSubset->ncSubset[k].start + 1;
        } else {
            start[j] = 0;
            count[j] = ncInqOut->dim[dimId].arrayLen;
            stride[j] = 1;
        }
    }
    bzero (&ncGetVarInp, sizeof (ncGetVarInp));
    ncGetVarInp.dataType = ncInqOut->var[varInx].dataType;
    ncGetVarInp.ncid = ncid;
    ncGetVarInp.varid =  ncInqOut->var[varInx].id;
    ncGetVarInp.ndim =  ncInqOut->var[varInx].nvdims;
    ncGetVarInp.start = start;
    ncGetVarInp.count = count;
    ncGetVarInp.stride = stride;

    status = rcNcGetVarsByType (conn, &ncGetVarInp, ncGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "getSingleNcVarData: rcNcGetVarsByType error for %s",
          ncInqOut->var[varInx].name);
    }
    return status;
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
        case NC_SHORT:
            rstrcpy (outString, "short", NAME_LEN);
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
    short myshort;

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
        case NC_SHORT:
            myshort = *(short int*) value;
	    snprintf (outString, NAME_LEN, "%hi", myshort);
	    *ptr+= sizeof (short);	/* advance pointer */
	    break;
        case NC_USHORT:
            myshort = *(short int*) value;
            snprintf (outString, NAME_LEN, "%hu", myshort);
            *ptr+= sizeof (short);      /* advance pointer */
            break;
	case NC_INT:
	    snprintf (outString, NAME_LEN, "%d", *(int *) value);
	    *ptr+= sizeof (int);	/* advance pointer */
	    break;
	case NC_UINT:
	    snprintf (outString, NAME_LEN, "%u", *(unsigned int *) value);
	    *ptr+= sizeof (int);	/* advance pointer */
	    break;
	case NC_INT64:
	    snprintf (outString, NAME_LEN, "%lld", *(rodsLong_t *) value);
	    *ptr+= sizeof (rodsLong_t);	/* advance pointer */
	    break;
	case NC_UINT64:
	    snprintf (outString, NAME_LEN, "%llu", *(rodsULong_t *) value);
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

#ifdef NETCDF_API
int
dumpNcInqOutToNcFile (rcComm_t *conn, int srcNcid, int noattrFlag,
ncInqOut_t *ncInqOut, char *outFileName)
{
    int i, j, dimId, status;
    int ncid, cmode;
    rodsLong_t start[NC_MAX_DIMS], stride[NC_MAX_DIMS], count[NC_MAX_DIMS];
    size_t lstart[NC_MAX_DIMS], lcount[NC_MAX_DIMS];
    ptrdiff_t lstride[NC_MAX_DIMS];
    ncGetVarInp_t ncGetVarInp;
    ncGetVarOut_t *ncGetVarOut = NULL;
    void *bufPtr;
    int dimIdArray[NC_MAX_DIMS];

    cmode = ncFormatToCmode (ncInqOut->format);
    status = nc_create (outFileName, cmode, &ncid);
    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "dumpNcInqOutToNcFile: nc_create error.  %s ", nc_strerror(status));
        status = NETCDF_CREATE_ERR - status;
        return status;
    }

    /* attrbutes */
    if (noattrFlag == False) {
        for (i = 0; i < ncInqOut->ngatts; i++) {
            bufPtr = ncInqOut->gatt[i].value.dataArray->buf;
            status = nc_put_att (ncid, NC_GLOBAL, ncInqOut->gatt[i].name,
              ncInqOut->gatt[i].dataType, ncInqOut->gatt[i].length, bufPtr);
            if (status != NC_NOERR) {
                rodsLog (LOG_ERROR,
                  "dumpNcInqOutToNcFile: nc_put_att error.  %s ", 
                  nc_strerror(status));
                status = NETCDF_PUT_ATT_ERR - status;
                closeAndRmNeFile (ncid, outFileName);
                return status;
            }
        }
    }
    /* dimensions */
    if (ncInqOut->ndims <= 0 || ncInqOut->dim == NULL)
        return USER__NULL_INPUT_ERR;
    for (i = 0; i < ncInqOut->ndims; i++) {
        if (ncInqOut->unlimdimid == ncInqOut->dim[i].id) {
            /* unlimited */
            status = nc_def_dim (ncid,  ncInqOut->dim[i].name, 
              NC_UNLIMITED, &ncInqOut->dim[i].myint);
        } else {
            status = nc_def_dim (ncid,  ncInqOut->dim[i].name, 
              ncInqOut->dim[i].arrayLen, &ncInqOut->dim[i].myint);
        }
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "dumpNcInqOutToNcFile: nc_def_dim error.  %s ", 
              nc_strerror(status));
            status = NETCDF_DEF_DIM_ERR - status;
            closeAndRmNeFile (ncid, outFileName);
            return status;
        }
    }
    /* variables */
    if (ncInqOut->nvars <= 0 || ncInqOut->var == NULL) {
        /* no variables */
        nc_close (ncid);
        return 0;
    }
    for (i = 0; i < ncInqOut->nvars; i++) {
        /* define the variables */
        for (j = 0; j < ncInqOut->var[i].nvdims;  j++) {
            dimId = ncInqOut->var[i].dimId[j];
            dimIdArray[j] = ncInqOut->dim[dimId].myint;
        }
        status = nc_def_var (ncid, ncInqOut->var[i].name, 
          ncInqOut->var[i].dataType, ncInqOut->var[i].nvdims, 
          dimIdArray, &ncInqOut->var[i].myint);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "dumpNcInqOutToNcFile: nc_def_var for %s error.  %s ",
              ncInqOut->var[i].name, nc_strerror(status));
            status = NETCDF_DEF_VAR_ERR - status;
            closeAndRmNeFile (ncid, outFileName);
            return status;
        }
        /* put the variable attributes */
        if (noattrFlag == False) {
            for (j = 0; j < ncInqOut->var[i].natts; j++) {
                ncGenAttOut_t *att = &ncInqOut->var[i].att[j];
                bufPtr = att->value.dataArray->buf;
                status = nc_put_att (ncid, ncInqOut->var[i].myint, att->name,
                  att->dataType, att->length, bufPtr);
                if (status != NC_NOERR) {
                    rodsLog (LOG_ERROR,
                      "dumpNcInqOutToNcFile: nc_put_att for %s error.  %s ",
                      ncInqOut->var[i].name, nc_strerror(status));
                    status = NETCDF_PUT_ATT_ERR - status;
                    closeAndRmNeFile (ncid, outFileName);
                    return status;
                }
            }
        }
    }
    nc_enddef (ncid);
    for (i = 0; i < ncInqOut->nvars; i++) {
        ncGenVarOut_t *var = &ncInqOut->var[i];
        for (j = 0; j < var->nvdims; j++) {
            dimId = var->dimId[j];
            start[j] = 0;
            lstart[j] = 0;
            count[j] = ncInqOut->dim[dimId].arrayLen;
            lcount[j] = ncInqOut->dim[dimId].arrayLen;
            stride[j] = 1;
            lstride[j] = 1;
        }
        bzero (&ncGetVarInp, sizeof (ncGetVarInp));
        ncGetVarInp.dataType = var->dataType;
        ncGetVarInp.ncid = srcNcid;
        ncGetVarInp.varid =  var->id;
        ncGetVarInp.ndim =  var->nvdims;
        ncGetVarInp.start = start;
        ncGetVarInp.count = count;
        ncGetVarInp.stride = stride;

        if (conn == NULL) {
            /* local call */
            status = _rsNcGetVarsByType (srcNcid, &ncGetVarInp, &ncGetVarOut);
        } else {
            status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);
        }
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "dumpNcInqOutToNcFile: rcNcGetVarsByType error for %s",
              ncInqOut->var[i].name);
            closeAndRmNeFile (ncid, outFileName);
            return status;
        }
        status = nc_put_vars (ncid, var->myint, lstart, 
         lcount, lstride, ncGetVarOut->dataArray->buf);
        freeNcGetVarOut (&ncGetVarOut);
        if (status != NC_NOERR) {
            rodsLogError (LOG_ERROR, status,
              "dumpNcInqOutToNcFile: nc_put_vars error for %s    %s",
              ncInqOut->var[i].name, nc_strerror(status));
            closeAndRmNeFile (ncid, outFileName);
            return NETCDF_PUT_VARS_ERR;
        }
    }
    nc_close (ncid);
    return 0;
}

int
dumpSubsetToFile (rcComm_t *conn, int srcNcid, int noattrFlag,
ncInqOut_t *ncInqOut, ncVarSubset_t *ncVarSubset, char *outFileName)
{
    int i, j, dimId, nvars, status;
    int ncid, cmode;
    rodsLong_t start[NC_MAX_DIMS], stride[NC_MAX_DIMS], count[NC_MAX_DIMS];
    size_t lstart[NC_MAX_DIMS], lcount[NC_MAX_DIMS];
    ptrdiff_t lstride[NC_MAX_DIMS];
    ncGetVarOut_t *ncGetVarOut = NULL;
    void *bufPtr;
    int dimIdArray[NC_MAX_DIMS];
    ncInqOut_t subsetNcInqOut;

    cmode = ncFormatToCmode (ncInqOut->format);
    status = nc_create (outFileName, cmode, &ncid);
    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "dumpSubsetToFile: nc_create error.  %s ", nc_strerror(status));
        status = NETCDF_CREATE_ERR - status;
        return status;
    }
    /* attrbutes */
    if (noattrFlag == False) {
        for (i = 0; i < ncInqOut->ngatts; i++) {
            bufPtr = ncInqOut->gatt[i].value.dataArray->buf;
            status = nc_put_att (ncid, NC_GLOBAL, ncInqOut->gatt[i].name,
              ncInqOut->gatt[i].dataType, ncInqOut->gatt[i].length, bufPtr);
            if (status != NC_NOERR) {
                rodsLog (LOG_ERROR,
                  "dumpSubsetToFile: nc_put_att error.  %s ",
                  nc_strerror(status));
                status = NETCDF_PUT_ATT_ERR - status;
                closeAndRmNeFile (ncid, outFileName);
                return status;
            }
        }
    }
    /* dimensions */
    if (ncInqOut->ndims <= 0 || ncInqOut->dim == NULL)
        return USER__NULL_INPUT_ERR;
    bzero (&subsetNcInqOut, sizeof (subsetNcInqOut));
    subsetNcInqOut.ndims = ncInqOut->ndims;
    subsetNcInqOut.format = ncInqOut->format;
    subsetNcInqOut.unlimdimid = -1;
    subsetNcInqOut.dim = (ncGenDimOut_t *)
      calloc (ncInqOut->ndims, sizeof (ncGenDimOut_t));
    for (i = 0; i < ncInqOut->ndims; i++) {
         rstrcpy (subsetNcInqOut.dim[i].name, ncInqOut->dim[i].name,
           LONG_NAME_LEN);
        /* have to use the full arrayLen instead of subsetted length
         * because it will be used in subsetting later */
        subsetNcInqOut.dim[i].arrayLen = ncInqOut->dim[i].arrayLen;
        if (ncInqOut->unlimdimid == ncInqOut->dim[i].id) {
            /* unlimited */
            status = nc_def_dim (ncid,  ncInqOut->dim[i].name,
              NC_UNLIMITED, &subsetNcInqOut.dim[i].id);
            subsetNcInqOut.unlimdimid = subsetNcInqOut.dim[i].id;
        } else {
            int arrayLen;
            for (j = 0; j < ncVarSubset->numSubset; j++) {
                if (strcmp (ncInqOut->dim[i].name,
                  ncVarSubset->ncSubset[j].subsetVarName) == 0) {
                    arrayLen = (ncVarSubset->ncSubset[j].end -
                     ncVarSubset->ncSubset[j].start) /
                     ncVarSubset->ncSubset[j].stride + 1;
                    break;
                }
            }
            if (j >= ncVarSubset->numSubset) 	/* no match */
                arrayLen = ncInqOut->dim[i].arrayLen;
            status = nc_def_dim (ncid,  ncInqOut->dim[i].name,
              arrayLen, &subsetNcInqOut.dim[i].id);
        }
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "dumpSubsetToFile: nc_def_dim error.  %s ",
              nc_strerror(status));
            status = NETCDF_DEF_DIM_ERR - status;
            closeAndRmNeFile (ncid, outFileName);
            return status;
        }
    }
    /* variables */
    if (ncInqOut->nvars <= 0 || ncInqOut->var == NULL) {
        /* no variables */
        nc_close (ncid);
        return 0;
    }
    /* screen the variables */
    subsetNcInqOut.var = (ncGenVarOut_t *)
      calloc (ncInqOut->nvars, sizeof (ncGenVarOut_t));
    nvars = 0;
    /* For subsequent subsetting and writing vars to a netcdf file,
     * subsetNcInqOut.var[i].id contains the var id of the source and
     * subsetNcInqOut.var[i].myint contains the var id of the target 
     */
    for (i = 0; i < ncInqOut->nvars; i++) {
        if (ncVarSubset->numVar > 1 || (ncVarSubset->numVar == 1 &&
          strcmp ((char *)ncVarSubset->varName, "all") == 0)) {
            /* do all var */
            subsetNcInqOut.var[subsetNcInqOut.nvars] = ncInqOut->var[i];
            subsetNcInqOut.nvars++;
            continue;
        }  
        for (j = 0; j < ncVarSubset->numVar; j++) {
            if (strcmp (&ncVarSubset->varName[j][LONG_NAME_LEN],
              ncInqOut->var[i].name) == 0) {
                subsetNcInqOut.var[subsetNcInqOut.nvars] = 
                  ncInqOut->var[i];
                subsetNcInqOut.nvars++;
                break;
            }
        }
    }

    for (i = 0; i < subsetNcInqOut.nvars; i++) {
        /* define the variables */
        for (j = 0; j < subsetNcInqOut.var[i].nvdims;  j++) {
            dimId = subsetNcInqOut.var[i].dimId[j];
            dimIdArray[j] = subsetNcInqOut.dim[dimId].id;
        }
        status = nc_def_var (ncid, subsetNcInqOut.var[i].name,
          subsetNcInqOut.var[i].dataType, subsetNcInqOut.var[i].nvdims,
          dimIdArray, &subsetNcInqOut.var[i].myint);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "dumpSubsetToFile: nc_def_var for %s error.  %s ",
              subsetNcInqOut.var[i].name, nc_strerror(status));
            status = NETCDF_DEF_VAR_ERR - status;
            closeAndRmNeFile (ncid, outFileName);
            return status;
        }
        /* put the variable attributes */
        if (noattrFlag == False) {
            for (j = 0; j < subsetNcInqOut.var[i].natts; j++) {
                ncGenAttOut_t *att = &subsetNcInqOut.var[i].att[j];
                bufPtr = att->value.dataArray->buf;
                status = nc_put_att (ncid, subsetNcInqOut.var[i].myint, att->name,
                  att->dataType, att->length, bufPtr);
                if (status != NC_NOERR) {
                    rodsLog (LOG_ERROR,
                      "dumpSubsetToFile: nc_put_att for %s error.  %s ",
                      subsetNcInqOut.var[i].name, nc_strerror(status));
                    status = NETCDF_PUT_ATT_ERR - status;
                    closeAndRmNeFile (ncid, outFileName);
                    return status;
                }
            }
        }
    }
    nc_enddef (ncid);

    for (i = 0; i < subsetNcInqOut.nvars; i++) {
        status = getSingleNcVarData (conn, srcNcid, i, &subsetNcInqOut,
          ncVarSubset, &ncGetVarOut, start, stride, count);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "dumpSubsetToFile: rcNcGetVarsByType error for %s",
              subsetNcInqOut.var[i].name);
            closeAndRmNeFile (ncid, outFileName);
            return status;
        }
        for (j = 0; j < subsetNcInqOut.var[i].nvdims; j++) {
            lstart[j] = (size_t) 0;
            lstride[j] = (ptrdiff_t) 1;
            lcount[j] = (size_t) count[j];
        }
        status = nc_put_vars (ncid, subsetNcInqOut.var[i].myint, lstart,
         lcount, lstride, ncGetVarOut->dataArray->buf);
        freeNcGetVarOut (&ncGetVarOut);
        if (status != NC_NOERR) {
            rodsLogError (LOG_ERROR, status,
              "dumpSubsetToFile: nc_put_vars error for %s    %s",
              subsetNcInqOut.var[i].name, nc_strerror(status));
            closeAndRmNeFile (ncid, outFileName);
            return NETCDF_PUT_VARS_ERR;
        }
    }
    if (subsetNcInqOut.dim != NULL) free (subsetNcInqOut.dim);
    if (subsetNcInqOut.var != NULL) free (subsetNcInqOut.var);
    nc_close (ncid);
    return 0;
}
#endif

int
ncFormatToCmode (int format)
{
    int cmode;

    switch (format) {
      case NC_FORMAT_CLASSIC:
        cmode = NC_CLASSIC_MODEL;
        break;
      case NC_FORMAT_64BIT:
        cmode = NC_64BIT_OFFSET;
        break;
      case NC_FORMAT_NETCDF4: 
        cmode = NC_NETCDF4;
        break;
      case NC_FORMAT_NETCDF4_CLASSIC:
        cmode = NC_NETCDF4|NC_CLASSIC_MODEL;
        break;
      default:
        rodsLog (LOG_ERROR,
          "ncFormatToCmode: Unknow format %d, use NC_CLASSIC_MODEL", format);
        cmode = NC_CLASSIC_MODEL;
    }
    return cmode;
}

#ifdef NETCDF_API
int
closeAndRmNeFile (int ncid, char *outFileName)
{
    nc_close (ncid);
    unlink (outFileName);
    return 0;
}
#endif

int
printNice (char *str, char *margin, int charPerLine)
{
    char tmpStr[META_STR_LEN];
    char *tmpPtr = tmpStr;
    int len = strlen (str);
    int c;

    if (len > META_STR_LEN) return USER_STRLEN_TOOLONG;

    rstrcpy (tmpStr, str, META_STR_LEN);
    while (len > 0) {
        if (len > charPerLine) {
            char *tmpPtr1 = tmpPtr;
            c = *(tmpPtr + charPerLine);
            *(tmpPtr + charPerLine) = '\0';	
            /* take out any \n */
            while (*tmpPtr1 != '\0') {
                /* if (*tmpPtr1 == '\n') {  */
                if (isspace (*tmpPtr1)) {
                    *tmpPtr1 = ' ';
                }
                tmpPtr1++;
            }
            printf ("%s%s\n", margin, tmpPtr);
            *(tmpPtr + charPerLine) = c;
            tmpPtr += charPerLine;
            len -= charPerLine;
        } else {
            printf ("%s%s", margin, tmpPtr);
            break;
        }
    }
    return 0;
}

/* parseNcVarSubset - ncVarSubset->subsetVarName is expected to be in the
 * form varName[start:stride:end]. Parse for integer value of start, stride
 * and end.
 */
int
parseNcSubset (ncSubset_t *ncSubset)
{
    char *endPtr, *tmpPtr1, *tmpPtr2;

    if ((endPtr = strchr (ncSubset->subsetVarName, '[')) == NULL) {
        rodsLog (LOG_ERROR,
          "parseNcSubset: subset input %s format error", 
          ncSubset->subsetVarName);
        return USER_INPUT_FORMAT_ERR;
    }
 
    tmpPtr1 = endPtr + 1;
    if ((tmpPtr2 = strchr (tmpPtr1, ':')) == NULL || !isdigit (*tmpPtr1)) {
        rodsLog (LOG_ERROR,
          "parseNcSubset: subset input %s format error", 
          ncSubset->subsetVarName);
        return USER_INPUT_FORMAT_ERR;
    }
    *tmpPtr2 = '\0';
    ncSubset->start = atoi (tmpPtr1);
    *tmpPtr2 = ':';
    tmpPtr1 = tmpPtr2 + 1;
    if ((tmpPtr2 = strchr (tmpPtr1, ':')) == NULL || !isdigit (*tmpPtr1)) {
        rodsLog (LOG_ERROR,
          "parseNcSubset: subset input %s format error",   
          ncSubset->subsetVarName);
        return USER_INPUT_FORMAT_ERR;
    }
    *tmpPtr2 = '\0';
    ncSubset->stride = atoi (tmpPtr1);
    *tmpPtr2 = ':';
    tmpPtr1 = tmpPtr2 + 1;
    if ((tmpPtr2 = strchr (tmpPtr1, ']')) == NULL || !isdigit (*tmpPtr1)) {
        rodsLog (LOG_ERROR,
          "parseNcSubset: subset input %s format error",
          ncSubset->subsetVarName);
        return USER_INPUT_FORMAT_ERR;
    }
    *tmpPtr2 = '\0';
    ncSubset->end = atoi (tmpPtr1);
    *endPtr = '\0'; 	/* truncate the name */
    return 0;
}

