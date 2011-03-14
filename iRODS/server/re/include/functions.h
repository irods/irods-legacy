/* For copyright information please refer to files in the COPYRIGHT directory
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "hashtable.h"
#include "region.h"
#include "parser.h"

#define MAX_FUNC_PARAMS 20
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
    void *(*func)();
    ExprType *type;
    struct function_desc *next;
} FunctionDesc ;

FunctionDesc *newFunctionDesc(/*int arity, */char *valueOrExpression, char* type, void *func, Region *r);
/*Hashtable *getSystemFunctionTypes(Region *r); */
void getSystemFunctions(Hashtable *ft, Region *r);

Res* eval(char *expr, Env *env, ruleExecInfo_t *rei, int saveREI, rError_t *errmsg, Region *r);

int getParamIOType(char *iotypes, int index);

FunctionDesc *getFuncDescFromChain(int n, FunctionDesc *fDesc);

char *errMsgToString(rError_t *errmsg, char *buf, int buflen /* = 0 */);

#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */

