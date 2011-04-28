/* For copyright information please refer to files in the COPYRIGHT directory
 */


#ifndef UTILS_H
#define UTILS_H
#include "debug.h"
#include "region.h"
#include "hashtable.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEBUG
#include "objInfo.h"
#include "reHelpers1.h"
#endif

#define MAX_PARAMS_LEN 100
#define MAX_ERRMSG_LEN (2*1024)

/* parser error -1203000 */
#define PARSER_ERROR -1203000
#define UNPARSED_SUFFIX -1203001
#define POINTER_ERROR -1203002
/* runtime error -1205000 */
#define RUNTIME_ERROR -1205000
#define DIVISION_BY_ZERO -1205001
#define BUFFER_OVERFLOW -1205002
#define UNSUPPORTED_OP_OR_TYPE -1205003
#define UNSUPPORTED_SESSION_VAR -1205004
#define UNABLE_TO_WRITE_LOCAL_VAR -1205005
#define UNABLE_TO_READ_LOCAL_VAR -1205006
#define UNABLE_TO_WRITE_SESSION_VAR -1205007
#define UNABLE_TO_READ_SESSION_VAR -1205008
#define UNABLE_TO_WRITE_VAR -1205009
#define UNABLE_TO_READ_VAR -1205010
#define PATTERN_NOT_MATCHED -1205011
#define STRING_OVERFLOW -1205012
/* system error -1207000 */
#define OUT_OF_MEMORY -1207001
#define UNKNOWN_ERROR -1207000
/* type error -1209000 */
#define TYPE_ERROR -1209000
#define FUNCTION_REDEFINITION -12090001

#define DATETIME_MS_T "DATETIME_MS_T"


#define TYPE(x) ((x)->exprType->nodeType)

#define T_CONS_TYPE_ARGS(x) ((x)->subtrees)
#define T_CONS_TYPE_ARG(x, n) ((x)->subtrees[n])
#define T_CONS_TYPE_NAME(x) ((x)->text)
#define T_CONS_ARITY(x) ((x)->degree)
#define T_CONS_VARARG(x) ((x)->vararg)
#define T_FUNC_PARAM_TYPE(x, n) ((x)->subtrees[0]->subtrees[n])
#define T_FUNC_RET_TYPE(x) ((x)->subtrees[1])
#define T_FUNC_ARITY(x) ((x)->subtrees[0]->degree)
#define T_FUNC_VARARG(x) ((x)->subtrees[0]->vararg)
#define T_VAR_ID(x) ((x)->value.vid)
#define T_VAR_DISJUNCT(x, n) ((x)->subtrees[n])
#define T_VAR_DISJUNCTS(x) ((x)->subtrees)
#define T_VAR_NUM_DISJUNCTS(x) ((x)->degree)

#define LIST "[]"
#define TUPLE "<>"
#define ST_TUPLE "()"
#define FUNC "->"

typedef struct node Node;
typedef struct node ExprType;
typedef struct node Res;
typedef ExprType *ExprTypePtr;
typedef struct bucket Bucket;
typedef Bucket *BucketPtr;

typedef enum node_type {
    TK_EOS = -1,
    N_ERROR = 0,
    TK_INT = 1,
    TK_DOUBLE = 2,
    TK_TEXT = 3,
    TK_STRING = 4,
    TK_BOOL = 5,
    TK_BACKQUOTED = 6,
    TK_DATETIME = 7, /* unused */
    TK_LOCAL_VAR = 10,
    TK_SESSION_VAR = 11,
    TK_OP = 12,
    TK_MISC_OP = 14,
    N_APPLICATION = 20,
    N_ACTIONS = 21,
    N_ACTIONS_RECOVERY = 22,
    N_RULE = 30,
    N_RULE_NAME = 31,
    N_PARAM_LIST = 32,
    N_PARAM_TYPE_LIST = 33,
    N_RULESET = 34,
    N_RULE_PACK = 35,
    N_VAL = 28,
    T_DYNAMIC = 100,
    T_UNSPECED = 101,
    T_ERROR = 200,
    T_DOUBLE = 201,
    T_INT = 202,
    T_STRING = 203,
    T_DATETIME = 204,
    T_BOOL = 205,
    T_CONS = 209,
    T_BREAK = 230,
    T_SUCCESS = 231,
    T_VAR = 300,
    T_IRODS = 400,
    T_TYPE = 500,
} NodeType;

enum vararg {
    ONCE = 0,
    STAR = 1,
    PLUS = 2
};

typedef struct env Env;

/* value */
union node_ext {
    int errcode;
    int vid;
    int strlen;
    double dval;
    time_t tval;
    struct {
        void *inOutStruct;
        bytesBuf_t *inOutBuffer;
    } uninterpreted;
};

typedef struct str_list StringList;
struct str_list {
    char *str;
    struct str_list *next;
};

struct env {
    Hashtable *current;
    Hashtable *funcDesc;
    Env *previous;
};


/* todo change text of dynamically allocated array */
#define MAX_TOKEN_TEXT_LEN 1023

typedef struct label {
    long exprloc;
    char *base;
} Label;
typedef struct token {
    NodeType type;
    char text[MAX_TOKEN_TEXT_LEN+1];
    int vars[100];
    long exprloc;
} Token;
struct node {
    NodeType nodeType; /* node type */
    ExprType *exprType; /* expression type */
    ExprType *coercionType; /* coercion type */
    char *text;
    long expr;
    int degree;
    enum vararg vararg; /* when this node represents a type or a pattern, this field indicates whether the trailing subtree represents varargs */
    struct node **subtrees;
    int typed;
    char *base;
    int coercionAllowed;
    union node_ext value;
};

