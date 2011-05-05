/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "utils.h"
#include "conversion.h"
/* make a new type by substituting tvars with fresh tvars */
ExprType *dupType(ExprType *ty, Region *r) {
    Hashtable *varTable = newHashTable(100);
    /* todo add oom handler here */
    ExprType *dup = dupTypeAux(ty, r, varTable);
    deleteHashTable(varTable, nop);
    return dup;


}

int typeEqSyntatic(ExprType *a, ExprType *b) {
    if(a->nodeType!=b->nodeType) {
        return 0;
    }
    switch(a->nodeType) {
        case T_CONS:
            if(T_CONS_ARITY(a) == T_CONS_ARITY(b) &&
                    strcmp(T_CONS_TYPE_NAME(a), T_CONS_TYPE_NAME(b)) == 0 &&
                    T_CONS_VARARG(a) == T_CONS_VARARG(b)) {
                int i;
                for(i=0;i<T_CONS_ARITY(a);i++) {
                    if(!typeEqSyntatic(T_CONS_TYPE_ARG(a, i),T_CONS_TYPE_ARG(b, i))) {
                        return 0;
                    }
                }
                return 1;
            }
            return 0;
        case T_VAR:
            return T_VAR_ID(a) == T_VAR_ID(b);
        case T_IRODS:
            return strcmp(a->text, b->text) == 0;
        default:
            return 1;
    }

}

ExprType *dupTypeAux(ExprType *ty, Region *r, Hashtable *varTable) {
    ExprType **paramTypes;
    int i;
    ExprType *newt;
    ExprType *exist;
    char *name;
    char buf[128];
    switch(ty->nodeType) {
        case T_CONS:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*T_CONS_ARITY(ty));
            for(i=0;i<T_CONS_ARITY(ty);i++) {
                paramTypes[i] = dupTypeAux(T_CONS_TYPE_ARG(ty, i),r,varTable);
            }
            newt = newConsTypeVarArg(T_CONS_ARITY(ty), T_CONS_VARARG(ty), T_CONS_TYPE_NAME(ty), paramTypes, r);
            newt->coercionAllowed = ty->coercionAllowed;
            return newt;
        case T_VAR:
            name = getTVarName(T_VAR_ID(ty), buf);
            exist = (ExprType *)lookupFromHashTable(varTable, name);
            if(exist != NULL)
                return exist;
            else {
                newt = newTVar2(T_VAR_NUM_DISJUNCTS(ty), T_VAR_DISJUNCTS(ty), r);
                newt->coercionAllowed = ty->coercionAllowed;
                insertIntoHashTable(varTable, name, newt);
                return newt;
            }
        default:
            return ty;
    }

}
int coercible(ExprType *a, ExprType *b) {
            return (a->nodeType!=T_CONS && a->nodeType == b->nodeType) ||
                   (b->nodeType == T_DOUBLE && a->nodeType == T_INT) ||
                   (b->nodeType == T_DOUBLE && a->nodeType == T_STRING) ||
                   (b->nodeType == T_INT && a->nodeType == T_DOUBLE) ||
                   (b->nodeType == T_INT && a->nodeType == T_STRING) ||
                   (b->nodeType == T_STRING && a->nodeType == T_INT) ||
                   (b->nodeType == T_STRING && a->nodeType == T_DOUBLE) ||
                   (b->nodeType == T_STRING && a->nodeType == T_BOOL) ||
                   (b->nodeType == T_BOOL && a->nodeType == T_STRING) ||
                   (b->nodeType == T_DATETIME && a->nodeType == T_INT) ||
                   (b->nodeType == T_DATETIME && a->nodeType == T_DOUBLE) ||
                   (b->nodeType == T_DYNAMIC) ||
                   (a->nodeType == T_DYNAMIC) ||
                   (a->nodeType==T_CONS && b->nodeType==T_CONS && coercible(T_CONS_TYPE_ARG(a, 0), T_CONS_TYPE_ARG(b, 0)));
}
/*
 * unify a free tvar or union type with some other type
 */
