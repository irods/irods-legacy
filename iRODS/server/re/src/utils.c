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
    if(a->nodeType!=b->nodeType || a->vararg != b->vararg) {
        return 0;
    }
    switch(a->nodeType) {
        case T_CONS:
        case T_TUPLE:
            if(T_CONS_ARITY(a) == T_CONS_ARITY(b) &&
                    (a->nodeType == T_TUPLE || strcmp(T_CONS_TYPE_NAME(a), T_CONS_TYPE_NAME(b)) == 0)) {
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
            newt = newConsType(T_CONS_ARITY(ty), T_CONS_TYPE_NAME(ty), paramTypes, r);
            break;
        case T_TUPLE:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*T_CONS_ARITY(ty));
            for(i=0;i<T_CONS_ARITY(ty);i++) {
                paramTypes[i] = dupTypeAux(T_CONS_TYPE_ARG(ty, i),r,varTable);
            }
            newt = newTupleType(T_CONS_ARITY(ty), paramTypes, r);
            break;
        case T_VAR:
            name = getTVarName(T_VAR_ID(ty), buf);
            exist = (ExprType *)lookupFromHashTable(varTable, name);
            if(exist != NULL)
                newt = exist;
            else {
                newt = newTVar2(T_VAR_NUM_DISJUNCTS(ty), T_VAR_DISJUNCTS(ty), r);
                insertIntoHashTable(varTable, name, newt);

            }
            break;
        case T_FLEX:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*1);
            paramTypes[0] = dupTypeAux(ty->subtrees[0],r,varTable);
            newt = newExprType(T_FLEX, 1, paramTypes, r);
            break;

        default:
            newt = ty;
    }
    newt->vararg = ty->vararg;
    newt->iotype = ty->iotype;
	return newt;
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
    	if(occursIn(type, expected)) {
    		return NULL;
    	}
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
    	if(occursIn(expected, type)) {
    		return NULL;
    	}
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
	if(type->vararg != expected->vararg) {
		return NULL;
	}
    char buf[128];
    /* dereference types to get the most specific type */
    /* as dereference only deref top level types, it is necessary to call dereference again */
    /* when unification is needed for subexpressions of the types which can be performed by calling this function */
    type = dereference(type, varTypes, r);
    expected = dereference(expected, varTypes, r);
    if(type->nodeType == T_UNSPECED) {
        return expected;
    }
    if(expected->nodeType == T_DYNAMIC) {
        return type;
    }
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
		if(strcmp(T_CONS_TYPE_NAME(type), T_CONS_TYPE_NAME(expected)) == 0
			&& T_CONS_ARITY(type) == T_CONS_ARITY(expected)) {
            ExprType **subtrees = (ExprType **) region_alloc(r, sizeof(ExprType *) * T_CONS_ARITY(expected));

			int i;
			for(i=0;i<T_CONS_ARITY(type);i++) {
				ExprType *elemType = unifyWith(
                                        T_CONS_TYPE_ARG(type, i),
                                        T_CONS_TYPE_ARG(expected, i),
                                        varTypes,r); /* unifyWithCoercion performs dereference */
				if(elemType == NULL) {
					return NULL;
				}
				subtrees[i] = elemType;
			}
			return dereference(newConsType(T_CONS_ARITY(expected), T_CONS_TYPE_NAME(expected), subtrees, r), varTypes, r);
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
            return newErrorType(TYPE_ERROR, r);
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
    node->coerce = 0;
    node->iotype = IO_TYPE_INPUT;
    node->value.constructTuple = 0;
    if(eloc!=NULL) {
        setBase(node, eloc->base, r);
    } else {
        setBase(node, "", r);
    }
	return node;
}
Node **allocSubtrees(Region *r, int size)
{
    return (Node**)region_alloc(r, sizeof (Node*) * size);
}


