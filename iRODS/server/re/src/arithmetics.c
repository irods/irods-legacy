/* For copyright information please refer to files in the COPYRIGHT directory
 */

#include "utils.h"
#include "parser.h"
#include "arithmetics.h"
#include "datetime.h"
#include "index.h"
#include "rules.h"
#include "functions.h"

#ifndef DEBUG
#include "regExpMatch.h"
#include "rodsErrorTable.h"
typedef struct {
  char action[MAX_ACTION_SIZE];
  int numberOfStringArgs;
  funcPtr callAction;
} microsdef_t;
extern int NumOfAction;
extern microsdef_t MicrosTable[];
#endif

#define ERROR(x, y) if(x) {if((y)!=NULL){(y)->type.t=ERROR;*errnode=node;}return;}
#define OUTOFMEMORY(x, res) if(x) {(res)->value.e = OUT_OF_MEMORY;TYPE(res) = ERROR;return;}

#define ERROR2(x,y) if(x) {localErrorMsg=(y);goto error;}
extern int GlobalAllRuleExecFlag;
extern int GlobalREDebugFlag;


int overflow(char* expr, int len) {
	int i;
	for(i = 0;i<len+1;i++) {
		if(expr[i]==0)
			return 0;
	}
	return 1;
}

Res* evaluateExpression3(Node *expr, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r) {
/*
    printTree(expr, 0);
*/
        char errbuf[ERR_MSG_LEN];
	char *oper1;
        Res *res = newRes(r);
	switch(expr->type) {
		case INT:
			res->type = newSimpType(T_INT,r);
			res->value.d=atof(expr->text);
                        break;
		case DOUBLE:
			res->type = newSimpType(T_DOUBLE,r);
			res->value.d=atof(expr->text);
                        break;
		case STRING:
			res = newStringRes(r, expr->text);
                        break;
		case TEXT:
			if(expr->text[0]=='$' || expr->text[0]=='*') {
				res = evaluateVar3(expr->text, expr, rei, reiSaveFlag,  env, errmsg,r);
                                break;
			} else if(strcmp(expr->text,"nop")==0) {
				res->type = newSimpType(T_INT,r);
				res->value.d = 0;
                                break;
			}

			// not a variable, evaluate as a function
		case APPLICATION:
			oper1 = expr->text;
                        // try to evaluate as a function,
/*
                        printf("start execing %s\n", oper1);
                        printEnvToStdOut(env);

*/
                        res = evaluateFunction3(oper1, expr->subtrees, expr->degree, expr, rei, reiSaveFlag, env, errmsg,r);
/*
                        printf("finish execing %s\n", oper1);
                        printEnvToStdOut(env);
*/
                        break;
		case ACTIONS:
                        generateErrMsg("error: evaluate actions using function evaluateExpression3, use function evaluateActions instead.", expr->expr, expr->base, errbuf);
                        addRErrorMsg(errmsg, -1, errbuf);
                        res = newErrorRes(r, -1);
                        break;
                default:
                        generateErrMsg("error: unsupported ast node type.", expr->expr, expr->base, errbuf);
                        addRErrorMsg(errmsg, -1, errbuf);
                        res = newErrorRes(r, -1);
                        break;
	}
        // coercions are applied at application locations only
        return res;
}