ExprType* unifyTVarL(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r) {
    char buf[128];
    if(T_VAR_NUM_DISJUNCTS(type)==0) { /* free */
        insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(type), buf), expected);
        return dereference(expected, varTypes, r);
    } else { /* union type */
        int i;
        ExprType *ty = NULL;
        for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                if(T_VAR_DISJUNCT(type,i)->nodeType == expected->nodeType) { /* union types can only include primitive types */
                    ty = expected;
                    break;
                }
        }
        if(ty != NULL) {
            insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(type), buf), expected);
        }
        return ty;
    }

}
ExprType* unifyTVarR(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r) {
    char buf[128];
    if(T_VAR_NUM_DISJUNCTS(expected)==0) { /* free */
        insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(expected), buf), type);
        return dereference(expected, varTypes, r);
    } else { /* union type */
        int i;
        ExprType *ty = NULL;
        for(i=0;i<T_VAR_NUM_DISJUNCTS(expected);i++) {
            if(type->nodeType == T_VAR_DISJUNCT(expected,i)->nodeType) { /* union types can only include primitive types */
                ty = type;
            }
        }
        if(ty != NULL) {
            insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(expected), buf), ty);
            return dereference(expected, varTypes, r);
        }
        return ty;
    }

}
/**
 * return The most general common instance of type and expected if unifiable
 *        NULL if false
 */
ExprType* unifyWith(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r) {
    char buf[128];
    /* dereference types to get the most specific type */
    /* as dereference only deref top level types, it is necessary to call dereference again */
    /* when unification is needed for subexpressions of the types which can be performed by calling this function */
    type = dereference(type, varTypes, r);
    expected = dereference(expected, varTypes, r);
    if(type->nodeType == T_VAR && expected->nodeType == T_VAR) {
        if(T_VAR_ID(type) == T_VAR_ID(expected)) {
            /* if both dereference to the same tvar then do not modify var types table */
            return type;
        } else if(T_VAR_NUM_DISJUNCTS(type) > 0 && T_VAR_NUM_DISJUNCTS(expected) > 0) {
            Node *c[10];
            Node** cp = c;
            int i,k;
            for(k=0;k<T_VAR_NUM_DISJUNCTS(expected);k++) {
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    if(T_VAR_DISJUNCT(type,i)->nodeType == T_VAR_DISJUNCT(expected,k)->nodeType) {
                        *(cp++)=T_VAR_DISJUNCT(expected,k);
                        break;
                    }
                }
            }
            if(cp == c) {
                return NULL;
            } else {
                ExprType *gcd;
                if(cp-c==1) {
                    gcd = *c;
                } else {
                    gcd = newTVar2(cp-c, c, r);
                }
                updateInHashTable(varTypes, getTVarName(T_VAR_ID(type), buf), gcd);
                updateInHashTable(varTypes, getTVarName(T_VAR_ID(expected), buf), gcd);
                return gcd;
            }
        } else {
            if(T_VAR_NUM_DISJUNCTS(type)==0) { /* free */
                insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(type), buf), expected);
                return dereference(expected, varTypes, r);
            } else if(T_VAR_NUM_DISJUNCTS(expected)==0) { /* free */
                insertIntoHashTable(varTypes, getTVarName(T_VAR_ID(expected), buf), type);
                return dereference(expected, varTypes, r);
            } else {
                /* error unreachable */
                return NULL;
            }
        }
    } else
	if(type->nodeType == T_VAR) {
            return unifyTVarL(type, expected, varTypes, r);
	} else if(expected->nodeType == T_VAR) {
            return unifyTVarR(type, expected, varTypes, r);
	} else {
            return unifyNonTvars(type, expected, varTypes, r);
	}
}
/**
 * Unify non tvar or union types.
 * return The most general instance if unifiable
 *        NULL if not
 */
