/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "index.h"
#include "functions.h"
#include "arithmetics.h"
#include "datetime.h"

#ifndef DEBUG
#include "execMyRule.h"
#include "msParam.h"
#include "reFuncDefs.h"
#include "rsMisc.h"
#include "stringOpr.h"
#endif

/* precond: len(valueOrExpression) < size(desc->valueOrExpression) */
FunctionDesc *newFunctionDesc(char *valueOrExpression, char *type, SmsiFuncPtrType func, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    /*desc->arity = arity; */
    desc->func = func;
    desc->next = NULL;
    desc->type = type == NULL? NULL:parseFuncTypeFromString(type, r);
    desc->fdtype = FD_FUNC;
    strcpy(desc->inOutValExp, valueOrExpression);
    return desc;
}
FunctionDesc *newConstructorDesc(char *type, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    /*desc->arity = arity; */
    desc->next = NULL;
    desc->type = parseFuncTypeFromString(type, r);
    desc->fdtype = FD_CONS;
    int i;
    for(i=0;i<T_FUNC_ARITY(desc->type);i++) {
        desc->inOutValExp[i] = 'i';
    }
    switch(T_FUNC_VARARG(desc->type)) {
        case ONCE:
            break;
        case PLUS:
            desc->inOutValExp[i++] = '+';
            break;
        case STAR:
            desc->inOutValExp[i++] = '*';
            break;
    }
    desc->inOutValExp[i++] = '\0';
    return desc;
}
FunctionDesc *newDeconstructorDesc(char *type, int proj, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    /*desc->arity = arity; */
    desc->next = NULL;
    desc->type = type == NULL? NULL:parseFuncTypeFromString(type, r);
    desc->fdtype = FD_DECONS;
    desc->proj = proj;
    strcpy(desc->inOutValExp, "i");
    return desc;
}
FunctionDesc *newFunctionDescChain(FunctionDesc *curr, FunctionDesc * next) {
    curr->next = next;
    return curr;
}
Node *wrapToActions(Node *node, Region *r) {
    if(node->nodeType!=N_ACTIONS) {
        Node *actions[1];
        actions[0] = node;
        Label expr;
        expr.base = node->base;
        expr.exprloc = node->expr;
        return createActionsNode(actions, 1, &expr, r);
    }
    return node;
}
Res *smsi_ifExec(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r) {
    Res *res = evaluateExpression3((Node *)params[0], 0,rei,reiSaveFlag,env,errmsg,r);
    if(TYPE(res) == T_ERROR) {
        return res;
    }
    if(res->value.dval == 0) {
        return evaluateActions(wrapToActions(params[2],r), wrapToActions(params[4],r),rei, reiSaveFlag, env, errmsg, r);
    } else {
        return evaluateActions(wrapToActions(params[1],r), wrapToActions(params[3],r),rei, reiSaveFlag, env, errmsg, r);
    }
}

Res *smsi_if2Exec(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r) {
    Res *res = evaluateExpression3((Node *)params[0], 0,rei,reiSaveFlag,env,errmsg,r);
    if(TYPE(res) == T_ERROR) {
        return res;
    }
    if(res->value.dval == 0) {
        return evaluateExpression3((Node *)params[2], 0,rei,reiSaveFlag,env,errmsg,r);
    } else {
        return evaluateExpression3((Node *)params[1], 0,rei,reiSaveFlag,env,errmsg,r);
    }
}

Res *smsi_do(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        switch(((Node *)params[0])->nodeType) {
            case N_ACTIONS:
                return evaluateActions((Node *)params[0], NULL, rei, reiSaveFlag,env, errmsg, r);
            default:
                return evaluateExpression3((Node *)params[0], 0,rei,reiSaveFlag,env,errmsg,r);
        }

}
Res *smsi_letExec(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r) {
    Res *res = evaluateExpression3(params[1], 0,rei,reiSaveFlag,env,errmsg,r);
    if(TYPE(res) == T_ERROR) {
            return res;
    }
    Env *nEnv = newEnv(newHashTable(100), env, env->funcDesc);
    Res *pres = matchPattern(params[0], res, nEnv, rei, reiSaveFlag, errmsg, r);
    if(TYPE(pres)==T_ERROR) {
        deleteEnv(nEnv, 1);
        return pres;
    }
/*                printTree(params[2], 0); */
    res = evaluateExpression3(params[2], 0,rei,reiSaveFlag,nEnv,errmsg,r);
    deleteEnv(nEnv, 1);
    return res;
}

Res *smsi_whileExec(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

    Res *cond, *res;
                    Region *rnew = make_region(0, r->label);
		while(1) {

			cond = evaluateExpression3((Node *)params[0], 0,rei,reiSaveFlag, env,errmsg,rnew);
			if(TYPE(cond) == T_ERROR) {
				res = cond;
                                break;
			}
			if(cond->value.dval == 0) {
                                res = newIntRes(r, 0);
                                break;
			}
			res = evaluateActions((Node *)params[1],(Node *)params[2], rei,reiSaveFlag, env,errmsg,rnew);
			if(TYPE(res) == T_ERROR) {
                            break;
			} else
			if(TYPE(res) == T_BREAK) {
                            res =
				newIntRes(r, 0);
                            break;
			} else
			if(TYPE(res) == T_SUCCESS) {
                            break;
			}
                    cpEnv(env, r);
                    region_free(rnew);

                    rnew = make_region(0, r->label);
                }
                    cpEnv(env, r);
                    res = cpRes(res, r);
                    region_free(rnew);
		return res;

}

Res *smsi_forExec(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

    Res *init, *cond, *res, *step;
    Region *rnew = make_region(0, r->label);
    init = evaluateExpression3((Node *)params[0], 0,rei,reiSaveFlag, env,errmsg,rnew);
    if(TYPE(init) == T_ERROR) {
        res = init;
        cpEnv(env, r);
        res =        cpRes(res, r);
        region_free(rnew);
        return res;
        }
    while(1) {

        cond = evaluateExpression3((Node *)params[1], 0,rei,reiSaveFlag, env,errmsg,rnew);
        if(TYPE(cond) == T_ERROR) {
            res = cond;
            break;
        }
        if(cond->value.dval == 0) {
            res = newIntRes(r, 0);
            break;
        }
        res = evaluateActions((Node *)params[3],(Node *)params[4], rei,reiSaveFlag, env,errmsg,rnew);
        if(TYPE(res) == T_ERROR) {
            break;
        } else
        if(TYPE(res) == T_BREAK) {
            res =
                newIntRes(r, 0);
            break;
        } else
        if(TYPE(res) == T_SUCCESS) {
            break;
        }
        step = evaluateExpression3((Node *)params[2], 0,rei,reiSaveFlag, env,errmsg,rnew);
        if(TYPE(step) == T_ERROR) {
            res = step;
            break;
        }
        cpEnv(env, r);
        region_free(rnew);

        rnew = make_region(0, r->label);
    }
    cpEnv(env, r);
    res = cpRes(res, r);
    region_free(rnew);
    return res;

}