Node *newExprType(NodeType type, int degree, Node **subtrees, Region *r) {
    ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
    t->base = NULL;
    t->exprType = NULL;
    t->subtrees = subtrees;
    t->degree = degree;
    t->nodeType = type;
    t->expr = 0;
    t->text = NULL;
    t->typed = 1;
    t->iotype = IO_TYPE_INPUT;
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
ExprType *newFuncType(ExprType *paramType, ExprType* retType, Region *r) {
    ExprType **typeArgs = (ExprType **)region_alloc(r, sizeof(ExprType *) * 2);
    typeArgs[0] = paramType;
    typeArgs[1] = retType;
    return newConsType(2, cpStringExt(FUNC, r), typeArgs, r);
}
ExprType *newFuncTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, ExprType* retType, Region *r) {
    return newFuncType(newTupleTypeVarArg(arity, vararg, paramTypes, r), retType, r);
}
ExprType *newConsType(int arity, char *cons, ExprType **paramTypes, Region *r) {
	ExprType *t = newExprType(T_CONS, arity, paramTypes, r);
    T_CONS_TYPE_NAME(t) = cpString(cons, r);
    return t;
}
ExprType *newTupleTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, Region *r) {
	ExprType *t = newExprType(T_TUPLE, arity, paramTypes, r);
    t->vararg = vararg;
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
        return newExprType(T_TUPLE, arity, typeArgs, r);
}
ExprType *newUnaryType(NodeType nodeType, ExprType *typeArg, Region *r) {
    ExprType **typeArgs = (ExprType**) region_alloc(r, sizeof(ExprType*));
    typeArgs[0] = typeArg;
    return newExprType(nodeType, 1, typeArgs, r);
}
/** Res functions */
/*FunctionDesc *newFunctionDesc(char *type, SmsiFuncPtrType func, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    desc->value.func = func;
    desc->exprType = type == NULL? NULL:parseFuncTypeFromString(type, r);
    desc->nodeType = N_C_FUNC;
    return desc;
}
FunctionDesc *newConstructorDesc(char *type, Region *r) {
    return newConstructorDesc2(parseFuncTypeFromString(type, r), r);
}

FunctionDesc *newConstructorDesc2(Node *type, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    desc->exprType = type;
    desc->nodeType = N_CONSTRUCTOR;
    return desc;
}
FunctionDesc *newDeconstructorDesc(char *type, int proj, Region *r) {
    FunctionDesc *desc = (FunctionDesc *) region_alloc(r, sizeof(FunctionDesc));
    desc->exprType = type == NULL? NULL:parseFuncTypeFromString(type, r);
    desc->nodeType = N_DECONSTRUCTOR;
    desc->value.proj = proj;
    return desc;
}*/
FunctionDesc *newFuncSymLink(char *fn , int nArgs, Region *r) {
    Res *desc = newRes(r);
    desc->nodeType = N_FUNC_SYM_LINK;
    desc->text = cpStringExt(fn ,r);
    desc->value.nArgs = nArgs;
    desc->exprType = newSimpType(T_DYNAMIC, r);
    return desc;
}

Node *newPartialApplication(Node *func, Node *arg, int nArgsLeft, Region *r) {
    Res *res1 = newRes(r);
    res1->nodeType = N_PARTIAL_APPLICATION;
    res1->value.nArgs = nArgsLeft;
    res1->degree = 2;
    res1->subtrees = (Res **)region_alloc(r, sizeof(Res *)*2);
    res1->subtrees[0] = func;
    res1->subtrees[1] = arg;
    return res1;
}

Node *newTupleRes(int arity, Res **comps, Region *r) {
	Res *res1 = newRes(r);
	res1->nodeType = N_TUPLE;
	res1->subtrees = comps;
	res1->degree = arity;
	ExprType **compTypes = (ExprType **)region_alloc(r, sizeof(ExprType *) * arity);
	int i;
	for(i=0;i<arity;i++) {
		compTypes[i] = comps[i]->exprType;
	}
	res1->exprType = newTupleType(arity, compTypes, r);
	return res1;
}
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
        res1->coercionType = NULL;
        res1->nodeType = N_VAL;
        res1->iotype = IO_TYPE_INPUT;
	return res1;
}
Res* newUninterpretedRes(Region *r, char *typeName, void *ioStruct, bytesBuf_t *ioBuf) {
	Res *res1 = newRes(r);
        res1->exprType = newIRODSType(typeName, r);
        res1->value.uninterpreted.inOutStruct = ioStruct;
        res1->value.uninterpreted.inOutBuffer = ioBuf;
	return res1;
}
Res* newIntRes(Region *r, int n) {
	Res *res1 = newRes(r);
        res1->exprType = newSimpType(T_INT,r);
        res1->value.dval = n;
	return res1;
}
Res* newDoubleRes(Region *r, double a) {
	Res *res1 = newRes(r);
        res1->exprType = newSimpType(T_DOUBLE,r);
        res1->value.dval = a;
	return res1;
}
Res* newBoolRes(Region *r, int n) {
	Res *res1 = newRes(r);
        res1->exprType = newSimpType(T_BOOL,r);
        res1->value.dval = n;
	return res1;
}
Res* newStringRes(Region *r, char *s) {
	Res *res1 = newRes(r);
        res1->exprType = newSimpType(T_STRING,r);
        res1->value.strlen = strlen(s);
        int size = (res1->value.strlen+1)*sizeof(char);
        res1->text = (char *)region_alloc(r, size);
        memcpy(res1->text, s, size);
	return res1;
}
Res* newUnspecifiedRes(Region *r) {
	Res *res1 = newRes(r);
        res1->exprType = newSimpType(T_UNSPECED,r);
        res1->text = cpStringExt("", r);
	return res1;
}
Res* newDatetimeRes(Region *r, long dt) {
	Res *res1 = newRes(r);
        res1->exprType = newSimpType(T_DATETIME,r);
        res1->value.tval = dt;
	return res1;
}
Res* newErrorRes(Region *r, int errcode) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->nodeType = N_ERROR;
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

