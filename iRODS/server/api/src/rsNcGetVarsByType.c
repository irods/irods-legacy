/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See dataObjGet.h for a description of this API call.*/

#include "ncGetVarsByType.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "objMetaOpr.h"
#include "physPath.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

int
rsNcGetVarsByType (rsComm_t *rsComm, ncGetVarInp_t *ncGetVarInp,
ncGetVarOut_t **ncGetVarOut)
{
    int remoteFlag;
    rodsServerHost_t *rodsServerHost = NULL;
    int l1descInx;
    ncGetVarInp_t myNcGetVarInp;
    int status = 0;

    if (getValByKey (&ncGetVarInp->condInput, NATIVE_NETCDF_CALL_KW) != 
      NULL) {
	/* just do nc_inq_YYYY */
	status = _rsNcGetVarsByType (ncGetVarInp->ncid, ncGetVarInp,
	  ncGetVarOut);
        return status;
    }
    l1descInx = ncGetVarInp->ncid;
    if (l1descInx < 2 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_ERROR,
          "rsNcGetVarsByType: l1descInx %d out of range",
          l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }
    if (L1desc[l1descInx].inuseFlag != FD_INUSE) return BAD_INPUT_DESC_INDEX;
    if (L1desc[l1descInx].remoteZoneHost != NULL) {
	myNcGetVarInp = *ncGetVarInp;
	myNcGetVarInp.ncid = L1desc[l1descInx].remoteL1descInx;

        /* cross zone operation */
	status = rcNcGetVarsByType (L1desc[l1descInx].remoteZoneHost->conn,
	  &myNcGetVarInp, ncGetVarOut);
    } else {
        remoteFlag = resoAndConnHostByDataObjInfo (rsComm,
	  L1desc[l1descInx].dataObjInfo, &rodsServerHost);
        if (remoteFlag < 0) {
            return (remoteFlag);
        } else if (remoteFlag == LOCAL_HOST) {
	    status = _rsNcGetVarsByType (L1desc[l1descInx].l3descInx,  
	      ncGetVarInp, ncGetVarOut);
            if (status < 0) {
                return status;
            }
        } else {
	    /* execute it remotely */
	    myNcGetVarInp = *ncGetVarInp;
	    myNcGetVarInp.ncid = L1desc[l1descInx].l3descInx;
	    addKeyVal (&myNcGetVarInp.condInput, NATIVE_NETCDF_CALL_KW, "");
            status = rcNcGetVarsByType (rodsServerHost->conn, &myNcGetVarInp, 
	      ncGetVarOut);
	    clearKeyVal (&myNcGetVarInp.condInput);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "rsNcGetVarsByType: rcNcGetVarsByType %d for %s error, status = %d",
                  L1desc[l1descInx].l3descInx,
                  L1desc[l1descInx].dataObjInfo->objPath, status);
                return (status);
            }
	}
    }
    return status;
}