Res *smsi_forEachExec(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r)
{
    Res *res = newRes(r);
    char* varName = ((Node *)subtrees[0])->text;
        Res* orig = evaluateVar3(varName, ((Node *)subtrees[0]), rei, reiSaveFlag, env, errmsg, r);
        if(TYPE(orig) != T_CONS &&
           (TYPE(orig) != T_IRODS || (
                strcmp(orig->exprType->text, StrArray_MS_T) != 0 &&
                strcmp(orig->exprType->text, IntArray_MS_T) != 0 &&
                strcmp(orig->exprType->text, GenQueryOut_MS_T) != 0))) {
            char errbuf[ERR_MSG_LEN], errbuf2[ERR_MSG_LEN];
            snprintf(errbuf, ERR_MSG_LEN, "%s is not a collection type.", typeName_Res(orig));
            generateErrMsg(errbuf, ((Node *)subtrees[0])->expr, ((Node *)subtrees[0])->base, errbuf2);
            addRErrorMsg(errmsg, -1, errbuf2);
            return newErrorRes(r, -1);
        }
        res = newIntRes(r, 0);
        if(TYPE(orig) == T_CONS && strcmp(orig->exprType->text, LIST) == 0) {
            int i;
            Res* elem;
            for(i=0;i<orig->degree;i++) {
                    elem = orig->subtrees[i];
                    setVariableValue(varName, elem, rei, env, errmsg, r);
                    res = evaluateActions(subtrees[1], subtrees[2], rei,reiSaveFlag, env,errmsg,r);
                    if(TYPE(res) == T_ERROR) {
                        break;
                    } else
                    if(TYPE(res) == T_BREAK) {
                        break;
                    } else
                    if(TYPE(res) == T_SUCCESS) {
                        break;
                    }
            }
            if(TYPE(res)!=T_ERROR) {
                res = newIntRes(r,0);
            }
        } else {
            int i;
            Res* elem;
            int len = getCollectionSize(orig->exprType->text, orig->value.uninterpreted.inOutStruct, r);
            for(i=0;i<len;i++) {
                    elem = getValueFromCollection(orig->exprType->text, orig->value.uninterpreted.inOutStruct, i, r);
                    setVariableValue(varName, elem, rei, env, errmsg, r);
                    res = evaluateActions((Node *)subtrees[1], (Node *)subtrees[2], rei,reiSaveFlag,  env,errmsg,r);
                    if(TYPE(res) == T_ERROR) {
                            break;
                    }
                    if(TYPE(res) == T_BREAK) {
                            break;
                    }
            }
            if(TYPE(res)!=T_ERROR) {
                res = newIntRes(r,0);
            }

        }
        setVariableValue(varName, orig, rei, env, errmsg, r);
        return res;
}
Res *smsi_break(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	Res *	res = newRes(r);
        res->exprType = newSimpType(T_BREAK, r);
        return res;
}
Res *smsi_succeed(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	Res *	res = newRes(r);
        res->exprType = newSimpType(T_SUCCESS, r);
        return res;
}
Res *smsi_fail(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	Res *	res = newRes(r);
        res->exprType = newSimpType(T_ERROR, r);
        res->value.errcode = n == 0 ?FAIL_ACTION_ENCOUNTERED_ERR:(int)((Res *)subtrees[0])->value.dval;
        return res;
}


Res *smsi_assign(Node **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

    /* An smsi shares the same env as the enclosing rule. */
    /* Therefore, our modification to the env is reflected to the enclosing rule automatically. */
    Res *val = evaluateExpression3((Node *)subtrees[1], 0, rei, reiSaveFlag,  env, errmsg,r);
    matchPattern(subtrees[0], val, env, rei, reiSaveFlag, errmsg, r);
    if(TYPE(val)==T_ERROR) {
        return val;
    }
    char *varName;
    varName = subtrees[0]->text;
    return setVariableValue(varName, val, rei, env, errmsg, r);
}

Res *smsi_listvars(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
/*
		char buf2[MAX_COND_LEN];
		char buf3[MAX_COND_LEN];
		sprintf(buf2, "len: %d\n", rei->msParamArray->len);
		for(i=0;i<rei->msParamArray->len;i++) {
			msParam_t *mP = rei->msParamArray->msParam[i];
			if(i!=0)strncat(buf2, ",", MAX_COND_LEN);
			strncat(buf2, mP->label, MAX_COND_LEN);
			strncat(buf2, "=", MAX_COND_LEN);
			if(mP->inOutStruct == NULL)
				strncat(buf2, "<null>", MAX_COND_LEN);
			else	{
				if (strcmp(mP->type, DOUBLE_MS_T) == 0) { // if the parameter is an integer
					snprintf(buf3, MAX_COND_LEN, "%f:",*(double *)mP->inOutStruct);
				} else if (strcmp(mP->type, INT_MS_T) == 0) { // if the parameter is an integer
					snprintf(buf3, MAX_COND_LEN, "%d:",*(int *)mP->inOutStruct);
				} else if (strcmp(mP->type, STR_MS_T) == 0) { // if the parameter is a string
					snprintf(buf3, MAX_COND_LEN, "%s:",(char *)mP->inOutStruct);
				} else if(strcmp(mP->type, DATETIME_MS_T) == 0) {
					snprintf(buf3, MAX_COND_LEN, "%ld:",*(time_t *)mP->inOutStruct);
				} else {
					snprintf(buf3, MAX_COND_LEN, "<value>:");
				}
				strncat(buf2, buf3, MAX_COND_LEN);
				strncat(buf2, mP->type, MAX_COND_LEN);
			}
		}
*/
        char buf[1024];
        printHashtable(env->current, buf);
        Res *res= newStringRes(r, buf);
        return res;
    }
Res *smsi_listcorerules(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *coll = newCollRes(coreRules.len, newSimpType(T_STRING, r), r);
    int i;
    for(i=0;i<coreRules.len;i++) {
        coll->subtrees[i] = newStringRes(r, coreRules.rules[i]->node->subtrees[0]->text);
    }
    return coll;
}
Res *smsi_true(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        return newBoolRes(r, 1);
}
Res *smsi_false(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        return newBoolRes(r, 0);
}

Res *smsi_max(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		int init=0;
		double max=0;
		int i;
		for(i=0;i<n;i++) {
			double x = ((Res *)params[i])->value.dval;
			if(init==0) {
				max = x;
				init = 1;
			} else {
				max = x > max? x: max;
			}
		}
		Res *res = newDoubleRes(r, max);
                return res;
        }
