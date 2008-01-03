/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* l3bundle.c - test the low level api */

#include "rodsClient.h" 


#define BUFSIZE	(1024*1024)

int
main(int argc, char **argv)
{
    rcComm_t *conn;
    rodsEnv myRodsEnv;
    int status;
    rErrMsg_t errMsg;
    subFile_t subFile;
    specColl_t specColl;
    bunSubFdOprInp_t bunSubFdOprInp;
    bytesBuf_t bunSubReadOutBBuf;
    rodsStat_t *rodsStat = NULL;
    bunSubLseekInp_t bunSubLseekInp;
    fileLseekOut_t *bunSubLseekOut = NULL;
    bunSubRenameInp_t bunSubRenameInp;
    rodsDirent_t *rodsDirent = NULL;

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
       exit (7);
    }

    memset (&subFile, 0, sizeof (subFile));
    memset (&specColl, 0, sizeof (specColl));
    subFile.specColl = &specColl;
    rstrcpy (specColl.collection, "/tempZone/home/rods/dir1", MAX_NAME_LEN);
    specColl.class = BUNDLE_COLL;
    specColl.type = HAAW_BUNDLE;
    rstrcpy (specColl.objPath, "/tempZone/home/rods/dir1/myBundle", 
      MAX_NAME_LEN);
    rstrcpy (specColl.resource, "demoResc", NAME_LEN);
    rstrcpy (specColl.phyPath, "/data/mwan/rods/Vault8/rods/dir1/myBundle", 
      MAX_NAME_LEN);
    rstrcpy (subFile.subFilePath, "/tempZone/home/rods/dir1/mySubFile",
      MAX_NAME_LEN);
    rstrcpy (subFile.addr.hostAddr, "srbbrick8.sdsc.edu", NAME_LEN);

    status = rcBunSubOpen (conn, &subFile);

    printf ("Porcessed rcBunSubOpen, status = %d\n", status);

    status = rcBunSubCreate (conn, &subFile);
    
    printf ("Porcessed rcBunSubCreate, status = %d\n", status);

    memset (&bunSubFdOprInp, 0, sizeof (bunSubFdOprInp));

    rstrcpy (bunSubFdOprInp.addr.hostAddr, "srbbrick8.sdsc.edu", NAME_LEN);
    bunSubFdOprInp.type = HAAW_BUNDLE;
    bunSubFdOprInp.fd = 10;
    bunSubFdOprInp.len = 1000;

    memset (&bunSubReadOutBBuf, 0, sizeof (bunSubReadOutBBuf));
    bunSubReadOutBBuf.buf = malloc (bunSubFdOprInp.len);

    status = rcBunSubRead (conn, &bunSubFdOprInp, &bunSubReadOutBBuf);

    printf ("Porcessed rcBunSubRead, status = %d\n", status);

    rstrcpy ((char *) bunSubReadOutBBuf.buf, "This is a test from client", 
      bunSubFdOprInp.len);

    bunSubReadOutBBuf.len = bunSubFdOprInp.len = 
      strlen ("This is a test from client") + 1;
    status = rcBunSubWrite (conn, &bunSubFdOprInp, &bunSubReadOutBBuf);

    printf ("Porcessed rcBunSubWrite, status = %d\n", status);

    status = rcBunSubClose (conn, &bunSubFdOprInp);

    printf ("Porcessed rcBunSubClose, status = %d\n", status);

    status = rcBunSubStat (conn, &subFile, &rodsStat);

    printf ("Porcessed rcBunSubStat, status = %d\n", status);

    if (rodsStat != NULL) {
	free (rodsStat);
	rodsStat = NULL;
    }

    status = rcBunSubFstat (conn, &bunSubFdOprInp, &rodsStat);

    printf ("Porcessed rcBunSubFstat, status = %d\n", status);
    
    if (rodsStat != NULL) {
        free (rodsStat);
        rodsStat = NULL;
    }

    memset (&bunSubLseekInp, 0, sizeof (bunSubLseekInp));

    rstrcpy (bunSubLseekInp.addr.hostAddr, "srbbrick8.sdsc.edu", NAME_LEN);
    bunSubLseekInp.type = HAAW_BUNDLE;
    bunSubLseekInp.fd = 10;
    bunSubLseekInp.offset = 10000;
    bunSubLseekInp.whence = 1;

    status = rcBunSubLseek (conn, &bunSubLseekInp, &bunSubLseekOut);

    printf ("Porcessed rcBunSubLseek, status = %d\n", status);

    if (bunSubLseekOut != NULL) {
        free (bunSubLseekOut);
        bunSubLseekOut = NULL;
    }

    memset (&bunSubRenameInp, 0, sizeof (bunSubRenameInp));
    bunSubRenameInp.subFile.specColl = &specColl;
    rstrcpy (bunSubRenameInp.subFile.subFilePath, 
      "/tempZone/home/rods/dir1/mySubFile", MAX_NAME_LEN);
    rstrcpy (bunSubRenameInp.subFile.addr.hostAddr, "srbbrick8.sdsc.edu", 
      NAME_LEN);
    rstrcpy (bunSubRenameInp.newSubFilePath, 
      "/tempZone/home/rods/dir1/mySubFile2", MAX_NAME_LEN);

    status = rcBunSubRename (conn, &bunSubRenameInp);

    printf ("Porcessed rcBunSubRename, status = %d\n", status);
    
    status = rcBunSubUnlink (conn, &subFile);
  
    printf ("Porcessed rcBunSubUnlink, status = %d\n", status);

    subFile.offset = 1000;
    status = rcBunSubTruncate (conn, &subFile);

    printf ("Porcessed rcBunSubTruncate, status = %d\n", status);

    status = rcBunSubMkdir (conn, &subFile);

    printf ("Porcessed rcBunSubMkdir, status = %d\n", status);

    status = rcBunSubRmdir (conn, &subFile);
    
    printf ("Porcessed rcBunSubRmdir, status = %d\n", status);

    status = rcBunSubOpendir (conn, &subFile);
   
    printf ("Porcessed rcBunSubOpendir, status = %d\n", status);

    status = rcBunSubReaddir (conn, &bunSubFdOprInp, &rodsDirent);
  
    printf ("Porcessed rcBunSubReaddir, status = %d\n", status);

    status = rcBunSubClosedir (conn, &bunSubFdOprInp);

    printf ("Porcessed rcBunSubClosedir, status = %d\n", status);

    rcDisconnect (conn);
} 

