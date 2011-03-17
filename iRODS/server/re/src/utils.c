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
            newt = newFuncTypeVarArg(ty->ext.func.arity, ty->ext.func.vararg, paramTypes, dupType(ty->ext.func.retType,r), r);
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
                newt = newTVar(r);
                newt->coercionAllowed = ty->coercionAllowed;
                int i;
                for(i=0;i<T_VAR_NUM_DISJUNCTS(ty);i++) {
                    T_VAR_DISJUNCT(newt, i) = T_VAR_DISJUNCT(ty, i);
                }
                T_VAR_NUM_DISJUNCTS(newt) = T_VAR_NUM_DISJUNCTS(ty);
                insertIntoHashTable(varTable, name, newt);
                return newt;
            }
        default:
            return ty;
    }

}
int coercible(ExprType *a, ExprType *b) {
            return (a->t!=T_CONS && a->t == b->t) ||
                   (b->t == T_DOUBLE && a->t == T_INT) ||
                   (b->t == T_DOUBLE && a->t == T_STRING) ||
                   (b->t == T_INT && a->t == T_DOUBLE) ||
                   (b->t == T_INT && a->t == T_STRING) ||
                   (b->t == T_STRING && a->t == T_INT) ||
                   (b->t == T_STRING && a->t == T_DOUBLE) ||
                   (b->t == T_STRING && a->t == T_BOOL) ||
                   (b->t == T_BOOL && a->t == T_STRING) ||
                   (b->t == T_DATETIME && a->t == T_INT) ||
                   (b->t == T_DATETIME && a->t == T_DOUBLE) ||
                   (b->t == T_DYNAMIC) ||
                   (a->t == T_DYNAMIC) ||
                   (a->t==T_CONS && b->t==T_CONS && coercible(T_CONS_TYPE_ARG(a, 0), T_CONS_TYPE_ARG(b, 0)));
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
                if(T_VAR_DISJUNCT(type,i) == expected->t) { /* union types can only include primitive types */
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
            if(type->t == T_VAR_DISJUNCT(expected,i)) { /* union types can only include primitive types */
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
    if(type->t == T_VAR && expected->t == T_VAR) {
        if(type->ext.tvar.vid == expected->ext.tvar.vid) {
            /* if both dereference to the same tvar then do not modify var types table */
            return type;
        } else if(type->ext.tvar.numDisjuncts > 0 && expected->ext.tvar.numDisjuncts > 0) {
            TypeConstructor c[10];
            TypeConstructor* cp = c;
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
                    gcd = newTVar(r);
                    T_VAR_NUM_DISJUNCTS(gcd) = cp-c;
                    memcpy(gcd->ext.tvar.disjuncts, c, (cp-c)*sizeof(TypeConstructor));
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
	if(type->t == T_VAR) {
            return unifyTVarL(type, expected, varTypes, r);
	} else if(expected->t == T_VAR) {
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
	if(type->t == T_CONS || expected->t == T_CONS) {
		if(
			strcmp(type->ext.cons.typeConsName, expected->ext.cons.typeConsName) == 0
			&& type->ext.cons.arity == expected->ext.cons.arity) {
			int i;
			for(i=0;i<type->ext.cons.arity;i++) {
				ExprType *elemType = unifyWith(
                                        type->ext.cons.typeArgs[i],
                                        expected->ext.cons.typeArgs[i],
                                        varTypes,r); /* unifyWithCoercion performs dereference */
				if(elemType == NULL) {
					return NULL;
				}
				expected->ext.cons.typeArgs[i] = elemType;
			}
			return dereference(expected, varTypes, r);
		} else {
			return NULL;
		}
        } else if(type->t == T_FUNC || expected->t == T_FUNC) {
		if(type->ext.func.arity == expected->ext.func.arity) {
			int i;
			for(i=0;i<type->ext.func.arity;i++) {
				ExprType *elemType = unifyWith(
                                        type->ext.func.paramsType[i],
                                        expected->ext.func.paramsType[i],
                                        varTypes,r);
				if(elemType == NULL) {
					return NULL;
				}
				expected->ext.func.paramsType[i] = elemType;
			}
                        ExprType *elemType = unifyWith(
                                type->ext.func.retType,
                                expected->ext.func.retType,
                                varTypes,r);
                        if(elemType == NULL) {
                                return NULL;
                        }
                        expected->ext.func.retType = elemType;
			return dereference(expected, varTypes, r);
		} else {
			return NULL;
		}
        } else if(type->t == T_IRODS || expected->t == T_IRODS) {
                if(strcmp(
                        type->ext.irods.name,
                        expected->ext.irods.name)!=0) {
                        return NULL;
                }
                return expected;
	} else if(expected->t == type->t) { /* primitive types */
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

ExprType *newTVar(Region *r) {
	ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
    t->coercionAllowed = 0;
    t->t=T_VAR;
    t->ext.tvar.vid = tvarNumber ++;
    t->ext.tvar.numDisjuncts = 0;
    /*char name[128]; */
    /*getTVarName(t->ext.tvar.vid, name); */
    return t;
}
ExprType *newSimpType(TypeConstructor type, Region *r) {
	ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
        t->t=type;
        t->coercionAllowed = 0;
        return t;
}
ExprType *newFuncType(int arity, ExprType **paramTypes, ExprType* retType, Region *r) {
	ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
        t->coercionAllowed = 0;
        t->t=T_FUNC;
        t->ext.func.vararg = ONCE;
        t->ext.func.arity = arity;
        t->ext.func.paramsType = paramTypes;
        t->ext.func.retType = retType;
        return t;
}
ExprType *newFuncTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, ExprType* retType, Region *r) {
	ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
        t->coercionAllowed = 0;
        t->t=T_FUNC;
        t->ext.func.vararg = vararg;
        t->ext.func.arity = arity;
        t->ext.func.paramsType = paramTypes;
        t->ext.func.retType = retType;
        return t;
}
/* precond: len(cons) should be smaller than the size of cons in t */
ExprType *newConsType(int arity, char *cons, ExprType **paramTypes, Region *r) {
	ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
        t->coercionAllowed = 0;
        t->t=T_CONS;
        t->ext.cons.arity = arity;
        t->ext.cons.typeArgs = paramTypes;
        strcpy(t->ext.cons.typeConsName, cons);
        return t;
}
/* precond: len(name) should be smaller than the size of t->ext.irods.name */
ExprType *newIRODSType(char *name, Region *r) {
        ExprType *t = (ExprType *)region_alloc(r,sizeof(ExprType));
        t->t = T_IRODS;
        t->coercionAllowed = 0;
        strcpy(t->ext.irods.name, name);
        return t;
}
ExprType *newCollType(ExprType *elemType, Region *r) {
        ExprType **typeArgs = (ExprType**) region_alloc(r, sizeof(ExprType*));
        typeArgs[0] = elemType;
        return newConsType(1, cpString(LIST, r), typeArgs, r);
}

/** Res functions */

Res* newCollRes(int size, ExprType *elemType, Region *r) {
	Res *res1 = newRes(r);
        res1->type = newCollType(elemType, r);
        res1->value.c.len = size;
        res1->value.c.elems = (Res **)region_alloc(r, sizeof(Res *)*size);
	return res1;
}
/* used in cpRes only */
Res* newCollRes2(int size, Region *r) {
	Res *res1 = newRes(r);
        res1->type = NULL;
        res1->value.c.len = size;
        res1->value.c.elems = (Res **)region_alloc(r, sizeof(Res *)*size);
	return res1;
}
Res* newRes(Region *r) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = NULL;
	return res1;
}
Res* newUninterpretedRes(Region *r, char *typeName, void *ioStruct, bytesBuf_t *ioBuf) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newIRODSType(typeName, r);
        res1->value.uninterpreted.inOutStruct = ioStruct;
        res1->value.uninterpreted.inOutBuffer = ioBuf;
	return res1;
}
Res* newIntRes(Region *r, int n) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newSimpType(T_INT,r);
        res1->value.d = n;
	return res1;
}
Res* newDoubleRes(Region *r, double a) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newSimpType(T_DOUBLE,r);
        res1->value.d = a;
	return res1;
}
Res* newBoolRes(Region *r, int n) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newSimpType(T_BOOL,r);
        res1->value.d = n;
	return res1;
}
/* precond: len(s) < size of res1->value.s */
Res* newStringRes(Region *r, char *s) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newSimpType(T_STRING,r);
        res1->value.s.len = strlen(s);
        int size = (res1->value.s.len+1)*sizeof(char);
        res1->value.s.pointer = (char *)region_alloc(r, size);
        memcpy(res1->value.s.pointer, s, size);
	return res1;
}
Res* newDatetimeRes(Region *r, long dt) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newSimpType(T_DATETIME,r);
        res1->value.t = dt;
	return res1;
}
Res* newErrorRes(Region *r, int errcode) {
	Res *res1 = (Res *) region_alloc(r,sizeof (Res));
        res1->type = newSimpType(T_ERROR,r);
        res1->value.e = errcode;
	return res1;
}

