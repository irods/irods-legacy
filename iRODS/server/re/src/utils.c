/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "utils.h"
#include "conversion.h"
/* make a new type by substituting tvars with fresh tvars */
ExprType *dupType(ExprType *ty, Region *r) {
    Hashtable *varTable = newHashTable(100);
    /* todo add oom handler here */
    Region *keyRegion = make_region(0, NULL);
    ExprType *dup = dupTypeAux(ty, r, varTable, keyRegion);
    deleteHashTable(varTable, nop);
    region_free(keyRegion);
    return dup;


}

int typeEqSyntatic(ExprType *a, ExprType *b) {
    if(a->t!=b->t) {
        return 0;
    }
    switch(a->t) {
        case T_FUNC:
            if(a->ext.func.arity == b->ext.func.arity &&
                    a->ext.func.vararg == b->ext.func.vararg) {
                int i;
                for(i=0;i<a->ext.func.arity;i++) {
                    if(!typeEqSyntatic(a->ext.func.paramsType[i],b->ext.func.paramsType[i])) {
                        return 0;
                    }
                }
                if(typeEqSyntatic(a->ext.func.retType,b->ext.func.retType)) {
                    return 1;
                }else {
                    return 0;
                }
            }
            return 0;
        case T_CONS:
            if(a->ext.cons.arity == b->ext.cons.arity &&
                    strcmp(a->ext.cons.typeConsName, b->ext.cons.typeConsName) == 0) {
                int i;
                for(i=0;i<a->ext.cons.arity;i++) {
                    if(!typeEqSyntatic(a->ext.cons.typeArgs[i],b->ext.cons.typeArgs[i])) {
                        return 0;
                    }
                }
                return 1;
            }
            return 0;
        case T_VAR:
            return a->ext.tvar.vid == b->ext.tvar.vid;
        case T_IRODS:
            return strcmp(a->ext.irods.name, b->ext.irods.name) == 0;
        default:
            return 1;
    }

}

ExprType *dupTypeAux(ExprType *ty, Region *r, Hashtable *varTable, Region *keyRegion) {
    ExprType **paramTypes;
    int i;
    ExprType *newt;
    char *name;
    char buf[128];
    switch(ty->t) {
        case T_FUNC:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*ty->ext.func.arity);
            for(i=0;i<ty->ext.func.arity;i++) {
                paramTypes[i] = dupTypeAux(ty->ext.func.paramsType[i],r,varTable,keyRegion);
            }
            newt = newFuncTypeVarArg(ty->ext.func.arity, ty->ext.func.vararg, paramTypes, dupTypeAux(ty->ext.func.retType,r, varTable, keyRegion), r);
            newt->coercionAllowed = ty->coercionAllowed;
            return newt;
        case T_CONS:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*ty->ext.cons.arity);
            for(i=0;i<ty->ext.cons.arity;i++) {
                paramTypes[i] = dupTypeAux(ty->ext.cons.typeArgs[i],r,varTable,keyRegion);
            }
            newt = newConsType(ty->ext.cons.arity, ty->ext.cons.typeConsName, paramTypes, r);
            newt->coercionAllowed = ty->coercionAllowed;
            return newt;
        case T_VAR:
            name = getTVarName(ty->ext.tvar.vid, buf);
            ExprType *exist = lookupFromHashTable(varTable, name);
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
            return (a->type!=T_CONS && a->type == b->type) ||
                   (b->type == T_DOUBLE && a->type == T_INT) ||
                   (b->type == T_DOUBLE && a->type == T_STRING) ||
                   (b->type == T_INT && a->type == T_DOUBLE) ||
                   (b->type == T_INT && a->type == T_STRING) ||
                   (b->type == T_STRING && a->type == T_INT) ||
                   (b->type == T_STRING && a->type == T_DOUBLE) ||
                   (b->type == T_STRING && a->type == T_BOOL) ||
                   (b->type == T_BOOL && a->type == T_STRING) ||
                   (b->type == T_DATETIME && a->type == T_INT) ||
                   (b->type == T_DATETIME && a->type == T_DOUBLE) ||
                   (b->type == T_DYNAMIC) ||
                   (a->type == T_DYNAMIC) ||
                   (a->type==T_CONS && b->type==T_CONS && coercible(T_CONS_TYPE_ARG(a, 0), T_CONS_TYPE_ARG(b, 0)));
}
/*
 * unify a free tvar or union type with some other type
 */
