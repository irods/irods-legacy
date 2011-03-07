/* For copyright information please refer to files in the COPYRIGHT directory
 */

#ifndef ARITHMETICS_H
#define ARITHMETICS_H
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include "conversion.h"
#include "parser.h"
#include "hashtable.h"
#define POSIX_REGEXP

#ifndef DEBUG

#include "msParam.h"
#include "rodsDef.h"

#endif

#define RETURN {goto ret;}

/** AST evaluators */
Res* evaluateActions(Node *acti, Node *reco, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r);
Res* evaluateExpression3(Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r);
Res* evaluateVar3(char* vn, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res* evaluateFunction3(char* fn, Node** args, int nargs, Node *node, ruleExecInfo_t* rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r);
Res* execAction3(char *fn, Res** args, int nargs, Node *node, Env *env, ruleExecInfo_t* rei, int reiSaveFlag, rError_t *errmsg, Region *r);
Res* execMicroService3 (char *inAction, Res** largs, int largc, Node *node, Env *env, ruleExecInfo_t *rei, rError_t *errmsg, Region *r);
int execRule(char *ruleName, Res** args, int argc,Env *outEnv, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r);
int execRuleNodeRes(Node *rule, Res** args, int argc,Env *outEnv, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r);
int executeRuleActionNode(Node *inAction, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r);
int executeRuleRecoveryNode(Node *ruleRecovery, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r);
int executeRuleBodyNode(char *action, Node *ruleAction, Node *ruleRecovery,
                   Env *env,
                   ruleExecInfo_t *rei, int reiSaveFlag , rError_t *errmsg, Region *r);

Res* getSessionVar(char *action,  char *varName,  ruleExecInfo_t *rei, Env *env, rError_t *errmsg, Region *r);
Res* processCoercion(Node *node, Res *res, Hashtable *tvarEnv, rError_t *errmsg, Region *r);

/** utilities */
char* getVariableName(Node *node);
int overflow(char*expr,int len);
char* matchWholeString(char *buf);

#endif
