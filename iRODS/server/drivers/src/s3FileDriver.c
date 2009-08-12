/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* s3FileDriver.c - The S3 file driver
 */


#include "s3FileDriver.h"
#include "rsGlobalExtern.h"

static int S3Initialized = 0;

int
s3FileUnlink (rsComm_t *rsComm, char *filename)
{
    int status;

    status = 0;

    return (status);
}

int
s3FileStat (rsComm_t *rsComm, char *filename, struct stat *statbuf)
{
    int status;
    
    status = 0;

    return (status);
}

int
s3FileMkdir (rsComm_t *rsComm, char *filename, int mode)
{
    int status;

    status = 0;

    return (status);
}       

int
s3FileChmod (rsComm_t *rsComm, char *filename, int mode)
{
    int status;

    status = 0;

    return (status);
}

int
s3FileRmdir (rsComm_t *rsComm, char *filename)
{
    int status;

    status = 0;

    return (status);
}

rodsLong_t
s3FileGetFsFreeSpace (rsComm_t *rsComm, char *path, int flag)
{
    int space = LARGE_SPACE;
    return (space * 1024 * 1024);
}

/* s3StageToCache - This routine is for testing the TEST_STAGE_FILE_TYPE.
 * Just copy the file from filename to cacheFilename. optionalInfo info
 * is not used.
 * 
 */
  
int
s3StageToCache (rsComm_t *rsComm, fileDriverType_t cacheFileType, 
int mode, int flags, char *filename, 
char *cacheFilename,  rodsLong_t dataSize,
keyValPair_t *condInput)
{
    int status;

    status = 0;

    return status;

}

/* s3SyncToArch - This routine is for testing the TEST_STAGE_FILE_TYPE.
 * Just copy the file from cacheFilename to filename. optionalInfo info
 * is not used.
 *
 */

int
s3SyncToArch (rsComm_t *rsComm, fileDriverType_t cacheFileType, 
int mode, int flags, char *filename,
char *cacheFilename,  rodsLong_t dataSize, keyValPair_t *condInput)
{
    int status;
    struct stat statbuf;

    status = stat (cacheFilename, &statbuf);

    if (status < 0) {
        status = UNIX_FILE_STAT_ERR - errno;
        rodsLog (LOG_ERROR, "s3SyncToArch: stat of %s error, status = %d",
         cacheFilename, status);
        return status;
    }

    if ((statbuf.st_mode & S_IFREG) == 0) {
        status = UNIX_FILE_STAT_ERR - errno;
        rodsLog (LOG_ERROR, "s3SyncToArch: %s is not a file, status = %d",
         cacheFilename, status);
        return status;
    }

    if (dataSize > 0 && dataSize != statbuf.st_size) {
        rodsLog (LOG_ERROR,
          "s3SyncToArch: %s inp size %lld does not match actual size %lld",
         cacheFilename, dataSize, statbuf.st_size);
        return SYS_COPY_LEN_ERR;
    }
    dataSize = statbuf.st_size;

    return status;
}

void 
responseCompleteCallback (S3Status status, const S3ErrorDetails *error,
void *callbackData)
{
    int i;
    put_object_callback_data *data = (put_object_callback_data *) callbackData;

    if (status > S3StatusOK) {
	if (data->status >= S3StatusOK) data->status = status;
    } else {
        data->status = status;
    }

    if (error) {
        if (error->message) {
            rodsLog (LOG_NOTICE,
             "responseCompleteCallback: Message: %s", error->message);
	}
        if (error->resource) {
            rodsLog (LOG_NOTICE,
             "responseCompleteCallback: resource: %s", error->resource);
        }
        if (error->furtherDetails) {
            rodsLog (LOG_NOTICE,
              "responseCompleteCallback: furtherDetails: %s", 
	      error->furtherDetails);
        }

        if (error->extraDetailsCount) {
            rodsLog (LOG_NOTICE,
             "responseCompleteCallback: extraDetails");

            for (i = 0; i < error->extraDetailsCount; i++) {
                printf("    %s: %s\n",
                  error->extraDetails[i].name,
                  error->extraDetails[i].value);
	    }
        }
    }
}

S3Status 
responsePropertiesCallback(const S3ResponseProperties *properties,
void *callbackData)
{

    return S3StatusOK;
}

int 
putObjectDataCallback (int bufferSize, char *buffer, void *callbackData)
{
    put_object_callback_data *data =
        (put_object_callback_data *) callbackData;
    int ret = 0;

    if (data->contentLength) {
        int length = ((data->contentLength > (unsigned) bufferSize) ?
                      (unsigned) bufferSize : data->contentLength);
        ret = read (data->fd, buffer, length);
    }
    data->contentLength -= ret;
    return ret;
}