ExprType* unifyTVarL(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r) {
    char buf[128];
    if(type->ext.tvar.numDisjuncts==0) { /* free */
        insertIntoHashTable(varTypes, getTVarName(type->ext.tvar.vid, buf), expected);
        return dereference(expected, varTypes, r);
    } else { /* union type */
        int i;
        ExprType *ty = NULL;
        for(i=0;i<type->ext.tvar.numDisjuncts;i++) {
                if(T_VAR_DISJUNCT(type,i) == expected->type) { /* union types can only include primitive types */
                    ty = expected;
                    break;
                }
        }
        if(ty != NULL) {
            insertIntoHashTable(varTypes, getTVarName(type->ext.tvar.vid, buf), expected);
        }
        return ty;
    }

}
ExprType* unifyTVarR(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r) {
    char buf[128];
    if(expected->ext.tvar.numDisjuncts==0) { /* free */
        insertIntoHashTable(varTypes, getTVarName(expected->ext.tvar.vid, buf), type);
        return dereference(expected, varTypes, r);
    } else { /* union type */
        int i;
        ExprType *ty = NULL;
        for(i=0;i<expected->ext.tvar.numDisjuncts;i++) {
            if(type->type == T_VAR_DISJUNCT(expected,i)) { /* union types can only include primitive types */
                ty = type;
            }
        }
        if(ty != NULL) {
            insertIntoHashTable(varTypes, getTVarName(expected->ext.tvar.vid, buf), ty);
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
    if(type->type == T_VAR && expected->type == T_VAR) {
        if(type->ext.tvar.vid == expected->ext.tvar.vid) {
            /* if both dereference to the same tvar then do not modify var types table */
            return type;
        } else if(type->ext.tvar.numDisjuncts > 0 && expected->ext.tvar.numDisjuncts > 0) {
            NodeType c[10];
            NodeType* cp = c;
            int i,k;
            for(k=0;k<expected->ext.tvar.numDisjuncts;k++) {
                for(i=0;i<type->ext.tvar.numDisjuncts;i++) {
                    if(T_VAR_DISJUNCT(type,i) == T_VAR_DISJUNCT(expected,k)) {
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
                    gcd = newSimpType(*c, r);
                } else {
                    gcd = newTVar2(cp-c, c, r);
                }
                updateInHashTable(varTypes, getTVarName(type->ext.tvar.vid, buf), gcd);
                updateInHashTable(varTypes, getTVarName(expected->ext.tvar.vid, buf), gcd);
                return gcd;
            }
        } else {
            if(type->ext.tvar.numDisjuncts==0) { /* free */
                insertIntoHashTable(varTypes, getTVarName(type->ext.tvar.vid, buf), expected);
                return dereference(expected, varTypes, r);
            } else if(expected->ext.tvar.numDisjuncts==0) { /* free */
                insertIntoHashTable(varTypes, getTVarName(expected->ext.tvar.vid, buf), type);
                return dereference(expected, varTypes, r);
            } else {
                /* error unreachable */
                return NULL;
            }
        }
    } else
	if(type->type == T_VAR) {
            return unifyTVarL(type, expected, varTypes, r);
	} else if(expected->type == T_VAR) {
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
	if(type->type == T_CONS || expected->type == T_CONS) {
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
        } else if(type->type == T_IRODS || expected->type == T_IRODS) {
                if(strcmp(type->text, expected->text)!=0) {
                        return NULL;
                }
                return expected;
	} else if(expected->type == type->type) { /* primitive types */
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
    return getTVarNameRegion(tvar->ext.tvar.vid, r);
}

/**
 * create a new node n
 */
Node *newNode(NodeType type, char* text, Label * eloc, Region *r) {
	Node *node = (Node *)region_alloc(r, sizeof(Node));
	if(node == NULL)
		return NULL;
	node->type=type;
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
        node->coercion = NULL;
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
    t->coercion = NULL;
    t->exprType = NULL;
    t->subtrees = subtrees;
    t->degree = degree;
    t->type = type;
    t->expr = 0;
    t->text = NULL;
    t->typed = 1;
    return t;

}


ExprType *newTVar2(int numDisjuncts, NodeType disjuncts[], Region *r) {
    ExprType *t = newExprType(T_VAR, 0, NULL, r);
    t->ext.tvar.vid = tvarNumber ++;
    t->ext.tvar.numDisjuncts = numDisjuncts;
    t->ext.tvar.disjuncts = numDisjuncts == 0? NULL : (NodeType *)region_alloc(r, sizeof(NodeType) * numDisjuncts);
    if(numDisjuncts!=0) {
        memcpy(t->ext.tvar.disjuncts, disjuncts, numDisjuncts*sizeof(NodeType));
    }
    return t;
}

ExprType *newTVar(Region *r) {
    ExprType *t = newExprType(T_VAR, 0, NULL, r);
    t->ext.tvar.vid = tvarNumber ++;
    t->ext.tvar.numDisjuncts = 0;
    t->ext.tvar.disjuncts = NULL;
    return t;
}

ExprType *newSimpType(NodeType type, Region *r) {
    return newExprType(type, 0, NULL, r);
}
ExprType *newFuncType(int arity, ExprType **paramTypes, ExprType* retType, Region *r) {
    return newFuncTypeVarArg(2, ONCE, paramTypes, retType, r);
}
ExprType *newFuncTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, ExprType* retType, Region *r) {
    ExprType **typeArgs = (ExprType **)region_alloc(r, sizeof(ExprType *) * 2);
    typeArgs[0] = newConsTypeVarArg(arity, vararg, TUPLE, paramTypes, r);
    typeArgs[1] = retType;
    return newConsType(2, FUNC, typeArgs, r);
}
ExprType *newConsType(int arity, char *cons, ExprType **paramTypes, Region *r) {
        return newConsTypeVarArg(arity, ONCE, cons, paramTypes, r);
}
ExprType *newConsTypeVarArg(int arity, enum vararg vararg, char *cons, ExprType **paramTypes, Region *r) {
	ExprType *t = newExprType(T_CONS, arity, paramTypes, r);
        t->ext.cons.vararg = vararg;
        T_CONS_TYPE_NAME(t) = (char *)region_alloc(r,(strlen(cons)+1) * sizeof(char));
        strcpy(T_CONS_TYPE_NAME(t), cons);
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
        return newConsType(1, LIST, typeArgs, r);
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
        res1->value.d = n;
	return res1;
}
Res* newDoubleRes(Region *r, double a) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_DOUBLE,r);
        res1->value.d = a;
	return res1;
}
Res* newBoolRes(Region *r, int n) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_BOOL,r);
        res1->value.d = n;
	return res1;
}
/* precond: len(s) < size of res1->value.s */
Res* newStringRes(Region *r, char *s) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_STRING,r);
        res1->value.s.len = strlen(s);
        int size = (res1->value.s.len+1)*sizeof(char);
        res1->text = (char *)region_alloc(r, size);
        memcpy(res1->text, s, size);
	return res1;
}
Res* newDatetimeRes(Region *r, long dt) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_DATETIME,r);
        res1->value.t = dt;
	return res1;
}
Res* newErrorRes(Region *r, int errcode) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->exprType = newSimpType(T_ERROR,r);
        res1->value.e = errcode;
	return res1;
}

