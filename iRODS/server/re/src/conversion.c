/* For copyright information please refer to files in the COPYRIGHT directory
 */
#include "arithmetics.h"
#include "index.h"
#include "datetime.h"

int addKeyVal(keyValPair_t *k, char * key, char *val);
char * getAttrNameFromAttrId(int id);


/************************ Microservice parameter to Res ***********************/
/**
 * convert string value to Res
 */
void convertStrValue(Res *res, char *val, Region *r) {
    int len = (strlen(val)+1)*sizeof(char);
    res->text = (char*)region_alloc(r, len);
    memcpy(res->text, val, len);
    res->value.s.len = strlen(val);
    res->exprType = newSimpType(T_STRING, r);
}
/**
 * convert int value to Res
 */
void convertIntValue(Res *res, int inval, Region *r) {
    res->value.d = inval;
    res->exprType = newSimpType(T_INT, r);
}
/**
 * convert double value to Res
 */
void convertDoubleValue(Res *res, double inval, Region *r) {
    res->value.d = inval;
    res->exprType = newSimpType(T_DOUBLE,r);
}
/**
 * adapted from reHelpers2.c
 * input
 *     typ: collection type
 *     inPtr: collection value
 *     inx: index of the element
 * output
 *     value: element value (new on heap)
 *     inx: index of the next element
 *     outtyp: the type of the element (new on heap)
 * return
 *     0
 * 	   NO_VALUES_FOUND
 *     USER_PARAM_TYPE_ERR
 */