ExprType* unifyNonTvars(ExprType *type, ExprType *expected, Hashtable *varTypes, Region *r) {
	if(type->nodeType == T_CONS && expected->nodeType == T_CONS) {
		if(
			strcmp(T_CONS_TYPE_NAME(type), T_CONS_TYPE_NAME(expected)) == 0
			&& T_CONS_ARITY(type) == T_CONS_ARITY(expected)) {
			int i;
			for(i=0;i<T_CONS_ARITY(type);i++) {
				ExprType *elemType = unifyWith(
                                        T_CONS_TYPE_ARG(type, i),
                                        T_CONS_TYPE_ARG(expected, i),
                                        varTypes,r); /* unifyWithCoercion performs dereference */
				if(elemType == NULL) {
					return NULL;
				}
				T_CONS_TYPE_ARG(expected, i) = elemType;
			}
			return dereference(expected, varTypes, r);
		} else {
			return NULL;
		}
        } else if(type->nodeType == T_IRODS || expected->nodeType == T_IRODS) {
                if(strcmp(type->text, expected->text)!=0) {
                        return NULL;
                }
                return expected;
	} else if(expected->nodeType == type->nodeType) { /* primitive types */
                return expected;
	} else {
            return NULL;
	}
}
/*
int unifyPrim(ExprType *type, TypeConstructor prim, Hashtable *typeVars, Region *r) {
	ExprType primType;
	primType.t = prim;
	ExprType *unifiedType = unifyNonTvars(type, &primType, typeVars, r);
	if(unifiedType == NULL) {
		return 0;
	} else {
		return 1;
	}
}
*/

/* counter for tvar generator */
int tvarNumber = 0;

/* utility function */
char* getTVarName(int vid, char name[128]) {
    snprintf(name, 128, "?%d",vid);
    return name;
}
char* getTVarNameRegion(int vid, Region *r) {
    char *name = (char *) region_alloc(r, sizeof(char)*128);
    snprintf(name, 128, "?%d",vid);
    return name;
}
char* getTVarNameRegionFromExprType(ExprType *tvar, Region *r) {
    return getTVarNameRegion(T_VAR_ID(tvar), r);
}

/**
 * create a new node n
 */
Node *newNode(NodeType type, char* text, Label * eloc, Region *r) {
	Node *node = (Node *)region_alloc(r, sizeof(Node));
	if(node == NULL)
		return NULL;
	node->nodeType=type;
        if(text!=NULL) {
            node->text = (char *)region_alloc(r,(strlen(text) + 1) * sizeof(char));
            strcpy(node->text, text);
        } else {
            node->text = NULL;
        }
	node->subtrees = NULL;
	node->degree = 0;
	node->expr = eloc == NULL? 0 : eloc->exprloc;
        node->typed = 0;
        node->exprType = NULL;
        node->coercionType = NULL;
        if(eloc!=NULL) {
            setBase(node, eloc->base, r);
        } else {
            setBase(node, "", r);
        }
	return node;
}

Node *newExprType(NodeType type, int degree, Node **subtrees, Region *r) {
    ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
    t->coercionAllowed = 0;
    t->base = NULL;
    t->coercionType = NULL;
    t->exprType = NULL;
    t->subtrees = subtrees;
    t->degree = degree;
    t->nodeType = type;
    t->expr = 0;
    t->text = NULL;
    t->typed = 1;
    return t;

}

int newTVarId() {
    return tvarNumber ++;
}
ExprType *newTVar2(int numDisjuncts, Node **disjuncts, Region *r) {
    ExprType *t = newExprType(T_VAR, 0, NULL, r);
    T_VAR_ID(t) = newTVarId();
    T_VAR_NUM_DISJUNCTS(t) = numDisjuncts;
    T_VAR_DISJUNCTS(t) = numDisjuncts == 0? NULL : (Node **)region_alloc(r, sizeof(Node *) * numDisjuncts);
    if(numDisjuncts!=0) {
        memcpy(T_VAR_DISJUNCTS(t), disjuncts, numDisjuncts*sizeof(Node *));
    }
    return t;
}

ExprType *newTVar(Region *r) {
    ExprType *t = newExprType(T_VAR, 0, NULL, r);
    T_VAR_ID(t) = newTVarId();
    T_VAR_NUM_DISJUNCTS(t) = 0;
    T_VAR_DISJUNCTS(t) = NULL;
    return t;
}

