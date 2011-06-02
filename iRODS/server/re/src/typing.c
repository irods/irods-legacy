/* For copyright information please refer to files in the COPYRIGHT directory
 */

#include "typing.h"
#include "functions.h"
#define ERROR(x) if(x) {goto error;}
#define ERROR2(x,y) if(x) {localErrorMsg=(y);goto error;}
#define N_BASE_TYPES 5
NodeType baseTypes[] = {
		T_INT,
		T_BOOL,
		T_DOUBLE,
		T_DATETIME,
		T_STRING
};
void doNarrow(Node **l, Node **r, int ln, int rn, int flex, Node **nl, Node **nr, int *nln, int *nrn);
Satisfiability createSimpleConstraint(ExprType *a, ExprType *b, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r);
ExprType *createType(ExprType *t, Node **nc, int nn, Hashtable *typingEnv, Hashtable *equivalence, Region *r);
ExprType *getFullyBoundedVar(Region *r);

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
	if(type->nodeType==T_DYNAMIC) {
        return 0;
    }
    if(expected->nodeType==T_DYNAMIC) {
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
	} else if ((type->nodeType == T_CONS && expected->nodeType == T_CONS) || (type->nodeType == T_TUPLE && expected->nodeType == T_TUPLE) ) {
		if(type->nodeType == T_CONS && strcmp(T_CONS_TYPE_NAME(type), T_CONS_TYPE_NAME(expected))!=0) {
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
char *getBaseTypeOrTVarId(ExprType *a, char buf[128]) {
	if(isBaseType(a)) {
		snprintf(buf, 128, "%s", typeName_ExprType(a));
	} else {
		getTVarName(a->value.vid, buf);
	}
	return buf;
}
int tautologyLtBase(ExprType *a, ExprType *b) {
	if(a->nodeType == b->nodeType) {
		return 1;
	}
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
int occursIn(ExprType *var, ExprType *type) {
	if(type->nodeType == T_VAR) {
		return T_VAR_ID(var) == T_VAR_ID(type);
	} else {
		int i;
		for(i=0;i<type->degree;i++) {
			if(occursIn(var, type->subtrees[i])) {
				return 1;
			}
		}
		return 0;
	}
}
ExprType* getEquivalenceClassRep(ExprType *varOrBase, Hashtable *equivalence) {
	ExprType *equiv1 = NULL, *equiv2;
	char name[128];
	equiv2 = varOrBase;
	int ref = 0;
	while(equiv2 != NULL) {
		equiv1 = equiv2;
		equiv2 = (ExprType *) lookupFromHashTable(equivalence, getBaseTypeOrTVarId(equiv1, name));
		ref ++;
	}
	if(ref > 1) {
		updateInHashTable(equivalence, getBaseTypeOrTVarId(varOrBase, name), equiv1);
	}
	return equiv1;

}


int occursInEquiv(ExprType *var, ExprType *type, Hashtable *equivalence) {
	if(type->nodeType == T_VAR && isBaseType(type)) {
		ExprType *varEquiv = getEquivalenceClassRep(var, equivalence);
		ExprType *typeEquiv = getEquivalenceClassRep(type, equivalence);
		return typeEqSyntatic(varEquiv, typeEquiv);
	} else {
		int i;
		for(i=0;i<type->degree;i++) {
			if(occursInEquiv(var, type->subtrees[i], equivalence)) {
				return 1;
			}
		}
		return 0;
	}
}

Satisfiability splitVarR(ExprType *consTuple, ExprType *var, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
	if(occursInEquiv(var, consTuple, equivalence) || isBaseType(getEquivalenceClassRep(var, equivalence))) {
		return ABSURDITY;
	}
    char tvarname[128];
    ExprType **typeArgs = (ExprType **) region_alloc(r, sizeof(ExprType *) * T_CONS_ARITY(consTuple));
    int i;
    ExprType *type;
    for(i=0;i<T_CONS_ARITY(consTuple);i++) {
        typeArgs[i] = newTVar(r);
    }
    if(consTuple->nodeType == T_CONS) {
        type = newConsType(T_CONS_ARITY(consTuple), T_CONS_TYPE_NAME(consTuple), typeArgs, r);
    } else {
        type = newTupleType(T_CONS_ARITY(consTuple), typeArgs, r);
    }
    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(var), tvarname), type);
    return splitConsOrTuple(consTuple, type, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

}
Satisfiability splitVarL(ExprType *var, ExprType *consTuple, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
	if(occursInEquiv(var, consTuple, equivalence) || isBaseType(getEquivalenceClassRep(var, equivalence))) {
		return ABSURDITY;
	}
    char tvarname[128];
    ExprType **typeArgs = (ExprType **) region_alloc(r, sizeof(ExprType *) * T_CONS_ARITY(consTuple));
    int i;
    ExprType *type;
    for(i=0;i<T_CONS_ARITY(consTuple);i++) {
        typeArgs[i] = newTVar(r);
    }
    if(consTuple->nodeType == T_CONS) {
        type = newConsType(T_CONS_ARITY(consTuple), T_CONS_TYPE_NAME(consTuple), typeArgs, r);
    } else {
        type = newTupleType(T_CONS_ARITY(consTuple), typeArgs, r);
    }
    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(var), tvarname), type);
    return splitConsOrTuple(type, consTuple, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

}

/*
 * simplify b which is a variable, bounded or unbounded, based on a which is a base type
 * returns 1 tautology
 *         0 contingency
 *         -1 absurdity
 */
Satisfiability simplifyR(ExprType *a, ExprType *b, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
	ExprType *bm;
	if(T_VAR_NUM_DISJUNCTS(b) == 0) {
			bm = getFullyBoundedVar(r);
	} else {
		bm = b;
	}
	Node *cl[100], *cr[100];
	int nln, nrn;
	doNarrow(&a, T_VAR_DISJUNCTS(bm), 1, T_VAR_NUM_DISJUNCTS(bm), flex, cl, cr, &nln, &nrn);
	ExprType *bn;
	if(nrn == 0) {
		return ABSURDITY;
	} else {
		bn = createType(b, cr, nrn, typingEnv, equivalence, r);
		if(bn == b) {
			return TAUTOLOGY;
		}
		return createSimpleConstraint(a, bn, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
	}
}

ExprType *getFullyBoundedVar(Region *r) {
	ExprType **ds = (ExprType **) region_alloc(r, sizeof(ExprType *) * N_BASE_TYPES);
	int i;
	for(i=0;i<N_BASE_TYPES;i++) {
		ds[i] = newSimpType(baseTypes[i], r);
	}
	ExprType *var = newTVar2(N_BASE_TYPES, ds, r);
	return var;
}

Satisfiability simplifyL(ExprType *a, ExprType *b, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
	ExprType *am;
	if(T_VAR_NUM_DISJUNCTS(a) == 0) {
		am = getFullyBoundedVar(r);
	} else {
		am = a;
	}
	Node *cl[100], *cr[100];
	int nln, nrn;
	doNarrow(T_VAR_DISJUNCTS(am), &b, T_VAR_NUM_DISJUNCTS(am), 1, flex, cl, cr, &nln, &nrn);
	ExprType *an;
	if(nln == 0) {
		return ABSURDITY;
	} else {
		an = createType(a, cl, nln, typingEnv, equivalence, r);
		if(an == a) {
			return TAUTOLOGY;
		}
		return createSimpleConstraint(an, b, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
	}

}

void addToEquivalenceClass(ExprType *a, ExprType *b, Hashtable *equivalence) {
	char name[128];
	ExprType *an = getEquivalenceClassRep(a, equivalence);
	ExprType *bn = getEquivalenceClassRep(b, equivalence);
	if(!typeEqSyntatic(an, bn)) {
		if(isBaseType(an)) {
			insertIntoHashTable(equivalence, getTVarName(T_VAR_ID(bn), name), an);
		} else {
			insertIntoHashTable(equivalence, getTVarName(T_VAR_ID(an), name), bn);
		}
	}
}
void doNarrow(Node **l, Node **r, int ln, int rn, int flex, Node **nl, Node **nr, int *nln, int *nrn) {
	int retl[100], retr[100];
	int i,k;
	for(i=0;i<ln;i++) {
		retl[i] = 0;
	}
	for(k=0;k<rn;k++) {
		retr[k] = 0;
	}
	for(k=0;k<rn;k++) {
		for(i=0;i<ln;i++) {
			if(splitBaseType(l[i], r[k], flex) == TAUTOLOGY) {
				retl[i] = 1;
				retr[k] = 1;
			/*	break;*/
			}
		}
	}
	*nln = 0;
	for(i=0;i<ln;i++) {
		if(retl[i]) {
			nl[(*nln)++] = l[i];
		}
	}
	*nrn = 0;
	for(k=0;k<rn;k++) {
		if(retr[k]) {
			nr[(*nrn)++] = r[k];
		}
	}
}

Satisfiability createSimpleConstraint(ExprType *a, ExprType *b, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
	char name[128];
	if(isBaseType(a) && isBaseType(b)) {
		return TAUTOLOGY;
	} else {
		addToEquivalenceClass(a, b, equivalence);
		if(flex) {
			listAppend(simpleTypingConstraints, newTypingConstraint(a, newUnaryType(T_FLEX, b, r), TC_LT, node, r), r);
			return CONTINGENCY;
		} else {
			if((a->nodeType == T_VAR && T_VAR_NUM_DISJUNCTS(a)==0) || isBaseType(b)) {
				insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), name), b);
			} else if((b->nodeType == T_VAR && T_VAR_NUM_DISJUNCTS(b)==0) || isBaseType(a)) {
				insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), name), a);
			} else { // T_VAR_NUM_DISJUNCTS(a) == T_VAR_NUM_DISJUNCTS(b)
				insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), name), b);
			}
			return TAUTOLOGY;
		}
	}
}
ExprType *createType(ExprType *t, Node **nc, int nn, Hashtable *typingEnv, Hashtable *equivalence, Region *r) {
	char name[128];
	ExprType *gcd;
	if (nn == T_VAR_NUM_DISJUNCTS(t)) {
		gcd = t;
	} else {
		if(nn==1) {
			gcd = *nc;
		} else {
			gcd = newTVar2(nn, nc, r);
		}
		insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(t), name), gcd);
		addToEquivalenceClass(t, gcd, equivalence);
	}
	return gcd;
}
Satisfiability narrow(ExprType *type, ExprType *expected, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {

	if(T_VAR_ID(type) == T_VAR_ID(expected)) {
		return TAUTOLOGY;
	} else if(T_VAR_NUM_DISJUNCTS(type) > 0 && T_VAR_NUM_DISJUNCTS(expected) > 0) {
		int nln, nrn;
		Node *cl[100], *cr[100];
		doNarrow(T_VAR_DISJUNCTS(type), T_VAR_DISJUNCTS(expected), T_VAR_NUM_DISJUNCTS(type), T_VAR_NUM_DISJUNCTS(expected), flex, cl, cr, &nln, &nrn);
		ExprType *an;
		ExprType *bn;
		if(nln == 0 || nrn == 0) {
			return ABSURDITY;
		} else {
			an = createType(type, cl, nln, typingEnv, equivalence, r);
			bn = createType(expected, cr, nrn, typingEnv, equivalence, r);
		}
		return createSimpleConstraint(an, bn, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
	} else if(T_VAR_NUM_DISJUNCTS(type)==0) { /* free */
		return createSimpleConstraint(type, expected, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
	} else /*if(T_VAR_NUM_DISJUNCTS(expected)==0)*/ { /* free */
		return createSimpleConstraint(type, expected, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
	}
}
Satisfiability splitConsOrTuple(ExprType *a, ExprType *b, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
/* split composite constraints with same top level type constructor */
	if((a->nodeType == T_CONS && strcmp(T_CONS_TYPE_NAME(a), T_CONS_TYPE_NAME(b)) != 0) ||
			T_CONS_ARITY(a) != T_CONS_ARITY(b)) {
		return ABSURDITY;
	} else {
		int i;
		Satisfiability ret = TAUTOLOGY;
		for(i=0;i<T_CONS_ARITY(a);i++) {
			ExprType *simpa = T_CONS_TYPE_ARG(a, i);
			ExprType *simpb = T_CONS_TYPE_ARG(b, i);
			Satisfiability sat = simplifyLocally(simpa, simpb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
			switch(sat) {
				case ABSURDITY:
					return ABSURDITY;
				case TAUTOLOGY:
					break;
				case CONTINGENCY:
					ret = CONTINGENCY;
					break;
			}

		}
		return ret;
	}
}


int isBaseType(ExprType *t) {
	int i;
	for(i=0;i<N_BASE_TYPES;i++) {
		if(t->nodeType == baseTypes[i]) {
			return 1;
		}
	}
	return 0;
}

Satisfiability splitBaseType(ExprType *tca, ExprType *tcb, int flex)
{
    return (flex && tautologyLtBase(tca, tcb)) || (!flex && typeEqSyntatic(tca, tcb)) ? TAUTOLOGY : ABSURDITY;
}

Satisfiability simplifyLocally(ExprType *tca, ExprType *tcb, int flex, Node *node, Hashtable *typingEnv, Hashtable *equivalence, List *simpleTypingConstraints, Region *r) {
/*
    char buf[1024], buf2[1024], buf3[1024], buf4[ERR_MSG_LEN];
    generateErrMsg("simplifyLocally: constraint generated from ", TC_NODE(tc)->expr, TC_NODE(tc)->base, buf4);
    printf(buf4);
    snprintf(buf, 1024, "\t%s<%s\n",
                typeToString(TC_A(tc), NULL, buf2, 1024),
                typeToString(TC_B(tc), NULL, buf3, 1024));
    printf("%s", buf);
    snprintf(buf, 1024, "\tinstantiated: %s<%s\n",
                typeToString(TC_A(tc), typingEnv, buf2, 1024),
                typeToString(TC_B(tc), typingEnv, buf3, 1024));
    printf("%s", buf);
*/
    if(tcb->nodeType == T_FLEX) {
    	tcb = tcb->subtrees[0];
    	flex = 1;
    }
    tca = dereference(tca, typingEnv, r);
    tcb = dereference(tcb, typingEnv, r);

	if(tca->nodeType == T_UNSPECED || tcb->nodeType == T_DYNAMIC) { /* is an undefined variable argument or a parameter with dynamic type */
		return TAUTOLOGY;
	}
	else if(isBaseType(tca) && isBaseType(tcb)) {
		return splitBaseType(tca, tcb, flex);
	} else if(tca->nodeType == T_VAR && tcb->nodeType == T_VAR) {
		return narrow(tca, tcb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

	} else if(tca->nodeType==T_VAR && isBaseType(tcb)) {
		return simplifyL(tca, tcb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

	} else if(tcb->nodeType==T_VAR && isBaseType(tca)) {
		return simplifyR(tca, tcb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

	} else if(tca->nodeType==T_VAR && (tcb->nodeType == T_CONS || tcb->nodeType == T_TUPLE)) {
		return splitVarL(tca, tcb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

	} else if(tcb->nodeType==T_VAR && (tca->nodeType == T_CONS || tca->nodeType == T_TUPLE)) {
		return splitVarR(tca, tcb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);

	} else if((tca->nodeType == T_CONS && tcb->nodeType == T_CONS)
			  || (tca->nodeType == T_TUPLE && tcb->nodeType == T_TUPLE)) {
		return splitConsOrTuple(tca, tcb, flex, node, typingEnv, equivalence, simpleTypingConstraints, r);
	} else {
		return ABSURDITY;
	}
}
/*
 * not 0 solved
 * 0 not solved
 */
Satisfiability solveConstraints(List *typingConstraints, Hashtable *typingEnv, rError_t *errmsg, Node ** errnode, Region *r) {
/*
    char buf0[1024];
    typingConstraintsToString(typingConstraints, typingEnv, buf0, 1024);
    printf("solving constraints: %s\n", buf0);
*/
	ListNode *nextNode = NULL;
    do {
        Satisfiability sat = simplify(typingConstraints, typingEnv, errmsg, errnode, r);
        if(sat == ABSURDITY) {
            return ABSURDITY;
        }
        int changed = 0;
        nextNode = typingConstraints->head;
        while(nextNode!=NULL && !changed) {
            TypingConstraint *tc = (TypingConstraint *)nextNode->value;
/*
            char buf2[1024], buf3[1024];
*/
            /* printf("dereferencing %s and %s.\n", typeToString(TC_A(tc), typingEnv, buf2, 1024), typeToString(TC_B(tc), typingEnv, buf3, 1024)); */
            ExprType *a = dereference(TC_A(tc), typingEnv, r);
            ExprType *b = dereference(TC_B(tc), typingEnv, r);
            if(b->nodeType == T_FLEX) {
            	b = b->subtrees[0];
            }
/*
            printf("warning: collasping %s with %s.\n", typeToString(a, typingEnv, buf2, 1024), typeToString(b, typingEnv, buf3, 1024));
*/
                        /*printVarTypeEnvToStdOut(typingEnv); */
            if (a->nodeType == T_VAR && b->nodeType == T_VAR && T_VAR_ID(a) == T_VAR_ID(b)) {
            	listRemove(typingConstraints, nextNode);
            	nextNode = typingConstraints->head;
                changed = 1;
/*            } else if (a->nodeType == T_VAR && T_VAR_NUM_DISJUNCTS(a) == 0 &&
            		(b->nodeType != T_VAR || T_VAR_NUM_DISJUNCTS(b) != 0)) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
                listRemove(typingConstraints, nextNode);
            	nextNode = typingConstraints->head;
                changed = 1;
            } else if (b->nodeType == T_VAR && T_VAR_NUM_DISJUNCTS(b) == 0 &&
            		(a->nodeType != T_VAR || T_VAR_NUM_DISJUNCTS(a) != 0)) {
                insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
                listRemove(typingConstraints, nextNode);
            	nextNode = typingConstraints->head;
                changed = 1;
            } else if (a->nodeType == T_VAR && b->nodeType == T_VAR &&
            		T_VAR_NUM_DISJUNCTS(a) != 0 && T_VAR_NUM_DISJUNCTS(b) != 0) {
                if(T_VAR_NUM_DISJUNCTS(a) > T_VAR_NUM_DISJUNCTS(b)) {
                    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(b), buf), a);
                } else {
                    insertIntoHashTable(typingEnv, getTVarName(T_VAR_ID(a), buf), b);
                }
                listRemove(typingConstraints, nextNode);
            	nextNode = typingConstraints->head;
                changed = 1;*/
            } else if (a->nodeType != T_VAR && b->nodeType != T_VAR) {
                printf("error: simplified type does not have variable on either side.\n");
            } else {
            	nextNode = nextNode->next;
            }
            /* printVarTypeEnvToStdOut(typingEnv); */
        }
    } while(nextNode != NULL);
    if(!consistent(typingConstraints, typingEnv, r)) {
    	return ABSURDITY;
    }
    return typingConstraints->head == NULL ? TAUTOLOGY : CONTINGENCY;
}

int consistent(List *typingConstraints, Hashtable *typingEnv, Region *r) {
	return 1;
}
Satisfiability simplify(List *typingConstraints, Hashtable *typingEnv, rError_t *errmsg, Node **errnode, Region *r) {
    ListNode *ln;
    int changed;
    Hashtable *equivalence = newHashTable(100);
    List *simpleTypingConstraints = newList(r);
    /* printf("start\n"); */
    /*char buf[1024];
	typingConstraintsToString(typingConstraints, typingEnv, buf, 1024);*/
    Satisfiability ret = TAUTOLOGY;
    do {
        changed = typingEnv->len;
    	ln = typingConstraints->head;
    	/*typingConstraintsToString(typingConstraints, typingEnv, buf, 1024);
    	printf("constraints: \n%s\n\n", buf);*/
        while(ln!=NULL) {
        	TypingConstraint *tc = (TypingConstraint *)ln->value;
            switch(simplifyLocally(TC_A(tc), TC_B(tc), 0, TC_NODE(tc), typingEnv, equivalence, simpleTypingConstraints, r)) {
                case TAUTOLOGY:
                	break;
                case CONTINGENCY:
                    /* printf("contingency\n"); */
                    /* printf("tautology\n"); */
                    /*    TypingConstraint *tcp;
					tcp = tc;
					while(tcp!=NULL) {
						printf("simplified %s<%s to %s<%s.\n",
								typeToString(a, NULL, buf3, 1024), typeToString(b, NULL, buf4, 1024),
								typeToString(tcp->a, NULL, buf1, 1024), typeToString(tcp->b, NULL, buf2, 1024));
						tcp = tcp->next;
					}*/
                	ret = CONTINGENCY;
                    break;
                case ABSURDITY:
                    *errnode = TC_NODE(tc);
                    char errmsgbuf1[ERR_MSG_LEN], errmsgbuf2[ERR_MSG_LEN], buf2[1024], buf3[1024];
                    snprintf(errmsgbuf1, ERR_MSG_LEN, "simplify: unsolvable typing constraint %s<%s.\n", typeToString(TC_A(tc), typingEnv, buf2, 1024), typeToString(TC_B(tc), typingEnv, buf3, 1024));
                    generateErrMsg(errmsgbuf1, (*errnode)->expr, (*errnode)->base, errmsgbuf2);
                    addRErrorMsg(errmsg, -1, errmsgbuf2);
                    /*printVarTypeEnvToStdOut(typingEnv); */
                    /* printf("absurdity\n"); */
                    return ABSURDITY;
            }
            ln = ln->next;
        }
    	/*typingConstraintsToString(typingConstraints, typingEnv, buf, 1024);
    	printf("simplified constraints: \n%s\n\n", buf);
    	printHashtable(typingEnv, buf);
    	printf("env: \n%s\n", buf);*/
        typingConstraints->head = simpleTypingConstraints->head;
        typingConstraints->tail = simpleTypingConstraints->tail;
        simpleTypingConstraints->head = simpleTypingConstraints->tail = NULL;
    } while(changed < typingEnv->len);

    return ret;
}

ExprType* typeFunction3(Node* node, Hashtable* funcDesc, Hashtable* var_type_table, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
    /*printTree(node, 0); */
    int i;
    char *localErrorMsg;
    ExprType *res3 = NULL;
    /*char buf[1024];*/
    /*printf("typeing %s\n",fn); */
    /*printVarTypeEnvToStdOut(var_type_table); */
    Node *fn = node->subtrees[0];
    Node *arg = node->subtrees[1];


    if(fn->nodeType == TK_TEXT && strcmp(fn->text, "foreach") == 0) {
        ERROR2(arg->nodeType != N_TUPLE || arg->degree != 3,"wrong number of arguments to microservice");
        ERROR2(arg->subtrees[0]->nodeType!=TK_VAR,"argument form error");
        char* varname = arg->subtrees[0]->text;
        ExprType *varType = (ExprType *)lookupFromHashTable(var_type_table, varname);
        ExprType *collType = varType == NULL? NULL:dereference(varType, var_type_table, r);
        if(collType!=NULL) {
            /* error if res is not a collection type or a type variable (primitive union type excluded) */
            ERROR2(collType->nodeType != T_CONS && (collType->nodeType != T_VAR || T_VAR_NUM_DISJUNCTS(collType)!=0 ), "foreach is applied to a non collection type");
            /* overwrite type of collection variable */
            if(collType->nodeType == T_VAR) {
                unifyTVarL(collType, newCollType(newTVar(r), r), var_type_table, r);
                collType = dereference(collType, var_type_table, r);
            }
            /* dereference element type as only top level vars are dereferenced by the dereference function and we are accessing a subtree of the type */
            updateInHashTable(var_type_table, varname, dereference(T_CONS_TYPE_ARG(collType, 0), var_type_table, r));
        } else {
            ExprType *elemType;
            insertIntoHashTable(var_type_table, varname, elemType = newTVar(r));
            collType = newCollType(elemType, r);
        }
        arg->subtrees[0]->exprType = collType;
        res3 = typeExpression3(arg->subtrees[1],funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
        ERROR2(res3->nodeType == T_ERROR, "foreach loop type error");
        res3 = typeExpression3(arg->subtrees[2],funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
        ERROR2(res3->nodeType == T_ERROR, "foreach recovery type error");
        arg->subtrees[0]->iotype = IO_TYPE_EXPRESSION;
        for(i = 1;i<3;i++) {
        	arg->subtrees[i]->iotype = IO_TYPE_ACTIONS;
        }
        ExprType **typeArgs = allocSubtrees(r, 3);
        typeArgs[0] = collType;
        typeArgs[1] = newTVar(r);
        typeArgs[2] = newTVar(r);
        arg->coercionType = newTupleType(3, typeArgs, r);

        updateInHashTable(var_type_table, varname, collType); /* restore type of collection variable */
        return res3;
    } else {
    	ExprType *fnType = typeExpression3(fn, funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
    	if(fnType->nodeType == T_ERROR) return fnType;
    	arg->value.constructTuple = 1; /* arg must be a N_TUPLE or N_IMPLICIT_TUPLE node */
		ExprType *argType = typeExpression3(arg, funcDesc, var_type_table,typingConstraints,errmsg,errnode,r);
		if(argType->nodeType == T_ERROR) return argType;

		ExprType *fType = fnType->nodeType == T_CONS && strcmp(fnType->text, FUNC) == 0 ? fnType : unifyWith(fnType, newFuncType(newTVar(r), newTVar(r), r), var_type_table, r);

		ERROR2(fType->nodeType == T_ERROR, "the first component of a function application does not have a function type");
		ExprType *paramType = dereference(fType->subtrees[0], var_type_table, r);
		ExprType *retType = dereference(fType->subtrees[1], var_type_table, r);

        ERROR2(fn->nodeType == TK_TEXT && strcmp(fn->text, "assign") == 0 &&
                arg->degree>0 &&
                !isPattern(arg->subtrees[0]), "the first argument of microservice assign is not a variable or a pattern");
        ERROR2(fn->nodeType == TK_TEXT && strcmp(fn->text, "let") == 0 &&
                arg->degree>0 &&
                !isPattern(arg->subtrees[0]), "the first argument of microservice let is not a variable or a pattern");

/*
            printf("start typing %s\n", fn);
*/
/*
            printTreeDeref(node, 0, var_type_table, r);
*/
        char buf[ERR_MSG_LEN];
                    /* char errbuf[ERR_MSG_LEN]; */
                    char typebuf[ERR_MSG_LEN];
                    char typebuf2[ERR_MSG_LEN];ExprType *t = NULL;
		if(fType->vararg!=ONCE) {
			/* generate instance of vararg tuple so that no vararg tuple goes into typing constraints */
			int fixParamN = paramType->degree - 1;
			int argN = node->subtrees[1] ->degree;
			int copyN = argN - fixParamN;
			ExprType **subtrees = paramType->subtrees;
			if(copyN < (fType->vararg == PLUS ? 1 : 0) || (fType->vararg == OPTIONAL && copyN > 1)) {
				snprintf(buf, 1024, "unsolvable vararg typing constraint %s < %s %s",
					typeToString(argType, var_type_table, typebuf, ERR_MSG_LEN),
					typeToString(paramType, var_type_table, typebuf2, ERR_MSG_LEN),
					fType->vararg == PLUS ? "*" : fType->vararg == OPTIONAL ? "?" : "+");
				ERROR2(1, buf);
			}
			ExprType **paramTypes = allocSubtrees(r, argN);
			int i;
			for(i = 0;i<fixParamN;i++) {
				paramTypes[i] = subtrees[i];
			}
			for(i=0;i<copyN;i++) {
				paramTypes[i+fixParamN] = subtrees[fixParamN];
			}
			t = newTupleType(argN, paramTypes, r);
		} else {
			t = paramType;
		}
		/*t = replaceDynamicWithNewTVar(t, r);
		argType = replaceDynamicWithNewTVar(argType, r);*/
        int ret = typeFuncParam(node->subtrees[1], argType, t, var_type_table, typingConstraints, errmsg, r);
        if(ret!=0) {
            *errnode = node->subtrees[1];
            ERROR2(ret != 0, "parameter type error");
        }
		int i;
		for(i=0;i<node->subtrees[1]->degree;i++) {
			node->subtrees[1]->subtrees[i]->iotype = t->subtrees[i]->iotype;
		}

        arg->coercionType = t; /* set coersion to parameter type */

/*
        printTreeDeref(node, 0, var_type_table, r);
        printf("finish typing %s\n", fn);
*/
        return instantiate(replaceDynamicWithNewTVar(retType, r), var_type_table, 0, r);
    }
    char errbuf[ERR_MSG_LEN];
    char errmsgbuf[ERR_MSG_LEN];
    error:
    *errnode = node;
    snprintf(errmsgbuf, ERR_MSG_LEN, "type error: %s in %s", localErrorMsg, fn->text);
    generateErrMsg(errmsgbuf, (*errnode)->expr, (*errnode)->base, errbuf);
    addRErrorMsg(errmsg, -1, errbuf);
    return newSimpType(T_ERROR,r);
}
ExprType *replaceDynamicWithNewTVar(ExprType *type, Region *r) {
		ExprType *newt = (ExprType *)region_alloc(r, sizeof(ExprType));
		*newt = *type;
		if(type->nodeType == T_DYNAMIC) {
			newt->nodeType = T_VAR;
			newt->value.vid = newTVarId();
		}
		int i;
		for(i=0;i<type->degree;i++) {
			newt->subtrees[i] = replaceDynamicWithNewTVar(type->subtrees[i], r);
		}
		return newt;
}
int typeFuncParam(Node *param, Node *paramType, Node *formalParamType, Hashtable *var_type_table, List *typingConstraints, rError_t *errmsg, Region *r) {
            /*char buf[ERR_MSG_LEN];
            char errbuf[ERR_MSG_LEN];
            char typebuf[ERR_MSG_LEN];
            char typebuf2[ERR_MSG_LEN]; */
            /* printf("typing param %s < %s\n",
                                typeToString(paramType, var_type_table, typebuf, ERR_MSG_LEN),
                                typeToString(formalParamType, var_type_table, typebuf2, ERR_MSG_LEN)); */


			TypingConstraint *tc = newTypingConstraint(paramType, formalParamType, TC_LT, param, r);
			listAppend(typingConstraints, tc, r);
			Node *errnode;
			Satisfiability tcons = simplify(typingConstraints, var_type_table, errmsg, &errnode, r);
			switch(tcons) {
				case TAUTOLOGY:
					break;
				case CONTINGENCY:
					break;
				case ABSURDITY:
				    return -1;
			}
            return 0;
}

ExprType* typeExpression3(Node *expr, Hashtable *funcDesc, Hashtable *varTypes, List *typingConstraints, rError_t *errmsg, Node **errnode, Region *r) {
    ExprType *res;
    ExprType **components;
    ExprType* t = NULL;
	int i;
	expr->typed = 1;
	switch(expr->nodeType) {
		case TK_INT:
            return expr->exprType = newSimpType(T_INT,r);
        case TK_DOUBLE:
            return expr->exprType = newSimpType(T_DOUBLE,r);
		case TK_STRING:
			return expr->exprType = newSimpType(T_STRING,r);
		case TK_VAR:
				t = (ExprType *)lookupFromHashTable(varTypes, expr->text);
				if(t==NULL) {
					/* define new variable */
					t = newTVar(r);
					insertIntoHashTable(varTypes, expr->text, t);
				}
                                t = dereference(t, varTypes, r);
				return expr->exprType = t;
		case TK_TEXT:
			if(strcmp(expr->text,"nop")==0) {
				return expr->exprType = newFuncType(newTupleType(0, NULL, r), newSimpType(T_INT, r), r);
			} else {
                /* not a variable, evaluate as a function */
			    FunctionDesc *fDesc = (FunctionDesc*)lookupFromHashTable(funcDesc, expr->text);
                if(fDesc!=NULL && fDesc->exprType!=NULL) {
                    return expr->exprType = dupType(fDesc->exprType, r);
                } else {
                    ExprType *paramType = newSimpType(T_DYNAMIC, r);
                    paramType->iotype = IO_TYPE_DYNAMIC;
                    ExprType *fType = newFuncType(newUnaryType(T_TUPLE, paramType, r), newSimpType(T_DYNAMIC, r), r);
                    fType->vararg = STAR;
                    return expr->exprType = fType;
                }
			}
        case N_TUPLE:
            components = (ExprType **) region_alloc(r, sizeof(ExprType *) * expr->degree);
            for(i=0;i<expr->degree;i++) {
                components[i] = typeExpression3(expr->subtrees[i], funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
                if(components[i]->nodeType == T_ERROR) {
                	return expr->exprType = components[i];
                }
            }
            if(expr->value.constructTuple || expr->degree != 1) {
            	return expr->exprType = newTupleType(expr->degree, components, r);
            } else {
            	return expr->exprType = components[0];
            }

		case N_APPLICATION:
                        /* try to type as a function */
                        /* the exprType is used to store the type of the return value */
                        return expr->exprType = typeFunction3(expr, funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
		case N_ACTIONS:
                if(expr->degree == 0) {
                    /* type of empty action sequence == T_INT */
                    return expr->exprType = newSimpType(T_INT, r);
                }
                for(i=0;i<expr->degree;i++) {
                    /*printf("typing action in actions"); */
    				res = typeExpression3(expr->subtrees[i], funcDesc, varTypes, typingConstraints, errmsg, errnode,r);
                    /*printVarTypeEnvToStdOut(varTypes); */
                    if(res->nodeType == T_ERROR) {
                        return expr->exprType = res;
                    }
                }
                return expr->exprType = res;
		case N_ACTIONS_RECOVERY:
                res = typeExpression3(expr->subtrees[0], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
                if(res->nodeType == T_ERROR) {
                    return expr->exprType = res;
                }
                res = typeExpression3(expr->subtrees[1], funcDesc, varTypes, typingConstraints, errmsg, errnode, r);
                return expr->exprType = res;
        default:
                break;
	}
	*errnode = expr;
    char errbuf[ERR_MSG_LEN], errbuf0[ERR_MSG_LEN];
	snprintf(errbuf0, ERR_MSG_LEN, "error: unsupported ast node %d", expr->nodeType);
	generateErrMsg(errbuf0, expr->expr, expr->base, errbuf);
    addRErrorMsg(errmsg, -1, errbuf);
	return expr->exprType = newSimpType(T_ERROR,r);
}
/*
 * This process is based on a few assumptions:
 * If the type coerced to cannot be inferred, i.e., an expression is to be coerced to some tvar, bounded to a union type or free,
 * then we leave further type checking to runtime, which can be done locally under the assumption of indistinguishable inclusion.
 *
 */
void postProcessCoercion(Node *expr, Hashtable *varTypes, rError_t *errmsg, Node **errnode, Region *r) {
    expr->coercionType = expr->coercionType==NULL?NULL:instantiate(expr->coercionType, varTypes, 0, r);
    expr->exprType = expr->exprType==NULL?NULL:instantiate(expr->exprType, varTypes, 0, r);
    int i;
    for(i=0;i<expr->degree;i++) {
        postProcessCoercion(expr->subtrees[i], varTypes, errmsg, errnode, r);
    }
    if(expr->coercionType!=NULL && expr->exprType!=NULL) {
                /*char buf[128];*/
                /*typeToString(expr->coercionType, NULL, buf, 128);
                printf("%s", buf);*/
            if(expr->nodeType == N_TUPLE) {
            	ExprType **csubtrees = expr->coercionType->subtrees;
            	int i;
            	for(i=0;i<expr->degree;i++) {
                    if(typeEqSyntatic(expr->subtrees[i]->exprType, csubtrees[i])) {
                		expr->subtrees[i]->coerce = 0;
                    } else {
                		expr->subtrees[i]->coerce = 1;
                    }
            	}
            }
    }
}
/*
 * convert single action to actions if the parameter is of type actions
 */
void postProcessActions(Node *expr, Hashtable *systemFunctionTables, rError_t *errmsg, Node **errnode, Region *r) {
    int i;
    switch(expr->nodeType) {
        case N_TUPLE:
            for(i=0;i<expr->degree;i++) {
                if(expr->subtrees[i]->iotype == IO_TYPE_ACTIONS && expr->subtrees[i]->nodeType != N_ACTIONS) {
                    expr->subtrees[i]->iotype = IO_TYPE_INPUT;
                    Node **params = (Node **)region_alloc(r, sizeof(Node *)*1);
                    params[0] = expr->subtrees[i];
                    Label pos;
                    pos.base = expr->base;
                    pos.exprloc = expr->expr;
                    expr->subtrees[i] = createActionsNode(params, 1, &pos, r);
                    expr->subtrees[i]->iotype = IO_TYPE_ACTIONS;
                    expr->subtrees[i]->exprType = params[0]->exprType;
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
