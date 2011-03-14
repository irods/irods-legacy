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
#define STRING_OVERFLOW -1205011
/* system error -1207000 */
#define OUT_OF_MEMORY -1207001
#define UNKNOWN_ERROR -1207000
/* type error -1209000 */
#define TYPE_ERROR -1209000

#define DATETIME_MS_T "DATETIME_MS_T"


#define TYPE(x) ((x)->type->t)

#define T_CONS_TYPE_ARG(x, n) ((x)->ext.cons.typeArgs[n])
#define T_CONS_TYPE_NAME(x) ((x)->ext.cons.typeConsName)
#define T_FUNC_PARAM_TYPE(x, n) ((x)->ext.func.paramsType[n])
#define T_FUNC_RET_TYPE(x) ((x)->ext.func.retType)
#define T_VAR_ID(x) ((x)->ext.tvar.vid)
#define T_VAR_DISJUNCT(x, n) ((x)->ext.tvar.disjuncts[n])
#define T_VAR_NUM_DISJUNCTS(x) ((x)->ext.tvar.numDisjuncts)
#define T_FUNC_ARITY(x) ((x)->ext.func.arity)
#define T_CONS_ARITY(x) ((x)->ext.cons.arity)

#define LIST "[]"

extern int typeNameLen;
extern char* typeName[];

typedef enum type_constructor {
        T_DYNAMIC = -1,
	T_ERROR = 0,
	T_DOUBLE = 1,
        T_INT = 2,
	T_STRING = 3,
	T_DATETIME = 4,
	T_BOOL = 5,
	T_FUNC = 6,
	T_CONS = 9,
	T_BREAK = 30,
	T_SUCCESS = 31,
	T_VAR = 100,
        T_IRODS = 200
} TypeConstructor;
typedef struct expr_type ExprType;
typedef ExprType *ExprTypePtr;
typedef struct bucket Bucket;
typedef Bucket *BucketPtr;

/* variable type */
struct vtype {
		int vid;
		int numDisjuncts;
		TypeConstructor disjuncts[100];
/*                TypeConstructor lowerBound; */
};
/* constructed type */
struct cons_type {
		char typeConsName[10]; /* constructor */
		int arity;
		ExprType** typeArgs;
};
struct irods_type {
        char name[100];
};
enum vararg {
    ONCE = 0,
    STAR = 1,
    PLUS = 2
};
struct func_type {
    int arity;
    ExprType **paramsType;
    ExprType *retType;
    enum vararg vararg;
};
union expr_type_ext {
    struct vtype tvar;
    struct cons_type cons;
    struct irods_type irods;
    struct func_type func;
};
/* expression type */
struct expr_type {
    TypeConstructor t;
    int coercionAllowed;
    union expr_type_ext ext;
};
/* value */
typedef struct res Res;
struct coll {
	int len;
	Res** elems;
};
struct string {
    int len;
    char *pointer;
};
union res_value {
    int e;
    double d;
    struct string s;
    time_t t;
    struct coll c;
    struct {
        void *inOutStruct;
        bytesBuf_t *inOutBuffer;
    } uninterpreted;
};

struct res {
	ExprType* type;
	union res_value value;
};

typedef struct str_list StringList;
struct str_list {
    char *str;
    struct str_list *next;
};

typedef struct env Env;
struct env {
    Hashtable *current;
    Hashtable *global;
    Hashtable *funcDesc;
};

typedef enum node_type {
	EOS = -1,
	ERROR = 0,
	DOUBLE = 1,
        INT = 11,
	TEXT = 2,
	STRING = 3,
	DATETIME = 4,
	APPLICATION = 5,
	ACTIONS = 6,
	RULE = 7,
	BOOL = 8,
	CONS = 9,
	RULESET = 10,
	TVAR = 100,
} NodeType;

/* todo change text of dynamically allocated array */
#define MAX_NODE_TEXT_LEN 1023

typedef struct label {
    long exprloc;
    char *base;
} Label;
typedef struct token {
	NodeType type;
	char text[MAX_NODE_TEXT_LEN+1];
        int vars[100];
        long exprloc;
} Token;
typedef struct node {
	NodeType type; /* syntax type */
	ExprType *exprType; /* expression type */
        ExprType *coercion; /* coersed type */
	char text[MAX_NODE_TEXT_LEN+1];
	long expr;
	int degree;
	struct node **subtrees;
        int typed;
        char *base;
} Node;

typedef Node *NodePtr;
typedef char *charPtr;

typedef struct typingConstraint TypingConstraint;
typedef enum typingConstraintType TypingConstraintType;

enum typingConstraintType {
    LT
};

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
ExprType *newTVar();
ExprType *newCollType(ExprType *elemType, Region *r);
ExprType *newFuncType(int arity, ExprType **paramTypes, ExprType *elemType, Region *r);
ExprType *newFuncTypeVarArg(int arity, enum vararg vararg, ExprType **paramTypes, ExprType *elemType, Region *r);
ExprType *newConsType(int arity, char *cons, ExprType **paramTypes, Region *r);
ExprType *newSimpType(TypeConstructor t, Region *r);
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

ExprType *dupTypeAux(ExprType *ty, Region *r, Hashtable *varTable, Region *keyRegion);
Res *cpRes(Res *res, Region *r);
char *cpString(char *res, Region *r);
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

Env *newEnv(Hashtable *current, Hashtable *gloabl, Hashtable *funcDesc);
void deleteEnv(Env *env, int deleteCurrent);

List *newList(Region *r);
ListNode *newListNode(void *value, Region *r);
void listAppend(List *list, void *value, Region *r);
void listAppendToNode(List *list, ListNode *node, void *value, Region *r);
void listRemove(List *list, ListNode *node);

TypingConstraint *newTypingConstraint(ExprType *a, ExprType *b, TypingConstraintType type, Node *node, Region *r);

char *getRuleBasePath(char *ruleBaseName, char rulesFileName[MAX_NAME_LEN]);
void generateErrMsgFromFile(char *msg, long errloc, char *ruleBaseName, char* ruleBasePath, char errbuf[ERR_MSG_LEN]);
char *generateErrMsg(char *msg, long errloc, char* ruleBaseName, char errbuf[ERR_MSG_LEN]);

int appendToByteBufNew(bytesBuf_t *bytesBuf, char *str);
void logErrMsg(rError_t *errmsg);
char *errMsgToString(rError_t *errmsg, char *buf, int buflen);

#endif