ExprType *newSimpType(NodeType type, Region *r) {
    return newExprType(type, 0, NULL, r);
}
ExprType *newErrorType(int errcode, Region *r) {
    Res *res = newExprType(T_ERROR, 0, NULL, r);
    res->value.errcode = errcode;
    return res;

}
ExprType *newFuncType(int arity, ExprType **paramTypes, ExprType* retType, Region *r) {
    return newFuncTypeVarArg(2, ONCE, paramTypes, retType, r);
}
ExprType *newFuncTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, ExprType* retType, Region *r) {
    ExprType **typeArgs = (ExprType **)region_alloc(r, sizeof(ExprType *) * 2);
    typeArgs[0] = newConsTypeVarArg(arity, vararg, cpStringExt(TUPLE, r), paramTypes, r);
    typeArgs[1] = retType;
    return newConsType(2, cpStringExt(FUNC, r), typeArgs, r);
}
ExprType *newConsType(int arity, char *cons, ExprType **paramTypes, Region *r) {
        return newConsTypeVarArg(arity, ONCE, cons, paramTypes, r);
}
ExprType *newConsTypeVarArg(int arity, enum vararg vararg, char *cons, ExprType **paramTypes, Region *r) {
	ExprType *t = newExprType(T_CONS, arity, paramTypes, r);
        T_CONS_VARARG(t) = vararg;
        T_CONS_TYPE_NAME(t) = cpString(cons, r);
        return t;
}

ExprType *newIRODSType(char *name, Region *r) {
        ExprType *t = newExprType(T_IRODS, 0, NULL, r);
        t->text = (char *)region_alloc(r,(strlen(name)+1) * sizeof(char));
        strcpy(t->text, name);
        return t;
}
ExprType *newCollType(ExprType *elemType, Region *r) {
        ExprType **typeArgs = (ExprType**) region_alloc(r, sizeof(ExprType*));
        typeArgs[0] = elemType;
        return newConsType(1, cpStringExt(LIST, r), typeArgs, r);
}

ExprType *newTupleType(int arity, ExprType **typeArgs, Region *r) {
        return newConsType(arity, cpStringExt(TUPLE, r), typeArgs, r);
}
/** Res functions */

Res* newCollRes(int size, ExprType *elemType, Region *r) {
	Res *res1 = newRes(r);
        res1->exprType = newCollType(elemType, r);
        res1->degree = size;
        res1->subtrees = (Res **)region_alloc(r, sizeof(Res *)*size);
	return res1;
}
/* used in cpRes only */
Res* newCollRes2(int size, Region *r) {
	Res *res1 = newRes(r);
        res1->exprType = NULL;
        res1->degree = size;
        res1->subtrees = (Res **)region_alloc(r, sizeof(Res *)*size);
	return res1;
}
Res* newRes(Region *r) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = NULL;
        res1->nodeType = N_VAL;
	return res1;
}
Res* newUninterpretedRes(Region *r, char *typeName, void *ioStruct, bytesBuf_t *ioBuf) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newIRODSType(typeName, r);
        res1->value.uninterpreted.inOutStruct = ioStruct;
        res1->value.uninterpreted.inOutBuffer = ioBuf;
	return res1;
}
Res* newIntRes(Region *r, int n) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_INT,r);
        res1->value.dval = n;
	return res1;
}
Res* newDoubleRes(Region *r, double a) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_DOUBLE,r);
        res1->value.dval = a;
	return res1;
}
Res* newBoolRes(Region *r, int n) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_BOOL,r);
        res1->value.dval = n;
	return res1;
}
/* precond: len(s) < size of res1->value.s */
Res* newStringRes(Region *r, char *s) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_STRING,r);
        res1->value.strlen = strlen(s);
        int size = (res1->value.strlen+1)*sizeof(char);
        res1->text = (char *)region_alloc(r, size);
        memcpy(res1->text, s, size);
	return res1;
}
Res* newDatetimeRes(Region *r, long dt) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_DATETIME,r);
        res1->value.tval = dt;
	return res1;
}
Res* newErrorRes(Region *r, int errcode) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_ERROR,r);
        res1->value.errcode = errcode;
	return res1;
}