Res *smsi_min(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		int init=0;
		double min=0;
		int i;
		for(i=0;i<n;i++) {
			double x = ((Res *)params[i])->value.dval;
			if(init==0) {
				min = x;
				init = 1;
			} else {
				min = x < min? x: min;
			}
		}
                Res *res = newDoubleRes(r, min);
                return res;
        }
        Res *smsi_average(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		double sum=0;
		int i;
		for(i=0;i<n;i++) {
			double x = ((Res *)params[i])->value.dval;
			sum += x;
		}
		Res *res = newDoubleRes(r, sum/n);
                return res;
	}
        Res *smsi_hd(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            if(params[0]->degree > 0) {
                return params[0]->subtrees[0];
            } else {
                addRErrorMsg(errmsg, -1, "error: hd: empty collection");
                return newErrorRes(r, -1);
            }
	}
        Res *smsi_tl(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            if(params[0]->degree > 0) {
		Res *res = newRes(r);
                ExprType *elemType = T_CONS_TYPE_ARG(((Res *)params[0])->exprType, 0);
		/* allocate memory for elements */
                res->exprType = newCollType(elemType, r);
                res->degree = params[0]->degree-1;
                res->subtrees = (Res **) region_alloc(r, sizeof(Res *)*res->degree);
		int i;
		for(i=0;i<res->degree;i++) {
			res->subtrees[i] = params[0]->subtrees[i+1];
		}
                return res;
            } else {
                addRErrorMsg(errmsg, -1, "error: tl: empty collection\n");
                return newErrorRes(r, -1);
            }
	}
        Res *smsi_cons(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            ExprType *elemType = params[0]->exprType;
            /* allocate memory for elements */
            res->exprType = newCollType(elemType, r);
            res->degree = params[1]->degree+1;
            res->subtrees = (Res **) region_alloc(r, sizeof(Res *)*res->degree);
            int i;
            res->subtrees[0] = params[0];
            for(i=1;i<res->degree;i++) {
                    res->subtrees[i] = params[1]->subtrees[i-1];
            }
                return res;
	}
        Res *smsi_setelem(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            Res *coll = params[0];
            Res *indexRes = params[1];
            Res *val = params[2];
            ExprType *elemType = coll->exprType;
            int index = (int)indexRes->value.dval;
            if(0>index || index >= coll->degree) {
                char buf[ERR_MSG_LEN];
                snprintf(buf, ERR_MSG_LEN, "setelem: index out of bound %d", index);
                addRErrorMsg(errmsg, -1, buf);
                return newErrorRes(r, -1);
            }

            /* allocate memory for elements */
            res->exprType = newCollType(elemType, r);
            res->degree = coll->degree;
            res->subtrees = (Res **) region_alloc(r, sizeof(Res *)*res->degree);
            memcpy(res->subtrees, coll->subtrees, sizeof(Res *)*res->degree);
            res->subtrees[index] = val;
            return res;
	}
        Res *smsi_list(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            ExprType *elemType =
                n == 0?newSimpType(T_DYNAMIC, r):params[0]->exprType;
            /* allocate memory for elements */
            res->exprType = newCollType(elemType, r);
            res->degree = n;
            res->subtrees = (Res **) region_alloc(r, sizeof(Res *)*n);
            int i;
            for(i=0;i<n;i++) {
                    res->subtrees[i] = params[i];
            }
            return res;
	}
        Res *smsi_tuple(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            /* allocate memory for element types */
            ExprType **elemTypes = (ExprType **)region_alloc(r, n*sizeof(ExprType *));
            int i;
            for(i=0;i<n;i++) {
                elemTypes[i] = params[i]->exprType;
            }
            res->exprType = newConsType(n, cpStringExt(TUPLE, r), elemTypes, r);
            res->degree = n;
            res->text = cpStringExt(TUPLE, r);
            /* allocate memory for elements */
            res->subtrees = (Res **) region_alloc(r, sizeof(Res *)*n);
            for(i=0;i<n;i++) {
                res->subtrees[i] = params[i];
            }
            return res;
	}
        Res *smsi_elem(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            char errbuf[ERR_MSG_LEN];
            int index = (int)params[1]->value.dval;
            if(TYPE(params[0]) == T_CONS) {
                if(index <0 || index >= params[0]->degree ) {
                    snprintf(errbuf, ERR_MSG_LEN, "error: index out of range %d.", index);
                    addRErrorMsg(errmsg, -1, errbuf);
                    return newErrorRes(r, -1);
                }
                Res *res = params[0]->subtrees[index];
                return res;
            } else {
                if(index <0 || index >= getCollectionSize(params[0]->exprType->text,
                        params[0]->value.uninterpreted.inOutStruct, r) ) {
                    snprintf(errbuf, ERR_MSG_LEN, "error: index out of range %d. %s", index, ((Res *)params[0])->exprType->text);
                    addRErrorMsg(errmsg, -1, errbuf);
                    return newErrorRes(r, -1);
                }
                Res *res2 = getValueFromCollection(params[0]->exprType->text,
                        params[0]->value.uninterpreted.inOutStruct,
                        index,r);

                return res2;
            }
	}
        Res *smsi_size(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res * res = newRes(r);
            res->exprType = newSimpType(T_INT,r);
                if(TYPE(params[0]) == T_CONS) {
                    res->value.dval = params[0]->degree;
                } else {
                    res->value.dval = getCollectionSize(params[0]->exprType->text,
                            params[0]->value.uninterpreted.inOutStruct,r);
                }
                return res;
	}
        Res *smsi_datetime(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
                char errbuf[ERR_MSG_LEN];
                Res *res = newRes(r);
		Res* timestr = params[0];
		char* format;
		if(TYPE(params[0])!=T_STRING ||
			(n == 2 && TYPE(params[1])!=T_STRING)) { /* error not a string */
                        res->exprType = newSimpType(T_ERROR,r);
			res->value.errcode = UNSUPPORTED_OP_OR_TYPE;
                        snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not apply datetime to type (%s[,%s]).", typeName_Res((Res *)params[0]), n==2?typeName_Res((Res *)params[1]):"null");
                        addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
		} else {
			if(n == 2) {
				format = params[1]->text;
			} else {
				format="";
			}
			strttime(timestr->text, format, &(res->value.tval));
                        res->exprType = newSimpType(T_DATETIME,r);
		}
                return res;
        }

