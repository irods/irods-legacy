/* 
 * irule - The irods utility to execute user composed rules.
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "getUtil.h"
void usage ();

int
parseMsInputParam (int argc, char **argv, int optInd, 
		   execMyRuleInp_t *execMyRuleInp, char *inBuf);

int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;
    execMyRuleInp_t execMyRuleInp;
    msParamArray_t *outParamArray = NULL;
    msParamArray_t msParamArray;

    int connFlag = 0;
    char saveFile[100];

    optStr = "ZhlvF:";
   
    status = parseCmdLineOpt (argc, argv, optStr, 1, &myRodsArgs);

    if (status < 0) {
	printf("Use -h for help.\n");
        exit (1);
    }
    if (myRodsArgs.help==True) {
        usage();
        exit(0);
    }

    memset (&execMyRuleInp, 0, sizeof (execMyRuleInp));
    memset (&msParamArray, 0, sizeof (msParamArray));
    execMyRuleInp.inpParamArray = &msParamArray;
    execMyRuleInp.condInput.len=0;

    if (myRodsArgs.test == True) {
       addKeyVal(&execMyRuleInp.condInput,"looptest","true");
    }
    if (myRodsArgs.file == True) { /* input file */
	FILE *fptr;
	int len;
	int gotRule = 0;
	char buf[META_STR_LEN];
	/*** RAJA ADDED TO USE INPUT FILE FROM AN iRODS OBJECT ***/
	if (!strncmp(myRodsArgs.fileString,"i:",2)) {
	  status = getRodsEnv (&myEnv);
	  
	  if (status < 0) {
	    rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
	    exit (1);
	  }
	  
	  conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
			    myEnv.rodsZone, 0, &errMsg);
	  
	  if (conn == NULL) {
	    exit (2);
	  }
	  
	  status = clientLogin(conn);
	  if (status != 0) {
	    rcDisconnect(conn);
	    exit (7);
	  }
	  if (status == 0) {
	    char *myargv[3];
	    int myargc, myoptind;
	    rodsPathInp_t rodsPathInp;
	    connFlag = 1;
	    
	    myargv[0] = strdup(myRodsArgs.fileString+2);
	    myargv[1] = saveFile;
	    myargc = 2;
	    myoptind = 0;
	    snprintf(saveFile,99,"/tmp/tmpiruleFile.%i.%i.ir",(unsigned int) time(0),getpid());
	    status = parseCmdLinePath (myargc,myargv,myoptind,&myEnv,
				       UNKNOWN_OBJ_T, UNKNOWN_FILE_T, 0, &rodsPathInp);
	    status = getUtil (conn, &myEnv, &myRodsArgs, &rodsPathInp);
	    if (status < 0) {
	      rcDisconnect(conn);
	      exit (3);
	    }
	    myRodsArgs.fileString = saveFile;
	    connFlag = 1;
	  }
	}
	/*** RAJA ADDED TO USE INPUT FILE FROM AN iRODS OBJECT ***/

	fptr = fopen (myRodsArgs.fileString, "r");

        if (fptr == NULL) {
            rodsLog (LOG_ERROR,
              "Cannot open input file %s. ernro = %d\n",
              myRodsArgs.fileString, errno);
	    exit (1);
	}

	while ((len = getLine (fptr, buf, META_STR_LEN)) > 0) {
	    if (buf[0] == '#') {
		continue;
	    }
	  if (myRodsArgs.longOption == True)
	    puts(buf);

	    if (gotRule == 0) {
		/* the input is a rule */
		rstrcpy (execMyRuleInp.myRule, buf, META_STR_LEN);
	    } else if (gotRule == 1) {
		parseMsInputParam (argc, argv, optind, &execMyRuleInp, buf);
	    } else if (gotRule == 2) {
                if (strcmp (buf, "null") != 0) {
        	    rstrcpy (execMyRuleInp.outParamDesc, buf, LONG_NAME_LEN);
		}
	        break;
	    } else {
		break;
	    }
	    gotRule++;
	}
	if (myRodsArgs.longOption == True)
	    puts("-----------------------------------------------------------------");

	if (gotRule != 2) {
	    rodsLog (LOG_ERROR, "Incomplete rule input for %s",
	      myRodsArgs.fileString);
	    exit (2);
	} 
	/*** RAJA ADDED TO USE INPUT FILE FROM AN iRODS OBJECT ***/
	if (connFlag == 1) {
	  fclose(fptr);
	  unlink(saveFile);
	}
	/*** RAJA ADDED TO USE INPUT FILE FROM AN iRODS OBJECT ***/
    } else {	/* command line input */
	int nArg = argc - optind;
        if (nArg < 3) {
            rodsLog (LOG_ERROR, "no input");
            printf("Use -h for help.\n");
            exit (3);
        }
	rstrcpy (execMyRuleInp.myRule, argv[optind], META_STR_LEN);
	parseMsInputParam (0,NULL, 0, &execMyRuleInp, argv[optind + 1]);
	if (strcmp (argv[optind + 2], "null") != 0) {
            rstrcpy (execMyRuleInp.outParamDesc, argv[optind + 2], 
	      LONG_NAME_LEN);
	}
    }


    if (connFlag == 0) {
      status = getRodsEnv (&myEnv);
      
      if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
      }

      conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
			myEnv.rodsZone, 0, &errMsg);
      
      if (conn == NULL) {
        rodsLogError (LOG_ERROR, errMsg.status, "rcConnect failure %s",
		      errMsg.msg);
        exit (2);
      }

      status = clientLogin(conn);
      if (status != 0) {
	rcDisconnect(conn);
	exit (7);
      }
    }
    if (myRodsArgs.verbose == True) {
        printf ("rcExecMyRule: %s\n", execMyRuleInp.myRule);
	printf ("outParamDesc: %s\n", execMyRuleInp.outParamDesc);
    }

    status = rcExecMyRule (conn, &execMyRuleInp, &outParamArray);

   if (myRodsArgs.test == True) {
      printErrorStack (conn->rError);
    }
 
    if (status < 0) {
      msParam_t *mP;
      execCmdOut_t *execCmdOut;
      rodsLogError (LOG_ERROR, status, "rcExecMyRule error. ");
      printErrorStack (conn->rError); 
      if ((mP = getMsParamByType (outParamArray, ExecCmdOut_MS_T)) != NULL) {
	execCmdOut = (execCmdOut_t *) mP->inOutStruct;
	if (execCmdOut->stdoutBuf.buf != NULL) 
	  fprintf(stdout,"%s", (char *) execCmdOut->stdoutBuf.buf);
	if (execCmdOut->stderrBuf.buf != NULL) 
	  fprintf(stderr,"%s",(char *) execCmdOut->stderrBuf.buf);
      }
	rcDisconnect(conn);
	exit (4);
    }

    if (myRodsArgs.verbose == True) {
        printf ("ExecMyRule completed successfully.    Output \n\n");
        printMsParam (outParamArray);
    }
    else {
      msParam_t *mP;
      execCmdOut_t *execCmdOut;
      if ((mP = getMsParamByType (outParamArray, ExecCmdOut_MS_T)) != NULL) {
	execCmdOut = (execCmdOut_t *) mP->inOutStruct;
	if (execCmdOut->stdoutBuf.buf != NULL) 
	  fprintf(stdout,"%s",(char *) execCmdOut->stdoutBuf.buf);
	if (execCmdOut->stderrBuf.buf != NULL) 
	  fprintf(stderr,"%s", (char *) execCmdOut->stderrBuf.buf);
      }
    }
    rcDisconnect(conn);
    exit(0);	

}