typedef Node *NodePtr;
typedef char *charPtr;

typedef struct typingConstraint TypingConstraint;
enum typingConstraintType {
    LT
};

typedef enum typingConstraintType TypingConstraintType;

struct typingConstraint {
   ExprType *a;
   ExprType *b;
   TypingConstraintType constraintType;
   Node *node;
   TypingConstraint *next;
};

typedef struct list List;
typedef struct listNode ListNode;

struct list {
    ListNode *head;
    ListNode *tail;
};

struct listNode {
    ListNode *next;
    void *value;
};

char* getTVarName(int vid, char name[128]);
char* getTVarNameRegion(int vid, Region *r);
char* getTVarNameRegionFromExprType(ExprType *tvar, Region *r);

/** type functions
 */
extern int tvarNumber;
ExprType *dupType(ExprType *ty, Region *r);
int typeEqSyntatic(ExprType *a, ExprType *b);

Node *newNode(NodeType type, char* text, Label * exprloc, Region *r);
Node *newExprType(NodeType t, int degree, Node **subtrees, Region *r);
int newTVarId();
ExprType *newTVar(Region *r);
ExprType *newTVar2(int numDisjuncts, Node **disjuncts, Region *r);
ExprType *newCollType(ExprType *elemType, Region *r);
ExprType *newTupleType(int arity, ExprType **typeArgs, Region *r);
ExprType *newFuncType(int arity, ExprType **paramTypes, ExprType *elemType, Region *r);
ExprType *newFuncTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, ExprType *elemType, Region *r);
ExprType *newConsType(int arity, char *cons, ExprType **paramTypes, Region *r);
ExprType *newConsTypeVarArg(int arity, enum vararg vararg, char *cons, ExprType **paramTypes, Region *r);
ExprType *newSimpType(NodeType t, Region *r);
ExprType *newErrorType(int errcode, Region *r);
ExprType *newIRODSType(char *name, Region *r);
ExprType *newStringPair();
void setStringPair(ExprType *ty, Region *r);
void replace(Hashtable *varTypes, int a, ExprType *b);
void replaceCons(ExprType *consType, int a, ExprType *b);
ExprType* dereference(ExprType *type, Hashtable *tt, Region *r);
ExprType *instantiate(ExprType *type, Hashtable *type_table, Region *r);

/** Res functions */
Res* newRes(Region *r);
Res* newIntRes(Region *r, int n);
Res* newDoubleRes(Region *r, double a);
Res* newBoolRes(Region *r, int n);
Res* newErrorRes(Region *r, int errcode);
/* precond: len(s) < size of res1->value.s */
Res* newStringRes(Region *r, char *s);
Res* newDatetimeRes(Region *r, long dt);
Res* newCollRes(int size, ExprType *elemType, Region *r);
Res* newUninterpretedRes(Region *r, char *typeName, void *ioStruct, bytesBuf_t *ioBuf);

ExprType *dupTypeAux(ExprType *ty, Region *r, Hashtable *varTable);
Res *cpRes(Res *res, Region *r);
char *cpString(char *res, Region *r);
char *cpStringExt(char* str, Region* r);
ExprType *cpType(ExprType *type, Region *r);
void cpHashtable(Hashtable *env, Region *r);
void cpEnv(Env *env, Region *r);
Res *setVariableValue(char *varName, Res *val, ruleExecInfo_t *rei, Env *env, rError_t *errmsg, Region *r);
ExprType* unifyWith(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r);
ExprType* unifyNonTvars(ExprType *type, ExprType *expected, Hashtable *varTypes, Region *r);
ExprType* unifyTVarL(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r);
ExprType* unifyTVarR(ExprType *type, ExprType* expected, Hashtable *varTypes, Region *r);

void printType(ExprType *type, Hashtable *var_types);
char *typeToString(ExprType *type, Hashtable *var_types, char *buf, int bufsize);

msParamArray_t *newMsParamArray();
void deleteMsParamArray(msParamArray_t *msParamArray);

/** debugging functions */
/*void errorInsert(char *errmsg, char *insert); */
int writeToTmp(char *fileName, char *text);
void printMsParamArray(msParamArray_t *msParamArray, char *buf2);
void printHashtable(Hashtable *env, char* buf2);
void printVarTypeEnvToStdOut(Hashtable *env);

Env *newEnv(Hashtable *current, Env *previous, Hashtable *funcDes);
void deleteEnv(Env *env, int deleteCurrent);
void *lookupFromEnv(Env *env, char *key);
void updateInEnv(Env *env, char *varname, Res *res);
void freeEnvUninterpretedStructs(Env *e);

List *newList(Region *r);
ListNode *newListNode(void *value, Region *r);
void listAppend(List *list, void *value, Region *r);
void listAppendToNode(List *list, ListNode *node, void *value, Region *r);
void listRemove(List *list, ListNode *node);

TypingConstraint *newTypingConstraint(ExprType *a, ExprType *b, TypingConstraintType type, Node *node, Region *r);

int appendToByteBufNew(bytesBuf_t *bytesBuf, char *str);
void logErrMsg(rError_t *errmsg);
char *errMsgToString(rError_t *errmsg, char *buf, int buflen);

int isPattern(Node *pattern);

#define CONCAT2(a,b) a##b
#define CONCAT(a,b) CONCAT2(a,b)


#endif
