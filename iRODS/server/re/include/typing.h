/* For copyright information please refer to files in the COPYRIGHT directory
 */

#ifndef TYPING_H
#define TYPING_H

#include "utils.h"
#include "hashtable.h"
#include "region.h"
#include "parser.h"

typedef enum satisfiability {
    TAUTOLOGY = 1, CONTINGENCY = 2, ABSURDITY = 4
} Satisfiability;

ExprType * typeExpression3(Node *expr, Hashtable *funcDesc, Hashtable *varTypes, List *typingConstraints, rError_t* errmsg, Node **errnode, Region *r);
int typeFuncParam(Node *param, Node *paramType, Node *formalParamType, Hashtable *var_type_table, List *typingConstraints, rError_t *errmsg, Region *r);
void postProcessActions(Node *expr, Hashtable *systemFunctionTables, rError_t *errmsg, Node **errnode, Region *r);
void postProcessCoercion(Node *expr, Hashtable *varTypes, rError_t* errmsg, Node **errnode, Region *r);
int tautologyLtBase(ExprType *a, ExprType *b);
int tautologyLt(ExprType *type, ExprType *expected);
Satisfiability simplifyR(ExprType *type, ExprType *expected, ExprType **bn, Region *r);
Satisfiability simplifyL(ExprType *type, ExprType *expected, ExprType **an, Region *r);
Satisfiability narrow(ExprType *type, ExprType *expected, ExprType **an, ExprType **bn, Region *r);
Satisfiability simplify(List *typingConstraints, Hashtable *typingEnv, rError_t* errmsg, Node **errnode, Region *r);
Satisfiability simplifyLocally(TypingConstraint *typingConstraints, Hashtable *typingEnv, Region *r);
int solveConstraints(List *typingConstraints, Hashtable *typingEnv, rError_t* errmsg, Node **errnode, Region *r);
#endif /* TYPING_H */