int
parseMsInputParam (int argc, char **argv, int optInd, 
		   execMyRuleInp_t *execMyRuleInp, char *inBuf)
{
    strArray_t strArray;
    int status, i,j;
    char *value;
    int nInput;
    char line[NAME_LEN];
    int promptF = 0;
    int labelF = 0;
    if (inBuf == NULL || strcmp (inBuf, "null") == 0) {
	execMyRuleInp->inpParamArray = NULL;
	return (0);
    }

    nInput = argc - optInd;
    memset (&strArray, 0, sizeof (strArray));

    status = parseMultiStr (inBuf, &strArray);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "parseMsInputParam: parseMultiStr error, status = %d", status);
	execMyRuleInp->inpParamArray = NULL;
        return (status);
    }
    value = strArray.value;
    /* each string is supposed to have to format label=value */
    for (i = 0; i < nInput; i++ ) {
      /* using the values from the input line following -F <filename> */
      if (!strcmp(argv[optInd + i],"prompt")) {
	promptF = 1;
	break;
      }
      if (!strcmp(argv[optInd + i],"default") || strlen(argv[optInd + i]) == 0) 
	continue;
      else if (*argv[optInd + i] == '*') {
	char *tmpPtr;
	if (i > 0 && labelF == 0)
	  return(CAT_INVALID_ARGUMENT);
	labelF = 1;
	if ((tmpPtr = strstr(argv[optInd + i],"=")) == NULL) 
	  return(CAT_INVALID_ARGUMENT);
	*tmpPtr = '\0';
	for (j = 0; j < strArray.len; j++) {
	  if (strstr(&value[j * strArray.size],argv[optInd + i]) == &value[j * strArray.size]){
	    *tmpPtr = '=';
	    rstrcpy (&value[j * strArray.size],argv[optInd + i],NAME_LEN);
	    break;
	  }
	}
	if (j == strArray.len)
	  printf("Ignoring Argument \"%s\"",argv[optInd + i]);
      }
      else {
	char *valPtr = &value[i * strArray.size];
	char *tmpPtr;
	if (labelF == 1)
	  return(CAT_INVALID_ARGUMENT);
	if ((tmpPtr = strstr (valPtr, "=")) != NULL) {
	  tmpPtr++;
	  rstrcpy (tmpPtr,argv[optInd + i], (int) NAME_LEN - (tmpPtr - valPtr + 1));
	}
      }
    }

    for (i = 0; i < strArray.len; i++) {
	char *valPtr = &value[i * strArray.size];
	char *tmpPtr;
	
	if ((tmpPtr = strstr (valPtr, "=")) != NULL) {
	    *tmpPtr = '\0';
	    tmpPtr++;
	    /** RAJA Jul 12 2007 changed it so that it can take input values from terminal
	    addMsParam (execMyRuleInp->inpParamArray, valPtr, STR_MS_T, 
	                strdup (tmpPtr), NULL);
            ** RAJA Jul 12 2007 changed it so that it can take input values from terminal **/
	    if (*tmpPtr == '$' ) {
	      /* If $ is used as a value in the input file for label=value then
		 the remaining command line arguments are taken as values.
		 If no command line arguments are given then the user is prompted
		 for the input value 
	      */
	      printf("Default %s=%s\n    New %s=",valPtr,tmpPtr+1,valPtr);
	      if (fgets(line,NAME_LEN,stdin) == NULL)
		return(CAT_INVALID_ARGUMENT);
	      if ((line[strlen(line)-1] = '\n')) 
		line[strlen(line)-1] = '\0';
	      if (strlen(line) == 0)
		addMsParam (execMyRuleInp->inpParamArray, valPtr,STR_MS_T,
			  strdup(tmpPtr+1), NULL);
	      else
		addMsParam (execMyRuleInp->inpParamArray, valPtr, STR_MS_T,
			  strdup(line), NULL);
	    }      
	    else if ( promptF == 1) {
	      /* the user has asked for prompting */
	      printf("Current %s=%s\n    New %s=",valPtr,tmpPtr,valPtr);
	      if (fgets(line,NAME_LEN,stdin) == NULL)
		return(CAT_INVALID_ARGUMENT);
	      if ((line[strlen(line)-1] = '\n')) 
		line[strlen(line)-1] = '\0';
	      if (strlen(line) == 0)
		addMsParam (execMyRuleInp->inpParamArray, valPtr,STR_MS_T,
			  strdup(tmpPtr), NULL);
	      else
		addMsParam (execMyRuleInp->inpParamArray, valPtr, STR_MS_T,
			  strdup(line), NULL);
	    }
	    else if (*tmpPtr == '\\') {
	      /* first '\'  is skipped. 
		 If you need to use '\' in the first letter add an additional '\'
		 if you have to use '$' in the first letter add a '\'  before that
	      */
	      tmpPtr++;
	      addMsParam (execMyRuleInp->inpParamArray, valPtr, STR_MS_T,
			  strdup (tmpPtr), NULL);
	    }
	    else {
	      addMsParam (execMyRuleInp->inpParamArray, valPtr,STR_MS_T,
			  strdup(tmpPtr), NULL);
	    }
	} else {
	    rodsLog (LOG_ERROR,
             "parseMsInputParam: inpParam %s format error", valPtr);
	}
    }

    return (0);
}
 


