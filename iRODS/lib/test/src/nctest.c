/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* nctest.c - test the high level api */

#include "rodsClient.h" 

#define TEST_PATH1 "/oneZone/home/rods/netcdf/sfc_pres_temp.nc"
#define TEST_PATH2 "/oneZone/home/rods/netcdf/coads_climatology.nc"

int
myInqVar (rcComm_t *conn, int ncid, char *name, int *dataType, int *ndim);
int
main(int argc, char **argv)
{
    rcComm_t *conn;
    rodsEnv myRodsEnv;
    int status, i;
    rErrMsg_t errMsg;
    ncOpenInp_t ncOpenInp;
    ncCloseInp_t ncCloseInp;
    ncInqIdInp_t ncInqIdInp;
    ncInqWithIdOut_t *ncInqWithIdOut = NULL;
    ncGetVarInp_t ncGetVarInp;
    ncGetVarOut_t *ncGetVarOut = NULL;
    int ncid1 = 0;
    int *londimid1 = NULL;
    int londimlen1 = 0;
    int lontype1 = 0;
    int *latdimid1 = NULL;
    int latdimlen1 = 0;
    int lattype1 = 0;
    int lonvarid1;
    int latvarid1;
    int tempvarid1;
    int temptype1 = 0;
    int presvarid1;
    int prestype1 = 0;
    int lonndim, latndim, tempndim, presndim;
    rodsLong_t start[NC_MAX_DIMS], stride[NC_MAX_DIMS], count[NC_MAX_DIMS];
#ifdef LIB_CF
    nccfGetVarInp_t nccfGetVarInp;
    nccfGetVarOut_t *nccfGetVarOut = NULL;
    float *mydata;
#endif

    status = getRodsEnv (&myRodsEnv);

    if (status < 0) {
	fprintf (stderr, "getRodsEnv error, status = %d\n", status);
	exit (1);
    }


    conn = rcConnect (myRodsEnv.rodsHost, myRodsEnv.rodsPort, 
      myRodsEnv.rodsUserName, myRodsEnv.rodsZone, 0, &errMsg);

    if (conn == NULL) {
        fprintf (stderr, "rcConnect error\n");
        exit (1);
    }

    status = clientLogin(conn);
    if (status != 0) {
        fprintf (stderr, "clientLogin error\n");
       rcDisconnect(conn);
       exit (1);
    }

    /* open an nc object */
    bzero (&ncOpenInp, sizeof (ncOpenInp_t));
    rstrcpy (ncOpenInp.objPath, TEST_PATH1, MAX_NAME_LEN);
    ncOpenInp.mode = NC_NOWRITE;

    status = rcNcOpen (conn, &ncOpenInp, &ncid1);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcOpen error for %s", ncOpenInp.objPath);
	exit (2);
    }

    /* inq the dimension length */
    bzero (&ncInqIdInp, sizeof (ncInqIdInp));
    ncInqIdInp.paramType = NC_DIM_T;
    ncInqIdInp.ncid = ncid1;
    rstrcpy (ncInqIdInp.name, "longitude", MAX_NAME_LEN);
    status = rcNcInqId (conn, &ncInqIdInp, &londimid1);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcInqId error for dim %s of %s", ncInqIdInp.name,
	  ncOpenInp.objPath);
        exit (3);
    }

    ncInqIdInp.myid = *londimid1;
    status = rcNcInqWithId (conn, &ncInqIdInp, &ncInqWithIdOut);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcInqWithId error for dim %s of %s", ncInqIdInp.name,
          ncOpenInp.objPath);
        exit (3);
    } else {
	printf ("%s ---- dim length = %lld\n", ncInqIdInp.name, 
	  ncInqWithIdOut->mylong);
	londimlen1 = ncInqWithIdOut->mylong;
	free (ncInqWithIdOut);
	ncInqWithIdOut = NULL;
    }

    rstrcpy (ncInqIdInp.name, "latitude", MAX_NAME_LEN);
    status = rcNcInqId (conn, &ncInqIdInp, &latdimid1);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcInqId error for dim %s of %s", ncInqIdInp.name,
	  ncOpenInp.objPath);
        exit (3);
    }

    ncInqIdInp.myid = *latdimid1;
    status = rcNcInqWithId (conn, &ncInqIdInp, &ncInqWithIdOut);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcInqWithId error for dim %s of %s", ncInqIdInp.name,
          ncOpenInp.objPath);
        exit (3);
    } else {
        printf ("%s ---- dim length = %lld\n", ncInqIdInp.name,
          ncInqWithIdOut->mylong);
	latdimlen1 = ncInqWithIdOut->mylong;
        free (ncInqWithIdOut);
	ncInqWithIdOut = NULL;
    }

    /* do the variables */
    lonvarid1 = myInqVar (conn, ncid1, "longitude", &lontype1, &lonndim);
    if (lonvarid1 < 0)  exit (3);

    latvarid1 = myInqVar (conn, ncid1, "latitude", &lattype1, &latndim);
    if (latvarid1 < 0)  exit (3);

    tempvarid1 = myInqVar (conn, ncid1, "temperature", &temptype1, &tempndim);
    if (tempvarid1 < 0)  exit (3);

    presvarid1 = myInqVar (conn, ncid1, "pressure", &prestype1, &presndim);
    if (presvarid1 < 0)  exit (3);

    /* get the variable values */
    start[0] = 0;
    count[0] = londimlen1;
    stride[0] = 1;
    bzero (&ncGetVarInp, sizeof (ncGetVarInp));
    ncGetVarInp.dataType = lontype1;
    ncGetVarInp.ncid = ncid1;
    ncGetVarInp.varid = lonvarid1;
    ncGetVarInp.ndim = lonndim;
    ncGetVarInp.start = start;
    ncGetVarInp.count = count;
    ncGetVarInp.stride = stride;
   
    status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcGetVarsByType error for longitude of %s", ncInqIdInp.name,
          ncOpenInp.objPath);
        exit (3);
    } else {
	printf ("longitude value: \n");
	for (i = 0; i < ncGetVarOut->dataArray->len; i++) {
	    float *fdata = (float *)  ncGetVarOut->dataArray->buf;
	    printf ("  %.2f", fdata[i]);
	}
	printf ("\n");
	freeNcGetVarOut (&ncGetVarOut);
    }

    start[0] = 0;
    count[0] = latdimlen1;
    stride[0] = 1;
    ncGetVarInp.dataType = lattype1;
    ncGetVarInp.varid = latvarid1;
    ncGetVarInp.ndim = latndim;

    status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcGetVarsByType error for latitude of %s", ncInqIdInp.name,
          ncOpenInp.objPath);
        exit (3);
    } else {
        printf ("latitude value: \n");
        for (i = 0; i < ncGetVarOut->dataArray->len; i++) {
            float *fdata = (float *)  ncGetVarOut->dataArray->buf;
            printf ("  %.2f", fdata[i]);
        }
        printf ("\n");
	freeNcGetVarOut (&ncGetVarOut);
    }

    start[0] = 0;
    start[1] = 0;
    count[0] = latdimlen1;
    count[1] = londimlen1;
    stride[0] = 1;
    stride[2] = 1;
    ncGetVarInp.dataType = prestype1;
    ncGetVarInp.varid = presvarid1;
    ncGetVarInp.ndim = presndim;

    status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcGetVarsByType error for pressure of %s", ncInqIdInp.name,
          ncOpenInp.objPath);
        exit (3);
    } else {
        printf ("pressure value: \n");
        for (i = 0; i < ncGetVarOut->dataArray->len; i++) {
            float *fdata = (float *)  ncGetVarOut->dataArray->buf;
            printf ("  %.2f", fdata[i]);
        }
        printf ("\n");
	freeNcGetVarOut (&ncGetVarOut);
    }

    start[0] = 0;
    start[1] = 0;
    count[0] = latdimlen1;
    count[1] = londimlen1;
    stride[0] = 1;
    stride[2] = 1;
    ncGetVarInp.dataType = temptype1;
    ncGetVarInp.varid = tempvarid1;
    ncGetVarInp.ndim = tempndim;

    status = rcNcGetVarsByType (conn, &ncGetVarInp, &ncGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcGetVarsByType error for temperature of %s", ncInqIdInp.name,
          ncOpenInp.objPath);
        exit (3);
    } else {
        printf ("temperature value: \n");
        for (i = 0; i < ncGetVarOut->dataArray->len; i++) {
            float *fdata = (float *)  ncGetVarOut->dataArray->buf;
            printf ("  %.2f", fdata[i]);
        }
        printf ("\n");
	freeNcGetVarOut (&ncGetVarOut);
    }

    /* close the file */
    bzero (&ncCloseInp, sizeof (ncCloseInp_t));
    ncCloseInp.ncid = ncid1;
    status = rcNcClose (conn, &ncCloseInp);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcClose error for %s", ncOpenInp.objPath);
        exit (9);
    }

