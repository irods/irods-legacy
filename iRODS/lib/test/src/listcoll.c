/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* l1rm.c - test the high level api */

#include "rodsClient.h" 
#include "miscUtil.h" 

#define USER_NAME	"rods"
#define RODS_ZONE	"temp"

/* NOTE : You have to change FILE_NAME, PAR_DIR_NAME, DIR_NAME and ADDR
 * for your own env */


#define DEST_RESC_NAME  "demoResc"
#define RESC_NAME   	"demoResc"

int
printCollection (rcComm_t *conn, collHandle_t *collHandle);

int
main(int argc, char **argv)
{
    rcComm_t *conn;
    rodsEnv myEnv;
    char *optStr;
    rErrMsg_t errMsg;
    int status;
    rodsArguments_t rodsArgs;
    int flag = 0;
    collHandle_t collHandle;

    optStr = "l";

    status = parseCmdLineOpt (argc, argv, optStr, 1, &rodsArgs);

    if (status < 0) {
        printf("parseCmdLineOpt error, status = %d.\n", status);
        exit (1);
    }

    if (argc - optind <= 0) {
        rodsLog (LOG_ERROR, "no input");
        exit (2);
    }

    memset (&errMsg, 0, sizeof (rErrMsg_t));

    status = getRodsEnv (&myEnv);

    if (status < 0) {
	fprintf (stderr, "getRodsEnv error, status = %d\n", status);
	exit (1);
    }


    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, USER_NAME,
      RODS_ZONE, 0, &errMsg);

    if (conn == NULL) {
        fprintf (stderr, "rcConnect error\n");
        exit (1);
    }

    status = clientLogin(conn);
    if (status != 0) {
        rcDisconnect(conn);
        exit (7);
    }

    if (rodsArgs.longOption == True) flag |= VERY_LONG_METADATA_FG; 

    status = rclOpenCollection (conn, argv[optind], flag, &collHandle);

    if (status < 0) {
        fprintf (stderr, "rclOpenCollection of %s error. status = %d\n",
	  argv[optind], status);
        exit (1);
    }

    status = printCollection (conn, &collHandle);
    rclCloseCollection (&collHandle);

    rcDisconnect (conn);

    exit (0);
} 

int
printCollection (rcComm_t *conn, collHandle_t *collHandle)
{
    int status;
    collEnt_t collEnt;

    while ((status = rclReadCollection (conn, collHandle, &collEnt)) >= 0) {
        if (collEnt.objType == DATA_OBJ_T) {
	    printf ("D - %s/%s\n", collEnt.collName, collEnt.dataName);
	    printf ("      dataId - %s\n", collEnt.dataId);
	    if ((collHandle->flag & LONG_METADATA_FG) > 0) {
	        printf ("      ownerName - %s\n", collEnt.ownerName);
		printf ("      createTime (UNIX clock in string) - %s\n", 
		  collEnt.createTime);
		printf ("      modifyTime (UNIX clock in string) - %s\n", 
		  collEnt.modifyTime);
		printf ("      chksum - %s\n", collEnt.chksum);
		printf ("      phyPath - %s\n", collEnt.phyPath);
	        printf ("      resource - %s\n", collEnt.resource);
		printf ("      replStatus - %d\n", collEnt.replStatus);
		printf ("      replNum - %d\n", collEnt.replNum);
		printf ("      dataSize - %lld\n", collEnt.dataSize);
	    }
	} else if (collEnt.objType == COLL_OBJ_T) {
	    collHandle_t subCollhandle;
	    printf ("C - %s\n", collEnt.collName);
	    printf ("      collOwner %s\n", collEnt.ownerName);
	    /* recursive print */
            status = rclOpenCollection (conn, collEnt.collName, 
	      collHandle->flag, &subCollhandle);

            if (status < 0) {
                fprintf (stderr, "rclOpenCollection of %s error. status = %d\n",
                  collEnt.collName, status);
	    }

	    printCollection (conn, &subCollhandle);
	    rclCloseCollection (&subCollhandle);
	}
    }

    if (status < 0 && status != CAT_NO_ROWS_FOUND) {
        return (status);
    } else {
	return (0);
    }
}
	