/* copy to new region
 * If the new region is the same as the old region then do not copy.
 */
Res *cpRes(Res *res, Region *r) {
    if(TYPE(res) == T_STRING) {
        if(IN_REGION(res, r)) {
            if(IN_REGION(res->text, r)) {
                res->exprType = cpType(res->exprType, r);
                return res;
            } else {
                res->text = cpString(res->text, r);
                res->exprType = cpType(res->exprType, r);
                return res;
            }
        } else {
            if(IN_REGION(res->text, r)) {
                Res *res0 = newRes(r);
                res0->value.s = res->value.s;
                res0->exprType = cpType(res->exprType, r);
                return res0;
            }
                Res *res0 = newRes(r);
                convertStrValue(res0, res->text, r);
                res0->exprType = cpType(res->exprType, r);
                return res0;
        }

    } else if (TYPE(res) == T_CONS) {
        if(IN_REGION(res, r)) {
            /* the collection is in region r, we need to inspect every element to see if they need to be copied */
            int i;
            for(i=0;i<res->degree;i++) {
                if(IN_REGION(res->subtrees[i], r)) {
                    /* the element is in region r */
                    res->subtrees[i] = cpRes(res->subtrees[i], r);
                    /* the assignment should not change the value */
                } else {
                    res->subtrees[i] = cpRes(res->subtrees[i], r);
                    /* the assignment should change the value */
                }
            }
            res->exprType = cpType(res->exprType, r);
            return res;

        } else {
            /* the collection is not in region r */
            Res *collRes = newCollRes2(res->degree, r);

            int i;
            for(i=0;i<res->degree;i++) {
                if(IN_REGION(res->subtrees[i], r)) {
                    /* the element is in region r */
                    collRes->subtrees[i] = cpRes(res->subtrees[i], r);
                } else {
                    collRes->subtrees[i] = cpRes(res->subtrees[i], r);
                }
            }
            collRes->exprType = cpType(res->exprType, r);
            return collRes;
        }
    } else {
        if(IN_REGION(res, r)) {
            res->exprType = cpType(res->exprType, r);
            return res;
        }
        Res *resCp = newRes(r);
        *resCp = *res;
        resCp->exprType = cpType(res->exprType, r);
        return resCp;
    }
}