/* copy to new region
 * If the new region is the same as the old region then do not copy.
 */
Res *cpRes(Res *res, Region *r) {
    if(TYPE(res) == T_STRING) {
        if(IN_REGION(res, r)) {
            if(IN_REGION(res->value.s.pointer, r)) {
                res->type = cpType(res->type, r);
                return res;
            } else {
                res->value.s.pointer = cpString(res->value.s.pointer, r);
                res->type = cpType(res->type, r);
                return res;
            }
        } else {
            if(IN_REGION(res->value.s.pointer, r)) {
                Res *res0 = newRes(r);
                res0->value.s = res->value.s;
                res0->type = cpType(res->type, r);
                return res0;
            }
                Res *res0 = newRes(r);
                convertStrValue(res0, res->value.s.pointer, r);
                res0->type = cpType(res->type, r);
                return res0;
        }

    } else if (TYPE(res) == T_CONS) {
        if(IN_REGION(res, r)) {
            /* the collection is in region r, we need to inspect every element to see if they need to be copied */
            int i;
            for(i=0;i<res->value.c.len;i++) {
                if(IN_REGION(res->value.c.elems[i], r)) {
                    /* the element is in region r */
                    res->value.c.elems[i] = cpRes(res->value.c.elems[i], r);
                    /* the assignment should not change the value */
                } else {
                    res->value.c.elems[i] = cpRes(res->value.c.elems[i], r);
                    /* the assignment should change the value */
                }
            }
            res->type = cpType(res->type, r);
            return res;

        } else {
            /* the collection is not in region r */
            Res *collRes = newCollRes2(res->value.c.len, r);

            int i;
            for(i=0;i<res->value.c.len;i++) {
                if(IN_REGION(res->value.c.elems[i], r)) {
                    /* the element is in region r */
                    collRes->value.c.elems[i] = cpRes(res->value.c.elems[i], r);
                } else {
                    collRes->value.c.elems[i] = cpRes(res->value.c.elems[i], r);
                }
            }
            collRes->type = cpType(res->type, r);
            return collRes;
        }
    } else {
        if(IN_REGION(res, r)) {
            res->type = cpType(res->type, r);
            return res;
        }
        Res *resCp = newRes(r);
        *resCp = *res;
        resCp->type = cpType(res->type, r);
        return resCp;
    }
}