Res* getValueFromCollection(char *typ, void *inPtr, int inx, Region *r) {
  Res *res;
  int i,j;

  if (!strcmp(typ,StrArray_MS_T)) {
    strArray_t *strA;
	/* ->size size of an element */
	/* ->len  length of the array */
    strA = (strArray_t  *) inPtr;
    if (inx >= strA->len) {
      return NULL;
    }
    res = newStringRes(r, strA->value+inx * strA->size);
    return res;
  }
  else if (!strcmp(typ,IntArray_MS_T)) {
	res = newRes(r);
	res->exprType = newSimpType(T_INT, r);
    intArray_t *intA;
    intA = (intArray_t *) inPtr;
    if (inx >= intA->len) {
      return NULL;
    }
    res->value.d = intA->value[inx];
    return res;
  }
  else if (!strcmp(typ,GenQueryOut_MS_T)) {
    keyValPair_t *k; /* element value */
    genQueryOut_t *g = (genQueryOut_t *) inPtr; /* the result set */
    char *cname, *aval; /* key and value */
    sqlResult_t *v; /* a result row */
    if (g->rowCnt == 0 || inx >= g->rowCnt) {
      return NULL;
    }
	k = (keyValPair_t *)malloc(sizeof(keyValPair_t));
        k->len = 0;
        k->keyWord = NULL;
        k->value = NULL;
	for (i = 0; i < g->attriCnt; i++) {
		v = g->sqlResult+i;
		cname = (char *) getAttrNameFromAttrId(v->attriInx);
		aval = v->value+ v->len*inx;
                j  = addKeyVal (k, cname,aval); /* addKeyVal duplicates the strings */
		if (j < 0)
			return NULL;
	}
	res = newUninterpretedRes(r, KeyValPair_MS_T, k, NULL);
    return res;
  }
  else
    return NULL;
}
int getCollectionSize(char *typ, void *inPtr, Region *r) {
  if (!strcmp(typ,StrArray_MS_T)) {
    strArray_t *strA;
	/* ->size size of an element */
	/* ->len  length of the array */
    strA = (strArray_t  *) inPtr;
    return strA->len;
  }
  else if (!strcmp(typ,IntArray_MS_T)) {
    intArray_t *intA;
    intA = (intArray_t *) inPtr;
    return intA->len;
  }
  else if (!strcmp(typ,GenQueryOut_MS_T)) {
    genQueryOut_t *g = (genQueryOut_t *) inPtr; /* the result set */
    return g->rowCnt;
  }
  else
    return(USER_PARAM_TYPE_ERR);
}
int convertMsParamToRes(msParam_t *mP, Res *res, rError_t *errmsg, Region *r) {
    #ifdef DEBUG
    writeToTmp("relog.txt", "type: ");
    writeToTmp("relog.txt", mP->type);
    writeToTmp("relog.txt", "\n");
    #endif
	if (strcmp(mP->type, DOUBLE_MS_T) == 0) { /* if the parameter is an integer */
		convertDoubleValue(res, *(double *)mP->inOutStruct,r);
		return 0;
	} else if (strcmp(mP->type, INT_MS_T) == 0) { /* if the parameter is an integer */
            /* this could be int, bool, or datatime */
            if(res->exprType == NULL) { /* output parameter */
                res->value.d = *(int *)mP->inOutStruct;
                res->exprType = newSimpType(T_INT, r);
            } else
            switch(TYPE(res)) {
                case T_INT:
                case T_BOOL:
                    res->value.d = *(int *)mP->inOutStruct;
                    break;
                case T_DATETIME:
                    res->value.t = (time_t)*(int *)mP->inOutStruct;
                    break;
                default:
                    convertIntValue(res, *(int *)mP->inOutStruct,r);
            }
            return 0;
	} else if (strcmp(mP->type, STR_MS_T) == 0) { /* if the parameter is a string */
		convertStrValue(res, (char *)mP->inOutStruct,r);
		return 0;
	} else if(strcmp(mP->type, DATETIME_MS_T) == 0) {
		res->value.t = *(time_t *)mP->inOutStruct;
		TYPE(res) = T_DATETIME;
		return 0;
/*
	} else if(strcmp(mP->type, StrArray_MS_T) == 0) {
		convertCollectionToRes(mP, res);
		return 1;
	} else if(strcmp(mP->type, IntArray_MS_T) == 0) {
		convertCollectionToRes(mP, res);
		return 1;
	} else if(strcmp(mP->type, GenQueryOut_MS_T) == 0) {
		convertCollectionToRes(mP, res);
		return 1;
*/
	} else {
            res->value.uninterpreted.inOutStruct = mP->inOutStruct;
            res->value.uninterpreted.inOutBuffer = mP->inpOutBuf;
            res->exprType = newIRODSType(mP->type,r);
            return 0;
        }
	return -1;


}
/************************ Microservice parameter type to ExprType ***********************/
ExprType *convertToExprType(char *type, Region *r) {
	if (strcmp(type, DOUBLE_MS_T) == 0) { /* if the parameter is an integer */
		return newSimpType(T_DOUBLE, r);
	} else if (strcmp(type, INT_MS_T) == 0) { /* if the parameter is an integer */
		return newSimpType(T_INT, r);
	} else if (strcmp(type, STR_MS_T) == 0) { /* if the parameter is a string */
		return newSimpType(T_STRING, r);
/*
	} else if(strcmp(type, DATETIME_MS_T) == 0) {
		return newSimp(STRING);
*/
	} else if(strcmp(type, StrArray_MS_T) == 0) {
		return newCollType(newSimpType(T_STRING, r), r);
	} else if(strcmp(type, IntArray_MS_T) == 0) {
		return newCollType(newSimpType(T_INT, r), r);
	} else if(strcmp(type, GenQueryOut_MS_T) == 0) {
		return newCollType(newIRODSType(KeyValPair_MS_T, r), r);
	} else {
		return newIRODSType(type, r);
	}
	return NULL;
}

/*
void convertCollectionToRes(msParam_t *mP, Res* res) {
	int i =0;
	char *typ = mP->type;
	void *inPtr = mP->inOutStruct;
	int len = getCollectionSize(typ, inPtr);
        char buf[1024];
        sprintf(buf, "%d\n", len);
        writeToTmp("relog.txt", buf);
	ExprType *elemType = convertToExprType(typ);
	setCollRes(res, len, elemType);
	freeType(elemType);
	for(i=0;i<len;i++) {
		res->value.c.elems[i]=getValueFromCollection(typ, inPtr, i);
	}
}
*/