Res *smsi_time(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        time_t t;
        time(&t);
        Res*res = newDatetimeRes(r, t);
        return res;
}
Res *smsi_timestr(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        char errbuf[ERR_MSG_LEN];
        Res *res = newRes(r);
        Res* dtime = params[0];
        char* format;
        if(TYPE(params[0])!=T_DATETIME ||
            (n == 2 && TYPE(params[1])!=T_STRING)) {
            res->exprType = newSimpType(T_ERROR,r);
            res->value.errcode = UNSUPPORTED_OP_OR_TYPE;
            snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not apply datetime to type (%s[,%s]).", typeName_Res((Res *)params[0]), n==2?typeName_Res((Res *)params[1]):"null");
            addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
        } else {
            if(n == 2) {
                    format = params[1]->text;
            } else {
                    format = "";
            }
            char buf[1024];
            ttimestr(buf, 1024-1, format, &dtime->value.tval);
            res = newStringRes(r, buf);
        }
        return res;
}
Res *smsi_type(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        Res *val = params[0], *res;
        char typeName[128];
        typeToString(val->exprType, NULL, typeName, 128);
        res=newStringRes(r, typeName);
        return res;
}
Res *smsi_arity(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		Res *val = params[0];
                int ruleInx = -1;
		if(findNextRule2(val->text, &ruleInx)<0) {
                    return newErrorRes(r, -1);
                }
                if(ruleInx >= MAX_NUM_APP_RULES)
                    return newIntRes(r, RULE_NODE_NUM_PARAMS(coreRules.rules[ruleInx-MAX_NUM_APP_RULES]->node));
                else
                    return newIntRes(r, RULE_NODE_NUM_PARAMS(appRules.rules[ruleInx]->node));
}
Res *smsi_str(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
                char errbuf[ERR_MSG_LEN];
		Res *val = params[0], *res;
		if(TYPE(val) == T_INT
		|| TYPE(val) == T_DOUBLE
		|| TYPE(val) == T_BOOL
		|| TYPE(val) == T_CONS
		|| TYPE(val) == T_STRING
		|| TYPE(val) == T_DATETIME) {
                    char *buf = convertResToString(val);
                    if(buf != NULL) {
                        res = newStringRes(r, buf);
                        free(buf);
                    } else {
                        res = newErrorRes(r, UNSUPPORTED_OP_OR_TYPE);
                        snprintf(errbuf, ERR_MSG_LEN, "error: converting value of type %s to string.", typeName_Res(val));
                        addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);

                    }
		} else {
                    res = newErrorRes(r, UNSUPPORTED_OP_OR_TYPE);
                    snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not convert %s to string.", typeName_Res(val));
                    addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
		}
                return res;
}
Res *smsi_double(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
                char errbuf[ERR_MSG_LEN];
                Res *val = params[0], *res = newRes(r);
		if(TYPE(val) == T_STRING) {
                    res->exprType = newSimpType(T_DOUBLE,r);
                    res->value.dval = atof(val->text);
		} else if(TYPE(val) == T_DATETIME) {
                    res->exprType = newSimpType(T_DOUBLE,r);
                    res->value.dval = (double)val->value.tval;
                } else if(TYPE(val) == T_DOUBLE) {
                    res = val;
                } else {
                    res->exprType = newSimpType(T_ERROR,r);
                    res->value.errcode = UNSUPPORTED_OP_OR_TYPE;
                    snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not convert %s to double.", typeName_Res(val));
                    addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
		}
                return res;
}
Res *smsi_int(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        char errbuf[ERR_MSG_LEN];
        Res *val = params[0], *res = newRes(r);
        if(TYPE(val) == T_STRING) {
            res->exprType = newSimpType(T_INT,r);
            res->value.dval = atoi(val->text);
        } else if(TYPE(val) == T_DOUBLE) {
            res->exprType = newSimpType(T_INT, r);
            res->value.dval = val->value.dval;
        } else if(TYPE(val) == T_INT) {
            res = val;
        } else {
            res->exprType = newSimpType(T_ERROR,r);
            res->value.errcode = UNSUPPORTED_OP_OR_TYPE;
            snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not convert %s to double.", typeName_Res(val));
            addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
        }
        return res;
}
Res *smsi_lmsg(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        writeToTmp("re.log", params[0]->text);
        Res *res = newIntRes(r, 0);
        return res;
}
Res *smsi_not(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    return newBoolRes(r, !(int)params[0]->value.dval?1:0);

}
Res *smsi_negate(Node **args, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, -(int)args[0]->value.dval);
    } else {
        return newDoubleRes(r, -args[0]->value.dval);
    }
}
Res *smsi_abs(Node **args, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    double val = args[0]->value.dval;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)(val < 0?-val:val));
    } else {
        return newDoubleRes(r, (val < 0?-val:val));
    }
}
Res *smsi_exp(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval;
    return newDoubleRes(r, exp(val));
}
Res *smsi_log(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval;
    return newDoubleRes(r, log(val));

}
Res *smsi_floor(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval;
    return newDoubleRes(r, floor(val));

}
Res *smsi_ceiling(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval;
    return newDoubleRes(r, ceil(val));

}

Res *smsi_and(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    return newBoolRes(r, ((int)args[0]->value.dval)&&((int)args[1]->value.dval)?1:0);

}

Res *smsi_or(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    return newBoolRes(r, ((int)args[0]->value.dval)||((int)args[1]->value.dval)?1:0);
}

Res *smsi_add(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval+args[1]->value.dval;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}
Res *smsi_subtract(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval-args[1]->value.dval;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}
Res *smsi_multiply(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.dval*args[1]->value.dval;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}
Res *smsi_divide(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(args[1]->value.dval == 0) {
            addRErrorMsg(errmsg,DIVISION_BY_ZERO, "error: division by zero.");
            return newErrorRes(r, DIVISION_BY_ZERO);
    }
    double val = args[0]->value.dval/args[1]->value.dval;
        return newDoubleRes(r, val);
}

