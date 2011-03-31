/* For copyright information please refer to files in the COPYRIGHT directory
 */

#include "typing.h"
#include "functions.h"
#define ERROR2(x,y) if(x) {localErrorMsg=(y);goto error;}
/**
 * return 0 to len-1 index of the parameter with type error
 *        -1 success
 */
int typeParameters(ExprType** paramTypes, int len, Node** subtrees, Hashtable* funcDesc, Hashtable *symbol_type_table, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
	int i;
	for(i=0;i<len;i++) {
		paramTypes[i] = dereference(typeExpression3(subtrees[i], funcDesc, symbol_type_table, typingConstraints, errmsg, errnode, r), symbol_type_table, r);
		if(paramTypes[i]->t == T_ERROR) {
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
        if (type->t == T_VAR) {
            if(T_VAR_NUM_DISJUNCTS(type) > 0) {
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    a.t = T_VAR_DISJUNCT(type,i);
                    if(!tautologyLt(&a, expected)) {
                        return 0;
                    }
                }
                return 1;
            } else {
                return 0;
            }

        } else if (expected->t == T_VAR) {
            if(T_VAR_NUM_DISJUNCTS(expected) > 0) {
                for(i=0;i<T_VAR_NUM_DISJUNCTS(expected);i++) {
                    b.t = T_VAR_DISJUNCT(expected,i);
                    if(!tautologyLt(type, &b)) {
                        return 0;
                    }
                }
                return 1;
            } else {
                return 0;
            }
        } else if (type->t == T_CONS && expected->t == T_CONS) {
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
        switch(a->t) {
            case T_INT:
                return b->t == T_INT ||
                        b->t == T_DOUBLE;
            case T_DOUBLE:
            case T_BOOL:
            case T_STRING:
                return a->t==b->t;
            default:
                return 0;
        }
}

/*
 * simplify expected which is a variable, bounded or unbounded, based on type which is not a variable
 * returns 1 tautology
 *         0 contingency
 *         -1 abserdity
 */
Satisfiability simplifyR(ExprType *type, ExprType *expected, ExprType **bn, Region *r) {
    ExprType b;
    if(expected->ext.tvar.numDisjuncts > 0) {
        TypeConstructor c[100];
        TypeConstructor* cp = c;
        int i;
        for(i=0;i<expected->ext.tvar.numDisjuncts;i++) {
            b.t = T_VAR_DISJUNCT(expected, i);;
            if(tautologyLt(type, &b)) {
                *(cp++)=T_VAR_DISJUNCT(expected, i);
            }
        }
        if(cp == c) {
            return ABSERDITY;
        } else {
            if(cp-c==1) {
                *bn = newSimpType(*c, r);
            } else if (cp-c == T_VAR_NUM_DISJUNCTS(expected)) {
                *bn = expected;
            } else {
                *bn = newTVar2(cp-c, c, r);
            }
            return TAUTOLOGY;
        }
    } else {
        *bn = expected;
        return CONTIGENCY;
    }
}

Satisfiability simplifyL(ExprType *type, ExprType *expected, ExprType **an, Region *r) {
    ExprType a;
    if(type->ext.tvar.numDisjuncts > 0) {
        TypeConstructor c[100];
        TypeConstructor* cp = c;
        int i;
        for(i=0;i<type->ext.tvar.numDisjuncts;i++) {
            a.t = T_VAR_DISJUNCT(type, i);
            if(tautologyLt(&a, expected)) {
                *(cp++)=T_VAR_DISJUNCT(type, i);
            }
        }
        if(cp == c) {
            return ABSERDITY;
        } else {
            if(cp-c==1) {
                *an = newSimpType(*c, r);
            } else if (cp-c == T_VAR_NUM_DISJUNCTS(type)) {
                *an = type;
            } else {
                *an = newTVar2(cp-c, c, r);
            }
            return TAUTOLOGY;
        }
    } else {
        *an = type;
        return CONTIGENCY;
    }
}
Satisfiability narrow(ExprType *type, ExprType *expected, ExprType **an, ExprType **bn, Region *r) {
    ExprType a, b;
        if(type->ext.tvar.vid == expected->ext.tvar.vid) {
            *an = type;
            *bn = expected;
            return CONTIGENCY;
        } else if(type->ext.tvar.numDisjuncts > 0 && expected->ext.tvar.numDisjuncts > 0) {
            TypeConstructor c[100];
            TypeConstructor* cp = c;
            int i,k;
            for(k=0;k<expected->ext.tvar.numDisjuncts;k++) {
                for(i=0;i<type->ext.tvar.numDisjuncts;i++) {
                    a.t = T_VAR_DISJUNCT(type,i);
                    b.t = T_VAR_DISJUNCT(expected,k);
                    if(tautologyLt(&a, &b)) {
                        *(cp++)=T_VAR_DISJUNCT(expected,k);
                        break;
                    }
                }
            }
            ExprType *gcd;
            if(cp == c) {
                return ABSERDITY;
            } else {
                if(cp-c==1) {
                    gcd = newSimpType(*c, r);
                } else if (cp-c == T_VAR_NUM_DISJUNCTS(expected)) {
                    gcd = expected;
                } else {
                    gcd = newTVar2(cp-c, c, r);
                }
                *bn = gcd;
            }
            cp = c;
            for(i=0;i<type->ext.tvar.numDisjuncts;i++) {
                for(k=0;k<expected->ext.tvar.numDisjuncts;k++) {
                    a.t = T_VAR_DISJUNCT(type,i);
                    b.t = T_VAR_DISJUNCT(expected,k);
                    if(tautologyLt(&a, &b)) {
                        *(cp++)=T_VAR_DISJUNCT(type,i);
                        break;
                    }
                }
            }
            if(cp-c==1) {
                gcd = newSimpType(*c, r);
            } else if(cp-c == T_VAR_NUM_DISJUNCTS(type)) {
                gcd = type;
            } else {
                gcd = newTVar2(cp-c, c, r);
            }
            *an = gcd;
            return CONTIGENCY;
        } else if(type->ext.tvar.numDisjuncts==0) { /* free */
            *an = type;
            *bn = expected;
            return CONTIGENCY;
        } else /*if(expected->ext.tvar.numDisjuncts==0)*/ { /* free */
            *an = type;
            *bn = expected;
            return CONTIGENCY;
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
    tc->a = instantiate(tc->a, typingEnv, r);
    tc->b = instantiate(tc->b, typingEnv, r);
    if(tautologyLt(tc->a, tc->b)) {
        return TAUTOLOGY;
    }
    ExprType *an, *bn;
    char tvarname[128];
    if(tc->a->t == TVAR && tc->b->t == TVAR) {
        Satisfiability sat = narrow(tc->a, tc->b, &an, &bn, r);
        if(tc->a!= an) {
            insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->a), tvarname), an);
            tc->a = an;
        }
        if(tc->b!= bn) {
            insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->b), tvarname), bn);
            tc->b = bn;
        }
        if((sat & ABSERDITY) == 0) {
            return CONTIGENCY;
        } else {
            return ABSERDITY;
        }
    } else if(tc->a->t==TVAR) {
        if(simplifyL(tc->a, tc->b, &an, r) != ABSERDITY) {
            if(tc->a!= an) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->a), tvarname), an);
                tc->a = an;
            }
            return CONTIGENCY;
        } else {
            return ABSERDITY;
        }

    } else if(tc->b->t==TVAR) {
        if(tc->b->ext.tvar.vid == 42) {
            printf("error");
        }
        if(simplifyR(tc->a, tc->b, &bn, r) != ABSERDITY) {
            if(tc->b!= bn) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(tc->b), tvarname), bn);
                tc->b = bn;
            }
            return CONTIGENCY;
        } else {
            return ABSERDITY;
        }

    } else if(tc->a->t == T_CONS && tc->b->t == T_CONS) {
        if(strcmp(T_CONS_TYPE_NAME(tc->a), T_CONS_TYPE_NAME(tc->b))!=0) {
            return ABSERDITY;
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
                    case ABSERDITY:
                        return ABSERDITY;
                    case TAUTOLOGY:
                        break;
                    case CONTIGENCY:
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
            return noconstr? TAUTOLOGY : CONTIGENCY;
        }
    } else {
        return ABSERDITY;
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
        if(sat == ABSERDITY) {
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
            if (a->t == T_VAR && b->t == T_VAR && T_VAR_ID(a) == T_VAR_ID(b)) {
            } else if (a->t == T_VAR && T_VAR_NUM_DISJUNCTS(a) == 0) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
            } else if (b->t == T_VAR && T_VAR_NUM_DISJUNCTS(b) == 0) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
            } else if (a->t == T_VAR && b->t == T_VAR) {
                if(T_VAR_NUM_DISJUNCTS(a) > T_VAR_NUM_DISJUNCTS(b)) {
                    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
                } else {
                    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
                }
            } else if (a->t == T_VAR) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
            } else if (b->t == T_VAR) {
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
                case ABSERDITY:
                    *errnode = ((TypingConstraint*)ln->value)->node;

                    snprintf(errmsgbuf1, ERR_MSG_LEN, "unsolvable %s<%s.\n", typeToString(a, typingEnv, buf2, 1024), typeToString(b, typingEnv, buf3, 1024));
                    generateErrMsg(errmsgbuf1, (*errnode)->expr, (*errnode)->base, errmsgbuf2);
                    addRErrorMsg(errmsg, -1, errmsgbuf2);
                    /*printVarTypeEnvToStdOut(typingEnv); */
            /* printf("abserdity\n"); */
                    return ABSERDITY;
                    TypingConstraint *curr;
                case CONTIGENCY:
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
            /* printf("contigency\n"); */
                    break;
            }
        }
    } while(changed);

    return CONTIGENCY;
}