/************************ Res to Microservice parameter ***********************/
int convertResToMsParam(msParam_t *var, Res *res, rError_t *errmsg) {
	strArray_t *arr;
	intArray_t *arr2;
        var->inpOutBuf = NULL;
        var->label = NULL;
            int i;
            int maxlen = 0;
			switch(TYPE(res)) {
				case T_ERROR: /* error message */
					var->inOutStruct = (int *)malloc(sizeof(int));
					*((int *)var->inOutStruct) = res->value.e;
					var->type = strdup(INT_MS_T);
					break;
				case T_DOUBLE: /* number */
					var->inOutStruct = (double *)malloc(sizeof(double));
					*((double *)var->inOutStruct) = res->value.d;
					var->type = strdup(DOUBLE_MS_T);
					break;
				case T_INT: /* number */
					var->inOutStruct = (int *)malloc(sizeof(int));
					*((int *)var->inOutStruct) = (int)res->value.d;
					var->type = strdup(INT_MS_T);
					break;
				case T_STRING: /* string */
					var->inOutStruct = strdup(res->text);
					var->type = strdup(STR_MS_T);
					break;
				case T_DATETIME: /* date time */
					/*var->inOutStruct = (time_t *)malloc(sizeof(time_t)); */
					/**((time_t *)var->inOutStruct) = res->value.t; */
					/*var->type = strdup(DATETIME_MS_T); */
                                    /* Here we pass datatime as an integer to reuse exiting packing instructions. Need to change to long int. */
					var->inOutStruct = (int *)malloc(sizeof(int));
					*((int *)var->inOutStruct) = (int)res->value.t;
					var->type = strdup(INT_MS_T);
					break;
				case T_CONS:
                                    if(strcmp(T_CONS_TYPE_NAME(res->exprType), LIST) == 0) {
					switch(T_CONS_TYPE_ARG(res->exprType, 0)->type) {
						case T_STRING:
							arr = (strArray_t *)malloc(sizeof(strArray_t));
							arr->len = res->degree;
							int i;
							maxlen = 0;
							for(i=0;i<res->degree;i++) {
								int slen = res->subtrees[i]->value.s.len;
								maxlen = maxlen < slen? slen: maxlen;
							}
							arr->size = maxlen;
							arr->value = (char *)malloc(sizeof(char)*maxlen*(arr->len));
							for(i=0;i<res->degree;i++) {
								strcpy(arr->value + maxlen*i, res->subtrees[i]->text);
							}
                                                        var->inOutStruct = arr;
                                                        var->type = strdup(StrArray_MS_T);
							break;
						case T_INT:
							arr2 = (intArray_t *)malloc(sizeof(intArray_t));
							arr2->len = res->degree;
							arr2->value = (int *)malloc(sizeof(int)*(arr2->len));
							for(i=0;i<res->degree;i++) {
								arr2->value[i] = (int)res->subtrees[i]->value.d;
							}
                                                        var->inOutStruct = arr2;
                                                        var->type = strdup(IntArray_MS_T);
							break;
						case T_IRODS:
                					var->inOutStruct = res->value.uninterpreted.inOutStruct;
                					var->inpOutBuf = res->value.uninterpreted.inOutBuffer;
                                			var->type = strdup(KeyValPair_MS_T);
                                                	break;
                                                default:
                                                    /* current there is no existing packing instructions for arbitrary collection */
                                                    /* report error */
                                                    addRErrorMsg(errmsg, -1, "no packing instruction for arbitrary collection type");
                                                    return -1;
					}
                                    } else {
                                        addRErrorMsg(errmsg, -1, "no packing instruction for arbitrary constructed type");
                                        return -1;
                                    }
                                    break;
                            case T_IRODS:
                                var->inOutStruct = res->value.uninterpreted.inOutStruct;
                                var->inpOutBuf = res->value.uninterpreted.inOutBuffer;
                                var->type = strdup(res->exprType->text);
                                break;
                            default:
                                /*error */
                                addRErrorMsg(errmsg, -1, "no packing instruction for arbitrary type");
                                return -1;
			}
                        return 0;
}
int convertEnvToMsParamArray(msParamArray_t *var, Env *env, rError_t *errmsg, Region *r) {
    if(convertHashtableToMsParamArray(var, env->global, errmsg, r)==0 &&
    convertHashtableToMsParamArray(var, env->current, errmsg, r)==0) {
        return 0;
    } else {
        return -1;
    }
}
int convertHashtableToMsParamArray(msParamArray_t *var, Hashtable *env, rError_t *errmsg, Region *r) {
	int i;
        if(var->msParam == NULL) {
            var->len = 0;
            var->msParam = (msParam_t **) malloc(sizeof(msParam_t *)*(env->len));
        } else {
            var->msParam = (msParam_t **) realloc(var->msParam, sizeof(msParam_t *)*(var->len + env->len));
        }
	for(i=0;i<env->size;i++) {
		struct bucket *b = env->buckets[i];
		while(b!=NULL) {
			Res *res = (Res *)b->value;
			msParam_t *v = (msParam_t *) malloc(sizeof(msParam_t));
			int ret = convertResToMsParam(v, res, errmsg);
			v->label = strdup(b->key);
                        if(ret != 0) {
                            /* error */
                            /* todo free msParamArray */
                            return ret;
                        }
			var->msParam[var->len++] = v;
			b=b->next;
		}
	}
        return 0;
}
int convertMsParamArrayToEnv(msParamArray_t *var, Hashtable *env, rError_t *errmsg, Region *r) {
	int i;
	for(i=0;i<var->len;i++) {
		Res *res = newRes(r);
		int ret = convertMsParamToRes(var->msParam[i], res, errmsg, r);
                if(ret != 0) {
                    return ret;
                }
		insertIntoHashTable(env, var->msParam[i]->label, res);
	}
        return 0;
}