void 
usage ()
{
   char *msgs[]={
"Usage : irule [--test] [-v] rule inputParam outParamDesc",
"Usage : irule [--test] [-v] [-l] -F inputFile [prompt | arg_1 arg_2 ...]",
"Submit a user defined rule to be executed by an irods server.",
"The first form requires 3 inputs: ",
"    1) rule - This is the rule to be executed.", 
"    2) inputParam - The input parameters. The input values for the rule is",
"       specified here. If there is no input, a string containing \"null\"",
"       must be specified.",
"    3) outParamDesc - Description for the set of output parmeters to be ",
"       returned. If there is no output, a string containing \"null\"", 
"       must be specified.",
"The second form reads the rule and arguments from the file: inputFile",
"    The first (non-comment) line is the rule.",
"    The remaining arguments are interpreted as input arguments for the rule.",
"    If prompt is the first remaining argument, the user will be prompted for values.",
"    The current value wil be shown and used if the user just presses return.",
"    Otherwise, the arguments are interpreted in two ways",
"    In the first way, the arguments have \"label=value\" format and only those given",
"    label-value pairs are replaced and other pairs are taken from the inputFile.",
"    All labels start with *.",
"    Alternatively, one can give all arguments as inputs without any labels",
"    In such a case the keyword default can be used to use the inputFile value",
"    Use \\ as the first letter in an argument as an escape.",
"The inputFile should contain 3 lines, the first line specifies the rule,",
"the second line the input arguments as label=value pairs separated by % and ",
"the third line contains output parameters as labels again separated by %.",
"If % is needed in an input value use %%.",
"A value of an input argument can be $. In such a case the user will be prompted.",
"One can provide a default value by giving it right after the $. In such a case, ",
"the value will be shown and used if the user presses return without giving a value.",
"The input or the output line can be just be the word null if no input or output is needed.",
"An example of the input is given in the file:",
"    clients/icommands/test/ruleInp1",
"In either form, the 'rule' is either a rule name or a rule definition (which may be a complete",
"rule or a subset).",
"To view the output (outParamDesc), use the -v option.",
"See ruleInp1 for an example outParamDesc.",
"Options are:",
" --test enable test mode so that the microservices are not executed, instead a loopback is performed",
" -F  inputFile - read the file for the input",
"     if a prefix i: is attached, the the file is fetched from iRODS collections",  
" -l  list file if -F option is used",
" -v  verbose",
" -h  this help",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) return;
      printf("%s\n",msgs[i]);
   }
}
