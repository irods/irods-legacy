/* For copyright information please refer to files in the COPYRIGHT directory
 */

#include "typing.h"
#include "functions.h"
#define ERROR(x) if(x) {goto error;}
#define ERROR2(x,y) if(x) {localErrorMsg=(y);goto error;}
/**
 * return 0 to len-1 index of the parameter with type error
 *        -1 success
 */
int typeParameters(ExprType** paramTypes, int len, Node** subtrees, Hashtable* funcDesc, Hashtable *symbol_type_table, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
	int i;
	for(i=0;i<len;i++) {
		paramTypes[i] = dereference(typeExpression3(subtrees[i], funcDesc, symbol_type_table, typingConstraints, errmsg, errnode, r), symbol_type_table, r);
		if(paramTypes[i]->nodeType == T_ERROR) {
			return i;
		}
	}
	return -1;
}

int tautologyLt(ExprType *type, ExprType *expected) {
    if(typeEqSyntatic(type, expected)) {
        return 1;
    }
        int i;
        ExprType a, b;
        if (type->nodeType == T_VAR) {
            if(T_VAR_NUM_DISJUNCTS(type) > 0) {
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    a.nodeType = T_VAR_DISJUNCT(type,i)->nodeType;
                    if(!tautologyLt(&a, expected)) {
                        return 0;
                    }
                }
                return 1;
            } else {
                return 0;
            }

        } else if (expected->nodeType == T_VAR) {
            if(T_VAR_NUM_DISJUNCTS(expected) > 0) {
                for(i=0;i<T_VAR_NUM_DISJUNCTS(expected);i++) {
                    b.nodeType = T_VAR_DISJUNCT(expected,i)->nodeType;
                    if(!tautologyLt(type, &b)) {
                        return 0;
                    }
                }
                return 1;
            } else {
                return 0;
            }
        } else if (type->nodeType == T_CONS && expected->nodeType == T_CONS) {
            if(strcmp(T_CONS_TYPE_NAME(type), T_CONS_TYPE_NAME(expected))!=0) {
                return 0;
            }
            int i;
            for(i=0;i<T_CONS_ARITY(type);i++) {
                if(tautologyLt(T_CONS_TYPE_ARG(type, 0), T_CONS_TYPE_ARG(expected, 0))==0) {
                    return 0;
                }
            }
            return 1;
        } else {
            return tautologyLtBase(type, expected);
        }
}

int tautologyLtBase(ExprType *a, ExprType *b) {
/*
        if(a==T_DYNAMIC || b==T_DYNAMIC) {
            return 1;
        }
*/
        switch(a->nodeType) {
            case T_INT:
                return b->nodeType == T_INT ||
                        b->nodeType == T_DOUBLE;
            case T_DOUBLE:
            case T_BOOL:
            case T_STRING:
                return a->nodeType==b->nodeType;
            default:
                return 0;
        }
}

/*
 * simplify expected which is a variable, bounded or unbounded, based on type which is not a variable
 * returns 1 tautology
 *         0 contingency
 *         -1 absurdity
 */
Satisfiability simplifyR(ExprType *type, ExprType *expected, ExprType **bn, Region *r) {
    ExprType b;
    if(T_VAR_NUM_DISJUNCTS(expected) > 0) {
        Node *c[100];
        Node **cp = c;
        int i;
        for(i=0;i<T_VAR_NUM_DISJUNCTS(expected);i++) {
            b.nodeType = T_VAR_DISJUNCT(expected, i)->nodeType;
            if(tautologyLt(type, &b)) {
                *(cp++)=T_VAR_DISJUNCT(expected, i);
            }
        }
        if(cp == c) {
            return ABSURDITY;
        } else {
            if(cp-c==1) {
                *bn = *c;
            } else if (cp-c == T_VAR_NUM_DISJUNCTS(expected)) {
                *bn = expected;
            } else {
                *bn = newTVar2(cp-c, c, r);
            }
            return TAUTOLOGY;
        }
    } else {
        *bn = expected;
        return CONTINGENCY;
    }
}