char *cpString(char *str, Region *r) {
    char *strCp = region_alloc(r, (strlen(str)+1) * sizeof(char) );
    strcpy(strCp, str);
    return strCp;
}
ExprType *cpType(ExprType *type, Region *r) {
    if(IN_REGION(type, r)) {
        return type;
    }
    ExprType **paramTypes;
    int i;
    ExprType *typeCp;
    switch(type->t) {
        case T_FUNC:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*type->ext.func.arity);
            for(i=0;i<type->ext.func.arity;i++) {
                paramTypes[i] = dupType(type->ext.func.paramsType[i],r);
            }
            return newFuncTypeVarArg(type->ext.func.arity, type->ext.func.vararg, paramTypes, cpType(type->ext.func.retType,r), r);
        case T_CONS:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*type->ext.cons.arity);
            for(i=0;i<type->ext.cons.arity;i++) {
                paramTypes[i] = cpType(type->ext.cons.typeArgs[i],r);
            }
            char *name = cpString(type->ext.cons.typeConsName, r);
            return newConsType(type->ext.cons.arity, name, paramTypes, r);
        case T_VAR:
        default:
            typeCp = newSimpType(type->t, r);
            *typeCp = *type;
            return typeCp;

    }
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
        setVarValue(varMap, rei, strdup(val->value.s.pointer));
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
#define BUFFER(buf, bufsize) buf+strlen(buf), bufsize-strlen(buf)
char* typeToString(ExprType *type, Hashtable *var_types, char *buf, int bufsize) {
    buf[0] = '\0';
    Region *r = make_region(0, NULL);
        ExprType *etype = type;
        if(etype->t == T_VAR && var_types != NULL) {
            /* dereference */
            etype = dereference(etype, var_types, r);
        }
        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", etype == NULL?"?":typeName_ExprType(etype));
        if(etype->t == T_VAR) {
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "%d", etype->ext.tvar.vid);
            if(T_VAR_NUM_DISJUNCTS(type)!=0) {
                snprintf(BUFFER(buf, bufsize), "{");
                int i;
                for(i=0;i<T_VAR_NUM_DISJUNCTS(type);i++) {
                    snprintf(BUFFER(buf, bufsize), "%s ", typeName_TypeConstructor(T_VAR_DISJUNCT(type, i)));
                }
                snprintf(BUFFER(buf, bufsize), "}");
            }
        } else if(etype->t == T_CONS) {
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s ", T_CONS_TYPE_NAME(etype));
            int i;
            for(i=0;i<T_CONS_ARITY(etype);i++) {
                typeToString(etype->ext.cons.typeArgs[i], var_types, buf+strlen(buf), bufsize-strlen(buf));
            }
        }


    region_free(r);
    return buf;

}
ExprType *dereference(ExprType *type, Hashtable *type_table, Region *r) {
    if(type->t == T_VAR) {
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

    switch(type->t) {
        case T_FUNC:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*type->ext.func.arity);
            for(i=0;i<type->ext.func.arity;i++) {
                paramTypes[i] = instantiate(type->ext.func.paramsType[i], type_table,r);
            }
            return newFuncTypeVarArg(type->ext.func.arity, type->ext.func.vararg, paramTypes, cpType(type->ext.func.retType,r), r);
        case T_CONS:
            paramTypes = (ExprType **) region_alloc(r,sizeof(ExprType *)*type->ext.cons.arity);
            for(i=0;i<T_CONS_ARITY(type);i++) {
                paramTypes[i] = instantiate(type->ext.cons.typeArgs[i], type_table, r);
                if(paramTypes[i]!=T_CONS_TYPE_ARG(type, i)) {
                    changed = 1;
                }
            }
            if(changed) {
                return newConsType(T_CONS_ARITY(type), T_CONS_TYPE_NAME(type), paramTypes, r);
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

void generateErrMsgFromFile(char *msg, long errloc, char *ruleBaseName, char* ruleBasePath, char errbuf[ERR_MSG_LEN]) {
    FILE *fp = fopen(ruleBasePath, "r");
    Pointer *e = newPointer(fp, ruleBaseName);
    Label l;
    l.base = ruleBaseName;
    l.exprloc = errloc;
    char buf[1024];
    dupLine(e, &l, 1024, buf);
    strncat(buf, "\n", 1024);
    int coor[2];
    getCoor(e, &l, coor);
    int i;
    for(i=0;i<coor[1];i++) {
        strncat(buf, " ", 1024);
    }
    strncat(buf, "^", 1024);
    /*printf("readRuleSetFromFile: error parsing rule: line %d, row %d\n%s\n", coor[0], coor[1], buf); */
    snprintf(errbuf, ERR_MSG_LEN,
            "%s\nline %d, row %d\n%s\n", msg, coor[0], coor[1], buf);
    deletePointer(e);
    /*fclose(fp); */

}
char *generateErrMsg(char *msg, long errloc, char *ruleBaseName, char errmsg[ERR_MSG_LEN]) {
    if(*ruleBaseName==0) {
        snprintf(errmsg, ERR_MSG_LEN, "<source>\n%s", msg); /* __source_is_unknown__ */
        return errmsg;
    }
    char ruleBasePath[MAX_NAME_LEN];
    getRuleBasePath(ruleBaseName, ruleBasePath);
    generateErrMsgFromFile(msg, errloc, ruleBaseName, ruleBasePath, errmsg);
    return errmsg;
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
        p = strlen(errbuf);
    }
    return errbuf;

}
