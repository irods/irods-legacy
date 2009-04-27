/* 
   irods standard i/o emulation library
   
   This is an initial, very incomplete, and experimental
   version, but I wanted to have it in CVS.

   This package converts Unix standard I/O calls (stdio.h; fopen,
   fread, fwrite, etc) to the equivalent irods calls.  To convert an
   application you just need to add an include file (isio.h) and
   recompile and relink with irods libraries.  fopen calls on files
   that include the IRODS_PREFIX string will be opened and handled as
   iRODS files; without the prefix this package will call the fopen
   family to handle them as regular local files.

   Like the fopen family, this library does (will do) some caching to
   avoid small I/O (network) calls.

   The user callable functions are defined in the isio.h and are of
   the form irodsNAME, such as irodsfopen.  Internal function names
   begin with 'isio'.

   The irods environment is assummed.  That is, like i-commands, this
   library needs to read the user's .irodsEnv and authentication files
   to be able to connect to an iRODS server.

 */

#include <stdio.h>
#include "rodsClient.h"
#include "dataObjRead.h"

#define IRODS_PREFIX "irods:"
#define ISIO_MAX_OPEN_FILES 20
#define ISIO_MIN_OPEN_FD 5

int debug=1;

long openFiles[ISIO_MAX_OPEN_FILES]={0,0,0,0,0,0,0,0,0,0,
				     0,0,0,0,0,0,0,0,0,0};

struct {
   FILE *fd;
   char *base;
   int bufferSize;
   char *cptr;
   int *count;
} cacheInfo[ISIO_MAX_OPEN_FILES];



static setupFlag=0;
char localZone[100]="";
rcComm_t *Comm;
rodsEnv myRodsEnv;

int
isioSetup() {
   int status;
   rErrMsg_t errMsg;
   char *mySubName;
   char *myName;

   if (debug) printf("isioSetup\n");

   status = getRodsEnv (&myRodsEnv);
   if (status < 0) {
      rodsLogError(LOG_ERROR, status, "isioSetup: getRodsEnv error.");
   }
   Comm = rcConnect (myRodsEnv.rodsHost, myRodsEnv.rodsPort, 
		     myRodsEnv.rodsUserName,
                     myRodsEnv.rodsZone, 0, &errMsg);

   if (Comm == NULL) {
      myName = rodsErrorName(errMsg.status, &mySubName);
      rodsLog(LOG_ERROR, "rcConnect failure %s (%s) (%d) %s",
	      myName,
	      mySubName,
	      errMsg.status,
	      errMsg.msg);
      status = errMsg.status;
      return(status);
   }

   status = clientLogin(Comm);
   if (status==0) {
      setupFlag=1;
   }
   return(status);
}

FILE *isioFileOpen(char *filename, char *modes) {
   int i;
   int status;
   dataObjInp_t dataObjInp;
   char **outPath;

   if (debug) printf("isioFileOpen: %s\n", filename);

   if (setupFlag==0) {
      status = isioSetup();
      if (status) return(NULL);
   }

   for (i=ISIO_MIN_OPEN_FD;i<ISIO_MAX_OPEN_FILES;i++) {
     if (openFiles[i]==0) break;
   }
   if (i>=ISIO_MAX_OPEN_FILES) {
     fprintf(stderr,"Too many open files in isioFileOpen\n");
     return(NULL);
   }

   memset (&dataObjInp, 0, sizeof (dataObjInp));
   status = parseRodsPathStr (filename , &myRodsEnv,
			      dataObjInp.objPath);
   if (status < 0) {
     rodsLogError (LOG_ERROR, status, "isioFileOpen");
     return(NULL);
   }

   dataObjInp.openFlags = O_RDONLY;
   if (strncmp(modes,"w",1)==0) {
      dataObjInp.openFlags = O_WRONLY;
   }

   status = rcDataObjOpen (Comm, &dataObjInp);

   if (status==CAT_NO_ROWS_FOUND &&
       dataObjInp.openFlags == O_WRONLY) {
      status = rcDataObjCreate(Comm, &dataObjInp);
   }
   if (status < 0) {
      rodsLogError (LOG_ERROR, status, "isioFileOpen");
      return(NULL);
   }
   openFiles[i]=status;
   cacheInfo[i].base=NULL;
   return((FILE *)i);
}

FILE *irodsfopen(char *filename, char *modes) {
   int status;
   int len;

   if (debug) printf("irodsfopen: %s\n", filename);

   len = strlen(IRODS_PREFIX);
   if (strncmp(filename,IRODS_PREFIX,len)==0) {
     return(isioFileOpen(filename+len, modes));
   }
   else {
     return(fopen(filename, modes));
   }
}

