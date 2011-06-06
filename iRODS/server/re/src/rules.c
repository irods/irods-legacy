/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "debug.h"
#include "rules.h"
#include "index.h"
#include "functions.h"
#include "arithmetics.h"
#include "configuration.h"

#define ERROR(cond) if(cond) { goto error; }

extern int GlobalAllRuleExecFlag;
#ifndef DEBUG
#include "reGlobalsExtern.h"
#include "reHelpers1.h"
typedef struct {
  char action[MAX_ACTION_SIZE];
  int numberOfStringArgs;
  funcPtr callAction;
} microsdef_t;
extern int NumOfAction;
extern microsdef_t MicrosTable[];
#endif

/**
 * Read a set of rules from files.
 * return 0 success
 *        otherwise error code
 */

int readRuleSetFromFile(char *ruleBaseName, RuleSet *ruleSet, int* errloc, rError_t *errmsg, Region *r) {
	char rulesFileName[MAX_NAME_LEN];
	getRuleBasePath(ruleBaseName, rulesFileName);

	return readRuleSetFromLocalFile(ruleBaseName, rulesFileName, ruleSet, errloc, errmsg,r);
}
int readRuleSetFromLocalFile(char *ruleBaseName, char *rulesFileName, RuleSet *ruleSet, int *errloc, rError_t *errmsg, Region *r) {

	FILE *file;
    char errbuf[ERR_MSG_LEN];
    file = fopen(rulesFileName, "r");
	if (file == NULL) {
            snprintf(errbuf, ERR_MSG_LEN,
                    "readRuleSetFromFile() could not open rules file %s\n",
                    rulesFileName);
            addRErrorMsg(errmsg, RULES_FILE_READ_ERROR, errbuf);
            return(RULES_FILE_READ_ERROR);
	}
    Pointer *e = newPointer(file, ruleBaseName);
    int ret = parseRuleSet(e, ruleSet, errloc, errmsg, r);
    deletePointer(e);
    if(ret == -1) {
        return -1;
    }

	Node *errnode;
	ExprType *restype = typeRuleSet(ruleSet, errmsg, &errnode, r);
	if(restype->nodeType == T_ERROR) {
	    *errloc = errnode->expr;
	    return -1;
	}

	return 0;
}
void addCmdExecOutToEnv(Env *global, Region *r) {
    execCmdOut_t *ruleExecOut = (execCmdOut_t *)malloc (sizeof (execCmdOut_t));
    memset (ruleExecOut, 0, sizeof (execCmdOut_t));
    Res *execOutRes = newRes(r);
	execOutRes->exprType  = newIRODSType(ExecCmdOut_MS_T, r);
	execOutRes->value.uninterpreted.inOutStruct = ruleExecOut;
    insertIntoHashTable(global->current, "ruleExecOut", execOutRes);

}

int parseAndComputeMsParamArrayToEnv(msParamArray_t *var, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r) {
    int i;
	for(i=0;i<var->len;i++) {
		Res *res = newRes(r);
		int ret = convertMsParamToRes(var->msParam[i], res, errmsg, r);
        if(ret != 0) {
            return ret;
        }
        if(TYPE(res) != T_STRING) {
            return -1;
        }

		char *varName = var->msParam[i]->label;
		char *expr = res->text;
		res = parseAndComputeExpression(expr, env, rei, reiSaveFlag, errmsg, r);
		if(res->nodeType  ==  N_ERROR) {
		    return res->value.errcode;
		}
        if(varName!=NULL) {
            updateInEnv(env, varName, res);
        }
	}
    return 0;

}
Env *defaultEnv(Region *r) {
    Env *global = newEnv(newHashTable(100), NULL, NULL);
    Env *env = newEnv(newHashTable(100), global, NULL);

    addCmdExecOutToEnv(global, r);
    return env;
}

int parseAndComputeRuleAdapter(char *rule, msParamArray_t *msParamArray, ruleExecInfo_t *rei, int reiSaveFlag, Region *r) {

    rError_t errmsgBuf;
    errmsgBuf.errMsg = NULL;
    errmsgBuf.len = 0;

    Env *env = defaultEnv(r);
    rei->status = 0;

    int rescode = 0;
    if(msParamArray!=NULL) {
        rescode = parseAndComputeMsParamArrayToEnv(msParamArray, env->previous, rei, reiSaveFlag, &errmsgBuf, r);
        ERROR(rescode < 0);
    }

    rescode = parseAndComputeRule(rule, env, rei, reiSaveFlag, &errmsgBuf, r);
    ERROR(rescode < 0);

    if(rei->msParamArray==NULL) {
        rei->msParamArray = newMsParamArray();
    }
    convertEnvToMsParamArray(rei->msParamArray, env, &errmsgBuf, r);

    freeRErrorContent(&errmsgBuf);
    deleteEnv(env, 3);

    return rescode;
error:
    logErrMsg(&errmsgBuf, &rei->rsComm->rError);
    rei->status = rescode;
    freeRErrorContent(&errmsgBuf);
    deleteEnv(env, 3);

    return rescode;


}

