/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* s3FileDriver.c - The HPSS file driver
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
putObjectDataCallback(int bufferSize, char *buffer, void *callbackData)
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
putFileIntoS3(char *fileName, char *s3ObjName)
{

  S3Status status;
  int intstatus;
  char *key;
  struct stat statBuf;
  uint64_t fileSize;
  char *accessKeyId;
  char *secretAccessKey;
  put_object_callback_data data;


  accessKeyId = getenv("S3_ACCESS_KEY_ID");
  if (accessKeyId == NULL) {
    printf("S3_ACCESS_KEY_ID environment variable is undefined");
    return(-1);
  }

  secretAccessKey = getenv("S3_SECRET_ACCESS_KEY");
  if (secretAccessKey == NULL) {
    printf("S3_SECRET_ACCESS_KEY environment variable is undefined");
    return(-1);
  }

  bzero (&data, sizeof (data));

  key = (char *) strchr(s3ObjName, '/');
  if (key == NULL) {
    printf("S3 Key for the Object Not defined\n");
    return(-1);
  }
  *key = '\0';
  key++;
  if (stat(fileName, &statBuf) == -1) {
    printf("Unknown input file");
    return(-1);
  }
  fileSize = statBuf.st_size;

    data.fd = open (fileName, O_RDONLY, 0);
    if (data.fd < 0) {
        data.status = UNIX_FILE_OPEN_ERR - errno;
        rodsLog (LOG_ERROR,
         "putFileIntoS3: open error for fileName %s, status = %d",
         fileName, data.status);
        return -1;
    }

  S3BucketContext bucketContext =
    {s3ObjName,  1, 0, accessKeyId, secretAccessKey};
  S3PutObjectHandler putObjectHandler =
    {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &putObjectDataCallback
    };


  if ((status = myS3Init ()) != S3StatusOK) return (status);

  S3_put_object(&bucketContext, key, fileSize, NULL, 0,
                &putObjectHandler, &data);
  if (data.status != S3StatusOK) {
        intstatus = myS3Error (data.status, S3_PUT_ERROR);
  } else {
	intstatus = 0;
  }
  /* S3_deinitialize(); */

  close (data.fd);
  return (intstatus);
}

int
myS3Init ()
{
    int status;
    if (S3Initialized) return 0;

    if ((status = S3_initialize ("s3", S3_INIT_ALL)) != S3StatusOK) {
	status = myS3Error (status, S3_INIT_ERROR);
    } else {
	S3Initialized = 1;
    }

    return status;
}

int
myS3Error (int status, int irodsErrorCode)
{
    if (status <= 0) return status;
     
     rodsLogError (LOG_ERROR, irodsErrorCode,
         "myS3Error: error:%s", S3_get_status_name(status));
    return (irodsErrorCode - status);
}
