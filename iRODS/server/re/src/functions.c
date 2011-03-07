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

// precond: len(valueOrExpression) < size(desc->valueOrExpression)
FunctionDesc *newFunctionDesc(char *valueOrExpression, char *type, void *func, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    //desc->arity = arity;
    desc->func = func;
    desc->next = NULL;
    desc->type = type == NULL? NULL:parseFuncTypeFromString(type, r);
    strcpy(desc->inOutValExp, valueOrExpression);
    return desc;
}
FunctionDesc *newFunctionDescChain(char *valueOrExpression, char *type, void *func, FunctionDesc * next, Region *r) {
    FunctionDesc *desc = newFunctionDesc(valueOrExpression, type, func, r);
    desc->next = next;
    return desc;
}

// precond: length of params == 3
Res *smsi_ifExec(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t* errmsg, Region *r) {
    Res *res = evaluateExpression3(params[0],rei,reiSaveFlag,env,errmsg,r);
    if(TYPE(res) == T_ERROR) {
            return res;
    }
//                printTree(params[2], 0);
    if(res->value.d == 0) {
        switch(((Node *)params[2])->type) {
            case ACTIONS:
                return evaluateActions(params[2], params[4], rei, reiSaveFlag, env, errmsg, r);
            default:
                return evaluateExpression3(params[2],rei,reiSaveFlag,env,errmsg,r);
        }

    } else {
        switch(((Node *)params[1])->type) {
            case ACTIONS:
                return evaluateActions(params[1], params[3], rei, reiSaveFlag, env, errmsg, r);
            default:
                return evaluateExpression3(params[1],rei,reiSaveFlag,env,errmsg,r);
    }
}
}

