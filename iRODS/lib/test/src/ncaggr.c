/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* nctest.c - test the iRODS NETCDF api */

#include "rodsClient.h" 

#define TEST_PATH1 "./ncdata/HFRadarCurrent"

int
genNcAggrInfo (char *testPath, ncAggrInfo_t *ncAggrInfo);
int 
setNcAggElement (int ncid, ncInqOut_t *ncInqOut, ncAggElement_t *ncAggElement);
unsigned int
getIntVar (int ncid, int varid, int dataType, rodsLong_t inx);
int
addNcAggElement (ncAggElement_t *ncAggElement, ncAggrInfo_t *ncAggrInfo);

int
main(int argc, char **argv)
{
    char *testPath;
    int status;
    ncAggrInfo_t ncAggrInfo;

    if (argc <= 1) {
        testPath = TEST_PATH1;
    } else {
	testPath = argv[1];
    }
    status = genNcAggrInfo (testPath, &ncAggrInfo);

    if (status < 0) {
        fprintf (stderr, "genNcAggrInfo of %s failed. status = %d\n", 
        testPath, status);
        exit (1);
    }
    exit (0);
}

int
genNcAggrInfo (char *dirPath, ncAggrInfo_t *ncAggrInfo)
{
    DIR *dirPtr;
    struct dirent *myDirent;
    int status = 0;
    char childPath[MAX_NAME_LEN];
    struct stat statbuf;
    ncInqInp_t ncInqInp;
    ncInqOut_t *ncInqOut = NULL;
    int ncid;
    ncAggElement_t ncAggElement;

    if (dirPath == NULL || ncAggrInfo == NULL) return USER__NULL_INPUT_ERR;

    dirPtr = opendir (dirPath);
    if (dirPtr == NULL) {
        rodsLog (LOG_ERROR,
        "genNcAggrInfo: opendir local dir error for %s, errno = %d\n",
         dirPath, errno);
        return (USER_INPUT_PATH_ERR);
    }
    bzero (ncAggrInfo, sizeof (ncAggrInfo_t));
    bzero (&ncAggElement, sizeof (ncAggElement));
    rstrcpy (ncAggrInfo->ncObjectName, dirPath, MAX_NAME_LEN);
    while ((myDirent = readdir (dirPtr)) != NULL) {
        if (strcmp (myDirent->d_name, ".") == 0 ||
          strcmp (myDirent->d_name, "..") == 0) {
            continue;
        }
        snprintf (childPath, MAX_NAME_LEN, "%s/%s",
          dirPath, myDirent->d_name);
        status = stat (childPath, &statbuf);
        if (status != 0) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: stat error for %s, errno = %d\n",
              childPath, errno);
            continue;
        } else if (statbuf.st_mode & S_IFDIR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: %s is a directory, errno = %d\n",
              childPath, errno);
            continue;
        }
        status = nc_open (childPath, NC_NOWRITE, &ncid);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: nc_open %s error, status = %d, %s",
              childPath, status, nc_strerror(status));
            continue;
        }
        /* do the general inq */
        bzero (&ncInqInp, sizeof (ncInqInp));
        ncInqInp.ncid = ncid;
        ncInqInp.paramType = NC_ALL_TYPE;
        ncInqInp.flags = NC_ALL_FLAG;

        status = ncInq (&ncInqInp, &ncInqOut);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "genNcAggrInfo: rcNcInq error for %s", childPath);
            return status;
        }
        status = setNcAggElement (ncid, ncInqOut, &ncAggElement);
        if (status < 0) break;
        rstrcpy (ncAggElement.fileName, childPath, MAX_NAME_LEN);
        status = addNcAggElement (&ncAggElement, ncAggrInfo);
        if (status < 0) break;
        bzero (&ncAggElement, sizeof (ncAggElement));
    }
    closedir (dirPtr);
    return 0;
}

int 
setNcAggElement (int ncid, ncInqOut_t *ncInqOut, ncAggElement_t *ncAggElement)
{
    int i, j;

    if (ncInqOut == NULL || ncAggElement == NULL) return USER__NULL_INPUT_ERR;

    for (i = 0; i < ncInqOut->ndims; i++) {
        if (strcasecmp (ncInqOut->dim[i].name, "time") == 0) break;
    }
    if (i >= ncInqOut->ndims) {
        /* no match */
        rodsLog (LOG_ERROR, 
          "genNcAggrInfo: 'time' dim does not exist");
        return NETCDF_DIM_MISMATCH_ERR;
    }
    for (j = 0; j < ncInqOut->nvars; j++) {
        if (strcmp (ncInqOut->dim[i].name, ncInqOut->var[j].name) == 0) {
            break;
        }
    }

    if (j >= ncInqOut->nvars) {
        /* no match */
        rodsLog (LOG_ERROR, 
          "genNcAggrInfo: 'time' var does not exist");
        return NETCDF_DIM_MISMATCH_ERR;
    }


    ncAggElement->arraylen = ncInqOut->dim[i].arrayLen;

    ncAggElement->startTime = getIntVar (ncid, ncInqOut->var[j].id, 
      ncInqOut->var[j].dataType, 0);
    ncAggElement->endTime = getIntVar (ncid, ncInqOut->var[j].id, 
      ncInqOut->var[j].dataType, ncInqOut->dim[i].arrayLen - 1);

    return 0;
}