Res *smsi_modulo(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(args[1]->value.dval == 0) {
            addRErrorMsg(errmsg, DIVISION_BY_ZERO, "error: division by zero.");
            return newErrorRes(r, DIVISION_BY_ZERO);
    }
    double val = ((int)args[0]->value.dval)%((int)args[1]->value.dval);
    if(TYPE(args[0])==T_INT&&TYPE(args[1])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}

Res *smsi_power(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = pow(args[0]->value.dval, args[1]->value.dval);
    return newDoubleRes(r, val);
}
Res *smsi_root(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(args[1]->value.dval == 0) {
        addRErrorMsg(errmsg, DIVISION_BY_ZERO, "error: division by zero.");
        return newErrorRes(r, DIVISION_BY_ZERO);
    }
    double val = pow(args[0]->value.dval, 1/args[1]->value.dval);
    return newDoubleRes(r, val);
}

Res *smsi_concat(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
/*    if(args[0]->value.s.len+args[1]->value.s.len>=1024) {
        //error
        addRErrorMsg(errmsg, STRING_OVERFLOW, "error: string too long.");
        return newErrorRes(r, STRING_OVERFLOW);
    } else {*/
    char *newbuf = (char *)malloc((args[0]->value.strlen + args[1]->value.strlen+1)*sizeof(char));

    strcpy(newbuf, args[0]->text);
    strcpy(newbuf+args[0]->value.strlen, args[1]->text);

    Res *res = newStringRes(r, newbuf);
    free(newbuf);
    return res;
    /*}*/
}

Res *smsi_lt(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.dval < args[1]->value.dval?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.tval, args[1]->value.tval)<0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->text, args[1]->text) <0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);

}
Res *smsi_le(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.dval <= args[1]->value.dval?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.tval, args[1]->value.tval)<=0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->text, args[1]->text) <=0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);

}
Res *smsi_gt(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.dval > args[1]->value.dval?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.tval, args[1]->value.tval)>0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->text, args[1]->text) >0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);

}
Res *smsi_ge(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.dval >= args[1]->value.dval?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.tval, args[1]->value.tval)>=0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->text, args[1]->text)>=0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);
}
Res *smsi_eq(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.dval == args[1]->value.dval?1:0);
            break;
        case T_BOOL:
            if(TYPE(args[1]) == T_BOOL)
                return newBoolRes(r, args[0]->value.dval == args[1]->value.dval?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.tval, args[1]->value.tval)==0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->text, args[1]->text) ==0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);
}
Res *smsi_neq(Node **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.dval != args[1]->value.dval?1:0);
            break;
        case T_BOOL:
            if(TYPE(args[1]) == T_BOOL)
                return newBoolRes(r, args[0]->value.dval == args[1]->value.dval?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.tval, args[1]->value.tval)!=0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->text, args[1]->text) !=0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);
}
Res *smsi_like_regex(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **params = (Res **)paramsr;
    Res *res = newRes(r);
        char *pattern;
        char *bufstr;
        pattern = params[1]->text;
        bufstr = strdup(params[0]->text);
        #ifdef _POSIX_VERSION
        /* make the regexp match whole strings */
        char *buf2;
        buf2 = matchWholeString(pattern);
        regex_t regbuf;
        regcomp(&regbuf,buf2,REG_EXTENDED);
        res->exprType = newSimpType(T_BOOL,r);
        res->value.dval = regexec(&regbuf,	bufstr, 0,0,0)==0?1:0;
        regfree(&regbuf);
        #else
        res->value.dval = match(pattern, expr1->value.s)==TRUE?1:0;
        #endif
        /*res->type = 2;
        strcpy(res->value.s,expr1->value.s);
        strcat(res->value.s,",");
        strcat(res->value.s,bufstr);
        strcat(res->value.s,",");
        strcat(res->value.s,expr2->value.s);
        strcat(res->value.s,",");
        strcat(res->value.s,buf);*/
        /*strcat(res->value.s,","); */
        /*strcat(res->value.s, ch); */
        free(buf2);
        free(bufstr);
        return res;
}
Res *smsi_notlike_regex(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **params = (Res **)paramsr;
    Res *res = newRes(r);

        char *pattern;
        char *bufstr;
        pattern = params[1]->text;
        bufstr = strdup(params[0]->text);
        #ifdef _POSIX_VERSION
        /* make the regexp match whole strings */
        char *buf2;
        buf2 = matchWholeString(pattern);

        regex_t regbuf;
        regcomp(&regbuf,buf2,REG_EXTENDED);
        res->exprType = newSimpType(T_BOOL,r);
        res->value.dval = regexec(&regbuf, bufstr, 0,0,0)==0?0:1;
        regfree(&regbuf);
        #else
        res->value.dval = match(buf, expr1->value.s)==FALSE?1:0;
        #endif
        /*char* ch=regexec(&regbuf, bufstr, 0,0,0)?"true":"false";
        res->type = 3;
        strcpy(res->value.s,expr1->value.s);
        strcat(res->value.s,",");
        strcat(res->value.s,bufstr);
        strcat(res->value.s,",");
        strcat(res->value.s,expr2->value.s);
        strcat(res->value.s,",");
        strcat(res->value.s,buf);
        strcat(res->value.s,",");
        strcat(res->value.s, ch);*/
        free(buf2);
        free(bufstr);
        return res;
}

Res *smsi_eval(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **params = (Res **)paramsr;
    /*printf("\neval: %s\n", params[0]->text); */
    return eval(params[0]->text, env, rei, reiSaveFlag, errmsg, r);
}

/**
 * Run node and return the errorcode.
 * If the execution is successful, the returned errorcode is 0.
 */
Res *smsi_errorcode(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *res;
    switch(((Node *)paramsr[0])->nodeType) {
            case N_ACTIONS:
                res = evaluateActions((Node *)paramsr[0], (Node *)paramsr[1], rei, reiSaveFlag,  env, errmsg, r);
            default:
                res = evaluateExpression3((Node *)paramsr[0], 0,rei,reiSaveFlag,env,errmsg,r);
        }
        switch(TYPE(res)) {
            case T_ERROR:
                return newIntRes(r, res->value.errcode);
            default:
                return newIntRes(r, 0);
        }
}

/**
 * Run node and return the errorcode.
 * If the execution is successful, the returned errorcode is 0.
 */
Res *smsi_errormsg(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    char *errbuf = (char *)malloc(ERR_MSG_LEN*1024*sizeof(char));
    Res *res;
    switch(((Node *)paramsr[0])->nodeType) {
            case N_ACTIONS:
                res = evaluateActions((Node *)paramsr[0], (Node *)paramsr[1], rei, reiSaveFlag,  env, errmsg, r);
            default:
                res = evaluateExpression3((Node *)paramsr[0], 0,rei,reiSaveFlag,env,errmsg,r);
    }
    paramsr[1] = newStringRes(r, errMsgToString(errmsg, errbuf, ERR_MSG_LEN*1024));
    freeRErrorContent(errmsg);
    free(errbuf);
    switch(TYPE(res)) {
        case T_ERROR:
            return newIntRes(r, res->value.errcode);
        default:
            return newIntRes(r, 0);
    }
}