/* copy to new region
 * If the new region is the same as the old region then do not copy.
 */
Res *cpRes(Res *res0, Region *r) {
    Res *res;
    if(!IN_REGION(res0, r)) {
        res = newRes(r);
        *res = *res0;
    } else {
        res = res0;
    }
    if(res->exprType!=NULL) {
        res->exprType = cpType(res->exprType, r);
    }
    if(res->text != NULL) {
        res->text = cpString(res->text, r);
    }
    int i;
    if(res->subtrees!=NULL) {
        if(!IN_REGION(res->subtrees, r)) {
            Node **temp = (Node **)region_alloc(r, sizeof(Node *) * res->degree);
            memcpy(temp, res->subtrees, sizeof(Node *) * res->degree);
            res->subtrees = temp;
        }
        for(i=0;i<res->degree;i++) {
            res->subtrees[i] = cpRes(res->subtrees[i], r);
        }
    }
    return res;
}

char *cpString(char *str, Region *r) {
    if(IN_REGION(str, r)) {
        return str;
    } else
    return cpStringExt(str, r);
}
char *cpStringExt(char *str, Region *r) {
    char *strCp = (char *)region_alloc(r, (strlen(str)+1) * sizeof(char) );
    strcpy(strCp, str);
    return strCp;
}
ExprType *cpType(ExprType *ty, Region *r) {
    if(IN_REGION(ty, r)) {
        return ty;
    }
    int i;
    ExprType *newt;
    newt = (ExprType *) region_alloc(r, sizeof(ExprType));
    memcpy(newt, ty, sizeof(ExprType));
    if(ty->subtrees != NULL) {
        newt->subtrees = (ExprType **) region_alloc(r,sizeof(ExprType *)*ty->degree);
        for(i=0;i<ty->degree;i++) {
            newt->subtrees[i] = cpType(ty->subtrees[i],r);
        }
    }
    if(ty->text != NULL) {
        newt->text = cpString(ty->text, r);
    }

    return newt;
}
/* copy res values from other region to r */
void cpHashtable(Hashtable *env, Region *r) {
	int i;

	for(i=0;i<env->size;i++) {
            struct bucket *b = env->buckets[i];
            while(b!=NULL) {
                b->value = cpRes((Res *)b->value, r);
                b= b->next;
            }
	}
}

void cpEnv(Env *env, Region *r) {
    cpHashtable(env->current, r);
    if(env->previous!=NULL) {
        cpEnv(env->previous, r);
    }
}

Res *setVariableValue(char *varName, Res *val, ruleExecInfo_t *rei, Env *env, rError_t *errmsg, Region *r) {
    int i;
    char *varMap;
    char errbuf[ERR_MSG_LEN];
    if (varName[0] == '$') {
        if(TYPE(val)!=T_STRING) {
            snprintf(errbuf, ERR_MSG_LEN, "error: assign a nonstring value to session variable %s.", varName);
            addRErrorMsg(errmsg, -1, errbuf);
            return newErrorRes(r, -1);
            /* todo find the proper error code */
        }
        i = getVarMap("", varName, &varMap, 0);
        if (i < 0) {
            snprintf(errbuf, ERR_MSG_LEN, "error: unsupported session variable \"%s\".",varName);
            addRErrorMsg(errmsg, UNSUPPORTED_SESSION_VAR, errbuf);
            return newErrorRes(r, UNSUPPORTED_SESSION_VAR);
        }
        setVarValue(varMap, rei, strdup(val->text));
        return newIntRes(r, 0);
    }
    else if(varName[0] == '*') {
        if(lookupFromEnv(env, varName)==NULL) {
            /* new variable */
            if(insertIntoHashTable(env->current, varName, val) == 0) {
                snprintf(errbuf, ERR_MSG_LEN, "error: unable to write to local variable \"%s\".",varName);
                addRErrorMsg(errmsg, UNSUPPORTED_SESSION_VAR, errbuf);
                return newErrorRes(r, UNABLE_TO_WRITE_LOCAL_VAR);
            }
        } else {
                updateInEnv(env, varName, val);
        }
        return newIntRes(r, 0);
    }
    return newIntRes(r, 0);
}