Res* processCoercion(Node *node, Res *res, Hashtable *tvarEnv, rError_t *errmsg, Region *r) {
        char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
        char *buf2;
        char buf3[ERR_MSG_LEN];
        // we can ignore non top level type constructors therefore, we only need to call dereference rather than instantiate
        ExprType *coercion = dereference(node->coercion, tvarEnv, r);
        if(coercion->t == T_VAR) {
            if(T_VAR_NUM_DISJUNCTS(coercion) == 0) {
                printf("error");
            }
            // here T_VAR must be a set of bounds
            // we fix the set of bounds to the default bound
            ExprType *defaultType = newSimpType(T_VAR_DISJUNCT(coercion,0), r);
            updateInHashTable(tvarEnv, getTVarName(coercion->ext.tvar.vid, buf), defaultType);
            coercion = defaultType;
        }
        if(typeEqSyntatic(coercion, res->type)) {
            return res;
        } else {
            switch(coercion->t) {
                case T_DYNAMIC:
                    return res;
                case T_INT:
                    switch(TYPE(res) ) {
                        case T_DOUBLE:
                        case T_BOOL:
                            if((int)res->value.d!=res->value.d) {
                                generateErrMsg("error: dynamic type conversion  double -> int: the double is not an integer", node->expr, node->base, buf);
                                addRErrorMsg(errmsg, -1, buf);
                                return newErrorRes(r, -1);
                            } else {
                                return newIntRes(r, (int)res->value.d);
                            }
                        case T_STRING:
                            return newIntRes(r, atoi(res->value.s.pointer));
                        default:
                            break;
                    }
                    break;
                case T_DOUBLE:
                    switch(TYPE(res) ) {
                        case T_INT:
                        case T_BOOL:
                            return newDoubleRes(r, res->value.d);
                        case T_STRING:
                            return newDoubleRes(r, atof(res->value.s.pointer));
                        default:
                            break;
                    }
                    break;
                case T_STRING:
                    switch(TYPE(res) ) {
                        case T_INT:
                        case T_DOUBLE:
                        case T_BOOL:
                            buf2 = convertResToString(res);

                            res = newStringRes(r, buf2);
                            free(buf2);
                            return res;
                        default:
                            break;
                    }
                    break;
                case T_BOOL:
                    switch(TYPE(res) ) {
                        case T_INT:
                        case T_DOUBLE:
                            return newBoolRes(r, res->value.d);
                        case T_STRING:
                            if(strcmp(res->value.s.pointer, "true")==0) {
                                return newBoolRes(r, 1);
                            } else if(strcmp(res->value.s.pointer, "false")==0) {
                                return newBoolRes(r, 0);
                            } else {
                                generateErrMsg("error: dynamic type conversion  string -> bool: the string is not in {true, false}", node->expr, node->base, buf);
                                addRErrorMsg(errmsg, -1, buf);
                                return newErrorRes(r, -1);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case T_CONS:
                    // we can ignore the not top level type constructor and leave type checking to when it is derefed
                    switch(TYPE(res)) {
                        case T_CONS:
                            return res;
                        case T_IRODS:
                            if(strcmp(res->type->ext.irods.name, IntArray_MS_T) == 0 ||
                               strcmp(res->type->ext.irods.name, StrArray_MS_T) == 0 ||
                               strcmp(res->type->ext.irods.name, GenQueryOut_MS_T) == 0) {
                                return res;
                            }
                        default:
                            break;
                    }
                case T_DATETIME:
                    switch(TYPE(res)) {
                        case T_INT:
                            newDatetimeRes(r, (time_t) res->value.d);
                        default:
                            break;
                    }
                default:
                    break;
            }
            Res mock;
            mock.type = coercion;
            snprintf(buf, ERR_MSG_LEN, "error: coerce from type %s to type %s", typeName_Res(res), typeName_Res(&mock));
            generateErrMsg(buf, node->expr, node->base, buf3);
            addRErrorMsg(errmsg, -1, buf3);
            TYPE(res)=T_ERROR;
            res->value.e = -1;
            return res;
        }
}

Res* evaluateActions(Node *expr, Node *reco, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r) {
/*
    printTree(expr, 0);
*/
        int i;
	switch(expr->type) {
		case ACTIONS:
                    for(i=0;i<expr->degree;i++) {
                            Node *nodei = expr->subtrees[i];
                            Res* res = evaluateExpression3(nodei, rei, reiSaveFlag, env, errmsg,r);
                            if(TYPE(res) == T_INT && res->value.d < 0) {
                                return newErrorRes(r, (int)res->value.d);
                            }
                            if(TYPE(res) == T_ERROR) {
                                // run recovery chain
                                if(reco!=NULL) {
                                    int i2;
                                    for(i2 = reco->degree-1>i?reco->degree-1:i;i2>=0;i2--) {
                                        evaluateExpression3(reco->subtrees[i2], rei, reiSaveFlag, env, errmsg, r);
                                    }
                                }
                                return res;
                            }
                            if(TYPE(res) == T_BREAK) {
                                return res;
                            } else
                            if(TYPE(res) == T_SUCCESS) {
                                return res;
                            }

                    }
                    return newIntRes(r, 0);
            default:
                break;
	}
        char errbuf[ERR_MSG_LEN];
        generateErrMsg("error: unsupported ast node type.", expr->expr, expr->base, errbuf);
	addRErrorMsg(errmsg, -1, errbuf);
	return newErrorRes(r, -1);
}

/**
 * evaluate function
 * provide env and region isolation for rules and external microservices
 * precond n <= MAX_PARAMS_LEN
 */
Res* evaluateFunction3(char* fn, Node** subtrees, int n, Node *node, ruleExecInfo_t* rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
    char buf2[ERR_MSG_LEN];
    #ifdef DEBUG
    sprintf(buf, "Action: %s\n", fn);
    writeToTmp("eval.log", buf);
    #endif

	int i;
	void* args[MAX_PARAMS_LEN];
        Res* res;
        Region *newRegion = make_region(0, NULL);
        Env *nEnv = newEnv(newHashTable(100), env->global, env->funcDesc);
        Hashtable *localTVarEnv = NULL;
        
        // look up function descriptor
        FunctionDesc *fd = lookupFromHashTable(env->funcDesc, fn);
        char *vOrE, desc[MAX_PARAMS_LEN];
        vOrE = desc;
        if(fd!=NULL) {
            // find matching arity
            int found = 0;
            while(fd!=NULL) {
                if((!strchr(fd->inOutValExp, '*') && !strchr(fd->inOutValExp, '+') && strlen(fd->inOutValExp) == n) ||
                        (strchr(fd->inOutValExp, '*') && strlen(fd->inOutValExp) - 2 <= n) ||
                        (strchr(fd->inOutValExp, '+') && strlen(fd->inOutValExp) - 1 <= n)) {
                    found = 1;
                    break;
                } else {
                    fd = fd->next;
                }
            }
            if(!found) {
                snprintf(buf, ERR_MSG_LEN, "error: arity mismatch for function %s", fn);
                generateErrMsg(buf, node->expr, node->base, buf2);
                addRErrorMsg(errmsg, -1, buf2);
                res = newErrorRes(r, -1);
                RETURN;
            }
        }
        if(fd!=NULL) {
            strncpy(vOrE, fd->inOutValExp, MAX_PARAMS_LEN);
        } else { // dynamically generate descriptor string based on the subtrees
            for(i=0;i<n;i++) {
                if(isLocalVariableNode(subtrees[i])) {
                    if(lookupFromHashTable(env->current, subtrees[i]->text)!=NULL ||
                       lookupFromHashTable(env->global, subtrees[i]->text)!=NULL     ) {
                        vOrE[i] = 'p';
                    } else {
                        vOrE[i] = 'o';
                    }
                } else if(isSessionVariableNode(subtrees[i])) {
                    vOrE[i] = 'p';
                } else {
                    vOrE[i] = 'i';
                }
            }
            vOrE[n] = '\0';
        }

        localTVarEnv = newHashTable(100);
        List *localTypingConstraints = newList(r);
        // evaluation parameters and try to resolve remaining tvars with unification
        for(i=0;i<n;i++) {
            switch(getParamIOType(vOrE,i)) {
                case 'i': // input
                case 'p': // input/output
                    args[i] = evaluateExpression3(subtrees[i], rei, reiSaveFlag,  env, errmsg, newRegion);
                    if(TYPE((Res *)args[i])==T_ERROR) {
                        res = args[i];
                        RETURN;
                    }
                    if(subtrees[i]->coercion!=NULL && subtrees[i]->coercion->t == TVAR) {
                        // ExprType *bn;
                        TypingConstraint *tc = newTypingConstraint(((Res *)args[i])->type, subtrees[i]->coercion, LT, subtrees[i] ,r);
                        char buf[ERR_MSG_LEN], buf2[1024], buf3[1024];
                        switch(simplifyLocally(tc, localTVarEnv, newRegion)) {
                            case ABSERDITY:
                                sprintf(buf, "error: runtime type inference param type: %s, arg type: %s",
                                        typeToString(subtrees[i]->coercion, localTVarEnv, buf3, 1024),
                                        typeToString(((Res *)args[i])->type, localTVarEnv, buf2, 1024));
                                generateErrMsg(buf, node->expr, node->base, buf2);
                                addRErrorMsg(errmsg, -1, buf2);
                                res = newErrorRes(r, -1);
                                RETURN;
                            case CONTIGENCY:
                                while(tc!=NULL) {
                                    listAppend(localTypingConstraints, tc, r);
                                    tc = tc->next;
                                }
                                break;
                            case TAUTOLOGY:
                                break;
                        }
                    }
                    break;
                case 'o': // output
                case 'e': // expression
                case 'a': // actions
                    break;
            }
        }
        // solve local typing constraints
        //typingConstraintsToString(localTypingConstraints, localTVarEnv, buf, 1024);
        //printf("%s\n", buf);
        Node *errnode;
        if(!solveConstraints(localTypingConstraints, localTVarEnv, errmsg, &errnode, r)) {
            res = newErrorRes(r, -1);
            RETURN;
        }
        //printVarTypeEnvToStdOut(localTVarEnv);
        // do the conversion
        for(i=0;i<n;i++) {
            switch(getParamIOType(vOrE,i)) {
                case 'i': // input
                case 'p': // input/output
                    if(subtrees[i]->coercion!=NULL) {
                        args[i] = processCoercion(subtrees[i], args[i], localTVarEnv, errmsg, newRegion);
                    }
                    if(TYPE((Res *)args[i])==T_ERROR) {
                        res = args[i];
                        RETURN;
                    }
                    break;
                case 'o': // output
                    args[i] = newStringRes(newRegion, "");
                    break;
                case 'e': // expression
                case 'a': // actions
                    args[i] = subtrees[i];
                    break;
            }
        }
        deleteHashTable(localTVarEnv, nop);
        localTVarEnv = NULL;
        if(fd!=NULL) {
            res = fd->func(args, n, node, rei, reiSaveFlag,  env, errmsg, newRegion);
        } else {
            res = execAction3(fn, (Res **)args, n, node, nEnv, rei, reiSaveFlag, errmsg, newRegion);
        }

        if(TYPE(res)==T_ERROR) {
            RETURN;
        }

        for(i=0;i<n;i++) {
            Res *resp = NULL;
            switch(getParamIOType(vOrE,i)) {
                case 'i': // input
                    break;
                case 'o': // output
                    // we don't do coercion here because we only apply coersion at application position
                    if(TYPE((Res *)args[i])==T_ERROR) {
                        res = args[i];
                        RETURN ;
                    }
                    resp = setVariableValue(subtrees[i]->text,args[i],rei,env,errmsg,r);
                    break;
                case 'p': // input/output
                    // we don't do coercion here because we only apply coersion at application position
                    if(TYPE((Res *)args[i])==T_ERROR) {
                        res = args[i];
                        RETURN;
                    }
                    resp = setVariableValue(subtrees[i]->text,args[i],rei,env,errmsg,r);
                    break;
                case 'e': // expression
                case 'a': // actions
                    break;
            }
            if(resp!=NULL && TYPE(resp)==T_ERROR) {
                res = resp;
                RETURN;
            }
        }
ret:
        if(localTVarEnv!=NULL) {
            deleteHashTable(localTVarEnv, nop);
        }
        deleteEnv(nEnv, 1);
        cpEnv(env,r);
        res = cpRes(res,r);
        region_free(newRegion);
        return res;

}

Res* evaluateVar3(char* vn, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
        char buf2[ERR_MSG_LEN];
	if(vn[0]=='*') { // local variable
		// try to get local var from env
		Res* res0 = (Res *)lookupFromHashTable(env->current, vn);
		if(res0==NULL) {
                    res0 = (Res *) lookupFromHashTable(env->global, vn);
                }
		if(res0==NULL) {
		    snprintf(buf, ERR_MSG_LEN, "error: unable to read local variable %s.", vn);
                    generateErrMsg(buf, node->expr, node->base, buf2);
                    addRErrorMsg(errmsg, UNABLE_TO_READ_LOCAL_VAR, buf2);
                    return newErrorRes(r, UNABLE_TO_READ_LOCAL_VAR);
		} else {
                    return res0;
		}
	} else if(vn[0]=='$') { // session variable
                Res *res = getSessionVar("",vn,rei, env, errmsg,r);
		if(res==NULL) {
		    snprintf(buf, ERR_MSG_LEN, "error: unable to read session variable %s.", vn);
                    generateErrMsg(buf, node->expr, node->base, buf2);
                    addRErrorMsg(errmsg, UNABLE_TO_READ_SESSION_VAR, buf2);
                    return newErrorRes(r, UNABLE_TO_READ_SESSION_VAR);
		} else {
                    return res;
                }
	} else {
		snprintf(buf, ERR_MSG_LEN, "error: unable to read variable %s.", vn);
                generateErrMsg(buf, node->expr, node->base, buf2);
                addRErrorMsg(errmsg, UNABLE_TO_READ_VAR, buf2);
                return newErrorRes(r, UNABLE_TO_READ_VAR);
	}
}

/**
 * return NULL error
 *        otherwise success
 */
Res* getSessionVar(char *action,  char *varName,  ruleExecInfo_t *rei, Env *env, rError_t *errmsg, Region *r) {
  char *varMap;
  void *varValue = NULL;
  int i, vinx;
  varValue = NULL;

  // Maps varName to the standard name and make varMap point to it.
  // It seems that for each pair of varName and standard name there is a list of actions that are supported.
  // vinx stores the index of the current pair so that we can start for the next pair if the current pair fails.
  vinx = getVarMap(action,varName, &varMap, 0); // reVariableMap.c
  while (vinx >= 0) {
	// Get the value of session variable referenced by varMap.
      i = getVarValue(varMap, rei, (char **)&varValue); // reVariableMap.c
      // convert to char * because getVarValue requires char *
      if (i >= 0) {
            if (varValue != NULL) {
                Res *res = NULL;
                FunctionDesc *fd = (FunctionDesc *) lookupBucketFromHashTable(env->funcDesc, varMap);
                if (fd == NULL) {
                    // default to string
                    res = newStringRes(r, (char *) varValue);
                    free(varValue);
                } else {
                    ExprType *type = T_FUNC_RET_TYPE(getFuncDescFromChain(0, fd)->type); // get var type from varMap
                    switch (type->t) {
                        case T_STRING:
                            res = newStringRes(r, (char *) varValue);
                            free(varValue);
                            break;
                        case T_INT:
                            res = newIntRes(r, *(int *) varValue);
                            free(varValue);
                            break;
                        case T_DOUBLE:
                            res = newDoubleRes(r, *(double *) varValue);
                            free(varValue);
                            break;
                        case T_IRODS:
                            res = newRes(r);
                            res->value.uninterpreted.inOutStruct = varValue;
                            res->value.uninterpreted.inOutBuffer = NULL;
                            res->type = type;
                            break;
                        default:
                            // unsupported type error
                            res = NULL;
                            addRErrorMsg(errmsg, -1, "error: unsupported session variable type");
                    }
                }
                free(varMap);
                return res;
            } else {
                return NULL;
            }
    } else if (i == NULL_VALUE_ERR) { // Try next varMap, starting from vinx+1.
      free(varMap);
      vinx = getVarMap(action,varName, &varMap, vinx+1);
    } else { // On error, return 0.
      free(varMap);
      if (varValue != NULL) free (varValue);
      return NULL;
    }
  }
  // varMap not found, return 0.
  if (varValue != NULL) free (varValue);
  return NULL;
}

char *matchWholeString(char *buf) {
    char *buf2 = (char *)malloc(sizeof(char)*strlen(buf)+2+1);
	buf2[0]='^';
	strcpy(buf2+1, buf);
	buf2[strlen(buf)+1]='$';
	buf2[strlen(buf)+2]='\0';
        return buf2;
}

char* getVariableName(Node *node) {
    return node->subtrees[0]->text;
}
/*
 * execute an external microserive or a rule
 */
Res* execAction3(char *actionName, Res** args, int nargs, Node *node, Env *env, ruleExecInfo_t* rei, int reiSaveFlag, rError_t *errmsg, Region *r)
{
	char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
	char buf2[ERR_MSG_LEN];
	char action[MAX_COND_LEN];
	strcpy(action, actionName);
	mapExternalFuncToInternalProc2(action);

	int actionInx = actionTableLookUp(action);
	if (actionInx < 0) { // rule
		// no action (microservice) found, try to lookup a rule
		int actionRet = execRule(actionName, args, nargs,  env, rei, reiSaveFlag, errmsg, r);
		if (actionRet == NO_RULE_FOUND_ERR || actionRet == NO_MORE_RULES_ERR) {
			snprintf(buf, ERR_MSG_LEN, "error: cannot find rule for action \"%s\" available: %d.", action, coreRules.len);
                        generateErrMsg(buf, node->expr, node->base, buf2);
                        addRErrorMsg(errmsg, NO_RULE_OR_MSI_FUNCTION_FOUND_ERR, buf2);
                        //dumpHashtableKeys(coreRules);
			return newErrorRes(r, NO_RULE_OR_MSI_FUNCTION_FOUND_ERR);
		}
		else {
			return newIntRes(r, actionRet);
		}
	} else { // microservice
		return execMicroService3(action, args, nargs, node, env, rei, errmsg, r);
	}
}

/**
 * execute micro service msiName
 */
Res* execMicroService3 (char *msName, Res **args, int nargs, Node *node, Env *env, ruleExecInfo_t *rei, rError_t *errmsg, Region *r) {
        msParamArray_t *origMsParamArray = rei->msParamArray;
	funcPtr myFunc = NULL;
	int actionInx;
	int numOfStrArgs;
	int i, ii;
	msParam_t *myArgv[MAX_PARAMS_LEN];
        Res *res;

	// look up the micro service
	actionInx = actionTableLookUp(msName);

        char errbuf[ERR_MSG_LEN];
	if (actionInx < 0) {
            int ret = NO_MICROSERVICE_FOUND_ERR;
            generateErrMsg("execMicroService3: no micro service found", node->expr, node->base, errbuf);
            addRErrorMsg(errmsg, ret, errbuf);
            rei->status = ret;
            return newErrorRes(r, ret);

        }
	myFunc =  MicrosTable[actionInx].callAction;
	numOfStrArgs = MicrosTable[actionInx].numberOfStringArgs;
	if (nargs != numOfStrArgs) {
            int ret = ACTION_ARG_COUNT_MISMATCH;
            generateErrMsg("execMicroService3: wrong number of arguments", node->expr, node->base, errbuf);
            addRErrorMsg(errmsg, ret, errbuf);
            rei->status = ret;
            return newErrorRes(r, ret);
        }

	// convert arguments from Res to msParam_t
        char buf[1024];
	for (i = 0; i < numOfStrArgs; i++) {
            myArgv[i] = (msParam_t *)malloc(sizeof(msParam_t));
            Res *res = args[i];
            if(res != NULL) {
                int ret =
                    convertResToMsParam(myArgv[i], res, errmsg);
                if(ret!=0) {
                    generateErrMsg("execMicroService3: error converting arguments to MsParam", node->subtrees[i]->expr, node->subtrees[i]->base, errbuf);
                    addRErrorMsg(errmsg, ret, errbuf);
                    rei->status = ret;
                    for(;i>=0;i--) {
                        if(TYPE(args[i])!=T_IRODS) {
                            free(myArgv[i]->inOutStruct);
                        }
                        free(myArgv[i]->label);
                        free(myArgv[i]->type);
                        free(myArgv[i]);
                    }
                    return newErrorRes(r, ret);
                }
            } else {
                myArgv[i]->inOutStruct = NULL;
                myArgv[i]->inpOutBuf = NULL;
                myArgv[i]->type = strdup(STR_MS_T);
            }
            sprintf(buf,"**%d",i);
            myArgv[i]->label = strdup(buf);
	}
        // convert env to msparam array
        rei->msParamArray = newMsParamArray();
        int ret = convertEnvToMsParamArray(rei->msParamArray, env, errmsg, r);
        if(ret!=0) {
            generateErrMsg("execMicroService3: error converting env to MsParamArray", node->expr, node->base, errbuf);
            addRErrorMsg(errmsg, ret, errbuf);
            rei->status = ret;
            res = newErrorRes(r,ret);
            RETURN;
        }


	if (numOfStrArgs == 0)
		ii = (*myFunc) (rei) ;
	else if (numOfStrArgs == 1)
		ii = (*myFunc) (myArgv[0],rei);
	else if (numOfStrArgs == 2)
		ii = (*myFunc) (myArgv[0],myArgv[1],rei);
	else if (numOfStrArgs == 3)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],rei);
	else if (numOfStrArgs == 4)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],rei);
	else if (numOfStrArgs == 5)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],myArgv[4],rei);
	else if (numOfStrArgs == 6)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],myArgv[4],myArgv[5],rei);
	else if (numOfStrArgs == 7)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],myArgv[4],myArgv[5],myArgv[6],rei);
	else if (numOfStrArgs == 8)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],myArgv[4],myArgv[5],myArgv[6],myArgv[7],rei);
	else if (numOfStrArgs == 9)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],myArgv[4],myArgv[5],myArgv[6],myArgv[7],myArgv[8],rei);
	else if (numOfStrArgs == 10)
		ii = (*myFunc) (myArgv[0],myArgv[1],myArgv[2],myArgv[3],myArgv[4],myArgv[5],myArgv[6],myArgv[7],
		                myArgv[8],myArgv [9],rei);
        if(ii<0) {
            rei->status = ii;

            res = newErrorRes(r, ii);
            RETURN;
        }
        // converts back env
	ret = updateMsParamArrayToEnv(rei->msParamArray, env, errmsg, r);
        if(ret!=0) {
            generateErrMsg("execMicroService3: error env from MsParamArray", node->expr, node->base, errbuf);
            addRErrorMsg(errmsg, ret, errbuf);
            rei->status = ret;

            res = newErrorRes(r, ret);
            RETURN;
        }
        // params
	for (i = 0; i < numOfStrArgs; i++) {
            if(myArgv[i] != NULL) {
                int ret =
                    convertMsParamToRes(myArgv[i], args[i], errmsg, r);
                if(ret!=0) {
                    generateErrMsg("execMicroService3: error converting arguments from MsParam", node->expr, node->base, errbuf);
                    addRErrorMsg(errmsg, ret, errbuf);
                    rei->status = ret;
                    res= newErrorRes(r, ret);
                    RETURN;
                }
            } else {
                args[i] = NULL;
            }
	}