Satisfiability simplifyL(ExprType *type, ExprType *expected, ExprType **an, Region *r) {
    ExprType a;
    if(T_VAR_NUM_DISJUNCTS(type) > 0) {
        Node* c[100];
        Node** cp = c;
        int i;
        for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
            a.nodeType = T_VAR_DISJUNCT(type, i)->nodeType;
            if(tautologyLt(&a, expected)) {
                *(cp++)=T_VAR_DISJUNCT(type, i);
            }
        }
        if(cp == c) {
            return ABSURDITY;
        } else {
            if(cp-c==1) {
                *an = *c;
            } else if (cp-c == T_VAR_NUM_DISJUNCTS(type)) {
                *an = type;
            } else {
                *an = newTVar2(cp-c, c, r);
            }
            return TAUTOLOGY;
        }
    } else {
        *an = type;
        return CONTINGENCY;
    }
}
Satisfiability narrow(ExprType *type, ExprType *expected, ExprType **an, ExprType **bn, Region *r) {
    ExprType a, b;
        if(T_VAR_ID(type) == T_VAR_ID(expected)) {
            *an = type;
            *bn = expected;
            return CONTINGENCY;
        } else if(T_VAR_NUM_DISJUNCTS(type) > 0 && T_VAR_NUM_DISJUNCTS(expected) > 0) {
            Node* c[100];
            Node** cp = c;
            int i,k;
            for(k=0;k<T_VAR_NUM_DISJUNCTS(expected);k++) {
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    a.nodeType = T_VAR_DISJUNCT(type,i)->nodeType;
                    b.nodeType = T_VAR_DISJUNCT(expected,k)->nodeType;
                    if(tautologyLt(&a, &b)) {
                        *(cp++)=T_VAR_DISJUNCT(expected,k);
                        break;
                    }
                }
            }
            ExprType *gcd;
            if(cp == c) {
                return ABSURDITY;
            } else {
                if(cp-c==1) {
                    gcd = *c;
                } else if (cp-c == T_VAR_NUM_DISJUNCTS(expected)) {
                    gcd = expected;
                } else {
                    gcd = newTVar2(cp-c, c, r);
                }
                *bn = gcd;
            }
            cp = c;
            for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                for(k=0;k<T_VAR_NUM_DISJUNCTS(expected);k++) {
                    a.nodeType = T_VAR_DISJUNCT(type,i)->nodeType;
                    b.nodeType = T_VAR_DISJUNCT(expected,k)->nodeType;
                    if(tautologyLt(&a, &b)) {
                        *(cp++)=T_VAR_DISJUNCT(type,i);
                        break;
                    }
                }
            }
            if(cp-c==1) {
                gcd = *c;
            } else if(cp-c == T_VAR_NUM_DISJUNCTS(type)) {
                gcd = type;
            } else {
                gcd = newTVar2(cp-c, c, r);
            }
            *an = gcd;
            return CONTINGENCY;
        } else if(T_VAR_NUM_DISJUNCTS(type)==0) { /* free */
            *an = type;
            *bn = expected;
            return CONTINGENCY;
        } else /*if(T_VAR_NUM_DISJUNCTS(expected)==0)*/ { /* free */
            *an = type;
            *bn = expected;
            return CONTINGENCY;
        }
}