void printType(ExprType *type, Hashtable *var_types) {
    char buf[1024];
    typeToString(type, var_types, buf, 1024);
    printf("%s", buf);
}

char* typeToString(ExprType *type, Hashtable *var_types, char *buf, int bufsize) {
    buf[0] = '\0';
    Region *r = make_region(0, NULL);
        ExprType *etype = type;
        if(etype->nodeType == T_VAR && var_types != NULL) {
            /* dereference */
            etype = dereference(etype, var_types, r);
        }
        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", etype == NULL?"?":typeName_ExprType(etype));
        if(etype->nodeType == T_VAR) {
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "%d", T_VAR_ID(etype));
            if(T_VAR_NUM_DISJUNCTS(type)!=0) {
                snprintf(buf+strlen(buf), bufsize-strlen(buf), "{");
                int i;
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", typeName_ExprType(T_VAR_DISJUNCT(type, i)));
                }
                snprintf(buf+strlen(buf), bufsize-strlen(buf), "}");
            }
        } else if(etype->nodeType == T_CONS) {
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", T_CONS_TYPE_NAME(etype));
            int i;
            for(i=0;i<T_CONS_ARITY(etype);i++) {
                typeToString(T_CONS_TYPE_ARG(etype, i), var_types, buf+strlen(buf), bufsize-strlen(buf));
            }
        }


    region_free(r);
    return buf;

}
ExprType *dereference(ExprType *type, Hashtable *type_table, Region *r) {
    if(type->nodeType == T_VAR) {
        char name[128];
        getTVarName(T_VAR_ID(type), name);
        /* printf("deref: %s\n", name); */
        ExprType *deref = (ExprType *)lookupFromHashTable(type_table, name);
        if(deref == NULL)
            return type;
        else
            return dereference(deref, type_table, r);
    }
    return type;
}

ExprType *instantiate(ExprType *type, Hashtable *type_table, Region *r) {
    ExprType **paramTypes;
    int i;
    ExprType *typeInst;
    int changed = 0;

    switch(type->nodeType) {
        case T_CONS:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*T_CONS_ARITY(type));
            for(i=0;i<T_CONS_ARITY(type);i++) {
                paramTypes[i] = instantiate(T_CONS_TYPE_ARG(type, i), type_table, r);
                if(paramTypes[i]!=T_CONS_TYPE_ARG(type, i)) {
                    changed = 1;
                }
            }
            if(changed) {
                return newConsTypeVarArg(T_CONS_ARITY(type), T_CONS_VARARG(type), T_CONS_TYPE_NAME(type), paramTypes, r);
            } else {
                return type;
            }
        case T_VAR:
            typeInst = dereference(type, type_table, r);
            if(typeInst == type) {
                return type;
            } else {
                return instantiate(typeInst, type_table, r);
            }
        default:
            return type;

    }
}

/** debuggging functions **/
int writeToTmp(char *fileName, char *text) {
    char buf[1024];
    strcpy(buf, "/tmp/");
    strcat(buf, fileName);
    FILE *fp = fopen(buf, "a");
    if(fp==NULL) {
        return 0;
    }
    fputs(text, fp);
    fclose(fp);
    return 1;
}
int writeIntToTmp(char *fileName, int text) {
    char buf[1024];
	snprintf(buf, 1024, "%d", text);
	writeToTmp(fileName, buf);
    return 1;
}

void printEnvToStdOut(Hashtable *env) {
    char buffer[1024];
    printHashtable(env, buffer);
    printf("%s\n", buffer);
/*
    int i;
    for(i=0;i<env->size;i++) {
        struct bucket *b = env->buckets[i];
        while(b!=NULL) {
            printf("%s=%s\n",b->key, TYPENAME((Res *)b->value));
            b=b->next;
        }
    }
*/
}

void printVarTypeEnvToStdOut(Hashtable *env) {
    int i;
    for(i=0;i<env->size;i++) {
        struct bucket *b = env->buckets[i];
        while(b!=NULL) {
            printf("%s=",b->key);
            printType((ExprType *)b->value, NULL/*env*/);
            printf("\n");
            b=b->next;
        }
    }
}