Res *smsi_delayExec(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r)
{
  int i;
  char actionCall[MAX_ACTION_SIZE];
  char recoveryActionCall[MAX_ACTION_SIZE];
  char delayCondition[MAX_ACTION_SIZE];

  Res **params = (Res **)paramsr;

  rstrcpy(delayCondition, params[0]->text, MAX_ACTION_SIZE);
  rstrcpy(actionCall, params[1]->text, MAX_ACTION_SIZE);
  rstrcpy(recoveryActionCall, params[2]->text, MAX_ACTION_SIZE);

  msParamArray_t *tmp = rei->msParamArray;
  rei->msParamArray = newMsParamArray();

/*
  int ret = convertEnvToMsParamArray(rei->msParamArray, env, errmsg, r);
  if(ret!=0) {
      return newErrorRes(r, ret);
  }
*/

  i = _delayExec(actionCall, recoveryActionCall, delayCondition, rei);

  deleteMsParamArray(rei->msParamArray);
  rei->msParamArray = tmp;

  return newIntRes(r, i);
}

Res *smsi_remoteExec(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r)
{
  int i;
  execMyRuleInp_t execMyRuleInp;
  msParamArray_t *outParamArray = NULL;
  char tmpStr[LONG_NAME_LEN];

  Res **params = (Res **)paramsr;

  memset (&execMyRuleInp, 0, sizeof (execMyRuleInp));
  execMyRuleInp.condInput.len=0;
  rstrcpy (execMyRuleInp.outParamDesc, ALL_MS_PARAM_KW, LONG_NAME_LEN);

  rstrcpy (tmpStr, params[0]->text, LONG_NAME_LEN);
  parseHostAddrStr (tmpStr, &execMyRuleInp.addr);

  if(strlen(params[3]->text) == 0) {
      snprintf(execMyRuleInp.myRule, META_STR_LEN, "remExec{%s}", params[2]->text);
  } else {
      snprintf(execMyRuleInp.myRule, META_STR_LEN, "remExec||%s|%s", params[2]->text, params[3]->text);
  }
  addKeyVal(&execMyRuleInp.condInput,"execCondition",params[1]->text);

  rei->msParamArray = newMsParamArray();
  int ret = convertEnvToMsParamArray(rei->msParamArray, env, errmsg, r);
  if(ret!=0) {
      return newErrorRes(r, ret);
  }
  execMyRuleInp.inpParamArray = rei->msParamArray;

  i = rsExecMyRule (rei->rsComm, &execMyRuleInp,  &outParamArray);

  updateMsParamArrayToEnv(rei->msParamArray, env, errmsg, r);
  deleteMsParamArray(rei->msParamArray);
  rei->msParamArray = NULL;
  return newIntRes(r, i);
}
int writeStringNew(char *writeId, char *writeStr, Env *env, Region *r) {
  execCmdOut_t *myExecCmdOut;
  Res *execOutRes;

  if (writeId != NULL && strcmp (writeId, "serverLog") == 0) {
    rodsLog (LOG_NOTICE, "writeString: inString = %s", writeStr);
    return 0;
  }
  if ((execOutRes = (Res *)lookupFromEnv(env, "ruleExecOut")) != NULL) {
    myExecCmdOut = (execCmdOut_t *)execOutRes->value.uninterpreted.inOutStruct;
  } else {
      Env *global = env;
      while(global->previous != NULL) {
          global = global->previous;
      }
    myExecCmdOut = (execCmdOut_t *)malloc (sizeof (execCmdOut_t));
    memset (myExecCmdOut, 0, sizeof (execCmdOut_t));
    execOutRes = newRes(r);
    execOutRes->exprType  = newIRODSType(ExecCmdOut_MS_T, r);
    execOutRes->value.uninterpreted.inOutStruct = myExecCmdOut;
    insertIntoHashTable(global->current, "ruleExecOut", execOutRes);
  }

  if (!strcmp(writeId,"stdout"))
    appendToByteBufNew(&(myExecCmdOut->stdoutBuf),(char *) writeStr);
  else if (!strcmp(writeId,"stderr"))
    appendToByteBufNew(&(myExecCmdOut->stderrBuf),(char *) writeStr);
  return 0;
}

Res *smsi_writeLine(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
#ifdef DEBUG
    printf("%s\n", ((Res *)paramsr[1])->text);
    return newIntRes(r, 0);
#else
  Res *where = (Res *)paramsr[0];
  char *whereId = where->text;
  Res *inString = (Res *)paramsr[1];

  if (strcmp (whereId, "serverLog") == 0) {
      rodsLog (LOG_NOTICE, "writeLine: inString = %s\n", inString->text);
      return newIntRes(r, 0);
  }

  int i = writeStringNew(whereId, inString->text, env, r);

  if (i < 0)
    return newErrorRes(r, i);

  i = writeStringNew(whereId, "\n", env, r);

  if (i < 0)
    return newErrorRes(r, i);
  else
    return newIntRes(r, i);
#endif
}
Res *smsi_writeString(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

  Res *where = (Res *)paramsr[0];
  char *whereId = where->text;
  Res *inString = (Res *)paramsr[1];

  int i = writeStringNew(whereId, inString->text, env, r);

  if (i < 0)
    return newErrorRes(r, i);
  else
    return newIntRes(r, i);
}

Res *smsi_triml(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    /* if the length of delim is 0, strstr should return str */
    Res *strres = (Res *)paramsr[0];
    Res *delimres = (Res *)paramsr[1];

    char *str = strres->text;
    char *delim = delimres->text;

    char *p = strstr(str, delim);
    if(p!=NULL) {
        /* found */
        return newStringRes(r, p+strlen(delim));
    } else {
        /* not found return the original string */
        return strres;
    }



}
Res *smsi_strlen(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *strres = (Res *)paramsr[0];
    return newIntRes(r, strlen(strres->text));
}

Res *smsi_substr(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *strres = (Res *)paramsr[0];
    Res *startres = (Res *)paramsr[1];
    Res *finishres = (Res *)paramsr[2];

    char *buf = strdup(strres->text + (int)startres->value.dval);
    buf[(int)finishres->value.dval - (int)startres->value.dval] = '\0';

    Res *retres = newStringRes(r, buf);
    free(buf);
    return retres;
}

Res *smsi_trimr(Node **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *strres = (Res *)paramsr[0];
    Res *delimres = (Res *)paramsr[1];

    char *str = strres->text;
    char *delim = delimres->text;

    if(strlen(delim)==0) {
        return strres;
    }

    char *p = strstr(str, delim);
    char *newp = NULL;
    while(p!=NULL) {
        newp = p;
        p = strstr(p+1, delim);
    }
    if(newp == NULL) {
        /* not found */
        return strres;
    } else {
        /* found set where newp points to to \0 */
        char temp = *newp;
        *newp = '\0';

        Res *res = newStringRes(r, str);
        /* restore */
        *newp = temp;
        return res;
    }

}