Satisfiability simplifyLocally(TypingConstraint *tc, Hashtable *typingEnv, Region *r) {
/*
    char buf[1024], buf2[1024], buf3[1024];
    snprintf(buf, 1024, "simplifyLocally: %s<%s\n",
                typeToString(((TypingConstraint *)tc)->a, NULL, buf2, 1024),
                typeToString(((TypingConstraint *)tc)->b, NULL, buf3, 1024));
    printf("%s", buf);
*/
    tc->a = instantiate(tc->a, typingEnv, 0, r);
    tc->b = instantiate(tc->b, typingEnv, 0, r);
    if(tautologyLt(tc->a, tc->b)) {
        return TAUTOLOGY;
    }
    ExprType *an, *bn;
    char tvarname[128];
    if(tc->a->nodeType == T_VAR && tc->b->nodeType == T_VAR) {
        Satisfiability sat = narrow(tc->a, tc->b, &an, &bn, r);
        if(tc->a!= an) {
            insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->a), tvarname), an);
            tc->a = an;
        }
        if(tc->b!= bn) {
            insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->b), tvarname), bn);
            tc->b = bn;
        }
        if((sat & ABSURDITY) == 0) {
            return CONTINGENCY;
        } else {
            return ABSURDITY;
        }
    } else if(tc->a->nodeType==T_VAR) {
        if(simplifyL(tc->a, tc->b, &an, r) != ABSURDITY) {
            if(tc->a!= an) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->a), tvarname), an);
                tc->a = an;
            }
            return CONTINGENCY;
        } else {
            return ABSURDITY;
        }

    } else if(tc->b->nodeType==T_VAR) {
        if(simplifyR(tc->a, tc->b, &bn, r) != ABSURDITY) {
            if(tc->b!= bn) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->b), tvarname), bn);
                tc->b = bn;
            }
            return CONTINGENCY;
        } else {
            return ABSURDITY;
        }

    } else if(tc->a->nodeType == T_CONS && tc->b->nodeType == T_CONS) {
        if(strcmp(T_CONS_TYPE_NAME(tc->a), T_CONS_TYPE_NAME(tc->b))!=0) {
            return ABSURDITY;
        } else {
            TypingConstraint *prev = NULL, *curr = NULL;
            int noconstr = 1;
            int i;
            ExprType *consa = tc->a;
            ExprType *consb = tc->b;
            for(i=0;i<T_CONS_ARITY(consa);i++) {
                ExprType *simpa = T_CONS_TYPE_ARG(consa, i);
                ExprType *simpb = T_CONS_TYPE_ARG(consb, i);
                if(noconstr) {
                    curr = tc;
                    curr->a = simpa;
                    curr->b = simpb;
                } else {
                    curr = newTypingConstraint(simpa, simpb, LT, tc->node, r);
                }
                Satisfiability sat = simplifyLocally(curr, typingEnv, r);
                switch(sat) {
                    case ABSURDITY:
                        return ABSURDITY;
                    case TAUTOLOGY:
                        break;
                    case CONTINGENCY:
                        if(noconstr) {
                            noconstr = 0;
                        } else {
                            prev->next = curr;
                        }
                        prev = curr;
                        while(prev->next != NULL) {
                            prev = prev->next;
                        }
                        break;
                }
            }
            return noconstr? TAUTOLOGY : CONTINGENCY;
        }
    } else {
        return ABSURDITY;
    }
}
/*
 * not 0 solved
 * 0 not solved
 */
