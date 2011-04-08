/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "debug.h"
#include "rules.h"
#include "index.h"
#include "functions.h"

#ifndef DEBUG
#include "reGlobalsExtern.h"
#include "reHelpers1.h"
extern int GlobalAllRuleExecFlag;
typedef struct {
  char action[MAX_ACTION_SIZE];
  int numberOfStringArgs;
  funcPtr callAction;
} microsdef_t;
extern int NumOfAction;
extern microsdef_t MicrosTable[];
#endif

RuleSet coreRules, appRules;

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
	int i = 0;
	FILE *file;
            char errbuf[ERR_MSG_LEN];
	file = fopen(rulesFileName, "r");
	if (file == NULL) {
            snprintf(errbuf, MAX_ERRMSG_LEN,
                    "readRuleSetFromFile() could not open rules file %s\n",
                    rulesFileName);
            addRErrorMsg(errmsg, RULES_FILE_READ_ERROR, errbuf);
            return(RULES_FILE_READ_ERROR);
	}
        Pointer pointer;
        Pointer *e = &pointer;
        initPointer(e, file, ruleBaseName);

	Token token;
        int ret = 1;
        /* parser variables */
        int backwardCompatible = 0;

        while(ret == 1) {
            nextTokenRuleGen(e, &token, 1);
            switch(token.type) {
                case ERROR:
                    ret = -1;
                    continue;
                case EOS:
                    ret = 0;
                    continue;
                case TEXT:
                    if(token.text[0] == '#') {
                        skipComments(e);
                        continue;
                    } else if(token.text[0] == '@') { /* directive */
                        nextTokenRuleGen(e, &token, 1);
                        if(strcmp(token.text, "backwardCompatible")==0) {
                            nextTokenRuleGen(e, &token, 1);
                            if(strcmp(token.text, "true")==0) {
                                backwardCompatible = 1;
                            } else if(strcmp(token.text, "false")==0) {
                                backwardCompatible = 0;
                            } else {
                                /* todo error handling */
                            }
                        } else {
                            /* todo error handling */
                        }
                        continue;
                    }
                    break;
                default:
                    break;
            }
            pushback(e, &token);

            Node *nodes[100];
            int n;
            parseRuleRuleGen(e, nodes, &n, backwardCompatible, errmsg, r);
            int k;
            for(k=0;k<n;k++) {
                Node *node = nodes[k];
                if(node->type == ERROR) {
                    /*deleteTree(node); */

                    *errloc = node->expr;
                    generateErrMsgFromFile("readRuleSetFromFile: error parsing rule.", *errloc, ruleBaseName, rulesFileName, errbuf);
                    addRErrorMsg(errmsg, PARSER_ERROR, errbuf);
                    /* skip the current line and try to parse the rule from the next line */
                    skipComments(e);
                    i++;
                    return -1;
                }
            ruleSet->rules[i] = node;
            i++;
    /*        printf("%s\n", node->subtrees[0]->text); */
            /*
            printTree(node, 0);
            getchar();
*/
            }
	}
	fclose (file);
	ruleSet->len = i;
	Node *errnode;
	ExprType *restype = typeRuleSet(ruleSet, errmsg, &errnode, r);
	if(restype->t == T_ERROR) {
	    *errloc = errnode->expr;
	    return -1;
	}
	/*if(inRuleStrct == &coreRuleStrct) {
		createRuleIndex(&coreRuleStrct, &coreRuleIndex);
	} else if(inRuleStrct == &appRuleStrct) {
		createRuleIndex(&appRuleStrct, &appRuleIndex);
	}*/
	return(0);
}
void addCmdExecOutToEnv(Hashtable *global, Region *r) {
    execCmdOut_t *ruleExecOut = (execCmdOut_t *)malloc (sizeof (execCmdOut_t));
    memset (ruleExecOut, 0, sizeof (execCmdOut_t));
    Res *execOutRes = newRes(r);
	execOutRes->type  = newIRODSType(ExecCmdOut_MS_T, r);
	execOutRes->value.uninterpreted.inOutStruct = ruleExecOut;
    insertIntoHashTable(global, "ruleExecOut", execOutRes);

}