int
_rsNcGetVarsByType (int ncid, ncGetVarInp_t *ncGetVarInp,
ncGetVarOut_t **ncGetVarOut)
{
    int status;
    size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
    ptrdiff_t stride[NC_MAX_DIMS];
    int i;
    int len = 1;
    int hasStride = 0;

    if (ncGetVarInp == NULL || ncGetVarOut == NULL) return USER__NULL_INPUT_ERR;

    for (i = 0; i < ncGetVarInp->ndim; i++) {
	start[i] = ncGetVarInp->start[i];
	count[i] = ncGetVarInp->count[i];
	stride[i] = ncGetVarInp->stride[i];
	if (count[i] <= 0) return 0;
	/* cal dataArray->len */
	if (stride[i] <= 0) {
	    stride[i] = 1;
	} else if (stride[i] > 1) {
	    hasStride = 1;
	}
	len = len * ((count[i] - 1) / stride[i] + 1);
    }
    if (len <= 0) return 0;
    *ncGetVarOut = (ncGetVarOut_t *) calloc (1, sizeof (ncGetVarOut_t));
    (*ncGetVarOut)->dataArray = (bytesBuf_t *) calloc (1, sizeof (bytesBuf_t));
    (*ncGetVarOut)->dataArray->len = len;

    switch (ncGetVarInp->dataType) {
      case NC_CHAR:
        (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (char) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "BytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_text (ncid, ncGetVarInp->varid, start, count,
              stride, (char *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_text (ncid, ncGetVarInp->varid, start, count,
              (char *) (*ncGetVarOut)->dataArray->buf);
        }
        break;
      case NC_BYTE:
      case NC_UBYTE:
        (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (char) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "BytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_uchar (ncid, ncGetVarInp->varid, start, count,
              stride, (unsigned char *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_uchar (ncid, ncGetVarInp->varid, start, count,
              (unsigned char *) (*ncGetVarOut)->dataArray->buf);
        }
        break;
#ifdef NETCDF_HDF
      case NC_STRING:
        (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (char *) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "strBytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_string (ncid, ncGetVarInp->varid, start, count,
              stride, (char **) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_string (ncid, ncGetVarInp->varid, start, count,
              (char **) (*ncGetVarOut)->dataArray->buf);
        }
        break;
#endif
      case NC_INT:
       (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (int) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "IntBytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_int (ncid, ncGetVarInp->varid, start, count,
              stride, (int *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_int (ncid, ncGetVarInp->varid, start, count,
              (int *) (*ncGetVarOut)->dataArray->buf);
        }
        break;
      case NC_UINT:
       (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (unsigned int) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "IntBytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_uint (ncid, ncGetVarInp->varid, start, count,
              stride, (unsigned int *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_uint (ncid, ncGetVarInp->varid, start, count,
              (unsigned int *) (*ncGetVarOut)->dataArray->buf);
        }
        break;
      case NC_INT64:
        (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (long long) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "Int64BytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_longlong (ncid, ncGetVarInp->varid, start, 
              count, stride, (long long *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_longlong (ncid, ncGetVarInp->varid, start, 
              count, (long long *) (*ncGetVarOut)->dataArray->buf);
        }     
        break;
      case NC_UINT64:
        (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (unsigned long long) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "Int64BytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_ulonglong (ncid, ncGetVarInp->varid, 
	      start, count, stride, 
	      (unsigned long long *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_ulonglong (ncid, ncGetVarInp->varid, start, 
              count, (unsigned long long *) (*ncGetVarOut)->dataArray->buf);
        }
        break;
      case NC_FLOAT:
	(*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (float) * len);
	rstrcpy ((*ncGetVarOut)->dataType_PI, "IntBytesBuf_PI", NAME_LEN);
	if (hasStride != 0) {
            status = nc_get_vars_float (ncid, ncGetVarInp->varid, start, count,
	      stride, (float *) (*ncGetVarOut)->dataArray->buf);
	} else {
            status = nc_get_vara_float (ncid, ncGetVarInp->varid, start, count,
	      (float *) (*ncGetVarOut)->dataArray->buf);
	}
	break;
      case NC_DOUBLE:
        (*ncGetVarOut)->dataArray->buf = calloc (1, sizeof (double) * len);
        rstrcpy ((*ncGetVarOut)->dataType_PI, "Int64BytesBuf_PI", NAME_LEN);
        if (hasStride != 0) {
            status = nc_get_vars_double (ncid, ncGetVarInp->varid, start, count,
              stride, (double *) (*ncGetVarOut)->dataArray->buf);
        } else {
            status = nc_get_vara_double (ncid, ncGetVarInp->varid, start, count,
              (double *) (*ncGetVarOut)->dataArray->buf);
        }
        break;
      default:
        rodsLog (LOG_ERROR,
          "_rsNcGetVarsByType: Unknow dataType %d", ncGetVarInp->dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }

    if (status != NC_NOERR) {
	freeNcGetVarOut (ncGetVarOut);
        rodsLog (LOG_ERROR,
          "_rsNcGetVarsByType:  nc_get_vars err varid %d dataType %d. %s ", 
	  ncGetVarInp->varid, ncGetVarInp->dataType, nc_strerror(status));
        status = NETCDF_GET_VARS_ERR - status;
    }
    return status;
}