unsigned int
getIntVar (int ncid, int varid, int dataType, rodsLong_t inx)
{
    size_t start[1], count[1];
    short int myshort;
    int myint;
    rodsLong_t mylong;
    float myfloat;
    double mydouble;
    unsigned int retint; 
    int status;
    

    start[0] = inx;
    count[0] = 1;

    if (dataType == NC_SHORT || dataType == NC_USHORT) {
        status = nc_get_vara (ncid, varid, start, count, (void *) &myshort);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: nc_get_vara error, status = %d, %s",
              status, nc_strerror(status));
            return NETCDF_GET_VARS_ERR - status;
        }
        retint = (unsigned int) myshort;
    } else if (dataType == NC_INT || dataType == NC_UINT) {
        status = nc_get_vara (ncid, varid, start, count, (void *) &myint);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: nc_get_vara error, status = %d, %s",
              status, nc_strerror(status));
            return NETCDF_GET_VARS_ERR - status;
        }
        retint = (unsigned int) myint;
    } else if (dataType == NC_INT64 || dataType == NC_UINT64) {
        status = nc_get_vara (ncid, varid, start, count, (void *) &mylong);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: nc_get_vara error, status = %d, %s",
              status, nc_strerror(status));
            return NETCDF_GET_VARS_ERR - status;
        }
        retint = (unsigned int) mylong;
    } else if (dataType == NC_FLOAT) {
        status = nc_get_vara (ncid, varid, start, count, (void *) &myfloat);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: nc_get_vara error, status = %d, %s",
              status, nc_strerror(status));
            return NETCDF_GET_VARS_ERR - status;
        }
        retint = (unsigned int) myfloat;
    } else if (dataType == NC_DOUBLE) {
        status = nc_get_vara (ncid, varid, start, count, (void *) &mydouble);
        if (status != NC_NOERR) {
            rodsLog (LOG_ERROR,
              "genNcAggrInfo: nc_get_vara error, status = %d, %s",
              status, nc_strerror(status));
            return NETCDF_GET_VARS_ERR - status;
        }
        retint = (unsigned int) mydouble;
    } else {
        rodsLog (LOG_ERROR,
          "getNcTypeStr: Unsupported dataType %d", dataType);
        return (NETCDF_INVALID_DATA_TYPE);
    }
    
    return retint;
}

int
addNcAggElement (ncAggElement_t *ncAggElement, ncAggrInfo_t *ncAggrInfo)
{
    int newNumFiles, i, j;
    ncAggElement_t *newElement;

    if (ncAggrInfo == NULL || ncAggElement == NULL) return USER__NULL_INPUT_ERR;

    if ((ncAggrInfo->numFiles % PTR_ARRAY_MALLOC_LEN) == 0) {
        newNumFiles =  ncAggrInfo->numFiles + PTR_ARRAY_MALLOC_LEN;
        newElement = (ncAggElement_t *) calloc (newNumFiles, 
          sizeof (ncAggElement_t));
        if (ncAggrInfo->numFiles > 0) {
            if (ncAggrInfo->ncAggElement == NULL) {
                rodsLog (LOG_ERROR,
                  "addNcAggElement: numFiles > 0 but cAggElement == NULL");
                return (NETCDF_VAR_COUNT_OUT_OF_RANGE);
            }
            memcpy (newElement, ncAggrInfo->ncAggElement, 
              ncAggrInfo->numFiles * sizeof (newElement));
            free (ncAggrInfo->ncAggElement);
        } 
        ncAggrInfo->ncAggElement = newElement;
    }

    if (ncAggrInfo->numFiles <= 0) {
        ncAggrInfo->ncAggElement[0] = *ncAggElement;
    } else {
        for (i = 0; i < ncAggrInfo->numFiles; i++) {
            ncAggElement_t *myElement = &ncAggrInfo->ncAggElement[i];
            if (ncAggElement->startTime < myElement->startTime || 
              (ncAggElement->startTime == myElement->startTime &&
              ncAggElement->endTime < myElement->endTime)) {
                /* insert */
                for (j = ncAggrInfo->numFiles; j > i; j--) {
                    ncAggrInfo->ncAggElement[j] = ncAggrInfo->ncAggElement[j-1];
                }
                ncAggrInfo->ncAggElement[i] = *ncAggElement;
                break;
            }
        }
        if (i >= ncAggrInfo->numFiles) {
            /* not inserted yet. put it at the end */
            ncAggrInfo->ncAggElement[i] = *ncAggElement;
        }
    }
    ncAggrInfo->numFiles++;            
    return 0;
}