/*
        char vn[100];
	for(i=0;i<numOfStrArgs;i++) {
            sprintf(vn,"**%d",i);
            largs[i] = lookupFromHashTable(env, vn);
	}
*/
	rei->status = ii;
        res = newIntRes(r, ii);
    ret:
        if(rei->msParamArray!=NULL && rei->msParamArray != origMsParamArray) {
            deleteMsParamArray(rei->msParamArray);
            rei->msParamArray = origMsParamArray;
        }
        for(i=0;i<numOfStrArgs;i++) {
            if(TYPE(args[i])!=T_IRODS) {
                free(myArgv[i]->inOutStruct);
            }
            free(myArgv[i]->label);
            free(myArgv[i]->type);
            free(myArgv[i]);
        }
        return res;
}


int execRuleFromCondIndex(char *ruleName, CondIndexVal *civ, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r) {
            //printTree(civ->condExp, 0);
            Res* res = evaluateExpression3(civ->condExp, rei, reiSaveFlag, env,  errmsg, r);
            if(TYPE(res) == T_ERROR) {
                return res->value.e;
            }
            if(TYPE(res) != T_STRING) {
                // todo try coercion
                addRErrorMsg(errmsg, -1, "error: the lhs of indexed rule condition does not evaluate to a string");
                return -1;
            }

            int *index = (int *)lookupFromHashTable(civ->valIndex, res->value.s.pointer);
            if(index == NULL) {
#ifndef DEBUG
                rodsLog (LOG_NOTICE,"applyRule Failed for action 1: %s with status %i",ruleName, NO_MORE_RULES_ERR);
#endif
                return(NO_MORE_RULES_ERR);
            }

            Node* rule = getRuleNode(*index+1000); // increase the index to move it to core rules index below 1000 are app rules

            ruleExecInfo_t  *saveRei;

            if (reiSaveFlag == SAVE_REI)
                saveRei = (ruleExecInfo_t  *) mallocAndZero(sizeof(ruleExecInfo_t));
            int status = execRuleNodeRes(rule, NULL, 0,  env, rei, reiSaveFlag, errmsg, r);

            if (status != 0) {
                rodsLog (LOG_NOTICE,"applyRule Failed for action : %s with status %i",ruleName, status);
            }
            if (reiSaveFlag == SAVE_REI)
                freeRuleExecInfoStruct(saveRei, 0);

            rei->status = status;
            return status;


}
/*
 * look up rule node by rulename from index
 * apply rule condition index if possilbe
 * call execRuleNodeRes until success or no more rules
 */