int updateMsParamArrayToEnv(msParamArray_t *var, Env *env, rError_t *errmsg, Region *r) {
	int i;
	for(i=0;i<var->len;i++) {
		Res *res = newRes(r);
		int ret = convertMsParamToRes(var->msParam[i], res, errmsg, r);
                if(ret != 0) {
                    return ret;
                }
		char *varName = var->msParam[i]->label;
                if(varName!=NULL) {
                    if(lookupFromHashTable(env->current, varName) != NULL) {
                        updateInHashTable(env->current, varName, res);
                    } else if(lookupFromHashTable(env->global, varName) != NULL){
                        updateInHashTable(env->global, varName, res);
                    } else {
                        insertIntoHashTable(env->current, varName, res);
                    }
                }
	}
        return 0;
}

/************* Res to String *********************/
char* convertResToString(Res *res0) {
    char *res;
	switch (TYPE(res0)) {
            case T_ERROR:
                res = (char *)malloc(sizeof(char)*1024);
			snprintf(res, 1024, "error %d", res0->value.e);
			return res;
            case T_INT:
            case T_DOUBLE:
                res = (char *)malloc(sizeof(char)*1024);
		    if(res0->value.d==(int)res0->value.d) {
				snprintf(res, 1024, "%d", (int)res0->value.d);
			} else {
				snprintf(res, 1024, "%f", res0->value.d);
			}
			return res;
            case T_STRING:
                if(res0->text == NULL) {
                    res = strdup("<null>");
                } else {
                    res = strdup(res0->text);
                }
			return res;
            case T_IRODS:
                res = (char *)malloc(sizeof(char)*1024);
                    if(strcmp(res0->exprType->text, KeyValPair_MS_T)==0) {
                        keyValPair_t *kvp = (keyValPair_t *) res0->value.uninterpreted.inOutStruct;
                        snprintf(res, 1024, "KeyValue[%d]:", kvp->len);
                        int i;
                        for(i=0;i<kvp->len;i++) {
                            snprintf(res + strlen(res), 1024 - strlen(res), "%s=%s;", kvp->keyWord[i],kvp->value[i]);
                        }

                    } else {
			snprintf(res, 1024, "<value>");
                    }
			return res;
            case T_BOOL:
                res = strdup(((int)res0->value.d)?"true":"false");
                return res;
            case T_CONS:
                res = (char *)malloc(sizeof(char)*1024);
                sprintf(res, "[");
                int i;
                for(i=0;i<res0->degree;i++) {
                    char *resElem = convertResToString(res0->subtrees[i]);
                    if(resElem == NULL) {
                        free(res);
                        return NULL;
                    } else {
                        snprintf(res+strlen(res), 1024-strlen(res), "%s%s", i==0?"":",",resElem);
                        free(resElem);
                    }
                }
                snprintf(res+strlen(res), 1024-strlen(res), "]");
                        return res;
            case T_DATETIME:
                        res = (char *)malloc(sizeof(char)*1024);
			ttimestr(res, 1024-1, "", &(res0->value.t));
                        return res;

            default:
                /*sprintf(res, "error: unsupported type %d", TYPE(res0)); */
                return NULL;
	}
}

