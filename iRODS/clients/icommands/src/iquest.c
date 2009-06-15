/* 
 * ils - The irods ls utility
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "rcMisc.h"
#include "lsUtil.h"
void usage ();


void
usage () {
   char *msgs[]={
"Usage : iquest [ [hint] format]  selectConditionString ",
"format is C format restricted to character strings.",
"selectConditionString is of the form: SELECT <attribute> [, <attribute>]* [WHERE <condition> [ AND <condition>]*]",
"attribute can be found using 'iquest attrs' command",
"condition is of the form: <attribute> <rel-op> <value>",
"rel-op is a relational operator: eg. =, <>, >,<, like, not like, between, etc.,",
"value is either a constant or a wild-carded expression.",
"One can also use a few aggregation operatos such as sum,count,min,max and avg.",
"Use % and _ as wild-cards, and use \\ to escape them",
"Options are:",
" -h  this help",
"Examples:\n",
" iquest \"SELECT DATA_NAME, DATA_CHECKSUM WHERE DATA_RESC_NAME like 'demo%'\"",
" iquest \"For %-12.12s size is %s\" \"SELECT DATA_NAME ,  DATA_SIZE  WHERE COLL_NAME = '/tempZone/home/rods'\"",
" iquest \"SELECT COLL_NAME WHERE COLL_NAME like '/tempZone/home/%'\"",
" iquest \"User %-6.6s has %-5.5s access to file %s\" \"SELECT USER_NAME,  DATA_ACCESS_NAME, DATA_NAME WHERE COLL_NAME = '/tempZone/home/rods'\"",
" iquest \" %-5.5s access has been given to user %-6.6s for the file %s\" \"SELECT DATA_ACCESS_NAME, USER_NAME, DATA_NAME WHERE COLL_NAME = '/tempZone/home/rods'\"",
" iquest \"SELECT RESC_NAME, RESC_LOC, RESC_VAULT_PATH, DATA_PATH WHERE DATA_NAME = 't2' AND COLL_NAME = '/tempZone/home/rods'\"",
" iquest \"User %-9.9s uses %14.14s bytes in %8.8s files in '%s'\" \"SELECT USER_NAME, sum(DATA_SIZE),count(DATA_NAME),RESC_NAME\"",
" iquest \"select sum(DATA_SIZE) where COLL_NAME = '/tempZone/home/rods'\"",
" iquest \"select sum(DATA_SIZE) where COLL_NAME like '/tempZone/home/rods%'\"",
" iquest \"select sum(DATA_SIZE), RESC_NAME where COLL_NAME like '/tempZone/home/rods%'\"",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("iquest");
}

int
queryAndShowStrCond(rcComm_t *conn, char *hint, char *format, char *selectConditionString)
{

  genQueryInp_t genQueryInp;
  int i;
  genQueryOut_t *genQueryOut = NULL;

  memset (&genQueryInp, 0, sizeof (genQueryInp_t));
  i = fillGenQueryInpFromStrCond(selectConditionString, &genQueryInp);
  if (i < 0)
    return(i);

  genQueryInp.maxRows= MAX_SQL_ROWS;
  genQueryInp.continueInx=0;
  i = rcGenQuery (conn, &genQueryInp, &genQueryOut);
  if (i < 0)
    return(i);

  i = printGenQueryOut(stdout, format,hint,  genQueryOut);
  if (i < 0)
    return(i);

  return(0);

}

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    

    optStr = "h";
   
    status = parseCmdLineOpt (argc, argv, optStr, 0, &myRodsArgs);

    if (status < 0) {
        printf("Use -h for help\n");
        exit (1);
    }
    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }
    if (myRodsArgs.optind == argc) {
      printf("StringCondition needed\n");
      usage();
      exit(0);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    if (myRodsArgs.optind == 1) {
       if (!strncmp(argv[argc-1], "attrs", 5)) {
	  showAttrNames();
	  exit(0);
       }
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 0, &errMsg);

    if (conn == NULL) {
        exit (2);
    }

    status = clientLogin(conn);
    if (status != 0) {
       exit (3);
    }

    if (myRodsArgs.optind == (argc - 3)) 
      status = queryAndShowStrCond(conn, argv[argc-3], argv[argc-2], argv[argc-1]);
    else if (myRodsArgs.optind == (argc - 2)) 
      status = queryAndShowStrCond(conn, NULL, argv[argc-2], argv[argc-1]);
    else 
      status = queryAndShowStrCond(conn, NULL, NULL, argv[argc-1]);
    rcDisconnect(conn);

    if (status < 0) {
      rodsLogError(LOG_ERROR,status,"iquest Error: queryAndShowStrCond failed");
      exit (4);
    } else {
      exit(0);
    }

}