int parseAndComputeRuleNewEnv( char *rule, ruleExecInfo_t *rei, int reiSaveFlag, msParamArray_t *msParamArray, rError_t *errmsg, Region *r) {
    Env *env = defaultEnv(r);

    int rescode = 0;

    if(msParamArray!=NULL) {
        rescode = convertMsParamArrayToEnv(msParamArray, env->previous, errmsg, r);
        ERROR(rescode < 0);
    }

    rescode = parseAndComputeRule(rule, env, rei, reiSaveFlag, errmsg, r);
    ERROR(rescode < 0);

    if(rei->msParamArray==NULL) {
        rei->msParamArray = newMsParamArray();
    }
    rescode = convertEnvToMsParamArray(rei->msParamArray, env, errmsg, r);
    ERROR(rescode < 0);
    deleteEnv(env, 3);
    return rescode;

error:

    deleteEnv(env, 3);
    return rescode;
}

/* parse and compute a rule */
int parseAndComputeRule(char *rule, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r) {
	if(overflow(rule, MAX_COND_LEN)) {
		addRErrorMsg(errmsg, BUFFER_OVERFLOW, "error: potential buffer overflow");
		return BUFFER_OVERFLOW;
	}
	Node *node;
    Pointer *e = newPointer2(rule);
    if(e == NULL) {
        addRErrorMsg(errmsg, POINTER_ERROR, "error: can not create a Pointer.");
        return POINTER_ERROR;
    }

    int tempLen = ruleEngineConfig.extRuleSet->len;

    int checkPoint=checkPointExtRuleSet();

    int errloc;
    /* add rules into ext rule set */
    if(parseRuleSet(e, ruleEngineConfig.extRuleSet, &errloc, errmsg, r) == -1) {
        deletePointer(e);
        return PARSER_ERROR;
    }

    /* save secondary index status */
    RuleEngineStatus cis = ruleEngineConfig.condIndexStatus;

    /* add rules into rule index */
	int i;
	for(i=tempLen;i<ruleEngineConfig.extRuleSet->len;i++) {
		prependRuleIntoIndex(ruleEngineConfig.extRuleSet->rules[i], i, r);
		if(lookupFromHashTable(ruleEngineConfig.condIndex, RULE_NAME(ruleEngineConfig.extRuleSet->rules[i]->node))!=NULL) {
			ruleEngineConfig.condIndexStatus = DISABLED;
		}
	}

    /* exec the first rule */
    RuleDesc *rd = ruleEngineConfig.extRuleSet->rules[tempLen];
	node = rd->node;

    Hashtable *varTypes = newHashTable(100);

    List *typingConstraints = newList(r);
    Node *errnode;
    ExprType *type = typeRule(rd, ruleEngineConfig.funcDescIndex, varTypes, typingConstraints, errmsg, &errnode, r);

	deleteHashTable(varTypes, nop);

    int rescode;
    if(type->nodeType!=T_ERROR) {
        Res *res = execRuleNodeRes(node, NULL, 0, env, rei, reiSaveFlag, errmsg,r);
        rescode = res->nodeType  ==  N_ERROR? res->value.errcode:0;
    } else {
        rescode = TYPE_ERROR;
    }

    /* remove rules from ext rule set */
    popExtRuleSet(checkPoint);

    /* restore secondary index status */
    if(ruleEngineConfig.condIndexStatus == DISABLED) {
    	ruleEngineConfig.condIndexStatus = cis;
    }

    return rescode;
}

