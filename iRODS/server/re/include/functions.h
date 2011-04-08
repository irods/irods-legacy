/* For copyright information please refer to files in the COPYRIGHT directory
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#include "hashtable.h"
#include "region.h"
#include "parser.h"

typedef Res *(*SmsiFuncPtrType)(Node **, int, Node *, ruleExecInfo_t *, int, Env *, rError_t *, Region *);
typedef struct function_desc {
    /*int arity; */
    char inOutValExp[10];
    /* a string indicating whether a value or an expression should be passed in or out */
    /* each char represents one param, */
    /* a = actions, */
    /* e = expresion, */
    /* i = input value */
    /* o = output variable */
    /* p = i/o variable */
    /* * = repeat previous */
    SmsiFuncPtrType func;
    ExprType *type;
    struct function_desc *next;
} FunctionDesc ;

FunctionDesc *newFunctionDesc(/*int arity, */char *valueOrExpression, char* type, SmsiFuncPtrType func, Region *r);
/*Hashtable *getSystemFunctionTypes(Region *r); */
void getSystemFunctions(Hashtable *ft, Region *r);

Res* eval(char *expr, Env *env, ruleExecInfo_t *rei, int saveREI, rError_t *errmsg, Region *r);

int getParamIOType(char *iotypes, int index);

FunctionDesc *getFuncDescFromChain(int n, FunctionDesc *fDesc);

#endif	/* FUNCTIONS_H */

