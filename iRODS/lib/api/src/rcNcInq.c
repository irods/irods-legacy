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
	if (ncInqOut->gatt[i].dataType == NC_CHAR) {
            /* assume it is a string */
            printf ("%s ;\n", (char *) bufPtr);
        } else {
            ncValueToStr (ncInqOut->gatt[i].dataType, &bufPtr, tempStr);
            printf ("%s ;\n", tempStr);
        }
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
            if (att->dataType == NC_CHAR) {
                /* assume it is a string */
                printf ("%s ;\n", (char *) bufPtr);
            } else {
	        ncValueToStr (att->dataType, &bufPtr, tempStr);
	        printf ("%s ;\n", tempStr);
            }
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
                  j ==  ncInqOut->var[i].nvdims - 1) {
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

int
dumpNcInqOutToNcFile (rcComm_t *conn, int srcNcid, ncInqOut_t *ncInqOut, 
char *outFileName)
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
        return 0;;
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

        status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);

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

int
closeAndRmNeFile (int ncid, char *outFileName)
{
    nc_close (ncid);
    unlink (outFileName);
    return 0;
}