int solveConstraints(List *typingConstraints, Hashtable *typingEnv, rError_t *errmsg, Node ** errnode, Region *r) {
/*
    char buf0[1024];
    typingConstraintsToString(typingConstraints, typingEnv, buf0, 1024);
    printf("solving constraints: %s\n", buf0);
*/
    char buf[128];
    while(typingConstraints->head != NULL) {
        Satisfiability sat = simplify(typingConstraints, typingEnv, errmsg, errnode, r);
        if(sat == ABSURDITY) {
            return 0;
        }
        ListNode *ln = typingConstraints->head;
        if(ln!=NULL) {
            TypingConstraint *tc = (TypingConstraint *)ln->value;
/*
            char buf2[1024], buf3[1024];
*/
            /* printf("dereferencing %s and %s.\n", typeToString(tc->a, typingEnv, buf2, 1024), typeToString(tc->b, typingEnv, buf3, 1024)); */
            ExprType *a = dereference(tc->a, typingEnv, r);
            ExprType *b = dereference(tc->b, typingEnv, r);
/*
            printf("warning: collasping %s with %s.\n", typeToString(a, typingEnv, buf2, 1024), typeToString(b, typingEnv, buf3, 1024));
*/
                        /*printVarTypeEnvToStdOut(typingEnv); */
            if (a->nodeType == T_VAR && b->nodeType == T_VAR && T_VAR_ID(a) == T_VAR_ID(b)) {
            } else if (a->nodeType == T_VAR && T_VAR_NUM_DISJUNCTS(a) == 0) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
            } else if (b->nodeType == T_VAR && T_VAR_NUM_DISJUNCTS(b) == 0) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
            } else if (a->nodeType == T_VAR && b->nodeType == T_VAR) {
                if(T_VAR_NUM_DISJUNCTS(a) > T_VAR_NUM_DISJUNCTS(b)) {
                    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
                } else {
                    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
                }
            } else if (a->nodeType == T_VAR) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
            } else if (b->nodeType == T_VAR) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
            } else {
                printf("error: simplified type does not have variable on either side.");

            }
            /* printVarTypeEnvToStdOut(typingEnv); */
            listRemove(typingConstraints, ln);
        } else {
            break;
        }
    }
    return 1;
}
Satisfiability simplify(List *typingConstraints, Hashtable *typingEnv, rError_t *errmsg, Node **errnode, Region *r) {
    ListNode *ln;
    int changed;
    /* printf("start\n"); */
    do {
        ln = typingConstraints->head;
        changed = 0;
        while(ln!=NULL) {
            TypingConstraint *tc = (TypingConstraint *)ln->value;
            ExprType *a = tc->a;
            ExprType *b = tc->b;
            char buf2[1024], buf3[1024], errmsgbuf1[ERR_MSG_LEN], errmsgbuf2[ERR_MSG_LEN];
            ListNode *next = ln->next;
/*
            printf("simplifying %s<%s.\n", typeToString(a, typingEnv, buf2, 1024), typeToString(b, typingEnv, buf3, 1024));
                    printVarTypeEnvToStdOut(typingEnv);
*/
            switch(simplifyLocally(tc, typingEnv, r)) {
                case TAUTOLOGY:
                    listRemove(typingConstraints, ln);
                    ln=next;
                    changed = 1;
            /* printf("tautology\n"); */
                    break;
                case ABSURDITY:
                    *errnode = ((TypingConstraint*)ln->value)->node;

                    snprintf(errmsgbuf1, ERR_MSG_LEN, "unsolvable %s<%s.\n", typeToString(a, typingEnv, buf2, 1024), typeToString(b, typingEnv, buf3, 1024));
                    generateErrMsg(errmsgbuf1, (*errnode)->expr, (*errnode)->base, errmsgbuf2);
                    addRErrorMsg(errmsg, -1, errmsgbuf2);
                    /*printVarTypeEnvToStdOut(typingEnv); */
            /* printf("absurdity\n"); */
                    return ABSURDITY;
                    TypingConstraint *curr;
                case CONTINGENCY:
                    curr = tc->next;
                    while(curr!=NULL) {
                        listAppendToNode(typingConstraints, ln, curr, r);
                        curr = curr->next;
                        ln = ln->next;
                    }
                    ln=next;
                    changed |= (a != tc->a || b != tc-> b);
                    /*if(a != tc->a || b != tc-> b) { */
                    /*    printf("simplified %s<%s to %s<%s.\n", */
                    /*            typeToString(a, typingEnv, buf2, 1024), typeToString(b, typingEnv, buf3, 1024), */
                    /*            typeToString(tc->a, typingEnv, buf2, 1024), typeToString(tc->b, typingEnv, buf3, 1024)); */
                    /*} */
            /* printf("contingency\n"); */
                    break;
            }
        }
    } while(changed);

    return CONTINGENCY;
}

