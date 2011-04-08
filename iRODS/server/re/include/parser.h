/* For copyright information please refer to files in the COPYRIGHT directory
 */


#ifndef PARSER_H
#define PARSER_H
#include "debug.h"
#include "region.h"
#include "hashtable.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEBUG
#include "objInfo.h"
#include "reHelpers1.h"
#endif

#define MAX_FUNC_PARAMS 20

#define MAX_PREC 10
#define MIN_PREC 0

typedef struct op {
    char* string;
    int arity;
    int prec;
} Op;

extern int num_ops;
extern Op new_ops[];

#define POINTER_BUF_SIZE 128
typedef struct pointer {
	FILE *fp; /* file */
	char buf[POINTER_BUF_SIZE]; /* buffer */
	int len; /* len of string in buf */
	int p; /* pointer to next char in buf */
	long fpos; /* position of the beginning of the buffer in file */
        int strp; /* pointer to next char in strbuf */
        char *strbuf; /* string buffer */
        int isFile;
        char *base; /* core, app, or empty string */
} Pointer;


/** utility functions */
/**
 * create a new node n, copy text[0..len-1] to the n.text
 */
void setBase(Node *node, char *base, Region *r);
Node **setDegree(Node *node, int d, Region *r);
Node *createUnaryFunctionNode(char *fn, Node *a, Label * exprloc, Region *r);
Node *createBinaryFunctionNode(char *fn, Node *a, Node *b, Label * exprloc, Region *r);
Node *createFunctionNode(char *fn, Node **params, int paramsLen, Label * exprloc, Region *r);
Node *createActionsNode(Node **params, int paramsLen, Label * exprloc, Region *r);
Node *createTextNode(char *t, Label * exprloc, Region *r);
Node *createNumberNode(char *t, Label * exprloc, Region *r);
Node *createStringNode(char *t, Label * exprloc, Region *r);
Node *createErrorNode(char *error, Label * exprloc, Region *r);

void nextTokenRuleGen(Pointer* expr, Token* token, int rulegen /* = rulegen */);
int nextString(Pointer *e, char *value, int vars[]);
int nextString2(Pointer *e, char *value, int vars[]);
int eol(char ch);
int isOp(char *token);
int isUnaryOp(Token* token);
int getPrecedence(Token* token);
void getCoor(Pointer *p, Label * errloc, int coor[2]);

/**
 * skip a token of type TEXT and text text, token will has type ERROR if the token does not match
 */
int skip(Pointer *expr, char *text, Token *token, int rulegen /* = rulegen */);
void skipWhitespace(Pointer *expr);
char *findLineCont(char *expr);

/**
 * Parse a rule, create node and store their ref in nodeRef.
 * If error, either nodeRef==NULL or nodeRef->type==ERROR.
 */
void parseRuleRuleGen(Pointer *expr, Node **nodeRef, int* numberOfRules, int backwardCompatible, rError_t *errmsg, Region *r);
void parseRuleName(Pointer *expr, Node **name, Region *r);
void parseRuleActions(Pointer *expr, Node **name, int rulegen, int backwardCompatible, Node** node2, rError_t *errmsg, Region *r);
void nextTermRuleGen(Pointer *expr, Node **node, int prec, int rulegen, rError_t *errmsg, Region *r);
void nextValueRuleGen(Pointer *expr, Node** node, int rulegen, rError_t *errmsg, Region *r);
void pushback(Pointer *e, Token *token);
void initPointer(Pointer *p, FILE* fp, char* ruleBaseName /* = NULL */);
void initPointer2(Pointer *p, char* buf);
Pointer *newPointer(FILE* buf, char *ruleBaseName);
Pointer *newPointer2(char* buf);
void deletePointer(Pointer* buf);

void skipComments(Pointer *e);
int nextChar(Pointer *p);
int lookAhead(Pointer *p, int n);

char* trim(char* str);
void trimquotes(char *string);
int isLocalVariableNode(Node *node);
int isSessionVariableNode(Node *node);
int isVariableNode(Node *node);

char *nextRuleSection(char *expr, char* section);
char *parseFunctionParameters(char *e, char *args[], int *argc);
char *functionParameters(char *expr, char* param);
ExprType *parseFuncTypeFromString(char *string, Region *r);

Label *getFPos(Label *label, Pointer *p);
void clearBuffer(Pointer *p);
void seekInFile(Pointer *p, long x);
void nextChars(Pointer *p, int len);

int dupLine(Pointer *p, Label * start, int n, char *buf);
int dupString(Pointer *p, Label * start, int n, char *buf);

StringList *getVarNamesInExprNode(Node *expr, Region *r);
StringList *getVarNamesInExprNodeAux(Node *expr, StringList* varnames, Region *r);
int eqExprNodeSyntactic(Node *a, Node *b);

int nextStringBase(Pointer *e, char *value, char* delim, int consumeDelim, char escape, int vars[]);
void convertStringToExpression(Token *token, char *base, Node **node, Region *r);
void nextActionBackwardCompatible(Pointer *e, Node **node, rError_t *errmsg, Region *r);
void nextActionArgumentStringBackwardCompatible(Pointer *e, Token *token);
void parseActionArgumentBackwardCompatible(Pointer *e, Node **node, rError_t *errmsg, Region *r);

char* typeName_Res(Res *s);
char* typeName_ExprType(ExprType *s);
char* typeName_NodeType(NodeType s);
void printTree(Node *n, int indent);
void printIndent(int indent);

char *getRuleBasePath(char *ruleBaseName, char rulesFileName[MAX_NAME_LEN]);
void generateErrMsgFromFile(char *msg, long errloc, char *ruleBaseName, char* ruleBasePath, char errbuf[ERR_MSG_LEN]);
void generateErrMsgFromSource(char *msg, long errloc, char *src, char errbuf[ERR_MSG_LEN]);
void generateErrMsgFromPointer(char *msg, Label *l, Pointer *e, char errbuf[ERR_MSG_LEN]);
char *generateErrMsg(char *msg, long errloc, char* ruleBaseName, char errbuf[ERR_MSG_LEN]);

#endif