int execRule(char *ruleName, Res** args, int argc, /*char** p, int pc, */Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r)
{
    int ruleInx, i, status;
    int  first = 0;
    ruleExecInfo_t  *saveRei;
    int reTryWithoutRecovery = 0;

    #ifdef DEBUG
    char buf[1024];
    snprintf(buf, 1024, "execRule: %s\n", ruleName);
    writeToTmp("entry.log", buf);
    #endif

    int tempFlag = GlobalAllRuleExecFlag;
    GlobalAllRuleExecFlag = 0;

    ruleInx = -1; /* new rule */
    char action[MAX_COND_LEN];
    strcpy(action, ruleName);
    mapExternalFuncToInternalProc2(action);

    // try to find rule by cond index
    if(condIndex!=NULL) { // if index has been initialized
        CondIndexVal *civ = (CondIndexVal *)lookupFromHashTable(condIndex, ruleName);
        if(civ != NULL) {
            if(argc!=0) {
#ifndef DEBUG
                rodsLog (LOG_NOTICE,"applyRule Failed for action 1: %s with status %i",action, NO_MORE_RULES_ERR);
#endif
                return(NO_MORE_RULES_ERR);
            }
            status = execRuleFromCondIndex(ruleName, civ,  env, rei, reiSaveFlag, errmsg, r);
            // restore global flag
            GlobalAllRuleExecFlag = tempFlag;
            return status;
        }
    }

    i = findNextRule2 (action,  &ruleInx);
    if (i != 0) {
        if(i == NO_MORE_RULES_ERR)
            return NO_RULE_FOUND_ERR;
        else
            return i;
    }

    int success = 0;
    while (i == 0) {
        Node* rule = getRuleNode(ruleInx);
        int inParamsCount = rule->subtrees[0]->degree;
        if (inParamsCount != argc) {
            i = findNextRule2(action, &ruleInx);
            continue;
        }

        if (GlobalREDebugFlag)
            reDebug("  GotRule", ruleInx, ruleName, NULL, rei); // pass in NULL for inMsParamArray for now
#ifndef DEBUG
        if (reTestFlag > 0) {
            if (reTestFlag == COMMAND_TEST_1)
                fprintf(stdout, "+Testing Rule Number:%i for Action:%s\n", ruleInx, action);
            else if (reTestFlag == HTML_TEST_1)
                fprintf(stdout, "+Testing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n", ruleInx, action);
            else if (rei != 0 && rei->rsComm != 0 && &(rei->rsComm->rError) != 0)
                rodsLog(LOG_NOTICE, "+Testing Rule Number:%i for Action:%s\n", ruleInx, action);
        }
#endif
        //printTree(rule, 0);
        if (reiSaveFlag == SAVE_REI) {
            if (first == 0) {
                saveRei = (ruleExecInfo_t *) mallocAndZero(sizeof (ruleExecInfo_t));
                i = copyRuleExecInfo(rei, saveRei);
                first = 1;
            } else if (reTryWithoutRecovery == 0) {
                i = copyRuleExecInfo(saveRei, rei);
            }
        }

        status = execRuleNodeRes(rule, args, argc,  env, rei, reiSaveFlag, errmsg, r);
        int ret = 0;
        if (tempFlag == 0) { // apply first rule
            switch (status) {
                case 0:
                    ret = 1;

                    break;
                case CUT_ACTION_PROCESSED_ERR:
                    ret = 1;
#ifndef DEBUG
                    rodsLog(LOG_NOTICE, "applyRule Failed for action : %s with status %i", action, status);
#endif
                    break;
                case RETRY_WITHOUT_RECOVERY_ERR:
                    reTryWithoutRecovery = 1;
                    break;
            }


        } else { // apply all rules
            switch (status) {
                case 0:
                    if (reiSaveFlag == SAVE_REI)
                        freeRuleExecInfoStruct(saveRei, 0);
                    success = 1;
                    break;
                case CUT_ACTION_PROCESSED_ERR:
                    ret = 1;
                    break;
                case CUT_ACTION_ON_SUCCESS_PROCESSED_ERR:
                    ret = 1;
                    status = 0;
                    break;
                case RETRY_WITHOUT_RECOVERY_ERR:
                    reTryWithoutRecovery = 1;
                    break;
            }
        }

        if (ret) {
            if (reiSaveFlag == SAVE_REI)
                freeRuleExecInfoStruct(saveRei, 0);
            // restore global flag
            GlobalAllRuleExecFlag = tempFlag;

            rei->status = status;
            return status;
        }

        i = findNextRule2(action, &ruleInx);
        if(i==0)
        addRErrorMsg(errmsg, status, "==========");
    }

    if (first == 1) {
            if (reiSaveFlag == SAVE_REI)
                    freeRuleExecInfoStruct(saveRei, 0);
    }
    // restore global flag
    GlobalAllRuleExecFlag = tempFlag;

    if(success) {
        return 0;
    } else if (i == NO_MORE_RULES_ERR) {
#ifndef DEBUG
            rodsLog (LOG_NOTICE,"applyRule Failed for action 1: %s with status %i",action, i);
#endif
        rei->status = i;
        return i;
    } else {
        if (status < 0) {
#ifndef DEBUG
            rodsLog (LOG_NOTICE,"applyRule Failed for action 2: %s with status %i",action, status);
#endif
        }
        rei->status = status;
        return status;
    }
}
/*
 * execute a rule given by an AST node
 * create a new environment and intialize it with parameters
 */