#ifdef LIB_CF
    /* open an nc object */
    bzero (&ncOpenInp, sizeof (ncOpenInp_t));
    rstrcpy (ncOpenInp.objPath, TEST_PATH1, MAX_NAME_LEN);
    ncOpenInp.mode = NC_NOWRITE;

    status = rcNcOpen (conn, &ncOpenInp, &ncid1);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcOpen error for %s", ncOpenInp.objPath);
        exit (2);
    }

    tempvarid1 = myInqVar (conn, ncid1, "temperature", &temptype1, &tempndim);
    if (tempvarid1 < 0)  exit (3);

    presvarid1 = myInqVar (conn, ncid1, "pressure", &prestype1, &presndim);
    if (presvarid1 < 0)  exit (3);

    /* pressure subset */
    bzero (&nccfGetVarInp, sizeof (nccfGetVarInp));
    nccfGetVarInp.ncid = ncid1;
    nccfGetVarInp.varid = presvarid1;
    nccfGetVarInp.latRange[0] = 30.0;
    nccfGetVarInp.latRange[1] = 41.0;
    nccfGetVarInp.lonRange[0] = -120.0;
    nccfGetVarInp.lonRange[1] = -96.0;
    nccfGetVarInp.maxOutArrayLen = 1000;

    status = rcNccfGetVara (conn, &nccfGetVarInp, &nccfGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNccfGetVara error for %s", ncOpenInp.objPath);
        exit (2);
    }

    printf (
      "pressure subset: nlat = %d, nlon = %d, dataType = %d, arrayLen = %d\n", 
      nccfGetVarOut->nlat, nccfGetVarOut->nlon, 
      nccfGetVarOut->dataArray->type, nccfGetVarOut->dataArray->len);
    mydata = (float *) nccfGetVarOut->dataArray->buf;
    printf ("pressure values: ");
    for (i = 0; i <  nccfGetVarOut->dataArray->len; i++) {
	printf (" %f", mydata[i]);
    }
    printf ("\n");
    freeNccfGetVarOut (&nccfGetVarOut);

    /* temperature subset */
    nccfGetVarInp.varid = tempvarid1;

    status = rcNccfGetVara (conn, &nccfGetVarInp, &nccfGetVarOut);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNccfGetVara error for %s", ncOpenInp.objPath);
        exit (2);
    }

    printf (
      "temperature subset: nlat = %d, nlon = %d, dataType = %d, arrLen = %d\n",
      nccfGetVarOut->nlat, nccfGetVarOut->nlon,
      nccfGetVarOut->dataArray->type, nccfGetVarOut->dataArray->len);
    mydata = (float *) nccfGetVarOut->dataArray->buf;
    printf ("temperature values: ");
    for (i = 0; i <  nccfGetVarOut->dataArray->len; i++) {
        printf (" %f", mydata[i]);
    }
    printf ("\n");
    freeNccfGetVarOut (&nccfGetVarOut);

    /* close the file */
    bzero (&ncCloseInp, sizeof (ncCloseInp_t));
    ncCloseInp.ncid = ncid1;
    status = rcNcClose (conn, &ncCloseInp);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcClose error for %s", ncOpenInp.objPath);
        exit (9);
    }