/* copy from old region to new region
 * If the new region is the same as the old region then do not copy.
 */
Res *cpRes2(Res *res0, Region *oldr, Region *r) {
    Res *res;
    if(IN_REGION(res0, oldr)) {
        res = newRes(r);
        *res = *res0;
    } else {
        res = res0;
    }
    if(res->exprType!=NULL) {
        res->exprType = cpType2(res->exprType, oldr, r);
    }
    if(res->text != NULL) {
        res->text = cpString2(res->text, oldr, r);
    }
    int i;
    if(res->subtrees!=NULL) {
        if(IN_REGION(res->subtrees, oldr)) {
            Node **temp = (Node **)region_alloc(r, sizeof(Node *) * res->degree);
            memcpy(temp, res->subtrees, sizeof(Node *) * res->degree);
            res->subtrees = temp;
        }
        for(i=0;i<res->degree;i++) {
            res->subtrees[i] = cpRes2(res->subtrees[i], oldr, r);
        }
    }
    return res;
}

char *cpString2(char *str, Region *oldr, Region *r) {
    if(!IN_REGION(str, oldr)) {
        return str;
    } else
    return cpStringExt(str, r);
}
ExprType *cpType2(ExprType *ty, Region *oldr, Region *r) {
    if(!IN_REGION(ty, oldr)) {
        return ty;
    }
    int i;
    ExprType *newt;
    newt = (ExprType *) region_alloc(r, sizeof(ExprType));
    memcpy(newt, ty, sizeof(ExprType));
    if(ty->subtrees != NULL) {
        newt->subtrees = (ExprType **) region_alloc(r,sizeof(ExprType *)*ty->degree);
        for(i=0;i<ty->degree;i++) {
            newt->subtrees[i] = cpType2(ty->subtrees[i],oldr, r);
        }
    }
    if(ty->text != NULL) {
        newt->text = cpString2(ty->text, oldr, r);
    }

    return newt;
}
/* copy res values from region oldr to r */
void cpHashtable2(Hashtable *env, Region *oldr, Region *r) {
	int i;

	for(i=0;i<env->size;i++) {
            struct bucket *b = env->buckets[i];
            while(b!=NULL) {
                b->value = cpRes2((Res *)b->value, oldr, r);
                b= b->next;
            }
	}
}

