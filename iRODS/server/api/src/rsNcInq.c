/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ncInq.h for a description of this API call.*/
#include "ncInq.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"


int
rsNcInq (rsComm_t *rsComm, ncInqIdInp_t *ncInqInp, ncInqOut_t **ncInqOut)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost = NULL;
    int l1descInx;
    ncInqIdInp_t mysNcInqInp;
    int status = 0;

    if (getValByKey (&ncInqInp->condInput, NATIVE_NETCDF_CALL_KW) !=
      NULL) {
        /* just do nc_inq */
        status = _rsNcInq (rsComm, ncInqInp->ncid, ncInqOut);
        return status;
    }
    l1descInx = ncInqInp->ncid;
    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_ERROR,
          "rsNcInq: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    if (L1desc[l1descInx].inuseFlag != FD_INUSE) return BAD_INPUT_DESC_INDEX;
    if (L1desc[l1descInx].remoteZoneHost != NULL) {
        bzero (&mysNcInqInp, sizeof (mysNcInqInp));
        mysNcInqInp.ncid = L1desc[l1descInx].remoteL1descInx;

        /* cross zone operation */
        status = rcNcInq (L1desc[l1descInx].remoteZoneHost->conn,
          &mysNcInqInp, ncInqOut);
    } else {
        remoteFlag = resoAndConnHostByDataObjInfo (rsComm,
          L1desc[l1descInx].dataObjInfo, &rodsServerHost);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
            status = _rsNcInq (rsComm, L1desc[l1descInx].l3descInx, 
	      ncInqOut);
            if (status < 0) {
                return status;
            }
        } else {
            /* execute it remotely */
            bzero (&mysNcInqInp, sizeof (mysNcInqInp));
            mysNcInqInp.ncid = L1desc[l1descInx].l3descInx;
            addKeyVal (&mysNcInqInp.condInput, NATIVE_NETCDF_CALL_KW, "");
            status = rcNcInq (rodsServerHost->conn, &mysNcInqInp,
              ncInqOut);
            clearKeyVal (&mysNcInqInp.condInput);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsNcInq: rcsNcInq %d for %s error, status = %d",
                  L1desc[l1descInx].l3descInx,
                  L1desc[l1descInx].dataObjInfo->objPath, status);
                return (status);
            }
        }
    }
    return status;
}

int
_rsNcInq (rsComm_t *rsComm, int ncid, ncInqOut_t **ncInqOut)
{
    int ndims, nvars, ngatts, unlimdimid, format;
    int status, i;
    ncGenDimOut_t *dim;
    ncGenVarOut_t *var;
    ncGenAttOut_t *gatt;
    size_t mylong = 0;
    int intArray[NC_MAX_VAR_DIMS];
    int myndim;

    *ncInqOut = NULL;
    status = nc_inq (ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "_rsNcInq: nc_inq error.  %s ", nc_strerror(status));
        status = NETCDF_INQ_ERR - status;
	return status;
    }

    status = nc_inq_format (ncid, &format);
    if (status != NC_NOERR) {
        rodsLog (LOG_ERROR,
          "_rsNcInq: nc_inq_format error.  %s ", nc_strerror(status));
        status = NETCDF_INQ_FORMAT_ERR - status;
        return status;
    }
    initNcInqOut (ndims, nvars, ngatts, unlimdimid, format, ncInqOut);

    /* inq dimension */
    dim = (*ncInqOut)->dim;
    for (i = 0; i < ndims; i++) {
	status = nc_inq_dim (ncid, i, dim[i].name, &mylong);
        if (status == NC_NOERR) {
	    dim[i].id = i;
	     dim[i].arrayLen = mylong;
        } else {
            rodsLog (LOG_ERROR,
              "_rsNcInq: nc_inq_dim error.  %s ", nc_strerror(status));
            status = NETCDF_INQ_DIM_ERR - status;
	    freeNcInqOut (ncInqOut);
            return status;
	}
    }

    /* inq variables */
    var = (*ncInqOut)->var;
    for (i = 0; i < nvars; i++) {
        var[i].id = i;
        status = nc_inq_var (ncid, i, var[i].name, &var[i].dataType, &myndim, 
          intArray, &var[i].natts);
        if (status == NC_NOERR) {
	    /* fill in att */
	    if (var[i].natts > 0) {
	        status = inqAtt (ncid, i, var[i].natts, &var[i].att);
	        if (status < 0) {
                    freeNcInqOut (ncInqOut);
                    return status;
		}
	    }
        } else {
            rodsLog (LOG_ERROR,
              "_rsNcInq: nc_inq_var error.  %s ", nc_strerror(status));
            status = NETCDF_INQ_VARS_ERR - status;
            freeNcInqOut (ncInqOut);
            return status;
        }
    }

    /* inq attributes */
    gatt = (*ncInqOut)->gatt;
    status = inqAtt (ncid, NC_GLOBAL, ngatts, &gatt);

    return status;
}