#endif

    exit (0);
} 

int
myInqVar (rcComm_t *conn, int ncid, char *name, int *dataType, int *ndim)
{
    int status, i;
    ncInqIdInp_t ncInqIdInp;
    ncInqWithIdOut_t *ncInqWithIdOut = NULL;
    int *myid = NULL;
    int tmpId;

    bzero (&ncInqIdInp, sizeof (ncInqIdInp));
    ncInqIdInp.ncid = ncid;
    ncInqIdInp.paramType = NC_VAR_T;
    rstrcpy (ncInqIdInp.name, name, MAX_NAME_LEN);
    status = rcNcInqId (conn, &ncInqIdInp, &myid);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcInqId error for var %s ", ncInqIdInp.name);
	return status;
    }

    ncInqIdInp.myid = *myid;
    status = rcNcInqWithId (conn, &ncInqIdInp, &ncInqWithIdOut);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status,
          "rcNcInqWithId error for dim %s", ncInqIdInp.name);
	return status;
    } else {
        printf ("%s ---- type = %d natts = %d, ndim = %d", ncInqIdInp.name,
          ncInqWithIdOut->type, ncInqWithIdOut->natts, ncInqWithIdOut->ndim);
        printf ("   dimid:");
        for (i = 0; i <  ncInqWithIdOut->ndim; i++) {
            printf ("    %d",  ncInqWithIdOut->intArray[i]);
        }
        printf ("\n");
	*dataType = ncInqWithIdOut->type;
	*ndim = ncInqWithIdOut->ndim;
	if (ncInqWithIdOut->intArray != NULL) free (ncInqWithIdOut->intArray);
	free (ncInqWithIdOut);
	ncInqWithIdOut = NULL;
    }
    tmpId = *myid;
    free (myid);
    return tmpId;
}