/**
 * replace type var a with type b in varTypes
 */
/*
void replace(Hashtable *varTypes, int a, ExprType *b) {
	int i;
	for(i=0;i<varTypes->size;i++) {
		struct bucket *bucket= varTypes->buckets[i];
		while(bucket!=NULL) {
			ExprType *t = (ExprType *)bucket->value;
			if(t->t==T_VAR && T_VAR_ID(t) == a) {
				bucket -> value = b;
			}
			bucket = bucket->next;
		}
	}
}
*/
msParamArray_t *newMsParamArray() {
  msParamArray_t *mpa = (msParamArray_t *)malloc(sizeof(msParamArray_t));
  mpa->len = 0;
  mpa->msParam = NULL;
  mpa->oprType = 0;
  return mpa;
}

void deleteMsParamArray(msParamArray_t *msParamArray) {
  clearMsParamArray(msParamArray,0); /* do not delete inOutStruct because global varaibles of iRODS type may share it */
                                    /* to do write a function that delete inOutStruct of msParamArray if it is not shared */
  free(msParamArray);

}

Env *newEnv(Hashtable *current, Env *previous, Hashtable *funcDesc) {
    Env *env = (Env *)malloc(sizeof(Env));
    env->current = current;
    env->previous = previous;
    env->funcDesc = funcDesc;
    return env;
}

void deleteEnv(Env *env, int deleteCurrent) {
    if(deleteCurrent>=1) {
        deleteHashTable(env->current, nop);
    }
    if(deleteCurrent==2) {
        if(env->previous!=NULL && env->previous->previous!=NULL) {
            deleteEnv(env->previous, deleteCurrent);
        }
    }
    if(deleteCurrent>=3) {
        if(env->previous==NULL) {
            deleteHashTable(env->funcDesc, nop);
        } else {
            deleteEnv(env->previous, deleteCurrent);

        }
    }
    free(env);
}

List *newList(Region *r) {
    List *l = (List *)region_alloc(r, sizeof (List));
    l->head = l->tail = NULL;
    return l;
}

ListNode *newListNode(void *value, Region *r) {
    ListNode *l = (ListNode *)region_alloc(r, sizeof (ListNode));
    l->next = NULL;
    l->value = value;
    return l;
}

void listAppend(List *list, void *value, Region *r) {
    ListNode *ln = newListNode(value, r);
    if(list->head != NULL) {
        list->tail = list->tail->next = ln;
    } else {
        list->head = list->tail = ln;
    }
}

void listAppendToNode(List *list, ListNode *node, void *value, Region *r) {
    ListNode *ln = newListNode(value, r);
    if(node->next != NULL) {
        ln->next = node->next;
        node->next = ln;
    } else {
        node->next = list->tail = ln;
    }
}