char *cpString(char *str, Region *r) {
    char *strCp = region_alloc(r, (strlen(str)+1) * sizeof(char) );
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

    switch(ty->type) {
        case T_VAR:
            T_VAR_DISJUNCTS(newt) = (NodeType *) region_alloc(r, sizeof(NodeType)*T_VAR_NUM_DISJUNCTS(ty));
            memcpy(T_VAR_DISJUNCTS(newt), T_VAR_DISJUNCTS(ty), sizeof(NodeType) * T_VAR_NUM_DISJUNCTS(ty));
            break;
        default:
            break;
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
    cpHashtable(env->global, r);
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
        if(lookupFromHashTable(env->current, varName)==NULL) {
        if(lookupFromHashTable(env->global, varName)==NULL) {
            /* new variable */
            if(insertIntoHashTable(env->current, varName, val) == 0) {
                snprintf(errbuf, ERR_MSG_LEN, "error: unable to write to local variable \"%s\".",varName);
                addRErrorMsg(errmsg, UNSUPPORTED_SESSION_VAR, errbuf);
                return newErrorRes(r, UNABLE_TO_WRITE_LOCAL_VAR);
            }
        }else {
                updateInHashTable(env->global, varName, val);

        }
        } else {
                updateInHashTable(env->current, varName, val);
        }
        return newIntRes(r, 0);
    }
    return newIntRes(r, 0);
}

void errorInsert(char *errmsg, char *insert) {
    			char buffer[MAX_ERRMSG_LEN+1];
			strncpy(buffer, insert, MAX_ERRMSG_LEN);
			strncat(buffer, errmsg, MAX_ERRMSG_LEN);
			strncpy(errmsg, buffer, MAX_ERRMSG_LEN);

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
        if(etype->type == T_VAR && var_types != NULL) {
            /* dereference */
            etype = dereference(etype, var_types, r);
        }
        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", etype == NULL?"?":typeName_ExprType(etype));
        if(etype->type == T_VAR) {
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "%d", etype->ext.tvar.vid);
            if(T_VAR_NUM_DISJUNCTS(type)!=0) {
                snprintf(buf+strlen(buf), bufsize-strlen(buf), "{");
                int i;
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", typeName_NodeType(T_VAR_DISJUNCT(type, i)));
                }
                snprintf(buf+strlen(buf), bufsize-strlen(buf), "}");
            }
        } else if(etype->type == T_CONS) {
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
    if(type->type == T_VAR) {
        char name[128];
        getTVarName(type->ext.tvar.vid, name);
        /* printf("deref: %s\n", name); */
        ExprType *deref = lookupFromHashTable(type_table, name);
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

    switch(type->type) {
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
			if(t->t==T_VAR && t->ext.tvar.vid == a) {
				bucket -> value = b;
			}
			bucket = bucket->next;
		}
	}
}
*/
/**
 * replace type var a with type b in a cons type
 */
/*
void replaceCons(ExprType *consType, int a, ExprType *b) {
	int i;
	for(i=0;i<consType->ext.constructed.arity;i++) {
		if(consType->ext.constructed.elemType[i]->t == T_CONS) {
			replaceCons(consType->ext.constructed.elemType[i], a, b);
		} else if(consType->ext.constructed.elemType[i]->t == T_VAR
			&& consType->ext.constructed.elemType[i]->ext.tvar.vid == a) {
				consType->ext.constructed.elemType[i] = b;
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

Env *newEnv(Hashtable *current, Hashtable *gloabl, Hashtable *funcDesc) {
    Env *env = (Env *)malloc(sizeof(Env));
    env->current = current;
    env->global = gloabl;
    env->funcDesc = funcDesc;
    return env;
}

void deleteEnv(Env *env, int deleteCurrent) {
    if(deleteCurrent>=1) {
        deleteHashTable(env->current, nop);
    }
    if(deleteCurrent>=2) {
        deleteHashTable(env->global, nop);
    }
    if(deleteCurrent>=3) {
        deleteHashTable(env->funcDesc, nop);
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
