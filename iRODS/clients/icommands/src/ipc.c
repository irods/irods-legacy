/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * ipc - The irods partial copy utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "rsyncUtil.h"

#define MAX_DATA_BUFFER_SIZE 5000000
void usage();

int
closeDataObj(rcComm_t *Comm, int fd) {
   openedDataObjInp_t dataObjCloseInp;
   int status;

   memset (&dataObjCloseInp, 0, sizeof (dataObjCloseInp));
   dataObjCloseInp.l1descInx = fd;

   status = rcDataObjClose(Comm, &dataObjCloseInp);
   if (status != 0) {
        rodsLogError (LOG_ERROR, status, "closeDataObj error,");
   }
   return(status);
}

void
disconnectAndExit(rcComm_t *Comm, int exitValue) {
   rcDisconnect(Comm);
   exit(exitValue);
}

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *Comm;
    rodsArguments_t myRodsArgs;
    char *optStr, nameBuffer[HUGE_NAME_LEN];
    rodsPathInp_t rodsPathInp;
    objType_t srcType, destType;
    int nArgv;
    int i;
    unsigned char *dataBuffer;
    int dataBufferSize=0;
    int fd_in, fd_out, rval, wval;
    int readSize;
    int dupSize=0;
    int totalCopied=0;

    bytesBuf_t dataObjReadInpBBuf;
    openedDataObjInp_t dataObjReadInp;

    openedDataObjInp_t dataObjWriteInp;
    bytesBuf_t dataObjWriteOutBBuf;


    dataObjInp_t dataObjInp;
    char *cp;

    int numParseFlag;
    char *numStart;
    char *numEnd;
    rodsLong_t srcOffset=0;
    rodsLong_t destOffset=0;

    optStr = "hfs:vV";
   
    status = parseCmdLineOpt (argc, argv, optStr, 1, &myRodsArgs);

    if (status < 0) {
        printf("use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    status = getRodsEnv (&myEnv);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (2);
    }

    nArgv = argc - optind;

    if (nArgv != 2) {   /* must be 2 inputs, simply src and dest */
        usage();
        exit (3);
    }

    /* Similar to how 'irsync' does it, look for "i:" in the source
       file name and if found set a flag and remove it from the
       name */
    i = argc-2;
    if (strcmp (argv[i], "i:") == 0) {
        srcType = UNKNOWN_OBJ_T;
        strcpy (argv[i], ".");
    } else if (strncmp (argv[i], "i:", 2) == 0) {
        destType = UNKNOWN_OBJ_T;
        strcpy (nameBuffer, argv[i] + 2);
        argv[i] = strdup (argv[i] + 2);
    } else {
        srcType = UNKNOWN_FILE_T;
    }

    /* look for [number] in the source file name and if found save the
       value and remove it from the name */
    numParseFlag=0;
    for (cp=argv[i];*cp!='\0';cp++) {
       if (numParseFlag==1 && *cp==']') {
          numEnd=cp;
       }
       if (numParseFlag==0 && *cp=='[') {
          numParseFlag=1;
          numStart=cp;
       }
    }
    if (numParseFlag) {
       srcOffset=(atoll(numStart+1));
       strcpy(numStart,numEnd+1);
       if (srcOffset <= 0) {
          fprintf(stderr,"Invalid offset into source file\n");
          exit(4);
       }
       if (myRodsArgs.veryVerbose) {
          printf("srcOffset  %lld\n",srcOffset);
       }
    }

    /* look for "i:" in the destination file name and if found set a flag
        and remove it from the name */
    i = argc-1;
    if (strcmp (argv[i], "i:") == 0) {
        destType = UNKNOWN_OBJ_T;
        strcpy (argv[argc-1], ".");
    } else if (strncmp (argv[i], "i:", 2) == 0) {
        destType = UNKNOWN_OBJ_T;
        strcpy (nameBuffer, argv[i] + 2);
        argv[i] = strdup (argv[i] + 2);
    } else {
        destType = UNKNOWN_FILE_T;
    }

    /* Look for [number] in the destination file name and if found
       save the value and remove it from the name */
    numParseFlag=0;
    for (cp=argv[i];*cp!='\0';cp++) {
       if (numParseFlag==1 && *cp==']') {
          numEnd=cp;
       }
       if (numParseFlag==0 && *cp=='[') {
          numParseFlag=1;
          numStart=cp;
       }
    }
    if (numParseFlag) {
       destOffset=(atoll(numStart+1));
       strcpy(numStart,numEnd+1);
       if (destOffset <= 0) {
          fprintf(stderr,"Invalid offset into destination file\n");
          exit(5);
       }
       if (myRodsArgs.veryVerbose) {
          printf("destOffset %lld\n",destOffset);
       }
    }

    /* Parse the command line path in the typical manner */
    status = parseCmdLinePath (argc, argv, optind, &myEnv,
      srcType, destType, 0, &rodsPathInp);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. "); 
        printf("use -h for help.\n");
        exit (6);
    }

    if (myRodsArgs.veryVerbose) {
       for (i=0;i<rodsPathInp.numSrc;i++) {
          printf("src %d: %s\n",i,rodsPathInp.srcPath[i].outPath);
       }
       printf("dest: %s\n",rodsPathInp.destPath[0].outPath);
       if (myRodsArgs.sizeFlag == True) {
          printf("size in arg:%lld\n",myRodsArgs.size);
       }
    }

    if (myRodsArgs.sizeFlag == True) {
       dupSize = myRodsArgs.size;
       if (dupSize <= 0) {
          fprintf(stderr,
         "Invalid size %d, negative (or very large) or 0 values not allowed\n",
             dupSize);             
          exit(7);
       }
    }

    Comm = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 1, &errMsg);
    if (Comm == NULL) {
        exit (8);
    }
   
    status = clientLogin(Comm);
    if (status != 0) {
       disconnectAndExit(Comm, 9);
    }

    /* Open source file or data-object */
    if (srcType == UNKNOWN_OBJ_T) {  /* iRODS data-object */
       memset (&dataObjInp, 0, sizeof (dataObjInp));
       status = parseRodsPathStr (rodsPathInp.srcPath[0].outPath, &myEnv,
                                  dataObjInp.objPath);
       if (status < 0) {
          rodsLogError (LOG_ERROR, status, "ipc");
          exit(10);
       }
       dataObjInp.openFlags = O_RDONLY;
       status = rcDataObjOpen (Comm, &dataObjInp);
       if (status==CAT_NO_ROWS_FOUND) {
          rodsLogError (LOG_ERROR, status, 
                "data object does not exist or user lacks access permission,");
          disconnectAndExit(Comm, 11);
       }
       if (status < 0) {
          rodsLogError (LOG_ERROR, status, "ipc rcDataObjOpen");
          disconnectAndExit(Comm, 12);
       }
       fd_in = status;
    }
    else {  /* local file */
       fd_in = open(rodsPathInp.srcPath[0].outPath, O_RDONLY,0);
       if (fd_in<0) {
          fprintf(stderr,"input file %s open error\n",
                  rodsPathInp.srcPath[0].outPath);
          disconnectAndExit(Comm, 13);
       }
    }

    /* Open destination file or data-object */
    if (destType == UNKNOWN_OBJ_T) {  /* iRODS data-object */
       memset (&dataObjInp, 0, sizeof (dataObjInp));
       status = parseRodsPathStr (rodsPathInp.destPath[0].outPath, &myEnv,
                                  dataObjInp.objPath);
       if (status < 0) {
          rodsLogError (LOG_ERROR, status, "ipc");
          exit(14);
       }
       dataObjInp.openFlags = O_WRONLY;
       status = rcDataObjCreate(Comm, &dataObjInp);
       if (myRodsArgs.veryVerbose) {
          printf("ipc rcDataObjCreate value (fd if positive) %d\n",
                  status);
       }
       fd_out = status;
    }
    else {  /* local file */
       fd_out = open(rodsPathInp.destPath[0].outPath,
                     O_CREAT|O_WRONLY|O_EXCL,0600);
    }

    /* 
    As done in other iRODS logic, if it can't create the file, assume
    it is because it exists, and if -f was included, try to open an
    existing one for writing.
    */
    if (fd_out < 0) {
       if (myRodsArgs.force == True) {
          if (destType == UNKNOWN_OBJ_T) {
             status = rcDataObjOpen (Comm, &dataObjInp);
             if (status < 0) {
                rodsLogError (LOG_ERROR, status, "ipc rcDataObjOpen");
             }
             fd_out = status;
          }
          else {
             fd_out=open(rodsPathInp.destPath[0].outPath,O_WRONLY);
          }
       }
    }

    /* Still failed */
    if (fd_out<0) {
       fprintf(stderr,"output file %s open error\n",
               rodsPathInp.destPath[0].outPath);
       if (status<0) {
          rodsLogError (LOG_ERROR, status, 
                "ipc open error,");
       }
       fprintf(stderr,
               "-f (force) must be included to overwrite an existing file\n");

       disconnectAndExit(Comm, 15);
    }

    /* Seek into source if requested */
    if (srcOffset > 0) {
       off_t seekStatus;
       if (srcType == UNKNOWN_OBJ_T) {  /* iRODS data-object */
          openedDataObjInp_t seekParam;
          fileLseekOut_t* seekResult = NULL;
          memset( &seekParam,  0, sizeof(openedDataObjInp_t) );
          seekParam.l1descInx = fd_in;
          seekParam.offset  = srcOffset;
          seekParam.whence  = SEEK_SET;
          status = rcDataObjLseek(Comm, &seekParam, &seekResult );
          if (status < 0) {
             rodsLogError (LOG_ERROR, status, 
                           "ipc source-object srcDataObjLSeek,");
             disconnectAndExit(Comm, 16);
          }
       }   
       else {                           /* local file */
          seekStatus = lseek(fd_in, srcOffset, SEEK_SET);
          if (seekStatus<0) {
             perror("ipc source file seek error");
             disconnectAndExit(Comm, 17);
          }
       }
    }

    /* Seek into destination if requested */
    if (destOffset > 0) {
       off_t seekStatus;
       if (destType == UNKNOWN_OBJ_T) { /* iRODS data-object */
          openedDataObjInp_t seekParam;
          fileLseekOut_t* seekResult = NULL;
          memset( &seekParam,  0, sizeof(openedDataObjInp_t) );
          seekParam.l1descInx = fd_out;
          seekParam.offset  = destOffset;
          seekParam.whence  = SEEK_SET;
          status = rcDataObjLseek(Comm, &seekParam, &seekResult );
          if (status < 0) {
             rodsLogError (LOG_ERROR, status, 
               "ipc destination-object rcDataObjLSeek,");
             disconnectAndExit(Comm, 18);
          }
       }
       else {                           /* local file */
          seekStatus = lseek(fd_out, destOffset, SEEK_SET);
          if (seekStatus<0) {
             perror("ipc destination file seek error");
             disconnectAndExit(Comm, 19);
          }
       }
    }

    /* Malloc the data buffer */
    dataBufferSize = MAX_DATA_BUFFER_SIZE;
    if (dupSize != 0 && dupSize < MAX_DATA_BUFFER_SIZE) {
       dataBufferSize = dupSize + 10; /* with a few bytes extra to be sure */
    }
    dataBuffer = (unsigned char *) malloc (dataBufferSize);
    if (dataBuffer==NULL) {
       perror("ipc malloc error");
       disconnectAndExit(Comm, 20);
    }


    /* Do a loop of reads and writes */
    do {
       readSize = dataBufferSize;
       if (dupSize != 0) {
          int leftToRead;
          leftToRead = dupSize-totalCopied;
          if (leftToRead==0) break;
          if (leftToRead < dataBufferSize) {
             readSize = leftToRead;
          }
       }

       if (srcType == UNKNOWN_OBJ_T) {
          dataObjReadInpBBuf.buf = dataBuffer;
          dataObjReadInpBBuf.len = readSize;
          memset(&dataObjReadInp, 0, sizeof (dataObjReadInp));
          dataObjReadInp.l1descInx = fd_in;
          dataObjReadInp.len = readSize;
          status = rcDataObjRead (Comm, &dataObjReadInp,
                                &dataObjReadInpBBuf);
          if (status < 0) {
             rodsLogError (LOG_ERROR, status, "ipc rcDataObjRead,");
             disconnectAndExit(Comm, 21);
          }
          rval = status;
       }
       else {
          rval = read(fd_in, dataBuffer, readSize);
       }
       if (myRodsArgs.veryVerbose) {
          printf("read status (bytes) : %d\n", rval);
       }
       if (rval < 0) {
          fprintf(stderr,"read error\n");
          disconnectAndExit(Comm, 22);
       }
       if (rval > 0) {
          if (destType == UNKNOWN_OBJ_T) {
             dataObjWriteOutBBuf.buf = dataBuffer;
             dataObjWriteOutBBuf.len = rval;
             memset(&dataObjWriteInp, 0, sizeof (dataObjWriteInp));
             dataObjWriteInp.l1descInx = fd_out;
             dataObjWriteInp.len = rval;
             status = rcDataObjWrite (Comm, &dataObjWriteInp,
                                     &dataObjWriteOutBBuf);
             if (status < 0) {
                rodsLogError (LOG_ERROR, status, "ipc rcDataObjWrite,");
                disconnectAndExit(Comm, 23);
             }
             wval = status;
          }
          else {
             wval = write(fd_out, dataBuffer, rval);
          }
          if (myRodsArgs.veryVerbose) {
             printf("write status (bytes): %d\n", wval);
          }
          if (wval != rval) {
             fprintf(stderr,"write error\n");
             disconnectAndExit(Comm, 24);
          }
          totalCopied+=rval;
       }
    } while (rval > 0);

    /* Close the files and finish up */
    if (myRodsArgs.verbose) {
       printf("Copied %d bytes\n",totalCopied);
    }

    if (srcType == UNKNOWN_OBJ_T) {
       status = closeDataObj(Comm, fd_in);
    }
    else {
       close(fd_in);
    }

    if (destType == UNKNOWN_OBJ_T) {
       status = closeDataObj(Comm, fd_out);
    }
    else {
       close(fd_out);
    }

    rcDisconnect(Comm);

    if (totalCopied==0) {
       fprintf(stderr, "Error, 0 bytes copied.\n");
       if (srcOffset > 0) {
          fprintf(stderr,
            "The offset (%lld) was likely beyond the end of the source file\n", 
            srcOffset);
       }
       status = -1; /* have it error exit */
    }

    if (status < 0) {
        exit (25);
    } else {
        exit(0);
    }
}