/* call an action with actionName and string parameters */
Res *computeExpressionWithParams( char *actionName, char **params, int paramsCount, ruleExecInfo_t *rei, int reiSaveFlag, msParamArray_t *msParamArray, rError_t *errmsg, Region *r) {
#ifdef DEBUG
    char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
    snprintf(buf, 1024, "computExpressionWithParams: %s\n", actionName);
    writeToTmp("entry.log", buf);
#endif

    if(overflow(actionName, MAX_COND_LEN)) {
            addRErrorMsg(errmsg, BUFFER_OVERFLOW, "error: potential buffer overflow");
            return newErrorRes(r, BUFFER_OVERFLOW);
    }
    int k;
    for(k=0;k<paramsCount;k++) {
        if(overflow(params[k], MAX_COND_LEN)) {
            addRErrorMsg(errmsg, BUFFER_OVERFLOW, "error: potential buffer overflow");
            return newErrorRes(r, BUFFER_OVERFLOW);
        }
    }

    Node** paramNodes = (Node **)region_alloc(r, sizeof(Node *)*paramsCount);
    int i;
    for(i=0;i<paramsCount;i++) {

        Node *node;

        /*Pointer *e = newPointer2(params[i]);

        if(e == NULL) {
            addRErrorMsg(errmsg, -1, "error: can not create Pointer.");
            return newErrorRes(r, -1);
        }

        node = parseTermRuleGen(e, 1, errmsg, r);*/
        node = newNode(TK_STRING, params[i], 0, r);
        /*if(node==NULL) {
            addRErrorMsg(errmsg, OUT_OF_MEMORY, "error: out of memory.");
            return newErrorRes(r, OUT_OF_MEMORY);
        } else if (node->nodeType == N_ERROR) {
            return newErrorRes(r, node->value.errcode);

        }*/

        paramNodes[i] = node;
    }

    Node *node = createFunctionNode(actionName, paramNodes, paramsCount, NULL, r);
    Env *global = newEnv(newHashTable(100), NULL, NULL);
    Env *env = newEnv(newHashTable(100), global, NULL);
    if(msParamArray!=NULL) {
        convertMsParamArrayToEnv(msParamArray, global, errmsg, r);
    }
    Res *res = computeExpressionNode(node, env, rei, reiSaveFlag, errmsg,r);
    deleteEnv(env, 3);
    return res;
}
ExprType *typeRule(RuleDesc *rule, Hashtable *funcDesc, Hashtable *varTypes, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
            /* printf("%s\n", node->subtrees[0]->text); */
			addRErrorMsg(errmsg, -1, ERR_MSG_SEP);
            char buf[ERR_MSG_LEN];
            Node *node = rule->node;
#if 0
            int arity = RULE_NODE_NUM_PARAMS(node); /* subtrees[0] = param list */
            Node *type = node->subtrees[0]->subtrees[1]; /* subtrees[1] = return type */
#endif

            ExprType *resType = typeExpression3(node->subtrees[1], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
            /*printf("Type %d\n",resType->t); */
            ERROR(resType->nodeType == T_ERROR);
            if(resType->nodeType != T_BOOL && resType->nodeType != T_VAR && resType->nodeType != T_DYNAMIC) {
            	char buf2[1024], buf3[ERR_MSG_LEN];
            	typeToString(resType, varTypes, buf2, 1024);
            	snprintf(buf3, ERR_MSG_LEN, "error: the type %s of the rule condition is not supported", buf2);
                generateErrMsg(buf3, node->subtrees[1]->expr, node->subtrees[1]->base, buf);
                addRErrorMsg(errmsg, TYPE_ERROR, buf);
                ERROR(1);
            }
            resType = typeExpression3(node->subtrees[2], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
            ERROR(resType->nodeType == T_ERROR);
            resType = typeExpression3(node->subtrees[3], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
            ERROR(resType->nodeType == T_ERROR);
            /* printVarTypeEnvToStdOut(varTypes); */
            ERROR(solveConstraints(typingConstraints, varTypes, errmsg, errnode, r) == ABSURDITY);
            int i;
            for(i =1;i<=3;i++) { // 1 = cond, 2 = actions, 3 = recovery
                postProcessCoercion(node->subtrees[i], varTypes, errmsg, errnode, r);
                postProcessActions(node->subtrees[i], funcDesc, errmsg, errnode, r);
            }
            /*printTree(node, 0); */
            return newSimpType(T_INT, r);

        error:
            snprintf(buf, ERR_MSG_LEN, "type error: in rule %s", node->subtrees[0]->text);
            addRErrorMsg(errmsg, -1, buf);
            return resType;

}

ExprType *typeRuleSet(RuleSet *ruleset, rError_t *errmsg, Node **errnode, Region *r) {
    Hashtable *funcDesc = ruleEngineConfig.funcDescIndex;
    Hashtable *ruleType = newHashTable(MAX_NUM_RULES * 2);
    ExprType *res;
    int i;
    for(i=0;i<ruleset->len;i++) {
        Hashtable *varTypes = newHashTable(100);
        List *typingConstraints = newList(r);
        RuleDesc *rule = ruleset->rules[i];
        ExprType *restype = typeRule(rule, funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
        /*char buf[1024]; */
        /*typingConstraintsToString(typingConstraints, NULL, buf, 1024); */
        /*printf("rule %s, typing constraints: %s\n", ruleset->rules[i]->subtrees[0]->text, buf); */
        deleteHashTable(varTypes, nop);
        if(restype->nodeType == T_ERROR) {
            res = restype;
            char *errbuf = (char *) malloc(ERR_MSG_LEN*1024*sizeof(char));
            errMsgToString(errmsg, errbuf, ERR_MSG_LEN*1024);
#ifdef DEBUG
            writeToTmp("ruleerr.log", errbuf);
            writeToTmp("ruleerr.log", "\n");
#endif
            rodsLog (LOG_ERROR, "%s", errbuf);
            free(errbuf);
            freeRErrorContent(errmsg);
            RETURN;
        }
        /* check that function names are unique */
        char *ruleName = rule->node->subtrees[0]->text;
        RuleDesc *rd = (RuleDesc *)lookupFromHashTable(ruleType, ruleName);
        if(rd!=NULL) {
            if(rule->ruleType == RK_FUNC || rd ->ruleType == RK_FUNC) {
                char errbuf[ERR_MSG_LEN];
                generateErrMsg("redefinition of function", rule->node->expr, rule->node->base, errbuf);
                addRErrorMsg(errmsg, FUNCTION_REDEFINITION, errbuf);
                res = newErrorType(FUNCTION_REDEFINITION, r);
                *errnode = rule->node;
                RETURN;
        }
            } else {
                insertIntoHashTable(ruleType, ruleName, rule);
            }
    }
    res = newSimpType(T_INT, r); /* Although a rule set does not have type T_INT, return T_INT to indicate success. */

ret:
    deleteHashTable(ruleType, nop);
    return res;
}

/* compute an expression given by an AST node */
Res* computeExpressionNode(Node *node, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t* errmsg, Region *r) {
    Hashtable *varTypes = newHashTable(100);
    Region *rNew = make_region(0, NULL);
    ExprType *resType;
    Node *en;
    Node **errnode = &en;
    Res* res;
    if(!node->typed) {
        /*printTree(node, 0); */
        List *typingConstraints = newList(r);
        resType = typeExpression3(node, ruleEngineConfig.funcDescIndex, varTypes, typingConstraints, errmsg, errnode, r);
        /*printf("Type %d\n",resType->t); */
        if(resType->nodeType == T_ERROR) {
            addRErrorMsg(errmsg, -1, "type error: in rule");
            res = newErrorRes(r,-1);
            RETURN;
        }
        postProcessCoercion(node, varTypes, errmsg, errnode, r);
        postProcessActions(node, ruleEngineConfig.funcDescIndex, errmsg, errnode, r);
        /*    printTree(node, 0); */
        deleteHashTable(varTypes, nop);
        varTypes = NULL;
        node->typed = 1;
    }
    res = evaluateExpression3(node, GlobalAllRuleExecFlag, 0, rei, reiSaveFlag, env, errmsg, rNew);

/*    switch (TYPE(res)) {
        case T_ERROR:
            addRErrorMsg(errmsg, -1, "error: in rule");
            break;
        default:
            break;
    }*/
    ret:
    if(varTypes!=NULL) {
        deleteHashTable(varTypes, nop);
    }
    res = cpRes(res, r);
    cpEnv(env, r);
    region_free(rNew);
    return res;
}

/* parse and compute an expression
 *
 */
Res *parseAndComputeExpression(char *expr, Env *env, ruleExecInfo_t *rei, int reiSaveFlag, rError_t *errmsg, Region *r) {
    Res *res = NULL;
    char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
    int rulegen;
    Node *node = NULL;

#ifdef DEBUG
    snprintf(buf, 1024, "parseAndComputeExpression: %s\n", expr);
    writeToTmp("entry.log", buf);
#endif
    if(overflow(expr, MAX_COND_LEN)) {
            addRErrorMsg(errmsg, BUFFER_OVERFLOW, "error: potential buffer overflow");
            return newErrorRes(r, BUFFER_OVERFLOW);
    }
    Pointer *e = newPointer2(expr);
    if(e == NULL) {
        addRErrorMsg(errmsg, -1, "error: can not create pointer.");
        res = newErrorRes(r, -1);
        RETURN;
    }
    rulegen = strstr(expr, "##")==NULL?1:0;
    node = parseTermRuleGen(e, rulegen, errmsg, r);
    if(node==NULL) {
            addRErrorMsg(errmsg, OUT_OF_MEMORY, "error: out of memory.");
            res = newErrorRes(r, OUT_OF_MEMORY);
            RETURN;
    } else if (node->nodeType == N_ERROR) {
            generateErrMsg("error: syntax error",node->expr, node->base, buf);
            addRErrorMsg(errmsg, PARSER_ERROR, buf);
            res = newErrorRes(r, PARSER_ERROR);
            RETURN;
    } else {
        Token token;
        nextTokenRuleGen(e, &token, 0);

        if(token.type!=TK_EOS) {
            Label pos;
            getFPos(&pos, e);
            generateErrMsg("error: unparsed suffix",pos.exprloc, pos.base, buf);
            addRErrorMsg(errmsg, UNPARSED_SUFFIX, buf);
/*            deletePointer(e); */
            res = newErrorRes(r, UNPARSED_SUFFIX);
            RETURN;
        }
    }

    res = computeExpressionNode(node, env, rei, reiSaveFlag, errmsg,r);
    ret:
    deletePointer(e);
    return res;
}

int generateRuleTypes(RuleSet *inRuleSet, Hashtable *symbol_type_table, Region *r)
{
	int i;
	for (i=0;i<inRuleSet->len;i++) {
            Node *ruleNode = inRuleSet->rules[i]->node;
            if(ruleNode == NULL)
                continue;
            char *key = ruleNode->subtrees[0]->text;
            int arity = RULE_NODE_NUM_PARAMS(ruleNode);

            ExprType **paramTypes = (ExprType**) region_alloc(r, sizeof(ExprType *)*arity);
            int k;
            for(k=0;k<arity;k++) {
                paramTypes[k] = newTVar(r);
            }
            ExprType *ruleType = newFuncTypeVarArg(arity, ONCE, paramTypes, newSimpType(T_INT, r), r);

            if (insertIntoHashTable(symbol_type_table, key,ruleType) == 0) {
                    return 0;
            }
	}
	return 1;
}

Node* getRuleNode(int ri)
{

	if(ri< APP_RULE_INDEX_OFF) {
		return ruleEngineConfig.extRuleSet->rules[ri]->node;
	} else
	if (ri < CORE_RULE_INDEX_OFF) {
		return ruleEngineConfig.appRuleSet->rules[ri]->node;
	} else {
		ri = ri - CORE_RULE_INDEX_OFF;
		return ruleEngineConfig.coreRuleSet->rules[ri]->node;
	}
	return(NULL);
}

int actionTableLookUp (char *action)
{
	int i;

	for (i = 0; i < NumOfAction; i++) {
		if (!strcmp(MicrosTable[i].action,action))
			return (i);
	}

	return (UNMATCHED_ACTION_ERR);
}
Res *parseAndComputeExpressionAdapter(char *inAction, msParamArray_t *inMsParamArray, ruleExecInfo_t *rei, int reiSaveFlag, Region *r) {
    int freeRei = 0;
    if(rei == NULL) {
        rei = (ruleExecInfo_t *) malloc(sizeof(ruleExecInfo_t));
        memset(rei, 0, sizeof(ruleExecInfo_t));
        freeRei = 1;
    }
    rei->status = 0;
    Env *global = newEnv(newHashTable(100), NULL, NULL);
    Env *env = newEnv(newHashTable(100),global, NULL);
    addCmdExecOutToEnv(global, r);

    Res *res;
    rError_t errmsgBuf;
    errmsgBuf.errMsg = NULL;
    errmsgBuf.len = 0;

    if(inMsParamArray!=NULL) {
        convertMsParamArrayToEnv(inMsParamArray, env, &errmsgBuf, r);
    }

    res = parseAndComputeExpression(inAction, env, rei, reiSaveFlag, &errmsgBuf, r);
    if(inMsParamArray != NULL) {
        clearMsParamArray(inMsParamArray, 0);
    	convertEnvToMsParamArray(inMsParamArray, env, &errmsgBuf, r);
    } else {
        freeEnvUninterpretedStructs(env);
    }

    deleteEnv(env, 3);
    if(res->nodeType == N_ERROR) {
        logErrMsg(&errmsgBuf, &rei->rsComm->rError);
        rei->status = res->value.errcode;
    }
    freeRErrorContent(&errmsgBuf);
    if(freeRei) {
        free(rei);
    }
    return res;

}
int overflow(char* expr, int len) {
	int i;
	for(i = 0;i<len+1;i++) {
		if(expr[i]==0)
			return 0;
	}
	return 1;
}