/* utilities */
FunctionDesc *getFuncDescFromChain(int n, FunctionDesc *fDesc) {
            ExprType *fTypeCopy = fDesc->type;

            while((T_FUNC_VARARG(fTypeCopy) == ONCE && n != T_FUNC_ARITY(fTypeCopy))
                    ||(T_FUNC_VARARG(fTypeCopy) == STAR && n < T_FUNC_ARITY(fTypeCopy) - 1)
                    ||(T_FUNC_VARARG(fTypeCopy) == PLUS && n < T_FUNC_ARITY(fTypeCopy))) {
                if(fDesc->next == NULL) {
                    return NULL;
                }
                fDesc = fDesc->next;
                fTypeCopy = fDesc->type;
            }
            return fDesc;
}

Res* eval(char *expr, Env *env, ruleExecInfo_t *rei, int saveREI, rError_t *errmsg, Region *r) {
    Pointer *e = newPointer2(expr);
    Res *res = parseAndComputeExpression(expr, env, rei, saveREI, errmsg, r);
    deletePointer(e);
    return res;
}

int getParamIOType(char *iotypes, int index) {
    int l = strlen(iotypes);
    int repeat = 0;
    if(iotypes[l-1] == '*') {
        l--;
        repeat = 1;
    }

    if(l>index) {
        return iotypes[index];
    } else if(repeat) {
        return iotypes[l-1];
    } else { /* error */
        return -1;
    }
}


Node *construct(char *fn, Node **args, int argc, Node *constype, Region *r) {
    Node *res = newRes(r);
    res->text = cpStringExt(fn, r);
    res->degree = argc;
    res->subtrees = (Node **)region_alloc(r, sizeof(Node *)*argc);
    memcpy(res->subtrees, args, sizeof(Node *)*argc);
    res->exprType = constype->subtrees[1];
    return res;
}

Node *deconstruct(char *fn, Node **args, int argc, int proj, rError_t*errmsg, Region *r) {
    Node *res = args[0]->subtrees[proj];
    return res;
}