void 
usage ()
{
   int i;
   char *msgs[]={
"Usage:",
"ipc [-fvVhs] Source-File-or-Object[offset] Destination-File-or-Object[offset]",
" ",
"Perform a partial copy to or from an iRODS data-object (file) and a",
"local file.",
" ",
"Similar to irsync, the string 'i:' is used to specify an irods file",
"(data-object).  The source-file and the destination-file can be either",
"an iRODS or local file.",
" ",
"The -s value option is used to specify the size, in bytes, that is to",
"be copied.  For example '-s 100000' will copy 100,000 bytes, or to the",
"end of the file, whichever is less.  When no size is provided, it will",
"copy to the end of the source file.",
" ",
"Either, both, or neither of the source and destination can have",
"offsets into the file for the starting postion of the copy (a 'seek'",
"operation is done) before starting the copy.  These are indicated by",
"square brackets and the number at the end of the name, for example",
"i:foo[100000] .  Note that if you have more than one file in your",
"current directory that start with the file name, you may need to use",
"single quotes around the file name, for example 'a[1000]', because the",
"shell (bash or csh) takes the [] expression as pattern matching; if",
"you have a file a and a1, an a[1000] expression will be converted to",
"a1 but 'a[1000]' will be passed to 'ipc' as is.",
" ",
"For source files (both types, i: or local), if the offset is beyond",
"the end of the file, zero bytes will be copied and 'ipc' will display",
"a warning message and exit with an error (non-zero) status.  For",
"destination files (either type, i: or local) with an offset beyond the",
"end, the destination file will be extended.",
" ",
"Without a -s size option, the copying will go to the end of the source",
"file.",
" ",
"It is usually best to use 'iput' and 'iget' to copy full files as",
"those i-commands handle large data transfers more efficiently",
"(parallel I/O streams, etc) and use 'ipc' to read or write relatively",
"small portions of large files.  But without either a -s size or []",
"offsets, ipc will do a full file copy, like 'iget' or 'iput'.",
" ",
"If the destination exists, you must include the -f (force) option to",
"overwrite it (the portion being updated).",
" ",
"The -v (verbose) option causes 'ipc' to display the number of bytes",
"copied.  The -V (very verbose) option displays that, the standard",
"iRODS environment information, and some details about the operations",
"being performed (similar to a '-debug' option).",
" ",
"Examples:",
" ",
"ipc -fs 10000 myfile[10000000] i:myfile[10000000]",
"     Copy 10,000 bytes from byte postion 10,000,000 from local file",
"     myfile to iRODS file myfile at the same 10,000,000 byte position.",
"     The -f (force) is needed to open the existing iRODS file",
"     for writing.",
" ",
"ipc i:/tempZone/home/rods/logFile[100000] logFileTail",
"     Copy starting at postion 100,000 in iRDOS file ",
"     /tempZone/home/rods/logFile to local file logFileTail.",
""};

   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("ipc");
}