int
inqAtt (int ncid, int varid, int natt, ncGenAttOut_t **attOut)
{
    int status, i;
    ncGenAttOut_t *myAttOut;
    nc_type dataType;
    size_t length;

    if (attOut == NULL) return USER__NULL_INPUT_ERR;

    if (natt <= 0) return 0;

    *attOut = NULL;
    myAttOut = (ncGenAttOut_t *) calloc (natt, sizeof (ncGenAttOut_t));

    for (i = 0; i < natt; i++) {
	status = nc_inq_attname (ncid, varid, i, myAttOut[i].name);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "inqAtt: nc_inq_attname error for ncid %d, varid %d, %s", 
	      ncid, varid, nc_strerror(status));
            status = NETCDF_INQ_ATT_ERR - status;
	    free (myAttOut);
            return status;
        }
	status = nc_inq_att (ncid, varid, myAttOut[i].name, &dataType, &length);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "inqAtt: nc_inq_att error for ncid %d, varid %d, %s", 
              ncid, varid, nc_strerror(status));
            status = NETCDF_INQ_ATT_ERR - status;
            free (myAttOut);
            return status;
        }   
	status = getAttValue (ncid, varid, myAttOut[i].name, dataType, length,
	  &myAttOut[i].value);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "inqAtt: getAttValue error for ncid %d, varid %d", ncid, varid);
            free (myAttOut);
            return status;
        }  
	myAttOut[i].dataType = dataType;
	myAttOut[i].length = length;
	myAttOut[i].id = i;
    }
    *attOut = myAttOut;
    
    return 0;
}

int
getAttValue (int ncid, int varid, char *name, int dataType, int length,
ncGetVarOut_t *value)
{
    int status;

    (value)->dataArray = (dataArray_t *) calloc (1, sizeof (dataArray_t));
    (value)->dataArray->len = length;
    (value)->dataArray->type = dataType;
    switch (dataType) {
      case NC_CHAR:
        value->dataArray->buf = calloc (length + 1, sizeof (char));
        rstrcpy (value->dataType_PI, "charDataArray_PI", NAME_LEN);
        status = nc_get_att_text (ncid, varid, name,
          (char *) (value)->dataArray->buf);
	(value)->dataArray->len = length + 1;
        break;
      case NC_BYTE:
      case NC_UBYTE:
        value->dataArray->buf = calloc (length, sizeof (char));
        rstrcpy (value->dataType_PI, "charDataArray_PI", NAME_LEN);
        status = nc_get_att_uchar (ncid, varid, name,
          (unsigned char *) (value)->dataArray->buf);
        break;
#ifdef NETCDF_HDF
      case NC_STRING:
        (value)->dataArray->buf = calloc (length + 1, sizeof (char *));
        rstrcpy ((value)->dataType_PI, "strDataArray_PI", NAME_LEN);
        status = nc_get_att_string (ncid, varid, name,
          (char **) (value)->dataArray->buf);
        break;
#endif
      case NC_INT:
       (value)->dataArray->buf = calloc (length, sizeof (int));
        rstrcpy ((value)->dataType_PI, "intDataArray_PI", NAME_LEN);
        status = nc_get_att_int (ncid, varid, name,
          (int *) (value)->dataArray->buf);
        break;
      case NC_UINT:
       (value)->dataArray->buf = calloc (length, sizeof (unsigned int));
        rstrcpy ((value)->dataType_PI, "intDataArray_PI", NAME_LEN);
        status = nc_get_att_uint (ncid, varid, name,
          (unsigned int *) (value)->dataArray->buf);
        break;
      case NC_INT64:
        (value)->dataArray->buf = calloc (length, sizeof (long long));
        rstrcpy ((value)->dataType_PI, "int64DataArray_PI", NAME_LEN);
        status = nc_get_att_longlong (ncid, varid, name,
          (long long *) (value)->dataArray->buf);
        break;
      case NC_UINT64:
        (value)->dataArray->buf = calloc (length, sizeof (unsigned long long));
        rstrcpy ((value)->dataType_PI, "int64DataArray_PI", NAME_LEN);
        status = nc_get_att_ulonglong (ncid, varid, name,
          (unsigned long long *) (value)->dataArray->buf);
        break;
      case NC_FLOAT:
        (value)->dataArray->buf = calloc (length, sizeof (float));
        rstrcpy ((value)->dataType_PI, "intDataArray_PI", NAME_LEN);
        status = nc_get_att_float (ncid, varid, name,
          (float *) (value)->dataArray->buf);
        break;
      case NC_DOUBLE:
        (value)->dataArray->buf = calloc (length, sizeof (double));
        rstrcpy ((value)->dataType_PI, "int64DataArray_PI", NAME_LEN);
        status = nc_get_att_double (ncid, varid, name,
          (double *) (value)->dataArray->buf);
        break;
      default:
        rodsLog (LOG_ERROR,
          "getAttValue: Unknow dataType %d", dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }

    if (status != NC_NOERR) {
        clearNcGetVarOut (value);
        rodsLog (LOG_ERROR,
          "getAttValue:  nc_get_att err varid %d dataType %d. %s ",
          varid, dataType, nc_strerror(status));
        status = NETCDF_GET_ATT_ERR - status;
    }
    return status;
}

