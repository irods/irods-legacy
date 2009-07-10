/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/****************************************************************************

   This file contains all the constant definitions used by Rule Engine

*****************************************************************************/

#ifndef RE_DEFINES_H
#define RE_DEFINES_H

#define MAX_NUM_OF_RULES  2048
#define MAX_NUM_OF_DVARS  2048
#define MAX_NUM_OF_FMAPS  2048

#define MAX_COND_LEN      MAX_NAME_LEN
#define MAX_OPER_LEN       NAME_LEN
#define MAX_RULE_LENGTH   MAX_NAME_LEN * 4
#define MAX_DVAR_LENGTH   MAX_NAME_LEN * 4
#define MAX_FMAP_LENGTH   MAX_NAME_LEN * 4
#define MAX_ACTION_SIZE    MAX_NAME_LEN * 5
#define MAX_ACTION_IN_RULE  30
#define MAX_NUM_OF_ARGS_IN_ACTION 10
#define MAX_ERROR_STRING  MAX_NAME_LEN
#define RULE_SET_DEF_LENGTH  MAX_NAME_LEN
#define RETESTFLAG        "RETESTFLAG"
#define RELOOPBACKFLAG        "RELOOPBACKFLAG"
/** Flags for testing the Rule Execution **/
#define LOG_TEST_1             1
#define HTML_TEST_1            2
#define COMMAND_TEST_1         3
#define HTML_TEST_MSI          4
#define COMMAND_TEST_MSI       5
#define LOOP_BACK_1            1
/** Flags for testing the Rule Execution **/

/* Macro for the Re  Test Stub */


#define RE_TEST_MACRO(msg)      \
  if (reTestFlag > 0 ) {        \
    if (reTestFlag == LOG_TEST_1) {     \
      rodsLog (LOG_NOTICE, msg);        \
    }   \
    if (reLoopBackFlag > 0)     \
      return(0);        \
  }



/*
#define RE_TEST_MACRO(msg)      \
  if (reTestFlag > 0 ) {        \
     if (reTestFlag == LOG_TEST_1) {     \
        rodsLogAndErrorMsg (LOG_NOTICE,&(rei->rsComm->rError),-1, msg);        \
     }   \
    if (reLoopBackFlag > 0)     \
       return(0);        \
  }

*/
#endif /* RE_DEFINES_H */