/******************* print functions **********************/
void printMsParamArray(msParamArray_t *msParamArray, char *buf2) {
    char buf3[MAX_COND_LEN];
    sprintf(buf2, "len: %d\n", msParamArray->len);
    int i;
    for(i=0;i<msParamArray->len;i++) {
        msParam_t *mP = msParamArray->msParam[i];
        if(i!=0)strncat(buf2, ",", MAX_COND_LEN);
        strncat(buf2, mP->label, MAX_COND_LEN);
        strncat(buf2, "=", MAX_COND_LEN);
        if(mP->inOutStruct == NULL) {
            strncat(buf2, "<null>", MAX_COND_LEN);
        } else {
            if (strcmp(mP->type, DOUBLE_MS_T) == 0) { /* if the parameter is an integer */
                snprintf(buf3, MAX_COND_LEN, "%f:",*(double *)mP->inOutStruct);
            } else if (strcmp(mP->type, INT_MS_T) == 0) { /* if the parameter is an integer */
                snprintf(buf3, MAX_COND_LEN, "%d:",*(int *)mP->inOutStruct);
            } else if (strcmp(mP->type, STR_MS_T) == 0) { /* if the parameter is a string */
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
    return;
}

void printHashtable(Hashtable *env, char* buf2) {
    int i;
    sprintf(buf2, "len: %d\n", env->len);
    int k = 0;
    for (i = 0; i < env->size; i++) {
        struct bucket *b = env->buckets[i];
        while (b != NULL) {
            Res *res = (Res *) b->value;
            if (k != 0)strncat(buf2, ",", MAX_COND_LEN);
            strncat(buf2, b->key, MAX_COND_LEN);
            strncat(buf2, "=", MAX_COND_LEN);
            if (res == NULL) {
                strncat(buf2, "<null>", MAX_COND_LEN);
            } else {
                char *buf4 = convertResToString(res);
                strncat(buf2, buf4, MAX_COND_LEN);
                strncat(buf2, ":", MAX_COND_LEN);
                strncat(buf2, typeName_Res(res), MAX_COND_LEN);
                free(buf4);
            }
            k++;
            b = b->next;
        }
    }
}

int convertResToIntReturnValue(Res *res) {
    int retVal;
    if(TYPE(res) == T_ERROR) {
       retVal = res->value.e;
    } else {
       retVal = (int)res->value.d;
    }
    return retVal;

}
