/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* rsRe.c - Routines for Server interfacing to the Rule Engine
 */

#include "rsGlobalExtern.h"
#include "reGlobalsExtern.h"

static char ruleSetInitialized[NAME_LEN]="";

/* initialize the Rule Engine if it hasn't been done yet */
int 
initRuleEngine(char *ruleSet, char *dvmSet, char* fnmSet) {
   int status;
   if (strcmp(ruleSet, ruleSetInitialized)==0) {
      return(0); /* already done */
   }
   status = initRuleStruct(ruleSet, dvmSet, fnmSet);
   if (status == 0) {
      rstrcpy(ruleSetInitialized, ruleSet, NAME_LEN);
   }
   return(status);
}