ExprType* typeFunction3(Node* node, Hashtable* funcDesc, Hashtable* var_type_table, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
    /*printTree(node, 0); */
    int i;
    char *localErrorMsg;
    ExprType *res3 = NULL;
    char *fn = node->text;
    char buf[1024];
    /*printf("typeing %s\n",fn); */
    /*printVarTypeEnvToStdOut(var_type_table); */
    if(strcmp(fn, "foreach") == 0) {
        ERROR2(node->degree != 3,"wrong number of arguments to microservice");
        ERROR2(node->subtrees[0]->nodeType!=TK_TEXT,"argument form error");
        char* varname = node->subtrees[0]->text;
        ExprType *varType = (ExprType *)lookupFromHashTable(var_type_table, varname);
        ExprType *collType = varType == NULL? NULL:dereference(varType, var_type_table, r);
        if(collType!=NULL) {
            /* error if res is not a collection type or a type variable (primitive union type excluded) */
            ERROR2(collType->nodeType != T_CONS && (collType->nodeType != T_VAR || T_VAR_NUM_DISJUNCTS(collType)!=0 ), "foreach is applied to a non collection type");
            /* overwrite type of collection variable */
            if(collType->nodeType == T_VAR) {
                unifyTVarL(collType, newCollType(newTVar(r), r), var_type_table,
                        r);
                collType = dereference(collType, var_type_table, r);
            }
            /* dereference element type as only top level vars are dereferenced by the dereference function and we are accessing a subtree of the type */
            updateInHashTable(var_type_table, varname, dereference(T_CONS_TYPE_ARG(collType, 0), var_type_table, r));
        } else {
            ExprType *elemType;
            insertIntoHashTable(var_type_table, varname, elemType = newTVar(r));
            collType = newCollType(elemType, r);
        }
        res3 = typeExpression3(node->subtrees[1],funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
        ERROR2(res3->nodeType == T_ERROR, "foreach loop type error");
        res3 = typeExpression3(node->subtrees[2],funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
        ERROR2(res3->nodeType == T_ERROR, "foreach recovery type error");

        updateInHashTable(var_type_table, varname, collType); /* restore type of collection variable */
        return res3;
    } else {
        ERROR2(strcmp(fn, "assign") == 0 &&
                node->degree>0 &&
                !isPattern(node->subtrees[0]), "the first argument of microservice assign is not a variable or a pattern");
        ERROR2(strcmp(fn, "let") == 0 &&
                node->degree>0 &&
                !isPattern(node->subtrees[0]), "the first argument of microservice let is not a variable or a pattern");

        FunctionDesc *fDesc = (FunctionDesc*)lookupFromHashTable(funcDesc, fn);
        if(fDesc!=NULL && fDesc->type!=NULL) {
            fDesc = getFuncDescFromChain(node->degree,fDesc);
            if(fDesc == NULL) {
                sprintf(buf, "wrong number of arguments to microservice %s %d", fn, node->degree);
                *errnode = node;
            }
            ERROR2(
                    fDesc == NULL,
                    buf
            );
            ExprType *fTypeCopy;

            fTypeCopy = dupType(fDesc->type, r);

/*
            printf("start typing %s\n", fn);
*/
            int param_type_i;
            for(i=0, param_type_i = 0;i<node->degree;i++) {
/*
                printTreeDeref(node, 0, var_type_table, r);
*/
                ExprType *paramType = typeExpression3(node->subtrees[i], funcDesc, var_type_table, typingConstraints, errmsg, errnode, r);
                ERROR2(paramType->nodeType == T_ERROR,"parameter type error");
                ExprType *formalParamType = T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i);
                int ret = typeFuncParam(node->subtrees[i], paramType, formalParamType, var_type_table, typingConstraints, errmsg, r);
                if(ret!=0) {
                    *errnode = node->subtrees[i];
                }
                ERROR2(ret != 0, "parameter type error");
                node->subtrees[i]->coercionType = formalParamType; /* set coersion to parameter type */
                if(param_type_i != T_FUNC_ARITY(fTypeCopy) - 1 || T_FUNC_VARARG(fTypeCopy)==ONCE) {
                    param_type_i++;
                }
            }
/*
            printTreeDeref(node, 0, var_type_table, r);
            printf("finish typing %s\n", fn);
*/
            return dereference(T_FUNC_RET_TYPE(fTypeCopy), var_type_table, r);

        } else {

            int i;
            Node* expr = node;
            ExprType *params[MAX_PARAMS_LEN];
            if((i=typeParameters(params, expr->degree, expr->subtrees, funcDesc, var_type_table, typingConstraints, errmsg, errnode,r)) >= 0) {
                return params[i];
        }
		/* unknown action */
            return newSimpType(T_DYNAMIC, r);
        }
    }
    char errbuf[ERR_MSG_LEN];
    error:
    snprintf(errbuf, ERR_MSG_LEN, "type error: %s in %s", localErrorMsg, fn);
    addRErrorMsg(errmsg, -1, errbuf);
    return newSimpType(T_ERROR,r);
}
int typeFuncParam(Node *param, Node *paramType, Node *formalParamType, Hashtable *var_type_table, List *typingConstraints, rError_t *errmsg, Region *r) {
            char buf[ERR_MSG_LEN];
            char errbuf[ERR_MSG_LEN];
            char typebuf[ERR_MSG_LEN];
            char typebuf2[ERR_MSG_LEN];
            /* printf("typing param %s < %s\n",
                                typeToString(paramType, var_type_table, typebuf, ERR_MSG_LEN),
                                typeToString(formalParamType, var_type_table, typebuf2, ERR_MSG_LEN)); */
            if(formalParamType->nodeType == T_DYNAMIC) {
            } else if(paramType->nodeType == T_DYNAMIC) {
            } else if(formalParamType->nodeType == T_FLEX) {
                ExprType *t = formalParamType->subtrees[0];
                TypingConstraint *tc = newTypingConstraint(paramType, t, LT, param, r);
                Satisfiability tcons = simplifyLocally(tc, var_type_table, r);
                switch(tcons) {
                    case TAUTOLOGY:
                        break;
                    case CONTINGENCY:
                        while(tc!=NULL) {
                            listAppend(typingConstraints, tc, r);
                            tc = tc->next;
                        }
                        break;
                    case ABSURDITY:
                        snprintf(buf, 1024, "unsolvable typing constraint %s < %s",
                            typeToString(tc->a, var_type_table, typebuf, ERR_MSG_LEN),
                            typeToString(tc->b, var_type_table, typebuf2, ERR_MSG_LEN));
                        generateErrMsg(buf, param->expr, param->base, errbuf);
                        ERROR(1);
                }
            } else {
                ExprType *gcd;
                gcd = unifyWith(
                    paramType,
                    formalParamType,
                    var_type_table,
                    r);
                if(gcd == NULL) {
                    snprintf(buf, ERR_MSG_LEN, "parameter type mismatch parameter type %s, argument type %s",
                            typeToString(instantiate(formalParamType, var_type_table, 0, r), NULL, typebuf, ERR_MSG_LEN),
                            typeToString(instantiate(paramType, var_type_table, 0, r), NULL, typebuf2, ERR_MSG_LEN));
                    generateErrMsg(buf, param->expr, param->base, errbuf);
                } else {
                    errbuf[0] = '\0';
                }
                ERROR(gcd == NULL);
            }
            return 0;
    error:
    addRErrorMsg(errmsg, -1, errbuf);
    return -1;
}

ExprType* typeExpression3(Node *expr, Hashtable *funcDesc, Hashtable *varTypes, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
    ExprType *res;
	int i;
	expr->typed = 1;
	switch(expr->nodeType) {
		case TK_INT:
			return newSimpType(T_INT,r);
                case TK_DOUBLE:
                        return newSimpType(T_DOUBLE,r);
		case TK_STRING:
			return newSimpType(T_STRING,r);
		case TK_TEXT:
			if(expr->text[0]=='$' || expr->text[0]=='*') {
				ExprType* t = (ExprType *)lookupFromHashTable(varTypes, expr->text);
				if(t==NULL) {
					/* define new variable */
					t = newTVar(r);
					insertIntoHashTable(varTypes, expr->text, t);
				}
                                t = dereference(t, varTypes, r);
				return t;
			} else if(strcmp(expr->text,"nop")==0) {
				return newSimpType(T_INT, r);
			}
			/* not a variable, evaluate as a function */
		case N_APPLICATION:
                        /* try to type as a function */
                        /* the exprType is used to store the type of the return value */
                        expr->exprType = typeFunction3(expr, funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
                        return expr->exprType;
		case N_ACTIONS:
                if(expr->degree == 0) {
                    /* type of empty action sequence == T_INT */
                    return newSimpType(T_INT, r);
                }
                for(i=0;i<expr->degree;i++) {
                    /*printf("typing action in actions"); */
    				res = typeExpression3(expr->subtrees[i], funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
                    /*printVarTypeEnvToStdOut(varTypes); */
                    if(res->nodeType == T_ERROR) {
                        return res;
                    }
                }
                return res;
		case N_ACTIONS_RECOVERY:
                res = typeExpression3(expr->subtrees[0], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
                if(res->nodeType == T_ERROR) {
                    return res;
                }
                res = typeExpression3(expr->subtrees[1], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
                return res;
        default:
                break;
	}
	*errnode = expr;
        char errbuf[ERR_MSG_LEN];
	snprintf(errbuf, ERR_MSG_LEN, "error: unsupported ast node %d", expr->nodeType);
        addRErrorMsg(errmsg, -1, errbuf);
	return newSimpType(T_ERROR,r);
}
/*
 * This process is based on a few assumptions:
 * If the type coerced to cannot be inferred, i.e., an expression is to be coerced to some tvar, bounded to a union type or free,
 * then we leave further type checking to runtime, which can be done locally under the assumption of indistinguishable inclusion.
 *
 */
void postProcessCoercion(Node *expr, Hashtable *varTypes, rError_t *errmsg, Node **errnode, Region *r) {
    if(expr->coercionType!=NULL) {
                /*char buf[128];*/
        ExprType *deref;
                /*typeToString(expr->coercionType, NULL, buf, 128);
                printf("%s", buf);*/
        deref = instantiate(expr->coercionType, varTypes, 0, r);
                /*typeToString(deref, NULL, buf, 128);
                printf("->%s\n", buf);*/

/*
                if(deref->t == T_VAR && T_VAR_NUM_DISJUNCTS(deref)>0) {
                    ExprType *simp = newSimpType(T_VAR_DISJUNCT(deref, 0), r);
                    insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(deref), buf), simp);
                    deref = simp;
                }
*/
        expr->coercionType = deref;
    }
    int i;
    for(i=0;i<expr->degree;i++) {
        postProcessCoercion(expr->subtrees[i], varTypes, errmsg, errnode, r);
    }
}
/*
 * convert single action to actions if the parameter is of type actions
 */
void postProcessActions(Node *expr, Hashtable *systemFunctionTables, rError_t *errmsg, Node **errnode, Region *r) {
    int i;
    FunctionDesc *fd;
    switch(expr->nodeType) {
        case N_APPLICATION:
            fd = (FunctionDesc *)lookupFromHashTable(systemFunctionTables, expr->text);
            if(fd!=NULL) {
                fd = getFuncDescFromChain(expr->degree, fd);
                for(i=0;i<expr->degree;i++) {
                    switch(getParamIOType(fd->inOutValExp, i)) {
                        case 'a':
                            if(expr->subtrees[i]->nodeType != N_ACTIONS) {
                                Node **params = (Node **)region_alloc(r, sizeof(Node *)*1);
                                params[0] = expr->subtrees[i];
                                Label pos;
                                pos.base = expr->base;
                                pos.exprloc = expr->expr;
                                expr->subtrees[i] = createActionsNode(params, 1, &pos, r);
                            }
                    }
                }
            }
            break;
        default:
            break;
    }
    for(i=0;i<expr->degree;i++) {
        postProcessActions(expr->subtrees[i], systemFunctionTables, errmsg, errnode, r);
    }
}
