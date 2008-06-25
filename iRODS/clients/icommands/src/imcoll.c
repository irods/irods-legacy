/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* 
 * imcoll - The irods mounted collection utility that deals with 
 * mounting/unmounting and the management of mounted structured files.
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "mcollUtil.h"
void usage ();

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    rodsPathInp_t rodsPathInp;
    int nArgv;
    

    optStr = "hUm:R:spvV";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
	printf("use -h for help.\n");
        exit (1);
    }

    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    nArgv = argc - optind;

    if (myRodsArgs.mountCollection == True && nArgv != 2) {      
	/* must have 2 inputs */
        usage ();
        exit (1);
    } else if (nArgv < 1) {
        usage ();
        exit (1);
    }

    status = getRodsEnv (&myEnv);
    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    if (*argv[optind] != '/' || (myRodsArgs.mountCollection == True &&
      *argv[optind + 1] != '/')) { 
	rodsLog (LOG_ERROR,
	 "Input path must be absolute");
	exit (1);
    }

    if (myRodsArgs.mountCollection == True) {
        if (myRodsArgs.unmount == True) {
            rodsLog (LOG_ERROR, "main: the U and m cannot be used together. ");
            exit (2);
        }
        status = parseCmdLinePath (argc, argv, optind, &myEnv,
          UNKNOWN_FILE_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);
    } else {		/* unmount, sync, purge, etc */ 
        status = parseCmdLinePath (argc, argv, optind, &myEnv,
          UNKNOWN_FILE_T, NO_INPUT_T, 0, &rodsPathInp);
    }

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. "); 
	printf("use -h for help.\n");
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 1, &errMsg);

    if (conn == NULL) {
        exit (2);
    }
   
    status = clientLogin(conn);
    if (status != 0) {
       rcDisconnect(conn);
        exit (7);
    }

    status = mcollUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);

    rcDisconnect(conn);

    if (status < 0) {
	exit (3);
    } else {
        exit(0);
    }

}

void 
usage ()
{
   char *msgs[]={
"Usage : imcoll -m mountType [-R resource] mountDirectory|structuredFilePath",
"               irodsCollection",
"Usage : imcoll -Usp irodsCollection",
" ",
"Used to manage (mount, unmount, synchronize and purge of cache) mounted",
"iRODS collections and the associated cache.",
"Full path must be supplied for both physicalFilePath and irodsPath.",
" ",
"The -m option can be used to associate (mount) an iRods collection with a",
"a physical directory (e.g.,a UNIX directory) or a structured file.",
"If the mountType is 'f' or 'filesystem', the first argument is the ", 
"UNIX directory path to be mounted. Only the top level collection/directory",
"will be registered. The entire content of the registered directory can", 
"then be accessed using iRods commands such as iput, iget, ils and the",
"client APIs. This is simlilar to mounting a UNIX file system except that",
"a UNIX directory is mounted to an iRods collection. For example,", 
"the following command mounts the /temp/myDir UNIX directory to the",
" /tempZone/home/myUser/mymount collection:",
"    imcoll -m filesystem /temp/myDir /tempZone/home/myUser/mymount",
" ",
"An admin user will be able to mount any Unix directory. But for normal user,",
"he/she needs to have a UNIX account on the server with the same name as",
"his/her iRods user account and only UNIX directory created with this",
"account can be mounted by the user. Access control to the mounted data",
"will be based on the access permission of the mounted collection.",
" ",
"If the mountType is 't' or 'tar', the first argument is the iRods", 
"logical path of a tar file which will be used as the 'strucuted file'",
"for the mounted collection. The [-R resource] option is used to",
"specify the resource to create this tar structured file in case it",
"does not already exist. Once the tar structured file is mounted, the",
"content of the tar file can be accessed using iRods commands such as iput", 
"ils, iget, and the client APIs. For example, the following command mounts",
"the iRods tar file /tZone/home/myUser/tar/foo.tar to the",
"/tZone/home/myUser/tarcoll collection:",
"    imcoll -m tar /tZone/home/myUser/tar/foo.tar /tZone/home/myUser/tardir",
" ",
"The tar structured file implementation uses a cache on the server",
"to cache the mounted tar file. i.e., the tar file is untarred to a cache",
"on the server before any iRods operation. The 'ils -L' command can be use",
"to list the properties of a mounted collection and the status of the",
"associated cache. For example, the following is the output of the ils",
"command:",
" ",
"  C- /tZone/home/myUser/tardir  tarStructFile  /tZone/home/myUser/tar/foo.tar  /data/Vault8/rods/tar/foo.tar.cacheDir0;;;demoResc;;;1",
" ",
"The output shows that /tZone/home/myUser/tardir is a tar structured file",
"mounted collection. The iRods path of the tar file is in",
"/tZone/home/myUser/tar/foo.tar. The last item actually contains 3 items",
"separated the string ;;;. It showed that the tar file is untarred into",
"the /data/Vault8/rods/tar/foo.tar.cacheDir0 directory in the demoResc",
"resource. The value of '1' for the last item showed that the cache content",
"has been changed (dirty) and the original tar file needs be synchronized",
"with the changes.",
"The -s option can be used to synchronize the tar file with the cache.",
"For example:",
" ",
"    imcoll -s /tZone/home/myUser/tardir"
" ",
"The -p option can be used to purge the cache.",
" ",
"For example:",
" ",
"    imcoll -p /tZone/home/myUser/tardir",
" ",
"The -s and -p can be used together to synchronize the tar file and then",
"purge the cache.", 
" ",
"NOTE: To use the tar structured file, the libtar software must be installed.",
"The link: https://www.irods.org/index.php/Mounted_iRODS_Collection",
"gives instructions for installing libtar.",
" ",
"If the mountType is 'h' or 'haaw', the first argument is the logical path",
"of a  haaw type structured file developed by UK eScience.",
" ",
"NOTE: the haaw type structured file has NOT yet been implemented.",
" ",
"The -U option can be used to unmount an iRods collection. For example, the",
"following command unmounts the /tempZone/home/myUser/mymount collection:",
"    imcoll -U /tempZone/home/myUser/mymount",
" ",
" ",
"Options are:",
" -R  resource - specifies the resource to store to. This can also be specified",
"     in your environment or via a rule set up by the administrator.",
" -m  mountType - mount a directory or structured file to a collection",
"       Valid mountType are f|filesystem, t|tar and h|haaw",
" -U  unmount the collection",
" -s  synchronize the tar file with the cache",
" -p  purge the associated cache",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) return;
      printf("%s\n",msgs[i]);
   }
}