void getSystemFunctions(Hashtable *ft, Region *r) {
    insertIntoHashTable(ft, "do", newFunctionDesc("e", "0->?", smsi_do, r));
    insertIntoHashTable(ft, "eval", newFunctionDesc("i", "s->?", smsi_eval, r));
    insertIntoHashTable(ft, "errorcode", newFunctionDescChain(
            newFunctionDesc("aa", "i * i->i", smsi_errorcode, r),
            newFunctionDesc("e", "0->i", smsi_errorcode, r)));
    insertIntoHashTable(ft, "errormsg", newFunctionDescChain(
            newFunctionDesc("aao", "i * i * s->i", smsi_errormsg, r),
            newFunctionDesc("eo", "0 * s->i", smsi_errormsg, r)));
    insertIntoHashTable(ft, "let", newFunctionDesc("eee", "0 * f 0 * 1->1", smsi_letExec, r));
    insertIntoHashTable(ft, "if2", newFunctionDesc("eeeee", "b * 0 * 0 * i * i->0", smsi_if2Exec, r));
    insertIntoHashTable(ft, "if", newFunctionDesc("eeeee", "b * 0 * 1 * i * i->i", smsi_ifExec, r));
    insertIntoHashTable(ft, "ifExec", newFunctionDesc("eeeee", "b * 0 * 1 * i * i->i", smsi_ifExec, r));
    insertIntoHashTable(ft, "for", newFunctionDesc("eeeaa", "0 * b * 1 * 2 * i->i",smsi_forExec, r));
    insertIntoHashTable(ft, "forExec", newFunctionDesc("eeeaa", "0 * b * 1 * 2 * i->i", smsi_forExec, r));
    insertIntoHashTable(ft, "while", newFunctionDesc("eaa", "b * 0 * i->i",smsi_whileExec, r));
    insertIntoHashTable(ft, "whileExec", newFunctionDesc("eaa", "b * 0 * i->i", smsi_whileExec, r));
    insertIntoHashTable(ft, "foreach", newFunctionDesc("eaa", "c 0 * 1 * i->i", smsi_forEachExec, r));
    insertIntoHashTable(ft, "forEachExec", newFunctionDesc("eaa", "c 0 * 1 * i->i", smsi_forEachExec, r));
    insertIntoHashTable(ft, "break", newFunctionDesc("", "->i", smsi_break, r));
    insertIntoHashTable(ft, "succeed", newFunctionDesc("", "->i", smsi_succeed, r));
    insertIntoHashTable(ft, "fail", newFunctionDescChain(
            newFunctionDesc("i", "i->i", smsi_fail, r),
            newFunctionDesc("", "->i", smsi_fail, r)));
    insertIntoHashTable(ft, "assign", newFunctionDesc("ee", "0 * f 0->i", smsi_assign, r));
    insertIntoHashTable(ft, "lmsg", newFunctionDesc("i", "s->i", smsi_lmsg, r));
    insertIntoHashTable(ft, "listvars", newFunctionDesc("", "->s", smsi_listvars, r));
    insertIntoHashTable(ft, "listcorerules", newFunctionDesc("", "->c s", smsi_listcorerules, r));
    insertIntoHashTable(ft, "true", newFunctionDesc("", "->b", smsi_true, r));
    insertIntoHashTable(ft, "false", newFunctionDesc("", "->b", smsi_false, r));
    insertIntoHashTable(ft, "time", newFunctionDesc("", "->t", smsi_time, r));
    insertIntoHashTable(ft, "timestr", newFunctionDesc("i", "t->s", smsi_timestr, r));
    insertIntoHashTable(ft, "str", newFunctionDesc("i", "0->s", smsi_str, r));
    insertIntoHashTable(ft, "datetime", newFunctionDesc("i", "s->t", smsi_datetime, r));
    insertIntoHashTable(ft, "timestrf", newFunctionDesc("ii", "t * s->s", smsi_timestr, r));
    insertIntoHashTable(ft, "datetimef", newFunctionDesc("ii", "s * s->t", smsi_datetime, r));
    insertIntoHashTable(ft, "double", newFunctionDesc("i", "f 0{s d t}->d", smsi_double, r));
    insertIntoHashTable(ft, "int", newFunctionDesc("i", "0{i s d}->i", smsi_int, r));
    insertIntoHashTable(ft, "list", newFunctionDesc("i*", "forall X, X*->c X", smsi_list, r));
    insertIntoHashTable(ft, "tuple",
            newFunctionDescChain(newConstructorDesc("-> <>", r),
            newFunctionDescChain(newConstructorDesc("A-> <A>", r),
            newFunctionDescChain(newConstructorDesc("A * B-> <A * B>", r),
            newFunctionDescChain(newConstructorDesc("A * B * C-> <A * B * C>", r),
            newFunctionDescChain(newConstructorDesc("A * B * C * D-> <A * B * C * D>", r),
            newFunctionDescChain(newConstructorDesc("A * B * C * D * E-> <A * B * C * D * E>", r),
            newFunctionDescChain(newConstructorDesc("A * B * C * D * E * F * G-> <A * B * C * D * E * F * G>", r),
            newFunctionDescChain(newConstructorDesc("A * B * C * D * E * F * G * H-> <A * B * C * D * E * F * G * H>", r),
            newFunctionDescChain(newConstructorDesc("A * B * C * D * E * F * G * H * I-> <A * B * C * D * E * F * G * H * I>", r),
            newConstructorDesc("A * B * C * D * E * F * G * H * I * J-> <A * B * C * D * E * F * G * H * I * J>", r)
            ))))))))));
    insertIntoHashTable(ft, "elem", newFunctionDesc("ii", "forall X, c X * i->X", smsi_elem, r));
    insertIntoHashTable(ft, "setelem", newFunctionDesc("iii", "forall X, c X * i * X->c X", smsi_setelem, r));
    insertIntoHashTable(ft, "hd", newFunctionDesc("i", "forall X, c X->X", smsi_hd, r));
    insertIntoHashTable(ft, "tl", newFunctionDesc("i", "forall X, c X->c X", smsi_tl, r));
    insertIntoHashTable(ft, "cons", newFunctionDesc("ii", "forall X, X * c X->c X", smsi_cons, r));
    insertIntoHashTable(ft, "size", newFunctionDesc("i", "forall X, c X->i", smsi_size, r));
    insertIntoHashTable(ft, "type", newFunctionDesc("i", "forall X, X->s",smsi_type, r));
    insertIntoHashTable(ft, "arity", newFunctionDesc("i", "s->i",smsi_arity, r));
    insertIntoHashTable(ft, "+", newFunctionDesc("ii", "forall X in {i d}, f X * f X->X",smsi_add, r));
    insertIntoHashTable(ft, "++", newFunctionDesc("ii", "f s * f s->s",smsi_concat, r));
    insertIntoHashTable(ft, "-", newFunctionDescChain(
            newFunctionDesc("ii", "forall X in {i d}, f X * f X->X",smsi_subtract, r),
            newFunctionDesc("i", "forall X in {i d}, X-> X", smsi_negate, r)));
    insertIntoHashTable(ft, "*", newFunctionDesc("ii", "forall X in {i d}, f X * f X->X",smsi_multiply, r));
    insertIntoHashTable(ft, "/", newFunctionDesc("ii", "forall X in {i d}, f X * f X->X",smsi_divide, r));
    insertIntoHashTable(ft, "%", newFunctionDesc("ii", "forall X in {i d}, f X * f X->X",smsi_modulo, r));
    insertIntoHashTable(ft, "^", newFunctionDesc("ii", "f d * f d->d",smsi_power, r));
    insertIntoHashTable(ft, "@", newFunctionDesc("ii", "f d * f d->d",smsi_root, r));
    insertIntoHashTable(ft, "log", newFunctionDesc("i", "f d->d",smsi_log, r));
    insertIntoHashTable(ft, "exp", newFunctionDesc("i", "f d->d",smsi_exp, r));
    insertIntoHashTable(ft, "!", newFunctionDesc("i", "b->b",smsi_not, r));
    insertIntoHashTable(ft, "&&", newFunctionDesc("ii", "b * b->b",smsi_and, r));
    insertIntoHashTable(ft, "||", newFunctionDesc("ii", "b * b->b",smsi_or, r));
    insertIntoHashTable(ft, "%%", newFunctionDesc("ii", "b * b->b",smsi_or, r));
    insertIntoHashTable(ft, "==", newFunctionDesc("ii", "forall X in {i d b s t}, f X * f X->b",smsi_eq, r));
    insertIntoHashTable(ft, "!=", newFunctionDesc("ii", "forall X in {i d b s t}, f X * f X->b",smsi_neq, r));
    insertIntoHashTable(ft, ">", newFunctionDesc("ii", "forall X in {i d s t}, f X * f X->b", smsi_gt, r));
    insertIntoHashTable(ft, "<", newFunctionDesc("ii", "forall X in {i d s t}, f X * f X->b", smsi_lt, r));
    insertIntoHashTable(ft, ">=", newFunctionDesc("ii", "forall X in {i d s t}, f X * f X->b", smsi_ge, r));
    insertIntoHashTable(ft, "<=", newFunctionDesc("ii", "forall X in {i d s t}, f X * f X->b", smsi_le, r));
    insertIntoHashTable(ft, "floor", newFunctionDesc("i", "f d->i", smsi_floor, r));
    insertIntoHashTable(ft, "ceiling", newFunctionDesc("i", "f d->i", smsi_ceiling, r));
    insertIntoHashTable(ft, "abs", newFunctionDesc("i", "f d->d", smsi_abs, r));
    insertIntoHashTable(ft, "max", newFunctionDesc("i*", "f d+->d", smsi_max, r));
    insertIntoHashTable(ft, "min", newFunctionDesc("i*", "f d+->d", smsi_min, r));
    insertIntoHashTable(ft, "average", newFunctionDesc("i*", "f d+->d", smsi_average, r));
    insertIntoHashTable(ft, "like", newFunctionDesc("ii", "s * s->b", smsi_like_regex, r));
    insertIntoHashTable(ft, "not like", newFunctionDesc("ii", "s * s->b", smsi_notlike_regex, r));
    insertIntoHashTable(ft, "delayExec", newFunctionDesc("iii", "s * s * s->i", smsi_delayExec, r));
    insertIntoHashTable(ft, "remoteExec", newFunctionDesc("iiii", "s * s * s * s->i", smsi_remoteExec,r));
    insertIntoHashTable(ft, "writeLine", newFunctionDesc("ii", "s * s->i", smsi_writeLine,r));
    insertIntoHashTable(ft, "writeString", newFunctionDesc("ii", "s * s->i", smsi_writeString,r));
    insertIntoHashTable(ft, "triml", newFunctionDesc("ii","s * s->s", smsi_triml, r));
    insertIntoHashTable(ft, "trimr", newFunctionDesc("ii","s * s->s", smsi_trimr, r));
    insertIntoHashTable(ft, "strlen", newFunctionDesc("i","s->i", smsi_strlen, r));
    insertIntoHashTable(ft, "substr", newFunctionDesc("iii","s * i * i->s", smsi_substr, r));
    insertIntoHashTable(ft, "pair", newConstructorDesc("forall X, forall Y, X * Y-> <X * Y>", r));
    insertIntoHashTable(ft, "fst", newDeconstructorDesc("forall X, forall Y, <X * Y>->X", 0, r));
    insertIntoHashTable(ft, "snd", newDeconstructorDesc("forall X, forall Y, <X * Y>->Y", 1, r));


}