void cpEnv2(Env *env, Region *oldr, Region *r) {
    cpHashtable2(env->current, oldr, r);
    if(env->previous!=NULL) {
        cpEnv2(env->previous, oldr, r);
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
    if(type->vararg!=ONCE) {
    	snprintf(buf+strlen(buf), bufsize-strlen(buf), "vararg ");
    }
        ExprType *etype = type;
        if(etype->nodeType == T_VAR && var_types != NULL) {
            /* dereference */
            etype = dereference(etype, var_types, r);
        }

        if(etype->nodeType == T_VAR) {
        	snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", etype == NULL?"?":typeName_ExprType(etype));
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
        	if(strcmp(etype->text, FUNC) == 0) {
        		snprintf(buf+strlen(buf), bufsize-strlen(buf), "(");
				typeToString(T_CONS_TYPE_ARG(etype, 0), var_types, buf+strlen(buf), bufsize-strlen(buf));
				snprintf(buf+strlen(buf), bufsize-strlen(buf), ")");
        		snprintf(buf+strlen(buf), bufsize-strlen(buf), "->");
        		typeToString(T_CONS_TYPE_ARG(etype, 1), var_types, buf+strlen(buf), bufsize-strlen(buf));
			} else {

        	snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", T_CONS_TYPE_NAME(etype));
            int i;
            if(T_CONS_ARITY(etype) != 0) {
				snprintf(buf+strlen(buf), bufsize-strlen(buf), "(");
				for(i=0;i<T_CONS_ARITY(etype);i++) {
					if(i!=0) {
						snprintf(buf+strlen(buf), bufsize-strlen(buf), ", ");
					}
					typeToString(T_CONS_TYPE_ARG(etype, i), var_types, buf+strlen(buf), bufsize-strlen(buf));
				}
				snprintf(buf+strlen(buf), bufsize-strlen(buf), ")");
            }
			}
        } else if(etype->nodeType == T_FLEX) {
        	snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", etype == NULL?"?":typeName_ExprType(etype));
            typeToString(etype->subtrees[0], var_types, buf+strlen(buf), bufsize-strlen(buf));
        } else if(etype->nodeType == T_TUPLE) {
        	if(T_CONS_ARITY(etype) == 0) {
        		snprintf(buf+strlen(buf), bufsize-strlen(buf), "unit");
        	} else {
        		if(T_CONS_ARITY(etype) == 1) {
            		snprintf(buf+strlen(buf), bufsize-strlen(buf), "(");
        		}
				int i;
				for(i=0;i<T_CONS_ARITY(etype);i++) {
					if(i!=0) {
						snprintf(buf+strlen(buf), bufsize-strlen(buf), " * ");
					}
					typeToString(T_CONS_TYPE_ARG(etype, i), var_types, buf+strlen(buf), bufsize-strlen(buf));
				}
        		if(T_CONS_ARITY(etype) == 1) {
            		snprintf(buf+strlen(buf), bufsize-strlen(buf), ")");
        		}
        	}
        } else {
        	snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", etype == NULL?"?":typeName_ExprType(etype));
        }


    region_free(r);
    return buf;

}
void typingConstraintsToString(List *typingConstraints, Hashtable *var_types, char *buf, int bufsize) {
    char buf2[1024];
    char buf3[1024];
    ListNode *p = typingConstraints->head;
    buf[0] = '\0';
    while(p!=NULL) {
        snprintf(buf + strlen(buf), bufsize-strlen(buf), "%s<%s\n",
                typeToString(TC_A((TypingConstraint *)p->value), NULL, /*var_types,*/ buf2, 1024),
                typeToString(TC_B((TypingConstraint *)p->value), NULL, /*var_types,*/ buf3, 1024));
        p=p->next;
    }
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

ExprType *instantiate(ExprType *type, Hashtable *type_table, int replaceFreeVars, Region *r) {
    ExprType **paramTypes;
    int i;
    ExprType *typeInst;
    int changed = 0;

    switch(type->nodeType) {
        case T_VAR:
            typeInst = dereference(type, type_table, r);
            if(typeInst == type) {
                return replaceFreeVars?newSimpType(T_UNSPECED, r): type;
            } else {
                return instantiate(typeInst, type_table, replaceFreeVars, r);
            }
        default:
            if(type->degree != 0) {
                paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*type->degree);
                for(i=0;i<type->degree;i++) {
                    paramTypes[i] = instantiate(type->subtrees[i], type_table, replaceFreeVars, r);
                    if(paramTypes[i]!=type->subtrees[i]) {
                        changed = 1;
                    }
                }
            }
            if(changed) {
                ExprType *inst = (ExprType *) region_alloc(r, sizeof(ExprType));
                *inst = *type;
                inst->subtrees = paramTypes;
                return inst;
            } else {
                return type;
            }

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

void printEnvToStdOut(Env *env) {
	Env *e = env;
    char buffer[1024];
    while(e!=NULL) {
    	if(e!=env)
    		printf("%s\n===========\n", buffer);
    	printHashtable(e->current, buffer);
    	e = e->previous;
    }
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

Env *newEnv(Hashtable *current, Env *previous, Env *callerEnv) {
    Env *env = (Env *)malloc(sizeof(Env));
    env->current = current;
    env->previous = previous;
    env->lower = callerEnv;
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
        if(env->previous!=NULL) {
            deleteEnv(env->previous, deleteCurrent);

        }
    }
    free(env);
}

Env* globalEnv(Env *env) {
        Env *global = env;
        while(global->previous!=NULL) {
            global = global->previous;
        }
        return global;
}

List *newList(Region *r) {
    List *l = (List *)region_alloc(r, sizeof (List));
    l->head = l->tail = NULL;
    return l;
}

ListNode *newListNodeNoRegion(void *value) {
    ListNode *l = (ListNode *)malloc(sizeof (ListNode));
    l->next = NULL;
    l->value = value;
    return l;
}
ListNode *newListNode(void *value, Region *r) {
    ListNode *l = (ListNode *)region_alloc(r, sizeof (ListNode));
    l->next = NULL;
    l->value = value;
    return l;
}

void listAppendNoRegion(List *list, void *value) {
    ListNode *ln = newListNodeNoRegion(value);
    if(list->head != NULL) {
        list->tail = list->tail->next = ln;
    } else {
        list->head = list->tail = ln;
    }
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
void listRemoveNoRegion(List *list, ListNode *node) {
    ListNode *prev = NULL, *curr = list->head;
    while(curr != NULL) {
        if(curr == node) {
            if(prev == NULL) {
                list->head = node->next;
            } else {
                prev->next = node->next;
            }
            free(node);
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    if(list->tail == node) {
        list->tail = prev;
    }

}

TypingConstraint *newTypingConstraint(ExprType *a, ExprType *b, NodeType type, Node *node, Region *r) {
    TypingConstraint *tc = (TypingConstraint *)region_alloc(r, sizeof (TypingConstraint));
    tc->subtrees = (Node **)region_alloc(r, sizeof(Node *)*4);
    TC_A(tc) = a;
    TC_B(tc) = b;
    tc->nodeType = type;
    TC_NODE(tc) = node;
    TC_NEXT(tc) = NULL;
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

void logErrMsg(rError_t *errmsg, rError_t *system) {
    char errbuf[ERR_MSG_LEN*1024];
    errMsgToString(errmsg, errbuf, ERR_MSG_LEN*1024);
#ifdef DEBUG
    writeToTmp("err.log", "begin errlog\n");
    writeToTmp("err.log", errbuf);
    writeToTmp("err.log", "end errlog\n");
#endif
    if(system!=NULL) {
    	rodsLogAndErrorMsg(LOG_ERROR, system,-1, "%s", errbuf);
    } else {
    	rodsLog (LOG_ERROR, "%s", errbuf);
    }
}

char *errMsgToString(rError_t *errmsg, char *errbuf, int buflen /* = 0 */) {
    errbuf[0] = '\0';
    int p = 0;
    int i;
    int first = 1;
    int restart = 0;
    for(i=errmsg->len-1;i>=0;i--) {
    	if(strcmp(errmsg->errMsg[i]->msg, ERR_MSG_SEP) == 0) {
    		if(first || restart)
    			continue;
    		else {
    			restart = 1;
    			continue;
    		}
    	}
    	if(restart) {
    		snprintf(errbuf+p, buflen-p, "%s\n", ERR_MSG_SEP);
    		p += strlen(errbuf+p);
    	}
        if(!first && !restart) {
            snprintf(errbuf+p, buflen-p, "caused by: %s\n", errmsg->errMsg[i]->msg);
        } else {
            snprintf(errbuf+p, buflen-p, "%s\n", errmsg->errMsg[i]->msg);
            first = 0;
    		restart = 0;

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

    if(pattern->nodeType == N_APPLICATION || pattern->nodeType == N_TUPLE) {
        int i;
        for(i=0;i<pattern->degree;i++) {
            if(!isPattern(pattern->subtrees[i]))
                return 0;
        }
        return 1;
    } else if(pattern->nodeType == TK_TEXT || pattern->nodeType == TK_VAR) {
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
Node *lookupAVUFromMetadata(Node *metadata, char *a) {
	int i;
	for(i=0;i<metadata->degree;i++) {
		if(strcmp(metadata->subtrees[i]->subtrees[0]->text, a) == 0) {
			return metadata->subtrees[i];
		}
	}
	return NULL;

}