int 
putFileIntoS3 (char *fileName, char *s3ObjName, rodsLong_t fileSize)
{

    S3Status status;
    char key[MAX_NAME_LEN], myBucket[MAX_NAME_LEN];
    s3Auth_t s3Auth;
    put_object_callback_data data;


    bzero (&data, sizeof (data));

    if ((status = parseS3Path (s3ObjName, myBucket, key)) < 0) return status;

    data.fd = open (fileName, O_RDONLY, 0);
    if (data.fd < 0) {
        status = UNIX_FILE_OPEN_ERR - errno;
        rodsLog (LOG_ERROR,
         "putFileIntoS3: open error for fileName %s, status = %d",
         fileName, status);
        return status;
    }

    S3BucketContext bucketContext =
      {myBucket,  1, 0, s3Auth.accessKeyId, s3Auth.secretAccessKey};
    S3PutObjectHandler putObjectHandler = {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &putObjectDataCallback
    };


    if ((status = myS3Init (&s3Auth)) != S3StatusOK) return (status);

    S3_put_object(&bucketContext, key, fileSize, NULL, 0,
                &putObjectHandler, &data);
    if (data.status != S3StatusOK) {
        status = myS3Error (data.status, S3_PUT_ERROR);
    }
    /* S3_deinitialize(); */

    close (data.fd);
    return (status);
}

int
myS3Init (s3Auth_t *s3Auth)
{
    int status = -1;
    if (S3Initialized) return 0;

    S3Initialized = 1;

    if ((s3Auth->accessKeyId = getenv("S3_ACCESS_KEY_ID")) != NULL) {
        if ((s3Auth->secretAccessKey = getenv("S3_SECRET_ACCESS_KEY")) 
	  != NULL) {
	    status = 0;
	}
    }

    if (status < 0) {
        if ((status = readS3AuthInfo (s3Auth)) < 0) {
            rodsLog (LOG_ERROR,
              "initHpssAuth: readHpssAuthInfo error. status = %d", status);
            return status;
        }
    }

    if ((status = S3_initialize ("s3", S3_INIT_ALL)) != S3StatusOK) {
	status = myS3Error (status, S3_INIT_ERROR);
    }

    return status;
}

int
readS3AuthInfo (s3Auth_t *s3Auth)
{
    FILE *fptr;
    char s3AuthFile[MAX_NAME_LEN];
    char inbuf[MAX_NAME_LEN];
    int lineLen, bytesCopied;
    int linecnt = 0;

    snprintf (s3AuthFile, MAX_NAME_LEN, "%-s/%-s",
      getConfigDir(), S3_AUTH_FILE);

    fptr = fopen (s3AuthFile, "r");

    if (fptr == NULL) {
        rodsLog (LOG_ERROR,
          "readS3AuthInfo: open S3_AUTH_FILE file %s err. ernro = %d",
          s3AuthFile, errno);
        return (SYS_CONFIG_FILE_ERR);
    }
    while ((lineLen = getLine (fptr, inbuf, MAX_NAME_LEN)) > 0) {
        char *inPtr = inbuf;
        if (linecnt == 0) {
            while ((bytesCopied = getStrInBuf (&inPtr, 
	      s3Auth->accessKeyId, &lineLen, LONG_NAME_LEN)) > 0) {
                linecnt ++;
                break;
            }
        } else if (linecnt == 1) {
            while ((bytesCopied = getStrInBuf (&inPtr, 
	      s3Auth->secretAccessKey, &lineLen, LONG_NAME_LEN)) > 0) {
                linecnt ++;
                break;
            }
        }
    }
    if (linecnt != 2)  {
        rodsLog (LOG_ERROR,
          "readS3AuthInfo: read %d lines in S3_AUTH_FILE file",
          linecnt);
        return (SYS_CONFIG_FILE_ERR);
    }
    return 0;
}

int
myS3Error (int status, int irodsErrorCode)
{
    if (status <= 0) return status;
     
     rodsLogError (LOG_ERROR, irodsErrorCode,
         "myS3Error: error:%s", S3_get_status_name(status));
    return (irodsErrorCode - status);
}

int
parseS3Path (char *s3ObjName, char *bucket, char *key)
{
    char tmpPath[MAX_NAME_LEN];
    char *tmpBucket, *tmpKey;

    rstrcpy (tmpPath, s3ObjName, MAX_NAME_LEN);
    /* skip the leading '/' */
    if (tmpPath[0] == '/') {
        tmpBucket = tmpPath + 1;
    } else {
        tmpBucket = tmpPath;
    }
    tmpKey = (char *) strchr (tmpBucket, '/');
    if (tmpKey == NULL) {
        rodsLog (LOG_ERROR,
         "putFileIntoS3:  problem parsing  %s", s3ObjName);
        return SYS_INVALID_FILE_PATH;
    }
    *tmpKey = '\0';
    tmpKey++;
    rstrcpy (bucket, tmpBucket, MAX_NAME_LEN);
    rstrcpy (key, tmpKey, MAX_NAME_LEN);

    return 0;
}