/* parse and compute a rule */
int computeRule( char *expr, ruleExecInfo_t *rei, int reiSaveFlag, msParamArray_t *msParamArray, rError_t *errmsg, Region *r) {
	if(overflow(expr, MAX_COND_LEN)) {
		addRErrorMsg(errmsg, BUFFER_OVERFLOW, "error: potential buffer overflow");
		return BUFFER_OVERFLOW;
	}
	Node *node;
        Pointer *e = newPointer2(expr);
        if(e == NULL) {
            addRErrorMsg(errmsg, 01, "error: can not create a Pointer.");
            return -1;
        }
        int nor;
	parseRuleRuleGen(e, &node, &nor, 0, errmsg, r);
        /* ignore rules other than the first rule */
	if(node==NULL) {
		addRErrorMsg(errmsg, OUT_OF_MEMORY, "error: out of memory.");
		return OUT_OF_MEMORY;
	} else if (node->type == ERROR) {
		char buf2[ERR_MSG_LEN];
                Label pos;
                getFPos(&pos, e);
		generateErrMsg("error: syntax error", pos.exprloc, pos.base, buf2);
                addRErrorMsg(errmsg, PARSER_ERROR, buf2);
                deletePointer(e);
		return PARSER_ERROR;
	} else {
            Token token;
            nextTokenRuleGen(e, &token, 0);

            if(token.type!=EOS) {
		char buf2[ERR_MSG_LEN];
                Label pos;
                getFPos(&pos, e);
                generateErrMsg("error: unparsed suffix",pos.exprloc, pos.base, buf2);
                addRErrorMsg(errmsg, UNPARSED_SUFFIX, buf2);
                deletePointer(e);
                return UNPARSED_SUFFIX;
            }
	}
    Hashtable *varTypes = newHashTable(100);
    
	Hashtable *funcDesc = newHashTable(100);
    getSystemFunctions(funcDesc, r);
    Hashtable *global = newHashTable(100);
    Env *env = newEnv(newHashTable(100), global, funcDesc);
    List *typingConstraints = newList(r);
    Node *errnode;
    ExprType *type = typeRule(node, funcDesc, varTypes, typingConstraints, errmsg, &errnode, r);

	deleteHashTable(varTypes, nop);

    int rescode;
    if(type->t!=T_ERROR) {
        addCmdExecOutToEnv(global, r);
        if(msParamArray!=NULL) {
            convertMsParamArrayToEnv(msParamArray, env->global, errmsg, r);
        }
        rescode = execRuleNodeRes(node, NULL, 0, env, rei, reiSaveFlag, errmsg,r);
        if(msParamArray!=NULL) {
            convertEnvToMsParamArray(msParamArray, env, errmsg, r);
        }
    } else {
        rescode = TYPE_ERROR;
    }
    deleteEnv(env, 3);

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

        /*Pointer *e = makePointer(params[i]);

        if(e == NULL) {
            strncpy(errmsg, "error: can not create temp file.", MAX_ERRMSG_LEN);
            return newErrorRes(r, -1);
        }

        nextTerm3(e, &node, MIN_PREC, 0,errmsg, r);*/
        node = newNode(STRING, params[i], 0, r);
	if(node==NULL) {
		addRErrorMsg(errmsg, OUT_OF_MEMORY, "error: out of memory.");
		return newErrorRes(r, OUT_OF_MEMORY);
	} else if (node->type == ERROR) {
		/*char buf[MAX_COND_LEN]; */
                /*int pos = getFPos(e); */
		/*memcpy(buf, actionName, pos*sizeof(char)); */
		/*buf[pos] = '\0'; */
		/*snprintf(errmsg, MAX_ERRMSG_LEN, "error: syntax error at %s%s%s", buf, ERROR_INDICATOR, actionName+pos); */
                /*freePointer(e); */
		return newErrorRes(r, PARSER_ERROR);
	} else {
/*            Token token; */
/*            nextToken3(e, &token, 0); */

/*            if(token.type!=EOS) { */
/*                char buf[MAX_COND_LEN]; */
/*                int pos = getFPos(e); */
/*		memcpy(buf, actionName, pos*sizeof(char)); */
/*		buf[pos] = '\0'; */
/*                snprintf(errmsg, MAX_ERRMSG_LEN, "error: unparsed suffix at %s%s%s",buf, ERROR_INDICATOR, actionName+pos); */
/*                freePointer(e); */
/*                return newErrorRes(r, UNPARSED_SUFFIX); */
/*            } */
	}

        paramNodes[i] = node;
    }

    Node *node = createFunctionNode(actionName, paramNodes, paramsCount, NULL, r);
    Hashtable *funcDesc = newHashTable(100);
    Hashtable *global = newHashTable(100);
    getSystemFunctions(funcDesc, r);
    Env *env = newEnv(newHashTable(100), global, funcDesc);
    if(msParamArray!=NULL) {
        convertMsParamArrayToEnv(msParamArray, env->global, errmsg, r);
    }
    Res *res = computeExpressionNode(node, env, rei, reiSaveFlag, errmsg,r);
    deleteEnv(env, 3);
    return res;
}
ExprType *typeRule(Node *node, Hashtable *funcDesc, Hashtable *varTypes, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
            /* printf("%s\n", node->subtrees[0]->text); */
            freeRErrorContent(errmsg);
            ExprType *resType = typeExpression3(node->subtrees[1], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
            /*printf("Type %d\n",resType->t); */
            if(resType->t == T_ERROR) {
                goto error;
            }
            resType = typeExpression3(node->subtrees[2], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
            if(resType->t == T_ERROR) {
                goto error;
            }
            resType = typeExpression3(node->subtrees[3], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
            if(resType->t == T_ERROR) {
                goto error;
            }
            /* printVarTypeEnvToStdOut(varTypes); */
            switch(solveConstraints(typingConstraints, varTypes, errmsg, errnode, r)) {
                case 0:
                    goto error;
                default:
                    break;
            }
            postProcessCoercion(node, varTypes, errmsg, errnode, r);
            postProcessActions(node, funcDesc, errmsg, errnode, r);
            freeRErrorContent(errmsg);
            return newSimpType(T_INT, r);
            /*printTree(node, 0); */
            char buf[ERR_MSG_LEN];
        error:
            snprintf(buf, ERR_MSG_LEN, "type error: in rule %s", node->subtrees[0]->text);
            addRErrorMsg(errmsg, -1, buf);
            char *errbuf = (char *) malloc(ERR_MSG_LEN*1024*sizeof(char));
            errMsgToString(errmsg, errbuf, ERR_MSG_LEN*1024);
#ifdef DEBUG
            writeToTmp("ruleerr.log", errbuf);
            writeToTmp("ruleerr.log", "\n");
#endif
            rodsLog (LOG_ERROR, "%s", errbuf);
            free(errbuf);
            freeRErrorContent(errmsg);
            return resType;

}

void typingConstraintsToString(List *typingConstraints, Hashtable *var_types, char *buf, int bufsize) {
    char buf2[1024];
    char buf3[1024];
    ListNode *p = typingConstraints->head;
    buf[0] = '\0';
    while(p!=NULL) {
        snprintf(buf + strlen(buf), bufsize-strlen(buf), "%s<%s\n",
                typeToString(((TypingConstraint *)p->value)->a, NULL, /*var_types,*/ buf2, 1024),
                typeToString(((TypingConstraint *)p->value)->b, NULL, /*var_types,*/ buf3, 1024));
        p=p->next;
    }
}
ExprType *typeRuleSet(RuleSet *ruleset, rError_t *errmsg, Node **errnode, Region *r) {
    Hashtable *funcDesc = newHashTable(100);
    getSystemFunctions(funcDesc, r);
    ExprType *res;
    int i;
    for(i=0;i<ruleset->len;i++) {
        Hashtable *varTypes = newHashTable(100);
        List *typingConstraints = newList(r);
        ExprType *restype = typeRule(ruleset->rules[i], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
        /*char buf[1024]; */
        /*typingConstraintsToString(typingConstraints, NULL, buf, 1024); */
        /*printf("rule %s, typing constraints: %s\n", ruleset->rules[i]->subtrees[0]->text, buf); */
        deleteHashTable(varTypes, nop);
        if(restype->t == T_ERROR) {
            res = restype;
            RETURN;
        }
    }
    res = newSimpType(T_INT, r); /* Although a rule set does not have type T_INT, return T_INT to indicate success. */

ret:
    deleteHashTable(funcDesc, nop);
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
        resType = typeExpression3(node, env->funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
        /*printf("Type %d\n",resType->t); */
        if(resType->t == T_ERROR) {
            addRErrorMsg(errmsg, -1, "type error: in rule");
            res = newErrorRes(r,-1);
            RETURN;
        }
        postProcessCoercion(node, varTypes, errmsg, errnode, r);
        postProcessActions(node, env->funcDesc, errmsg, errnode, r);
        /*    printTree(node, 0); */
        deleteHashTable(varTypes, nop);
        varTypes = NULL;
        node->typed = 1;
    }
    res = evaluateExpression3(node, rei, reiSaveFlag, env, errmsg, rNew);

    switch (TYPE(res)) {
        case T_ERROR:
            addRErrorMsg(errmsg, -1, "error: in rule");
            break;
        default:
            break;
    }
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
    Res *res;
    char buf[ERR_MSG_LEN>1024?ERR_MSG_LEN:1024];
    int rulegen;
    
#ifdef DEBUG
    snprintf(buf, 1024, "parseAndComputeExpression: %s\n", expr);
    writeToTmp("entry.log", buf);
#endif
    if(overflow(expr, MAX_COND_LEN)) {
            addRErrorMsg(errmsg, BUFFER_OVERFLOW, "error: potential buffer overflow");
            return newErrorRes(r, BUFFER_OVERFLOW);
    }
    Node *node;
    Pointer *e = newPointer2(expr);
    if(e == NULL) {
        addRErrorMsg(errmsg, -1, "error: can not create pointer.");
        res = newErrorRes(r, -1);
        RETURN;
    }
    rulegen = strstr(expr, "##")==NULL?1:0;
    nextTermRuleGen(e, &node, MIN_PREC, rulegen,errmsg, r);
    if(node==NULL) {
            addRErrorMsg(errmsg, OUT_OF_MEMORY, "error: out of memory.");
            res = newErrorRes(r, OUT_OF_MEMORY);
            RETURN;
    } else if (node->type == ERROR) {
            char buf2[ERR_MSG_LEN];
            Label pos;
            getFPos(&pos, e);
            generateErrMsg("error: syntax error",pos.exprloc, pos.base, buf2);
            addRErrorMsg(errmsg, PARSER_ERROR, buf);
            deletePointer(e);
            res = newErrorRes(r, PARSER_ERROR);
            RETURN;
    } else {
        Token token;
        nextTokenRuleGen(e, &token, 0);

        if(token.type!=EOS) {
            char buf2[ERR_MSG_LEN];
            Label pos;
            getFPos(&pos, e);
            generateErrMsg("error: unparsed suffix",pos.exprloc, pos.base, buf2);
            addRErrorMsg(errmsg, UNPARSED_SUFFIX, buf2);
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
            Node *ruleNode = inRuleSet->rules[i];
            if(ruleNode == NULL)
                continue;
            char *key = ruleNode->subtrees[0]->text;
            int arity = ruleNode->subtrees[0]->degree;

            ExprType **paramTypes = (ExprType**) region_alloc(r, sizeof(ExprType *)*arity);
            int k;
            for(k=0;k<arity;k++) {
                paramTypes[k] = newTVar(r);
            }
            ExprType *ruleType = newFuncType(arity, paramTypes, newSimpType(T_INT, r), r);

            if (insertIntoHashTable(symbol_type_table, key,ruleType) == 0) {
                    return 0;
            }
	}
	return 1;
}

int initializeEnv(Node *ruleHead, Res *args[MAX_NUM_OF_ARGS_IN_ACTION], int argc, Hashtable *env, Region *r) {


	Node** args2 = ruleHead->subtrees;
/*	int argc2 = ruleHead->degree; */
	int i;
        /*getSystemFunctions(env, r); */
	for (i = 0; i < argc ; i++) {
		insertIntoHashTable(env, args2[i]->text, args[i]);
	}
	return (0);
}


void copyFromEnv(Res **args, char **inParams, int inParamsCount, Hashtable *env, Region *r) {
	int i;
	for(i=0;i<inParamsCount;i++) {
		args[i]= cpRes((Res *)lookupFromHashTable(env, inParams[i]),r);
	}
}
Node* getRuleNode(int ri)
{

	if (ri < MAX_NUM_APP_RULES) {
		return appRules.rules[ri];
	} else {
		ri = ri - MAX_NUM_APP_RULES;
		return coreRules.rules[ri];
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

Res *parseAndComputeExpressionNewEnv(char *inAction, msParamArray_t *inMsParamArray,
		  ruleExecInfo_t *rei, int reiSaveFlag, Region *r) {
    int freeRei = 0;
    if(rei == NULL) {
        rei = (ruleExecInfo_t *) malloc(sizeof(ruleExecInfo_t));
        memset(rei, 0, sizeof(ruleExecInfo_t));
        freeRei = 1;
    }
    rei->status = 0;
    Env *env = newEnv(newHashTable(100),newHashTable(100),newHashTable(100));
    addCmdExecOutToEnv(env->global, r);
    getSystemFunctions(env->funcDesc, r);

    Res *res;
    rError_t errmsgBuf;
    errmsgBuf.errMsg = NULL;
    errmsgBuf.len = 0;
    if(inMsParamArray!=NULL) {
        convertMsParamArrayToEnv(inMsParamArray, env->current, &errmsgBuf, r);
    }

    res = parseAndComputeExpression(inAction, env, rei, reiSaveFlag, &errmsgBuf, r);
    if(rei->msParamArray != NULL) {
        clearMsParamArray(rei->msParamArray, 0);
    	convertEnvToMsParamArray(rei->msParamArray, env, &errmsgBuf, r);
    }
    
    deleteEnv(env, 3);
    if(TYPE(res)==T_ERROR) {
        logErrMsg(&errmsgBuf);
    }
    freeRErrorContent(&errmsgBuf);
    if(freeRei) {
        free(rei);
    }
    return res;

}