ExprType* typeFunction3(Node* node, Hashtable* funcDesc, Hashtable* var_type_table, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
    /*printTree(node, 0); */
    int i;
    char *localErrorMsg;
    ExprType *res3 = NULL;
    char *fn = node->text;
    char buf[1024];
    char buf2[1024];
    char buf3[1024];
    /*printf("typeing %s\n",fn); */
    /*printVarTypeEnvToStdOut(var_type_table); */
    if(strcmp(fn, "foreach") == 0) {
        ERROR2(node->degree != 3,"wrong number of arguments to microservice");
        ERROR2(node->subtrees[0]->type!=TEXT,"argument form error");
        char* varname = node->subtrees[0]->text;
        ExprType *varType = (ExprType *)lookupFromHashTable(var_type_table, varname);
        ExprType *collType = varType == NULL? NULL:dereference(varType, var_type_table, r);
        if(collType!=NULL) {
            /* error if res is not a collection type or a type variable (primitive union type excluded) */
            ERROR2(collType->t != CONS && (collType->t != TVAR || T_VAR_NUM_DISJUNCTS(collType)!=0 ), "foreach is applied to a non collection type");
            /* overwrite type of collection variable */
            if(collType->t == TVAR) {
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
        ERROR2(res3->t == T_ERROR, "foreach loop type error");
        res3 = typeExpression3(node->subtrees[2],funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
        ERROR2(res3->t == T_ERROR, "foreach recovery type error");

        updateInHashTable(var_type_table, varname, collType); /* restore type of collection variable */
        return res3;
    } else {
        ERROR2(strcmp(fn, "assign") == 0 &&
                node->degree != 2, "wrong number of arguments to microservice assign");
        ERROR2(strcmp(fn, "assign") == 0 &&
                node->subtrees[0]->type!=TEXT, "the first argument of microservice assign is a complex expression");

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
                ERROR2(paramType->t == T_ERROR,"parameter type error");
                ExprType *gcd;
                if(T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i)->t == T_DYNAMIC) {
                    gcd = paramType;
                } else if(paramType->t == T_DYNAMIC) {
                    gcd = T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i);
                } else if(T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i)->coercionAllowed) {
                    gcd = T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i);
                        TypingConstraint *tc = newTypingConstraint(paramType, gcd, LT, node -> subtrees[i], r);
                        Satisfiability tcons = simplifyLocally(tc, var_type_table, r);
                        switch(tcons) {
                            case TAUTOLOGY:
                                break;
                            case CONTIGENCY:
                                while(tc!=NULL) {
                                    listAppend(typingConstraints, tc, r);
                                    tc = tc->next;
                                }
                                break;
                            case ABSERDITY:
                                *errnode = node->subtrees[i];
                                snprintf(buf, 1024, "unsolvable typing constraint %s < %s",
                                    typeToString(tc->a, var_type_table, buf2, 1024),
                                    typeToString(tc->b, var_type_table, buf3, 1024));
                                char buf4[ERR_MSG_LEN];
                                generateErrMsg(buf, (*errnode)->expr, (*errnode)->base, buf4);
                                ERROR2(1, buf4);
                        }
/*                        printType(paramType, NULL); */
/*                        printType(gcd, NULL); */
                } else {
                    gcd = unifyWith(
                        paramType,
                        T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i),
                        var_type_table,
                        r);
                    char buf[1024];
                    if(gcd == NULL) {
                        *errnode = node->subtrees[i];
                        snprintf(buf, 1024, "parameter %d type mismatch parameter type %s, argument type %s", i,
                                typeName_ExprType(instantiate(paramType, var_type_table, r)),
                                typeName_ExprType(instantiate(T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i), var_type_table, r)));
                    } else {
                        buf[0] = '\0';
                    }
                    ERROR2(gcd == NULL,buf);
                }
                node->subtrees[i]->coercion = T_FUNC_PARAM_TYPE(fTypeCopy,param_type_i); /* set coersion to parameter */
                if(param_type_i != T_FUNC_ARITY(fTypeCopy) - 1 || fTypeCopy->ext.func.vararg==ONCE) {
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


ExprType* typeExpression3(Node *expr, Hashtable *funcDesc, Hashtable *varTypes, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
    ExprType *res;
	int i;
	expr->typed = 1;
	switch(expr->type) {
		case INT:
			return newSimpType(T_INT,r);
                case DOUBLE:
                        return newSimpType(T_DOUBLE,r);
		case STRING:
			return newSimpType(T_STRING,r);
		case TEXT:
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
		case APPLICATION:
                        /* try to type as a function */
                        return typeFunction3(expr, funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
		case ACTIONS:
                        if(expr->degree == 0) {
                            /* type of empty action sequence == T_INT */
                            return newSimpType(T_INT, r);
                        }
                        for(i=0;i<expr->degree;i++) {
                                /*printf("typing action in actions"); */
    				res = typeExpression3(expr->subtrees[i], funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
                                /*printVarTypeEnvToStdOut(varTypes); */
				if(res->t == T_ERROR) {
					return res;
				}
			}
			return res;
            default:
                break;
	}
	*errnode = expr;
        char errbuf[ERR_MSG_LEN];
	snprintf(errbuf, ERR_MSG_LEN, "error: unsupported ast node %d", expr->type);
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
    if(expr->coercion!=NULL) {
        ExprType *deref;
	switch(expr->coercion->t) {
            case TVAR:
                deref = instantiate(expr->coercion, varTypes, r);
/*
                char buf[128];
                if(deref->t == T_VAR && T_VAR_NUM_DISJUNCTS(deref)>0) {
                    ExprType *simp = newSimpType(T_VAR_DISJUNCT(deref, 0), r);
                    insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(deref), buf), simp);
                    deref = simp;
                }
*/
                expr->coercion = deref;
                break;
            default:
                break;
	}
    }
    int i;
/*
    if(expr->coercion!= NULL && expr->coercion->t == T_ERROR) {
        printf("error");
    }
*/
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
    switch(expr->type) {
        case APPLICATION:
            fd = (FunctionDesc *)lookupFromHashTable(systemFunctionTables, expr->text);
            if(fd!=NULL) {
                fd = getFuncDescFromChain(expr->degree, fd);
                for(i=0;i<expr->degree;i++) {
                    switch(getParamIOType(fd->inOutValExp, i)) {
                        case 'a':
                            if(expr->subtrees[i]->type != ACTIONS) {
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