int
isioFileRead(int fileIndex, void *buffer, int maxToRead) {
   int status;
   openedDataObjInp_t dataObjReadInp;
   bytesBuf_t dataObjReadOutBBuf;

   if (debug) printf("isioFileRead: %d\n", fileIndex);

   dataObjReadOutBBuf.buf = buffer;
   dataObjReadOutBBuf.len = maxToRead;

   memset(&dataObjReadInp, 0, sizeof (dataObjReadInp));

   dataObjReadInp.l1descInx = openFiles[fileIndex];
   dataObjReadInp.len = maxToRead;

   return(rcDataObjRead (Comm, &dataObjReadInp,
			 &dataObjReadOutBBuf));
}

size_t irodsfread(void *buffer, size_t itemsize, int nitems, FILE *fi_stream) {
   int i;
   i = (int)fi_stream;

   if (debug) printf("isiofread: %d\n", i);

   if (i<ISIO_MAX_OPEN_FILES && i>=ISIO_MIN_OPEN_FD && openFiles[i]>0) {
      return(isioFileRead(i, buffer, itemsize*nitems));
   }
   else {
      return(fread(buffer, itemsize, nitems, fi_stream));
   }
}

int
isioFileWrite(int fileIndex, void *buffer, int maxToWrite) {
   int status;
   openedDataObjInp_t dataObjWriteInp;
   bytesBuf_t dataObjWriteOutBBuf;

   if (debug) printf("isioFileWrite: %d\n", fileIndex);

   dataObjWriteOutBBuf.buf = buffer;
   dataObjWriteOutBBuf.len = maxToWrite;

   memset(&dataObjWriteInp, 0, sizeof (dataObjWriteInp));

   dataObjWriteInp.l1descInx = openFiles[fileIndex];
   dataObjWriteInp.len = maxToWrite;

   return(rcDataObjWrite (Comm, &dataObjWriteInp,
			 &dataObjWriteOutBBuf));
}

size_t 
irodsfwrite(void *buffer, size_t itemsize, int nitems, FILE *fi_stream) {
   int i;
   i = (int)fi_stream;
   if (debug) printf("isiofwrite: %d\n", i);
   if (i<ISIO_MAX_OPEN_FILES && i>=ISIO_MIN_OPEN_FD && openFiles[i]>0) {
      return(isioFileWrite(i, buffer, itemsize*nitems));
   }
   else {
      return(fwrite(buffer, itemsize, nitems, fi_stream));
   }
}

int
isioFileClose(int fileIndex) {
   openedDataObjInp_t dataObjCloseInp;
   if (debug) printf("isioFileClose: %d\n", fileIndex);
   memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
   dataObjCloseInp.l1descInx = openFiles[fileIndex];
   return(rcDataObjClose(Comm, &dataObjCloseInp));
}

size_t irodsfclose(FILE *fi_stream) {
   int i;
   i = (int)fi_stream;
   if (debug) printf("isiofclose: %d\n", i);
   if (i<ISIO_MAX_OPEN_FILES && i>=ISIO_MIN_OPEN_FD && openFiles[i]>0) {
      return(isioFileClose(i));
   }
   else {
      return(fclose(fi_stream));
   }
}


int
isioFileSeek(int fileIndex, long offset, int whence) {
//   fileLseekInp_t seekParam;
   openedDataObjInp_t seekParam;
   fileLseekOut_t* seekResult = NULL;
   int status;
   if (debug) printf("isioFileSeek: %d\n", fileIndex);
   memset( &seekParam,  0, sizeof(fileLseekInp_t) );
   seekParam.l1descInx = openFiles[fileIndex];
   seekParam.offset  = offset;
   seekParam.whence  = whence;
   status = rcDataObjLseek(Comm, &seekParam, &seekResult );
   if ( status < 0 ) {
      rodsLogError (LOG_ERROR, status, "isioFileSeek");
   }
   return(status);
}

int
irodsfseek(FILE *fi_stream, long offset, int whence) {
   int i;
   i = (int)fi_stream;
   if (debug) printf("isiofseek: %d\n", i);
   if (i<ISIO_MAX_OPEN_FILES && i>=ISIO_MIN_OPEN_FD && openFiles[i]>0) {
      return(isioFileSeek(i,offset,whence));
   }
   else {
      return(fclose(fi_stream));
   }
}


int
irodsexit(int exitValue) {
   int status;
   if (debug) printf("irodsexit: %d\n", exitValue);
   if (setupFlag>0) {
      status = rcDisconnect(Comm);
   }
   exit(exitValue);
}