Res *smsi_do(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        switch(((Node *)params[0])->type) {
            case ACTIONS:
                return evaluateActions(params[0], NULL, rei, reiSaveFlag,env, errmsg, r);
            default:
                return evaluateExpression3(params[0],rei,reiSaveFlag,env,errmsg,r);
        }

}
Res *smsi_whileExec(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

    Res *cond, *res;
                    Region *rnew = make_region(0, r->label);
		while(1) {

			cond = evaluateExpression3(params[0],rei,reiSaveFlag, env,errmsg,rnew);
			if(TYPE(cond) == T_ERROR) {
				res = cond;
                                break;
			}
			if(cond->value.d == 0) {
                                res = newIntRes(r, 0);
                                break;
			}
			res = evaluateActions(params[1],params[2], rei,reiSaveFlag, env,errmsg,rnew);
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

Res *smsi_forExec(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

    Res *init, *cond, *res, *step;
    Region *rnew = make_region(0, r->label);
    init = evaluateExpression3(params[0],rei,reiSaveFlag, env,errmsg,rnew);
    if(TYPE(init) == T_ERROR) {
        res = init;
        cpEnv(env, r);
        res =        cpRes(res, r);
        region_free(rnew);
        return res;
        }
    while(1) {

        cond = evaluateExpression3(params[1],rei,reiSaveFlag, env,errmsg,rnew);
        if(TYPE(cond) == T_ERROR) {
            res = cond;
            break;
        }
        if(cond->value.d == 0) {
            res = newIntRes(r, 0);
            break;
        }
        res = evaluateActions(params[3],params[4], rei,reiSaveFlag, env,errmsg,rnew);
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
        step = evaluateExpression3(params[2],rei,reiSaveFlag, env,errmsg,rnew);
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

Res *smsi_forEachExec(void **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r)
{
    Res *res = newRes(r);
    char* varName = ((Node *)subtrees[0])->text;
        Res* orig = evaluateVar3(varName, ((Node *)subtrees[0]), rei, reiSaveFlag, env, errmsg, r);
        if(TYPE(orig) != CONS &&
           (TYPE(orig) != T_IRODS || (
                strcmp(orig->type->ext.irods.name, StrArray_MS_T) != 0 &&
                strcmp(orig->type->ext.irods.name, IntArray_MS_T) != 0 &&
                strcmp(orig->type->ext.irods.name, GenQueryOut_MS_T) != 0))) {
            char errbuf[ERR_MSG_LEN], errbuf2[ERR_MSG_LEN];
            snprintf(errbuf, ERR_MSG_LEN, "%s is not a collection type.", typeName_Res(orig));
            generateErrMsg(errbuf, ((Node *)subtrees[0])->expr, ((Node *)subtrees[0])->base, errbuf2);
            addRErrorMsg(errmsg, -1, errbuf2);
            return newErrorRes(r, -1);
        }
        res = newIntRes(r, 0);
        if(TYPE(orig) == CONS) {
            int i;
            Res* elem;
            for(i=0;i<orig->value.c.len;i++) {
                    elem = orig->value.c.elems[i];
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
            int len = getCollectionSize(orig->type->ext.irods.name, orig->value.uninterpreted.inOutStruct, r);
            for(i=0;i<len;i++) {
                    elem = getValueFromCollection(orig->type->ext.irods.name, orig->value.uninterpreted.inOutStruct, i, r);
                    setVariableValue(varName, elem, rei, env, errmsg, r);
                    res = evaluateActions(subtrees[1], subtrees[2], rei,reiSaveFlag,  env,errmsg,r);
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
Res *smsi_break(void **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	Res *	res = newRes(r);
                res->type = newSimpType(T_BREAK, r);
		return res;
}
Res *smsi_succeed(void **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	Res *	res = newRes(r);
                res->type = newSimpType(T_SUCCESS, r);
		return res;
}
Res *smsi_fail(void **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

	Res *	res = newRes(r);
        			res->type = newSimpType(T_ERROR, r);
                                res->value.e = n == 0 ?FAIL_ACTION_ENCOUNTERED_ERR:(int)((Res *)subtrees[0])->value.d;
		return res;
}

Res *smsi_assign(void **subtrees, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

    // An smsi shares the same env as the enclosing rule.
    // Therefore, our modification to the env is reflected to the enclosing rule automatically.
    Res *val = evaluateExpression3(subtrees[1], rei, reiSaveFlag,  env, errmsg,r);
    if(TYPE(val)==T_ERROR) {
        return val;
    }
    char *varName;
    varName = ((Node*)subtrees[0])->text; // the first subtree is an ACTIONS node that has only one TEXT subtree
//		char buf[MAX_COND_LEN * 2];
    return setVariableValue(varName, val, rei, env, errmsg, r);
}

Res *smsi_listvars(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
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
Res *smsi_listcorerules(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *coll = newCollRes(coreRules.len, newSimpType(T_STRING, r), r);
    int i;
    for(i=0;i<coreRules.len;i++) {
        coll->value.c.elems[i] = newStringRes(r, coreRules.rules[i]->subtrees[0]->text);
    }
    return coll;
}
Res *smsi_true(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        return newBoolRes(r, 1);
}
Res *smsi_false(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        return newBoolRes(r, 0);
}

Res *smsi_max(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		int init=0;
		double max=0;
		int i;
		for(i=0;i<n;i++) {
			double x = ((Res *)params[i])->value.d;
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
Res *smsi_min(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		int init=0;
		double min=0;
		int i;
		for(i=0;i<n;i++) {
			double x = ((Res *)params[i])->value.d;
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
        Res *smsi_average(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		double sum=0;
		int i;
		for(i=0;i<n;i++) {
			double x = ((Res *)params[i])->value.d;
			sum += x;
		}
		Res *res = newDoubleRes(r, sum/n);
                return res;
	}
        Res *smsi_hd(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            if(((Res *)params[0])->value.c.len > 0) {
                return ((Res *)params[0])->value.c.elems[0];
            } else {
                addRErrorMsg(errmsg, -1, "error: hd: empty collection");
                return newErrorRes(r, -1);
            }
	}
        Res *smsi_tl(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            if(((Res *)params[0])->value.c.len > 0) {
		Res *res = newRes(r);
                ExprType *elemType = ((Res *)params[0])->type->ext.cons.typeArgs[0];
		// allocate memory for elements
                res->type = newCollType(elemType, r);
                res->value.c.len = ((Res *)params[0])->value.c.len-1;
                res->value.c.elems = (Res **) region_alloc(r, sizeof(Res *)*res->value.c.len);
		int i;
		for(i=0;i<res->value.c.len;i++) {
			res->value.c.elems[i] = ((Res *)params[0])->value.c.elems[i+1];
		}
                return res;
            } else {
                addRErrorMsg(errmsg, -1, "error: tl: empty collection\n");
                return newErrorRes(r, -1);
            }
	}
        Res *smsi_cons(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            ExprType *elemType = ((Res *)params[0])->type;
            // allocate memory for elements
            res->type = newCollType(elemType, r);
            res->value.c.len = ((Res *)params[1])->value.c.len+1;
            res->value.c.elems = (Res **) region_alloc(r, sizeof(Res *)*res->value.c.len);
            int i;
            res->value.c.elems[0] = params[0];
            for(i=1;i<res->value.c.len;i++) {
                    res->value.c.elems[i] = ((Res *)params[1])->value.c.elems[i-1];
            }
                return res;
	}
        Res *smsi_setelem(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            Res *coll = (Res *)params[0];
            Res *indexRes = (Res *)params[1];
            Res *val = (Res *)params[2];
            ExprType *elemType = coll->type;
            int index = (int)indexRes->value.d;
            if(0>index || index >= coll->value.c.len) {
                char buf[ERR_MSG_LEN];
                snprintf(buf, ERR_MSG_LEN, "setelem: index out of bound %d", index);
                addRErrorMsg(errmsg, -1, buf);
                return newErrorRes(r, -1);
            }

            // allocate memory for elements
            res->type = newCollType(elemType, r);
            res->value.c.len = coll->value.c.len;
            res->value.c.elems = (Res **) region_alloc(r, sizeof(Res *)*res->value.c.len);
            memcpy(res->value.c.elems, coll->value.c.elems, sizeof(Res *)*res->value.c.len);
            res->value.c.elems[index] = val;
            return res;
	}
        Res *smsi_list(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
            ExprType *elemType =
                n == 0?newSimpType(T_DYNAMIC, r):((Res *)params[0])->type;
            // allocate memory for elements
            res->type = newCollType(elemType, r);
            res->value.c.len = n;
            res->value.c.elems = (Res **) region_alloc(r, sizeof(Res *)*n);
            int i;
            for(i=0;i<n;i++) {
                    res->value.c.elems[i] = params[i];
            }
            return res;
	}
        Res *smsi_elem(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            char errbuf[ERR_MSG_LEN];
            int index = (int)((Res *)params[1])->value.d;
            if(TYPE((Res *)params[0]) == CONS) {
                if(index <0 || index >= ((Res *)params[0])->value.c.len ) {
                    snprintf(errbuf, ERR_MSG_LEN, "error: index out of range %d.", index);
                    addRErrorMsg(errmsg, -1, errbuf);
                    return newErrorRes(r, -1);
                }
                Res *res = ((Res *)params[0])->value.c.elems[index];
                return res;
            } else {
                if(index <0 || index >= getCollectionSize(((Res *)params[0])->type->ext.irods.name,
                        ((Res *)params[0])->value.uninterpreted.inOutStruct, r) ) {
                    snprintf(errbuf, ERR_MSG_LEN, "error: index out of range %d. %s", index, ((Res *)params[0])->type->ext.irods.name);
                    addRErrorMsg(errmsg, -1, errbuf);
                    return newErrorRes(r, -1);
                }
                Res *res2 = getValueFromCollection(((Res *)params[0])->type->ext.irods.name,
                        ((Res *)params[0])->value.uninterpreted.inOutStruct,
                        index,r);

                return res2;
            }
	}
        Res *smsi_pair(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res *res = newRes(r);
		// allocate memory for elements
            ExprType **typeArgs = (ExprType **) region_alloc(r, sizeof(ExprType *) * 2);
            res->value.c.len = 2;
            res->value.c.elems = (Res **) region_alloc(r, sizeof(Res *)*2);
            int i;
            for(i=0;i<2;i++) {
                    typeArgs[i] = ((Res *)params[i])->type;
                    res->value.c.elems[i] = (Res *)params[i];
            }
            res->type = newConsType(2, cpString("P2", r), typeArgs, r);
            return res;
	}
        Res *smsi_fst(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		int index = 0;
                Res *res = ((Res *)params[0])->value.c.elems[index];
                return res;
	}
        Res *smsi_snd(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		int index = 1;
                Res *res = ((Res *)params[0])->value.c.elems[index];
                return res;
	}
        Res *smsi_size(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
            Res * res = newRes(r);
            res->type = newSimpType(T_INT,r);
                if(TYPE((Res *)params[0]) == CONS) {
                    res->value.d = ((Res *)params[0])->value.c.len;
                } else {
                    res->value.d = getCollectionSize(((Res *)params[0])->type->ext.irods.name,
                            ((Res *)params[0])->value.uninterpreted.inOutStruct,r);
                }
                return res;
	}
        Res *smsi_datetime(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
                char errbuf[ERR_MSG_LEN];
                Res *res = newRes(r);
		Res* timestr = params[0];
		char* format;
		if(TYPE((Res *)params[0])!=STRING ||
			(n == 2 && TYPE((Res *)params[1])!=STRING)) { // error not a string
                        res->type = newSimpType(T_ERROR,r);
			res->value.e = UNSUPPORTED_OP_OR_TYPE;
                        snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not apply datetime to type (%s[,%s]).", typeName_Res(params[0]), n==2?typeName_Res(params[1]):"null");
                        addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
		} else {
			if(n == 2) {
				format = ((Res *)params[1])->value.s.pointer;
			} else {
				format="";
			}
			strttime(timestr->value.s.pointer, format, &(res->value.t));
                        res->type = newSimpType(T_DATETIME,r);
		}
                return res;
        }

Res *smsi_time(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        time_t t;
        time(&t);
        Res*res = newDatetimeRes(r, t);
        return res;
}
Res *smsi_timestr(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        char errbuf[ERR_MSG_LEN];
        Res *res = newRes(r);
        Res* dtime = params[0];
        char* format;
        if(TYPE((Res *)params[0])!=DATETIME ||
            (n == 2 && TYPE((Res *)params[1])!=STRING)) {
            res->type = newSimpType(T_ERROR,r);
            res->value.e = UNSUPPORTED_OP_OR_TYPE;
            snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not apply datetime to type (%s[,%s]).", typeName_Res(params[0]), n==2?typeName_Res(params[1]):"null");
            addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
        } else {
            if(n == 2) {
                    format = ((Res *)params[1])->value.s.pointer;
            } else {
                    format = "";
            }
            char buf[1024];
            ttimestr(buf, 1024-1, format, &dtime->value.t);
            res = newStringRes(r, buf);
        }
        return res;
}
Res *smsi_type(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        Res *val = params[0], *res;
        char typeName[128];
        typeToString(val->type, NULL, typeName, 128);
        res=newStringRes(r, typeName);
        return res;
}
Res *smsi_arity(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
		Res *val = params[0];
                int ruleInx = -1;
		if(findNextRule2(val->value.s.pointer, &ruleInx)<0) {
                    return newErrorRes(r, -1);
                }
                if(ruleInx >= 1000)
                    return newIntRes(r, coreRules.rules[ruleInx-1000]->subtrees[0]->degree);
                else
                    return newIntRes(r, appRules.rules[ruleInx]->subtrees[0]->degree);
}
Res *smsi_str(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
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
Res *smsi_double(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
                char errbuf[ERR_MSG_LEN];
                Res *val = params[0], *res = newRes(r);
		if(TYPE(val) == T_STRING) {
                    res->type = newSimpType(T_DOUBLE,r);
                    res->value.d = atof(val->value.s.pointer);
		} else if(TYPE(val) == T_DATETIME) {
                    res->type = newSimpType(T_DOUBLE,r);
                    res->value.d = (double)val->value.t;
                } else if(TYPE(val) == T_DOUBLE) {
                    res = val;
                } else {
                    res->type = newSimpType(T_ERROR,r);
                    res->value.e = UNSUPPORTED_OP_OR_TYPE;
                    snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not convert %s to double.", typeName_Res(val));
                    addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
		}
                return res;
}
Res *smsi_int(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        char errbuf[ERR_MSG_LEN];
        Res *val = params[0], *res = newRes(r);
        if(TYPE(val) == T_STRING) {
            res->type = newSimpType(T_INT,r);
            res->value.d = atoi(val->value.s.pointer);
        } else if(TYPE(val) == T_DOUBLE) {
            res->type = newSimpType(T_INT, r);
            res->value.d = val->value.d;
        } else if(TYPE(val) == T_INT) {
            res = val;
        } else {
            res->type = newSimpType(T_ERROR,r);
            res->value.e = UNSUPPORTED_OP_OR_TYPE;
            snprintf(errbuf, ERR_MSG_LEN, "error: unsupported operator or type. can not convert %s to double.", typeName_Res(val));
            addRErrorMsg(errmsg, UNSUPPORTED_OP_OR_TYPE, errbuf);
        }
        return res;
}
Res *smsi_lmsg(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
        writeToTmp("re.log", ((Res *)params[0])->value.s.pointer);
        Res *res = newIntRes(r, 0);
        return res;
}
Res *smsi_not(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    return newBoolRes(r, !(int)((Res *)params[0])->value.d?1:0);

}
Res *smsi_negate(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, -(int)args[0]->value.d);
    } else {
        return newDoubleRes(r, -args[0]->value.d);
    }
}
Res *smsi_abs(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)(val < 0?-val:val));
    } else {
        return newDoubleRes(r, (val < 0?-val:val));
    }
}
Res *smsi_exp(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d;
    return newDoubleRes(r, exp(val));
}
Res *smsi_log(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d;
    return newDoubleRes(r, log(val));

}
Res *smsi_floor(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d;
    return newDoubleRes(r, floor(val));

}
Res *smsi_ceiling(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d;
    return newDoubleRes(r, ceil(val));

}

Res *smsi_and(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    return newBoolRes(r, ((int)args[0]->value.d)&&((int)args[1]->value.d)?1:0);

}

Res *smsi_or(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    return newBoolRes(r, ((int)args[0]->value.d)||((int)args[1]->value.d)?1:0);
}

Res *smsi_add(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d+args[1]->value.d;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}
Res *smsi_subtract(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d-args[1]->value.d;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}
Res *smsi_multiply(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = args[0]->value.d*args[1]->value.d;
    if(TYPE(args[0])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}
Res *smsi_divide(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(args[1]->value.d == 0) {
            addRErrorMsg(errmsg,DIVISION_BY_ZERO, "error: division by zero.");
            return newErrorRes(r, DIVISION_BY_ZERO);
    }
    double val = args[0]->value.d/args[1]->value.d;
        return newDoubleRes(r, val);
}

Res *smsi_modulo(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(args[1]->value.d == 0) {
            addRErrorMsg(errmsg, DIVISION_BY_ZERO, "error: division by zero.");
            return newErrorRes(r, DIVISION_BY_ZERO);
    }
    double val = ((int)args[0]->value.d)%((int)args[1]->value.d);
    if(TYPE(args[0])==T_INT&&TYPE(args[1])==T_INT) {
        return newIntRes(r, (int)val);
    } else {
        return newDoubleRes(r, val);
    }
}

Res *smsi_power(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    double val = pow(args[0]->value.d, args[1]->value.d);
    return newDoubleRes(r, val);
}
Res *smsi_root(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    if(args[1]->value.d == 0) {
        addRErrorMsg(errmsg, DIVISION_BY_ZERO, "error: division by zero.");
        return newErrorRes(r, DIVISION_BY_ZERO);
    }
    double val = pow(args[0]->value.d, 1/args[1]->value.d);
    return newDoubleRes(r, val);
}

Res *smsi_concat(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
/*    if(args[0]->value.s.len+args[1]->value.s.len>=1024) {
        //error
        addRErrorMsg(errmsg, STRING_OVERFLOW, "error: string too long.");
        return newErrorRes(r, STRING_OVERFLOW);
    } else {*/
    char *newbuf = (char *)malloc((args[0]->value.s.len + args[1]->value.s.len+1)*sizeof(char));

    strcpy(newbuf, args[0]->value.s.pointer);
    strcpy(newbuf+args[0]->value.s.len, args[1]->value.s.pointer);

    Res *res = newStringRes(r, newbuf);
    free(newbuf);
    return res;
    /*}*/
}

Res *smsi_lt(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.d < args[1]->value.d?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.t, args[1]->value.t)<0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->value.s.pointer, args[1]->value.s.pointer) <0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);

}
Res *smsi_le(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.d <= args[1]->value.d?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.t, args[1]->value.t)<=0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->value.s.pointer, args[1]->value.s.pointer) <=0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);

}
Res *smsi_gt(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.d > args[1]->value.d?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.t, args[1]->value.t)>0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->value.s.pointer, args[1]->value.s.pointer) >0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);

}
Res *smsi_ge(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.d >= args[1]->value.d?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.t, args[1]->value.t)>=0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->value.s.pointer, args[1]->value.s.pointer)>=0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);
}
Res *smsi_eq(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.d == args[1]->value.d?1:0);
            break;
        case T_BOOL:
            if(TYPE(args[1]) == T_BOOL)
                return newBoolRes(r, args[0]->value.d == args[1]->value.d?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.t, args[1]->value.t)==0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->value.s.pointer, args[1]->value.s.pointer) ==0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);
}
Res *smsi_neq(void **params, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **args = (Res **)params;
    switch(TYPE(args[0])) {
        case T_INT:
        case T_DOUBLE:
            if(TYPE(args[1]) == T_INT ||
                    TYPE(args[1]) == T_DOUBLE)
                return newBoolRes(r, args[0]->value.d != args[1]->value.d?1:0);
            break;
        case T_BOOL:
            if(TYPE(args[1]) == T_BOOL)
                return newBoolRes(r, args[0]->value.d == args[1]->value.d?1:0);
            break;
        case T_DATETIME:
            if(TYPE(args[1]) == T_DATETIME)
                return newBoolRes(r, difftime(args[0]->value.t, args[1]->value.t)!=0?1:0);
            break;
        case T_STRING:
            if(TYPE(args[1]) == T_STRING)
                return newBoolRes(r, strcmp(args[0]->value.s.pointer, args[1]->value.s.pointer) !=0?1:0);
            break;
        default:
            break;
    }
    char errbuf[ERR_MSG_LEN];
    snprintf(errbuf, ERR_MSG_LEN, "type error: comparing between %s and %s", typeName_Res(args[0]), typeName_Res((args[1])));
    addRErrorMsg(errmsg, -1, errbuf);
    return newErrorRes(r, -1);
}
Res *smsi_like_regex(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **params = (Res **)paramsr;
    Res *res = newRes(r);
        char *pattern;
        char *bufstr;
        pattern = params[1]->value.s.pointer;
        bufstr = strdup(params[0]->value.s.pointer);
        #ifdef _POSIX_VERSION
        // make the regexp match whole strings
        char *buf2;
        buf2 = matchWholeString(pattern);
        regex_t regbuf;
        regcomp(&regbuf,buf2,REG_EXTENDED);
        res->type = newSimpType(T_BOOL,r);
        res->value.d = regexec(&regbuf,	bufstr, 0,0,0)==0?1:0;
        regfree(&regbuf);
        #else
        res->value.d = match(pattern, expr1->value.s)==TRUE?1:0;
        #endif
        /*res->type = 2;
        strcpy(res->value.s,expr1->value.s);
        strcat(res->value.s,",");
        strcat(res->value.s,bufstr);
        strcat(res->value.s,",");
        strcat(res->value.s,expr2->value.s);
        strcat(res->value.s,",");
        strcat(res->value.s,buf);*/
        //strcat(res->value.s,",");
        //strcat(res->value.s, ch);
        free(buf2);
        free(bufstr);
        return res;
}
Res *smsi_notlike_regex(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **params = (Res **)paramsr;
    Res *res = newRes(r);

        char *pattern;
        char *bufstr;
        pattern = params[1]->value.s.pointer;
        bufstr = strdup(params[0]->value.s.pointer);
        #ifdef _POSIX_VERSION
        // make the regexp match whole strings
        char *buf2;
        buf2 = matchWholeString(pattern);

        regex_t regbuf;
        regcomp(&regbuf,buf2,REG_EXTENDED);
        res->type = newSimpType(T_BOOL,r);
        res->value.d = regexec(&regbuf, bufstr, 0,0,0)==0?0:1;
        regfree(&regbuf);
        #else
        res->value.d = match(buf, expr1->value.s)==FALSE?1:0;
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

Res *smsi_eval(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res **params = (Res **)paramsr;
    //printf("\neval: %s\n", params[0]->value.s.pointer);
    return eval(params[0]->value.s.pointer, env, rei, reiSaveFlag, errmsg, r);
}

/**
 * Run node and return the errorcode.
 * If the execution is successful, the returned errorcode is 0.
 */
Res *smsi_errorcode(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *res;
    switch(((Node *)paramsr[0])->type) {
            case ACTIONS:
                res = evaluateActions(paramsr[0], paramsr[1], rei, reiSaveFlag,  env, errmsg, r);
            default:
                res = evaluateExpression3(paramsr[0],rei,reiSaveFlag,env,errmsg,r);
        }
        switch(TYPE(res)) {
            case T_ERROR:
                return newIntRes(r, res->value.e);
            default:
                return newIntRes(r, 0);
        }
}

/**
 * Run node and return the errorcode.
 * If the execution is successful, the returned errorcode is 0.
 */
Res *smsi_errormsg(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    char *errbuf = (char *)malloc(ERR_MSG_LEN*1024*sizeof(char));
    Res *res;
    switch(((Node *)paramsr[0])->type) {
            case ACTIONS:
                res = evaluateActions(paramsr[0], paramsr[1], rei, reiSaveFlag,  env, errmsg, r);
            default:
                res = evaluateExpression3(paramsr[0],rei,reiSaveFlag,env,errmsg,r);
    }
    paramsr[1] = newStringRes(r, errMsgToString(errmsg, errbuf, ERR_MSG_LEN*1024));
    freeRErrorContent(errmsg);
    free(errbuf);
    switch(TYPE(res)) {
        case T_ERROR:
            return newIntRes(r, res->value.e);
        default:
            return newIntRes(r, 0);
    }
}

Res *smsi_delayExec(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r)
{
  int i;
  char actionCall[MAX_ACTION_SIZE];
  char recoveryActionCall[MAX_ACTION_SIZE];
  char delayCondition[MAX_ACTION_SIZE];

  Res **params = (Res **)paramsr;

  rstrcpy(delayCondition, params[0]->value.s.pointer, MAX_ACTION_SIZE);
  rstrcpy(actionCall, params[1]->value.s.pointer, MAX_ACTION_SIZE);
  rstrcpy(recoveryActionCall, params[2]->value.s.pointer, MAX_ACTION_SIZE);

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

Res *smsi_remoteExec(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r)
{
  int i;
  execMyRuleInp_t execMyRuleInp;
  msParamArray_t *outParamArray = NULL;
  char tmpStr[LONG_NAME_LEN];

  Res **params = (Res **)paramsr;

  memset (&execMyRuleInp, 0, sizeof (execMyRuleInp));
  execMyRuleInp.condInput.len=0;
  rstrcpy (execMyRuleInp.outParamDesc, ALL_MS_PARAM_KW, LONG_NAME_LEN);

  rstrcpy (tmpStr, params[0]->value.s.pointer, LONG_NAME_LEN);
  parseHostAddrStr (tmpStr, &execMyRuleInp.addr);

  snprintf(execMyRuleInp.myRule, META_STR_LEN, "remExec||%s|%s", params[2]->value.s.pointer, params[3]->value.s.pointer);
  addKeyVal(&execMyRuleInp.condInput,"execCondition",params[1]->value.s.pointer);

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
  if ((execOutRes = (Res *)lookupFromHashTable(env->global, "ruleExecOut")) != NULL) {
    myExecCmdOut = (execCmdOut_t *)execOutRes->value.uninterpreted.inOutStruct;
  } else {
    myExecCmdOut = malloc (sizeof (execCmdOut_t));
    memset (myExecCmdOut, 0, sizeof (execCmdOut_t));
    execOutRes = newRes(r);
    execOutRes->type  = newIRODSType(ExecCmdOut_MS_T, r);
    execOutRes->value.uninterpreted.inOutStruct = myExecCmdOut;
    insertIntoHashTable(env->global, "ruleExecOut", execOutRes);
  }

  if (!strcmp(writeId,"stdout"))
    appendToByteBufNew(&(myExecCmdOut->stdoutBuf),(char *) writeStr);
  else if (!strcmp(writeId,"stderr"))
    appendToByteBufNew(&(myExecCmdOut->stderrBuf),(char *) writeStr);
  return 0;
}

Res *smsi_writeLine(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
#ifdef DEBUG
    printf("%s\n", ((Res *)paramsr[1])->value.s.pointer);
    return newIntRes(r, 0);
#else
  Res *where = (Res *)paramsr[0];
  char *whereId = where->value.s.pointer;
  Res *inString = (Res *)paramsr[1];

  if (strcmp (whereId, "serverLog") == 0) {
      rodsLog (LOG_NOTICE, "writeLine: inString = %s\n", inString->value.s.pointer);
      return newIntRes(r, 0);
  }

  int i = writeStringNew(whereId, inString->value.s.pointer, env, r);

  if (i < 0)
    return newErrorRes(r, i);

  i = writeStringNew(whereId, "\n", env, r);

  if (i < 0)
    return newErrorRes(r, i);
  else
    return newIntRes(r, i);
#endif
}
Res *smsi_writeString(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {

  Res *where = (Res *)paramsr[0];
  char *whereId = where->value.s.pointer;
  Res *inString = (Res *)paramsr[1];

  int i = writeStringNew(whereId, inString->value.s.pointer, env, r);

  if (i < 0)
    return newErrorRes(r, i);
  else
    return newIntRes(r, i);
}

Res *smsi_triml(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    // if the length of delim is 0, strstr should return str
    Res *strres = (Res *)paramsr[0];
    Res *delimres = (Res *)paramsr[1];

    char *str = strres->value.s.pointer;
    char *delim = delimres->value.s.pointer;

    char *p = strstr(str, delim);
    if(p!=NULL) {
        // found
        return newStringRes(r, p+strlen(delim));
    } else {
        // not found return the original string
        return strres;
    }



}
Res *smsi_trimr(void **paramsr, int n, Node *node, ruleExecInfo_t *rei, int reiSaveFlag, Env *env, rError_t *errmsg, Region *r) {
    Res *strres = (Res *)paramsr[0];
    Res *delimres = (Res *)paramsr[1];

    char *str = strres->value.s.pointer;
    char *delim = delimres->value.s.pointer;

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
        // not found
        return strres;
    } else {
        // found set where newp points to to \0
        char temp = *newp;
        *newp = '\0';

        Res *res = newStringRes(r, str);
        // restore
        *newp = temp;
        return res;
    }

}

// utilities
FunctionDesc *getFuncDescFromChain(int n, FunctionDesc *fDesc) {
            ExprType *fTypeCopy = fDesc->type;

            while((fTypeCopy->ext.func.vararg == ONCE && n != fTypeCopy->ext.func.arity)
                    ||(fTypeCopy->ext.func.vararg == STAR && n < fTypeCopy->ext.func.arity - 1)
                    ||(fTypeCopy->ext.func.vararg == PLUS && n < fTypeCopy->ext.func.arity)) {
                if(fDesc->next == NULL) {
                    return NULL;
                }
                fDesc = fDesc->next;
                fTypeCopy = fDesc->type;
            }
            return fDesc;
}

Res* eval(char *expr, Env *env, ruleExecInfo_t *rei, int saveREI, rError_t *errmsg, Region *r) {
    Node *node;
    Pointer *e = newPointer2(expr);
    nextTermRuleGen(e, &node, MIN_PREC, 0,errmsg, r);
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
    } else { // error
        return -1;
    }
}

ExprType *getTypeFromChar(char **ch, Hashtable *vtable, Region *r) {
    ExprType *restype = NULL;
    char *chEnd;
        switch(tolower(**ch)) {
            case '?':
                (*ch)++;
                restype = newSimpType(T_DYNAMIC, r);
                break;
            case 'i':
                (*ch)++;
                restype = newSimpType(T_INT, r);
                break;
            case 'd':
                (*ch)++;
                restype = newSimpType(T_DOUBLE, r);
                break;
            case 'b':
                (*ch)++;
                restype = newSimpType(T_BOOL, r);
                break;
            case 't':
                (*ch)++;
                restype = newSimpType(T_DATETIME, r);
                break;
            case 's':
                (*ch)++;
                restype = newSimpType(T_STRING, r);
                break;
            case 'c':
                (*ch)++;
                restype = newCollType(getTypeFromChar(ch, vtable, r),r);
                break;
            case 'p': // product type
                (*ch)++;
                int arity = **ch - '0';
                char name[3];
                snprintf(name, 3, "P%d", arity);
                ExprType **typeArgs = (ExprType **) region_alloc(r, sizeof(ExprType *) * arity);
                int i;
                (*ch)++;
                for(i=0;i<arity;i++) {
                    typeArgs[i] = getTypeFromChar(ch, vtable, r);
                }
                restype = newConsType(arity, cpString(name, r), typeArgs, r);
                break;
            case 'f': // flexible type, non dynamic coercion allowed
                (*ch)++;
                restype = getTypeFromChar(ch, vtable, r);
                restype->coercionAllowed = 1; // = 0;
                break;
            case '`': // irods type
                chEnd = *ch;
                while(*chEnd!='`') {
                    if(*chEnd == '\0') {
                        // todo handle error
                    }
                    chEnd++;
                }
                // temporarily change chEnd to \0
                *chEnd = '\0';
                restype = newIRODSType((*ch)+1, r);
                // restore chEnd
                *chEnd = '`';
                *ch = chEnd+1;
                break;
            default:
                if(isdigit(**ch)) {
                    char buf[128];
                    char *vname = getTVarName(**ch-'0', buf);
                    (*ch)++;

                    ExprType *tvar;
                    if((tvar = lookupFromHashTable(vtable, vname))==NULL) {
                        insertIntoHashTable(vtable, vname, tvar = newTVar(r));
                        if(**ch == '{') {
                            // union
                            (*ch)++;
                            while(**ch != '}') {
                                T_VAR_DISJUNCT(tvar, T_VAR_NUM_DISJUNCTS(tvar)++) = getTypeFromChar(ch, vtable, r)->t;
                            }
                            (*ch)++; // skip }
                        }
                    }
                    restype = tvar;
                } else {
                    (*ch)++;
                    //error
                    return NULL;
                }
                break;
        }
        return restype;

}
/*
 * parse the string for an ExprType
 * supported arg and ret types:
 * ? dynamic
 * i int
 * b boolean
 * d double
 * t datetime
 * s string
 * c collection
 * p(0|...|9)type...type product
 * f flexible
 * (0|...|9){type...type} union
 * `irods PI` irods
 * 0|...|9 tvar
 *
 * type...type(*|+)?>type
 */
ExprType *parseFuncTypeFromString(char *string, Region *r) {
    int vararg;
    int arity = 0;
    if(strchr(string,'*')) {
        vararg = STAR;
    } else if(strchr(string, '+')) {
        vararg = PLUS;
    } else {
        vararg = ONCE;
    }

    ExprType **paramTypes = (ExprType **) region_alloc(r, sizeof(ExprType *)*MAX_FUNC_PARAMS); // assume that there are 10 params maximum
    int i;
    Hashtable *vt = newHashTable(MAX_FUNC_PARAMS);
    i=0;
    char *p = string;
    while(*p!='>' && *p!='+' && *p!='*') {
        paramTypes[i]=getTypeFromChar(&p,vt, r);
        i++;
    }
    arity = i;
    if(*p=='*') {
        p++; // skip *
    }
    if(*p=='+') {
        p++; // skip +
    }
    if(*p=='>') {
        p++; // skip >
    }
    //p = string + (strlen(string)-1);
    ExprType *retType = getTypeFromChar(&p,vt, r);
    ExprType *exprType = newFuncTypeVarArg(arity, vararg, paramTypes, retType, r);
    deleteHashTable(vt, nop);
    return exprType;
}
char *errMsgToString(rError_t *errmsg, char *errbuf, int buflen /* = 0 */) {
    errbuf[0] = '\0';
    int p = 0;
    int i;
    for(i=errmsg->len-1;i>=0;i--) {
        if(i!=errmsg->len-1) {
            snprintf(errbuf+p, buflen-p, "caused by: %s\n", errmsg->errMsg[i]->msg);
        } else {
            snprintf(errbuf+p, buflen-p, "%s\n", errmsg->errMsg[i]->msg);
        }
        p = strnlen(errbuf, buflen);
    }
    return errbuf;

}


void getSystemFunctions(Hashtable *ft, Region *r) {
    insertIntoHashTable(ft, "do", newFunctionDesc("e", "0>?", smsi_do, r));
    insertIntoHashTable(ft, "eval", newFunctionDesc("i", "s>?", smsi_eval, r));
    insertIntoHashTable(ft, "errorcode", newFunctionDescChain("aa", "ii>i", smsi_errorcode, newFunctionDesc("e", "0>i", smsi_errorcode, r),r));
    insertIntoHashTable(ft, "errormsg", newFunctionDescChain("aao", "iis>i", smsi_errormsg, newFunctionDesc("eo", "0s>i", smsi_errormsg, r),r));
    insertIntoHashTable(ft, "if", newFunctionDesc("eeeee", "b00ii>0", smsi_ifExec, r));
    insertIntoHashTable(ft, "ifExec", newFunctionDesc("eeeee", "b00ii>0", smsi_ifExec, r));
    insertIntoHashTable(ft, "for", newFunctionDesc("eeeaa", "0b1ii>i",smsi_forExec, r));
    insertIntoHashTable(ft, "forExec", newFunctionDesc("eeeaa", "0b1ii>i", smsi_forExec, r));
    insertIntoHashTable(ft, "while", newFunctionDesc("eaa", "bii>i",smsi_whileExec, r));
    insertIntoHashTable(ft, "whileExec", newFunctionDesc("eaa", "bii>i", smsi_whileExec, r));
    insertIntoHashTable(ft, "foreach", newFunctionDesc("eaa", "c0ii>i", smsi_forEachExec, r));
    insertIntoHashTable(ft, "forEachExec", newFunctionDesc("eaa", "c0ii>i", smsi_forEachExec, r));
    insertIntoHashTable(ft, "break", newFunctionDesc("", ">i", smsi_break, r));
    insertIntoHashTable(ft, "succeed", newFunctionDesc("", ">i", smsi_succeed, r));
    insertIntoHashTable(ft, "fail", newFunctionDescChain("i", "i>i", smsi_fail, newFunctionDesc("", ">i", smsi_fail, r), r));
    insertIntoHashTable(ft, "assign", newFunctionDesc("ee", "0f0>i", smsi_assign, r));
    insertIntoHashTable(ft, "lmsg", newFunctionDesc("i", "s>i", smsi_lmsg, r));
    insertIntoHashTable(ft, "listvars", newFunctionDesc("", ">s", smsi_listvars, r));
    insertIntoHashTable(ft, "listcorerules", newFunctionDesc("", ">cs", smsi_listcorerules, r));
    insertIntoHashTable(ft, "true", newFunctionDesc("", ">b", smsi_true, r));
    insertIntoHashTable(ft, "false", newFunctionDesc("", ">b", smsi_false, r));
    insertIntoHashTable(ft, "time", newFunctionDesc("", ">t", smsi_time, r));
    insertIntoHashTable(ft, "timestr", newFunctionDesc("i", "t>s", smsi_timestr, r));
    insertIntoHashTable(ft, "str", newFunctionDesc("i", "0>s", smsi_str, r));
    insertIntoHashTable(ft, "datetime", newFunctionDesc("i", "s>t", smsi_datetime, r));
    insertIntoHashTable(ft, "timestrf", newFunctionDesc("ii", "ts>s", smsi_timestr, r));
    insertIntoHashTable(ft, "datetimef", newFunctionDesc("ii", "ss>t", smsi_datetime, r));
    insertIntoHashTable(ft, "double", newFunctionDesc("i", "0{sdt}>d", smsi_double, r));
    insertIntoHashTable(ft, "int", newFunctionDesc("i", "0{isd}>i", smsi_int, r));
    insertIntoHashTable(ft, "list", newFunctionDesc("i*", "0*>c0", smsi_list, r));
    insertIntoHashTable(ft, "elem", newFunctionDesc("ii", "c0i>0", smsi_elem, r));
    insertIntoHashTable(ft, "setelem", newFunctionDesc("iii", "c0i0>c0", smsi_setelem, r));
    insertIntoHashTable(ft, "hd", newFunctionDesc("i", "c0>0", smsi_hd, r));
    insertIntoHashTable(ft, "tl", newFunctionDesc("i", "c0>c0", smsi_tl, r));
    insertIntoHashTable(ft, "cons", newFunctionDesc("ii", "0c0>c0", smsi_cons, r));
    insertIntoHashTable(ft, "size", newFunctionDesc("i", "c0>i", smsi_size, r));
    insertIntoHashTable(ft, "type", newFunctionDesc("i", "0>s",smsi_type, r));
    insertIntoHashTable(ft, "arity", newFunctionDesc("i", "s>i",smsi_arity, r));
    insertIntoHashTable(ft, "+", newFunctionDesc("ii", "f0{id}f0>0",smsi_add, r));
    insertIntoHashTable(ft, "++", newFunctionDesc("ii", "fsfs>s",smsi_concat, r));
    insertIntoHashTable(ft, "-", newFunctionDescChain("ii", "f0{id}f0>0",smsi_subtract, newFunctionDesc("i", "f0{id}>f0{id}", smsi_negate, r), r));
    insertIntoHashTable(ft, "*", newFunctionDesc("ii", "f0{id}f0>0",smsi_multiply, r));
    insertIntoHashTable(ft, "/", newFunctionDesc("ii", "f0{id}f0>0",smsi_divide, r));
    insertIntoHashTable(ft, "%", newFunctionDesc("ii", "f0{id}f0>0",smsi_modulo, r));
    insertIntoHashTable(ft, "^", newFunctionDesc("ii", "fdfd>d",smsi_power, r));
    insertIntoHashTable(ft, "@", newFunctionDesc("ii", "fdfd>d",smsi_root, r));
    insertIntoHashTable(ft, "log", newFunctionDesc("i", "fd>d",smsi_log, r));
    insertIntoHashTable(ft, "exp", newFunctionDesc("i", "fd>d",smsi_exp, r));
    insertIntoHashTable(ft, "!", newFunctionDesc("i", "b>b",smsi_not, r));
    insertIntoHashTable(ft, "&&", newFunctionDesc("ii", "bb>b",smsi_and, r));
    insertIntoHashTable(ft, "%%", newFunctionDesc("ii", "bb>b",smsi_or, r));
    insertIntoHashTable(ft, "==", newFunctionDesc("ii", "f0{dbst}f0>b",smsi_eq, r));
    insertIntoHashTable(ft, "!=", newFunctionDesc("ii", "f0{dbst}f0>b",smsi_neq, r));
    insertIntoHashTable(ft, ">", newFunctionDesc("ii", "f0{dst}f0>b", smsi_gt, r));
    insertIntoHashTable(ft, "<", newFunctionDesc("ii", "f0{dst}f0>b", smsi_lt, r));
    insertIntoHashTable(ft, ">=", newFunctionDesc("ii", "f0{dst}f0>b", smsi_ge, r));
    insertIntoHashTable(ft, "<=", newFunctionDesc("ii", "f0{dst}f0>b", smsi_le, r));
    insertIntoHashTable(ft, "floor", newFunctionDesc("i", "fd>i", smsi_floor, r));
    insertIntoHashTable(ft, "ceiling", newFunctionDesc("i", "fd>i", smsi_ceiling, r));
    insertIntoHashTable(ft, "abs", newFunctionDesc("i", "fd>d", smsi_abs, r));
    insertIntoHashTable(ft, "max", newFunctionDesc("i*", "fd+>d", smsi_max, r));
    insertIntoHashTable(ft, "min", newFunctionDesc("i*", "fd+>d", smsi_min, r));
    insertIntoHashTable(ft, "average", newFunctionDesc("i*", "fd+>d", smsi_average, r));
    insertIntoHashTable(ft, "like", newFunctionDesc("ii", "ss>b", smsi_like_regex, r));
    insertIntoHashTable(ft, "not like", newFunctionDesc("ii", "ss>b", smsi_notlike_regex, r));
    insertIntoHashTable(ft, "delayExec", newFunctionDesc("iii", "sss>i", smsi_delayExec, r));
    insertIntoHashTable(ft, "remoteExec", newFunctionDesc("iiii", "ssss>i", smsi_remoteExec,r));
    insertIntoHashTable(ft, "writeLine", newFunctionDesc("ii", "ss>i", smsi_writeLine,r));
    insertIntoHashTable(ft, "writeString", newFunctionDesc("ii", "ss>i", smsi_writeString,r));
    insertIntoHashTable(ft, "triml", newFunctionDesc("ii","ss>s", smsi_triml, r));
    insertIntoHashTable(ft, "trimr", newFunctionDesc("ii","ss>s", smsi_trimr, r));
    insertIntoHashTable(ft, "pair", newFunctionDesc("ii","01>p201", smsi_pair, r));
    insertIntoHashTable(ft, "fst", newFunctionDesc("i","p201>0", smsi_fst, r));
    insertIntoHashTable(ft, "snd", newFunctionDesc("i","p201>1", smsi_snd, r));


}