int execRuleNodeRes(Node *rule, Res** args, int argc, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r)
{
	int i, status;
	Node* ruleCondition;
	Node* ruleAction;
	Node* ruleRecovery;
	Node* ruleHead;
        ruleHead = rule->subtrees[0];
        ruleRecovery = rule->subtrees[3];
        ruleCondition = rule->subtrees[1];
        ruleAction = rule->subtrees[2];

        if(ruleHead->degree != argc) {
            char buf[1024];
            snprintf(buf, 1024, "error: cannot apply rule %s becasue wrong number of arguments, declared %d, supplied %d.", ruleHead->text, ruleHead->degree, argc);
            addRErrorMsg(errmsg, -1, buf);
            return -1;
        }
        char action[MAX_COND_LEN];
	strcpy(action, ruleHead->text);
	char* paramsNames[MAX_NUM_OF_ARGS_IN_ACTION];
        Node** paramsNodes = ruleHead->subtrees;
        int inParamsCount = ruleHead->degree;
        int k;
        for (k = 0; k < inParamsCount ; k++) {
            paramsNames[k] =  paramsNodes[k]->text;
        }

        Env *envNew = newEnv(newHashTable(100), env->global, env->funcDesc);
        Region *rNew = make_region(0, NULL);
        i = initializeEnv(ruleHead,args,argc, envNew->current,rNew);
        if (i != 0)
            return(i);

        Res *res = evaluateExpression3(ruleCondition, rei, reiSaveFlag,  envNew, errmsg, rNew);
        // todo consolidate every error into T_ERROR except OOM
        if (TYPE(res)!=T_ERROR && res->value.d) {
#ifndef DEBUG
#if 0
            if (reTestFlag > 0) {
                    if (reTestFlag == COMMAND_TEST_1)
                            fprintf(stdout,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
                    else if (reTestFlag == HTML_TEST_1)
                            fprintf(stdout,"+Executing Rule Number:<FONT COLOR=#FF0000>%i</FONT> for Action:<FONT COLOR=#0000FF>%s</FONT><BR>\n",ruleInx,action);
                    else
                            rodsLog (LOG_NOTICE,"+Executing Rule Number:%i for Action:%s\n",ruleInx,action);
            }
#endif
#endif
            status = executeRuleBodyNode(action, ruleAction, ruleRecovery,  envNew, rei, reiSaveFlag, errmsg,rNew);
            // copy parameters
            copyFromEnv(args, paramsNames, inParamsCount, envNew->current, r);
            // copy global variables
            cpHashtable(envNew->global, r);
            deleteEnv(envNew, 1);
            region_free(rNew);
            if ( status == 0) {
                return(0);
            } else if ( status == CUT_ACTION_PROCESSED_ERR ) {
                return(status);
            }
            if (status < 0) {
                    #ifndef DEBUG
                    rodsLog (LOG_NOTICE,"applyRule Failed for action 2: %s with status %i",action, status);
                    #endif
            }
            return(status);
        } else {
            cpHashtable(envNew->global, r);
            deleteEnv(envNew, 1);
            region_free(rNew);
            return (RULE_FAILED_ERR);
        }

}
/*
 * execute an action
 * return 0 if succeeded and the error code if failed
 */

int executeRuleActionNode(Node *inAction, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r)
{
	Res *res =
	evaluateExpression3(inAction, rei, reiSaveFlag,  env, errmsg, r);
	//sprintf(buf, "%s return value type: %d, value: %d", inAction, res.type, res.value.e);
	//RE_TEST_MACRO(buf);
	if(TYPE(res) == T_ERROR) {
#ifndef DEBUG
            char errbuf[ERR_MSG_LEN];
            errMsgToString(errmsg, errbuf, ERR_MSG_LEN);
            rodsLogAndErrorMsg(LOG_NOTICE, &(rei->rsComm->rError),-1, errbuf);
#endif
            return res->value.e < 0? res->value.e : UNKNOWN_ERROR;
        } else if (TYPE(res) == T_INT) {
#ifndef DEBUG
            if(res->value.d<0) {
            	char buf[MAX_COND_LEN];
                sprintf(buf, "error %d in action: %s", (int)res->value.d, inAction->text);
		rodsLogAndErrorMsg(LOG_NOTICE, &(rei->rsComm->rError),-1, buf);
            }
#endif
            return 0;
        } else if(TYPE(res) == T_SUCCESS) {
                return 1;
        } else {
		// discard result
		return 0;
	}
}
int executeRuleRecoveryNode(Node *ruleRecovery, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r)
{
	if (ruleRecovery == NULL)
		return(0);
	#ifndef DEBUG
	if (reTestFlag > 0) {
		if (reTestFlag == COMMAND_TEST_1 || COMMAND_TEST_MSI)
			fprintf(stdout,"***RollingBack\n");
		else if (reTestFlag == HTML_TEST_1)
			fprintf(stdout,"<FONT COLOR=#FF0000>***RollingBack</FONT><BR>\n");
		else  if (reTestFlag == LOG_TEST_1)
			if (rei != NULL && rei->rsComm != NULL && &(rei->rsComm->rError) != NULL)
				rodsLog (LOG_NOTICE,"***RollingBack\n");
	}
	#endif
	return(executeRuleActionNode(ruleRecovery,  env, rei, reiSaveFlag, errmsg, r));
}

int executeRuleBodyNode(char *action, Node *ruleAction, Node *ruleRecovery,
                   Env *env,
                   ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r)
{
	int i,j=0,k,n;
	Node **actionArray = ruleAction->subtrees;
	Node **recoveryArray = ruleRecovery->subtrees;
	int cutFlag = 0;
	n = ruleRecovery->degree;

	for (i = 0; i < ruleAction->degree; i++) {
		if (strcmp(actionArray[i]->text,"cut")==0) {
			#ifndef DEBUG
			if (reTestFlag > 0) {
				if (reTestFlag == COMMAND_TEST_1 || COMMAND_TEST_MSI)
					fprintf(stdout,"!!!Processing a cut\n");
				else if (reTestFlag == HTML_TEST_1 || HTML_TEST_MSI)
					fprintf(stdout,"<FONT COLOR=#FF0000>!!!Processing a cut</FONT><BR>\n");
				else  if (reTestFlag == LOG_TEST_1)
					if (rei != NULL && rei->rsComm != NULL && &(rei->rsComm->rError) != NULL)
						rodsLog (LOG_NOTICE,"!!!Processing a cut\n");
			}
			#endif
			cutFlag = 1;
			j = 0;
			continue;
		}
		j = executeRuleActionNode(actionArray[i],  env, rei, reiSaveFlag, errmsg, r);
                if (j != 0) {
                    break;
                }
	}
	if (j == BREAK_ACTION_ENCOUNTERED_ERR)
		return(j);
	if (j == RETRY_WITHOUT_RECOVERY_ERR)
		return(j);
	if (j < 0) {
#ifndef DEBUG
		sprintf(tmpStr,"executeRuleAction Failed for %s",actionArray[i]->text);
		rodsLogError(LOG_ERROR,j,tmpStr);
		rodsLog (LOG_NOTICE,"executeRuleBody: Micro-service or Action %s Failed with status %i",actionArray[i]->text,j);
#endif
		for ( i=i>=n-1?n-1:i; i >= 0 ; i--) {
			if (strcmp(recoveryArray[i]->text,"nop")==0 || strcmp(recoveryArray[i]->text,"null")==0)
				continue;
			k = executeRuleRecoveryNode(recoveryArray[i],  env, rei, reiSaveFlag, errmsg,r);
			if (k < 0) {
#ifndef DEBUG
				sprintf(tmpStr,"executeRuleRecovery Failed for %s",recoveryArray[i]->text);
				rodsLogError(LOG_ERROR,k,tmpStr);
#endif
				j = k;
				break;
			}
		}
		if (cutFlag == 1)
			return(CUT_ACTION_PROCESSED_ERR);
		else
			return(j);
	}

	if (cutFlag == 1)
		return(CUT_ACTION_ON_SUCCESS_PROCESSED_ERR);
	/** this allows use of cut as the last msi in the body so that other rules will not be processed
	    even when the current rule succeeds **/
	return(0);
}