void listRemove(List *list, ListNode *node) {
    ListNode *prev = NULL, *curr = list->head;
    while(curr != NULL) {
        if(curr == node) {
            if(prev == NULL) {
                list->head = node->next;
            } else {
                prev->next = node->next;
            }
            /*free(node); */
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    if(list->tail == node) {
        list->tail = prev;
    }

}

TypingConstraint *newTypingConstraint(ExprType *a, ExprType *b, TypingConstraintType type, Node *node, Region *r) {
    TypingConstraint *tc = (TypingConstraint *)region_alloc(r, sizeof (TypingConstraint));
    tc->a = a;
    tc->b = b;
    tc->constraintType = type;
    tc->node = node;
    tc->next = NULL;
    return tc;
}

char *getRuleBasePath(char *ruleBaseName, char rulesFileName[MAX_NAME_LEN]) {
    char *configDir = getConfigDir ();
    snprintf (rulesFileName, MAX_NAME_LEN, "%s/reConfigs/%s.re", configDir,ruleBaseName);
    return rulesFileName;

}


int appendToByteBufNew(bytesBuf_t *bytesBuf, char *str) {
  int i,j;
  char *tBuf;

  i = strlen(str);
  j = 0;
  if (bytesBuf->buf == NULL) {
    bytesBuf->buf = malloc (i + 1 + MAX_NAME_LEN * 5);
    memset(bytesBuf->buf, 0, i + 1 + MAX_NAME_LEN * 5);
    strcpy((char *)bytesBuf->buf, str);
    bytesBuf->len = i + 1 + MAX_NAME_LEN * 5; /* allocated length */
  }
  else {
    j = strlen((char *)bytesBuf->buf);
    if ((i + j) < bytesBuf->len) {
      strcat((char *)bytesBuf->buf, str);
    }
    else { /* needs more space */
      tBuf = (char *) malloc(j + i + 1 + (MAX_NAME_LEN * 5));
      strcpy(tBuf,(char *)bytesBuf->buf);
      strcat(tBuf,str);
      free (bytesBuf->buf);
      bytesBuf->len = j + i + 1 + (MAX_NAME_LEN * 5);
      bytesBuf->buf = tBuf;
    }
  }
  return 0;
}

void logErrMsg(rError_t *errmsg) {
    char errbuf[ERR_MSG_LEN*1024];
    errMsgToString(errmsg, errbuf, ERR_MSG_LEN*1024);
#ifdef DEBUG
    writeToTmp("err.log", "begin errlog\n");
    writeToTmp("err.log", errbuf);
    writeToTmp("err.log", "end errlog\n");
#endif
    rodsLog (LOG_ERROR, "%s", errbuf);
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
        p += strlen(errbuf+p);
    }
    return errbuf;

}

void *lookupFromEnv(Env *env, char *key) {
    void* val = lookupFromHashTable(env->current, key);
    if(val==NULL && env->previous!=NULL) {
        val = lookupFromEnv(env->previous, key);
    }
    return val;
}

void updateInEnv(Env *env, char *varName, Res *res) {
    Env *defined = env;

    while(defined  != NULL && lookupFromHashTable(defined->current, varName) == NULL) {
        defined  = defined ->previous;
    }
    if(defined != NULL) {
        updateInHashTable(defined->current, varName, res);
    } else {
        insertIntoHashTable(env->current, varName, res);
    }
}

void freeEnvUninterpretedStructs(Env *e) {
    Hashtable *ht = e->current;
    int i;
    for(i=0;i<ht->size;i++) {
        struct bucket *b = ht->buckets[i];
        while(b!=NULL) {
            Res *res = (Res *) b->value;
            if(TYPE(res) == T_IRODS) {
                if(res->value.uninterpreted.inOutStruct!=NULL) {
                    free(res->value.uninterpreted.inOutStruct);
                }
                if(res->value.uninterpreted.inOutBuffer!=NULL) {
                    free(res->value.uninterpreted.inOutBuffer);
                }
            }
            b=b->next;
        }
    }
    if(e->previous!=NULL) {
        freeEnvUninterpretedStructs(e->previous);
    }
}
int isPattern(Node *pattern) {

    if(pattern->nodeType == N_APPLICATION) {
        int i;
        for(i=0;i<pattern->degree;i++) {
            if(!isPattern(pattern->subtrees[i]))
                return 0;
        }
        return 1;
    } else if(pattern->nodeType == TK_TEXT) {
        return 1;
    } else {
        return 0;

    }
}

int isRecursive(Node *rule) {
    return invokedIn(rule->subtrees[0]->text, rule->subtrees[1]) ||
        invokedIn(rule->subtrees[0]->text, rule->subtrees[2]) ||
        invokedIn(rule->subtrees[0]->text, rule->subtrees[3]);

}

int invokedIn(char *fn, Node *expr) {
    int i;
    switch(expr->nodeType) {
        case N_APPLICATION:
            if(strcmp(expr->text, fn) == 0) {
                return 1;
            }

        case N_ACTIONS:
        case N_ACTIONS_RECOVERY:
            for(i=0;i<expr->degree;i++) {
                if(invokedIn(fn, expr->subtrees[i])) {
                    return 1;
                }
            }
            break;
        default:
            break;
    }

    return 0;
}
