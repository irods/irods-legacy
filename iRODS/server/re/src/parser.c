/* For copyright information please refer to files in the COPYRIGHT directory
 */

#include <stdlib.h>
#include "parser.h"
#include "rules.h"
#define ERROR(x) if(x) { goto error; }
int num_ops = 28;
Op new_ops[] = {
    {"-",1,10},
    {"++",2,6},
    {"+",2,6},
    {"-",2,6},
    {"*",2,7},
    {"/",2,7},
    {"&&",2,3},
    {"%%",2,2},
    {"%",2,7},
    {"<=",2,5},
    {">=",2,5},
    {"<",2,5},
    {">",2,5},
    {"==",2,4},
    {"!=",2,4},
    {"!",1,10},
    {"like regex",2,8},
    {"not like regex",2,8},
    {"like",2,8},
    {"not like",2,8},
    {"^",2,8},
    {"@",2,8},
    {"floor",1,10},
    {"ceiling",1,10},
    {"log",1,10},
    {"exp",1,10},
    {"abs",1,10},
    {"=",2,1}
};
int typeNameLen = 10;
char* typeName[] = {"ERROR", "DOUBLE", "INT", "STRING", "DATETIME","BOOL","FUNC",NULL,NULL,"CONS"};

/***** utility functions *****/
/**
 * create a new node n, copy text[0..len-1] to the n.text
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
void setBase(Node *node, char *base, Region *r) {
/*    if(strcmp(base, "test")!=0) { */
/*        printf("error"); */
/*    } */
        node->base= (char *)region_alloc(r, sizeof(char)*(strlen(base)+1));
        strcpy(node->base, base);

}
/**
 * set the degree of a node and allocate subtrees array
 */
Node **setDegree(Node *node, int d, Region *r) {
	node->degree = d;
	node->subtrees = (Node **)region_alloc(r,d*sizeof(Node *));
	if(node->subtrees==NULL) {
		return NULL;
	}
	return node->subtrees;
}
/*
Node *dupNode(Node *node, Region* r) {
    Node *dup = newNode(node->type, node->text, node->expr, r);
    Node **subdup = setDegree(dup, node->degree, r);
    int i;
    for(i = 0;i<node->degree;i++) {
        subdup[i] = dupNode(node->subtrees[i],r);
    }
    return dup;
}*/
Node *createUnaryFunctionNode(char *fn, Node *a, Label * expr, Region *r) {
	Node *node = newNode(APPLICATION, fn, expr, r);
	if(node == NULL) {
		return NULL;
	}
	setDegree(node, 1,r);
	node->subtrees[0] = a;
	return node;
}
Node *createBinaryFunctionNode(char *fn, Node *a, Node *b, Label * expr, Region *r) {
	Node *node = newNode(APPLICATION, fn, expr,r);
	if(node == NULL) {
		return NULL;
	}
	setDegree(node, 2, r);
	node->subtrees[0] = a;
	node->subtrees[1] = b;
	return node;
}
Node *createFunctionNode(char *fn, Node **params, int paramsLen, Label * exprloc, Region *r) {
	Node *node = newNode(APPLICATION, fn, exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	setDegree(node, paramsLen, r);
	memcpy(node->subtrees, params, paramsLen*sizeof(Node *));
	return node;
}
Node *createActionsNode(Node **params, int paramsLen, Label * exprloc, Region *r) {
	Node *node = newNode(ACTIONS, "ACTIONS", exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	setDegree(node, paramsLen,r);
	memcpy(node->subtrees, params, paramsLen*sizeof(Node *));
	return node;
}
Node *createTextNode(char *t, Label * exprloc, Region *r) {
	Node *node = newNode(TEXT, t, exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	return node;
}
Node *createIntNode(char *t, Label * exprloc, Region *r) {
	Node *node = newNode(INT, t, exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	return node;
}
Node *createDoubleNode(char *t, Label * exprloc, Region *r) {
	Node *node = newNode(DOUBLE, t, exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	return node;
}
Node *createStringNode(char *t, Label * exprloc, Region *r) {
	Node *node = newNode(STRING, t, exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	return node;
}
Node *createErrorNode(char *error, Label * exprloc, Region *r) {
	Node *node = newNode(ERROR, error, exprloc, r);
	if(node == NULL) {
		return NULL;
	}
	return node;
}
int isLocalVariableNode(Node *node) {
    return
        node->type==TEXT &&
        node->text[0] == '*';
}
int isSessionVariableNode(Node *node) {
    return
        node->type==TEXT &&
        node->text[0] == '$';
}
int isVariableNode(Node *node) {
    return
        isLocalVariableNode(node) ||
        isSessionVariableNode(node);
}
void skipWhitespace(Pointer *expr) {
	char ch;
	ch = lookAhead(expr, 0);
	while (ch!=-1 && (ch==' ' || ch=='\t' || ch=='\r' || ch=='\n')) {
		ch = nextChar(expr);
	}
}
void skipComments(Pointer *e) {
    char ch = lookAhead(e, 0);
    while(ch!='\n') {
        ch = nextChar(e);
    }
}
char *findLineCont(char *expr) {
	char *e = expr + strlen(expr);
	while (e!=expr) {
		e--;
		if(*e==' ' || *e=='\t' || *e=='\r' || *e=='\n') continue;
		if(*e=='\\' || *e==',' || *e==';' || *e=='|' || *e=='#') return e;
		break;
	}
	return NULL;
}

/**
 * skip a token of type TEXT and text text, token will has type ERROR if the token does not match
 * return 0 failure 1 success
 */
int skip(Pointer *e, char *text, Token *token, int rulegen /* = rulegen */) {
	nextTokenRuleGen(e, token, rulegen);
	if(token->type!=TEXT || strcmp(token->text,text)!=0) {
		token->type = ERROR;
		return 0;
	}
	return 1;
}

void nextTokenRuleGen(Pointer* e, Token* token, int rulegen /* = rulegen */) {
    while (1) {
        skipWhitespace(e);
        Label start;
        Label pos;
        getFPos(&start, e);
        token->exprloc = start.exprloc;
        char ch = lookAhead(e, 0);
        if (ch == -1) { /* reach the end of stream */
            token->type = EOS;
            strcpy(token->text, "EOS");
            break;
        } else {
            int i;
            if (ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == ',' || ch == '|' || ch == ';') {
                *(token->text) = ch;
                (token->text)[1] = '\0';
                token->type = TEXT;
                nextChar(e);
                break;
            } else if (ch == '#') {
                if (!rulegen && lookAhead(e, 1) == '#') {
                    token->text[0] = ch;
                    token->text[1] = lookAhead(e, 1);
                    token->text[2] = '\0';
                    token->type = TEXT;
                    nextChar(e);
                    nextChar(e);
                    break;
                } else {
                    skipComments(e);
                    continue;
                }
            } else if (ch == ':') {
                if (rulegen && lookAhead(e, 1) == ':'
                        && lookAhead(e, 2) == ':') {
                    token->text[0] = ch;
                    token->text[1] = lookAhead(e, 1);
                    token->text[2] = lookAhead(e, 2);
                    token->text[3] = '\0';
                    token->type = TEXT;
                    nextChar(e);
                    nextChar(e);
                } else {
                    *(token->text) = ch;
                    (token->text)[1] = '\0';
                    token->type = TEXT;
                }
                nextChar(e);
                break;
            } else if (ch == '*' || ch == '$') { /* variable */
                ch = nextChar(e);
                if (ch == '_' || isalpha(ch)) {
                    ch = nextChar(e);
                    while (ch == '_' || isalnum(ch)) {
                        ch = nextChar(e);
                    }
                    getFPos(&pos, e);
                    dupString(e, &start, (int) (pos.exprloc - start.exprloc), token->text);
                    token->type = TEXT;
                    break;
                } else {
                    seekInFile(e, start.exprloc);
                }
            }
            char op[100];
            dupString(e, &start, 10, op);
            int found = 0;
            for (i = 0; i < num_ops; i++) {
                int len = strlen(new_ops[i].string);
                if (strncmp(op, new_ops[i].string, len) == 0 &&
                        (!isalpha(new_ops[i].string[0])
                        || !isalnum(op[len]))) {
                    strcpy(token->text, new_ops[i].string);
                    token->type = TEXT;
                    nextChars(e, len);
                    found = 1;
                    break;
                }
            }
            if (found)
                break;
            if (isdigit(ch) || ch == '.') {
                ch = nextChar(e);
                while (isdigit(ch) || ch == '.') {
                    ch = nextChar(e);
                }
                    getFPos(&pos, e);
                    dupString(e, &start, (int) (pos.exprloc - start.exprloc), token->text);
                if (strchr(token->text, '.')) {
                    token->type = DOUBLE;
                } else {
                    token->type = INT;
                }
            } else if (ch == '_' || isalpha(ch)) {
                ch = nextChar(e);
                while (ch == '_' || isalnum(ch)) {
                    ch = nextChar(e);
                }
                    getFPos(&pos, e);
                    dupString(e, &start, (int) (pos.exprloc - start.exprloc), token->text);
                token->type = TEXT;
            } else if (ch == '\"') {
                nextString(e, token->text, token->vars);
                token->type = STRING;
            } else if (ch == '\'') {
                nextString2(e, token->text, token->vars);
                token->type = STRING;
            } else {
                token->type = ERROR;
            }
            break;
        }
    }
    /*
        if(token->type==TEXT || token->type==STRING) {
            if(
              strcmp(token->text,"msiMakeGenQuery") ==0
            ||strcmp(token->text,"msiGetSystemTime") ==0
            ||strcmp(token->text,"msiGetValByKey") ==0
            ||
                    strcmp(token->text,"if") ==0
                    ) {
            puts(token->text);
                int i=1+2;
                return;
            }
        }
     */
}

void pushback(Pointer *e, Token *token) {
    if(token->type == EOS) {
        return;
    }
	seekInFile(e, token->exprloc);
}

int eol(char ch) {
	return ch=='\n' || ch=='\r';
}

/**
 * Parse a rule, create node and store their ref in nodeRef.
 * If error, either nodeRef==NULL or nodeRef->type==ERROR.
 */
void parseRuleRuleGen(Pointer *e, Node **nodeRef, int* numberOfRules, int backwardCompatible, rError_t *errmsg,Region *r) {
	Label start;
                    Label pos;
        getFPos(&start, e);
	Token token;
	Node *name=NULL, *cond=NULL, *acti[2], *reco[2], *rule = newNode(RULE,"RULE", &start,r);
    acti[0] = NULL;
    acti[1] = NULL;
    reco[0] = NULL;
    reco[1] = NULL;

	*nodeRef=NULL;
	ERROR(rule ==NULL);
	*nodeRef=rule;

	parseRuleName(e, &name,r);
	ERROR(name==NULL || name->type == ERROR);
        nextTokenRuleGen(e, &token, 1);
	ERROR(token.type != TEXT);
        int rulegen;
        if(strcmp(token.text, "{") == 0) {
            rulegen = 1;
        } else if(strcmp(token.text, "|") == 0) {
            rulegen = 0;
        } else {
            ERROR(1);
        }
        if(rulegen) {
            int done = 0;
            *numberOfRules = 0;
            while(!done) {
                nextTokenRuleGen(e, &token, rulegen);
                ERROR(token.type==ERROR);
                if(token.type == TEXT &&
                   (strcmp(token.text, "ON") == 0 ||
                    strcmp(token.text, "on") == 0 ||
                    strcmp(token.text, "ORON") == 0 ||
                    strcmp(token.text, "oron") == 0)) {
                    nextTermRuleGen(e, &cond, MIN_PREC, rulegen,errmsg,r);
                    ERROR(skip(e, "{", &token, rulegen) == 0);
                    parseRuleActions(e, acti, rulegen, backwardCompatible, acti+1,errmsg,r);
                    ERROR(acti[0]==NULL || acti[0]->type == ERROR);
                    ERROR(skip(e, "}", &token, rulegen) == 0);
                } else if(token.type == TEXT && (strcmp(token.text, "OR") == 0 || strcmp(token.text, "or") == 0)) {
                    cond = createTextNode("true", getFPos(&pos, e),r);
                    setBase(cond, e->ruleBaseName, r);
                    ERROR(skip(e, "{", &token, rulegen) == 0);
                    parseRuleActions(e, acti, rulegen, backwardCompatible, acti+1,errmsg,r);
                    ERROR(acti[0]==NULL || acti[0]->type == ERROR);
                    ERROR(skip(e, "}", &token, rulegen) == 0);
                } else if(token.type == TEXT && strcmp(token.text, "}") == 0) {
                    pushback(e, &token);
                    done = 1;
                    if(*numberOfRules==0) {
                        cond = createTextNode("true", getFPos(&pos, e),r);
                        acti[0] = createActionsNode(NULL, 0, getFPos(&pos, e), r);
                        setBase(acti[0], e->ruleBaseName, r);
                        acti[1] = createActionsNode(NULL, 0, getFPos(&pos, e), r);
                        setBase(acti[1], e->ruleBaseName, r);
                    } else {
                        break;
                    }
                } else {
                    pushback(e, &token);
                    cond = createTextNode("true", getFPos(&pos, e),r);
                    setBase(cond, e->ruleBaseName, r);
                    parseRuleActions(e, acti, rulegen, backwardCompatible, acti+1,errmsg,r);
                    ERROR(acti[0]==NULL || acti[0]->type == ERROR);
                    done = 1;
                }
                Node** subtrees = setDegree(rule, 4,r);
                ERROR(subtrees == NULL);
                subtrees[0] = name;
                subtrees[1] = cond;
                subtrees[2] = acti[0];
                subtrees[3] = acti[1];
                cond = acti[0] = acti[1] = NULL;
                nodeRef[(*numberOfRules)++] = rule;
                rule = newNode(RULE,"RULE", &start,r);
            }

            skip(e, "}", &token, rulegen);
            ERROR(token.type==ERROR);
            /*deleteTree(rule); */
        }
        else {
            Label condstart;
            getFPos(&condstart, e);
            *numberOfRules = 1;
            nextTokenRuleGen(e, &token, rulegen);
            if(token.type==TEXT && strcmp(token.text, "|") == 0)
            { /* empty condition */
                cond = createTextNode("true", &condstart,r);
            }
            else
            {
                pushback(e, &token);
                nextTermRuleGen(e, &cond, MIN_PREC, rulegen, errmsg,r);
                nextTokenRuleGen(e, &token, rulegen);
                ERROR(token.type!=TEXT || (strcmp(token.text, "|") != 0 && strcmp(token.text, "{") != 0));
            }

            parseRuleActions(e, acti, rulegen, backwardCompatible, acti+1,errmsg, r);
            ERROR(acti[0]==NULL || acti[0]->type == ERROR);
            /*deleteTree(acti[1]); */
            acti[1] = NULL;
            skip(e, "|", &token, rulegen);
            ERROR(token.type == ERROR);
            parseRuleActions(e, reco, rulegen, backwardCompatible, reco+1,errmsg, r);
            ERROR(reco[0]==NULL || reco[0]->type == ERROR);
            /*deleteTree(reco[1]); */
            reco[1] = NULL;
            Node** subtrees = setDegree(rule, 4,r);
            ERROR(subtrees == NULL);
            subtrees[0] = name;
            subtrees[1] = cond;
            subtrees[2] = acti[0];
            subtrees[3] = reco[0];
        }



	return;
error:
	if(name!=NULL) {
            rule = name;
        }
	if(cond!=NULL) {
            rule = cond;
        }
	if(acti[0]!=NULL) {
            rule = acti[0];
        }
	if(acti[1]!=NULL) {
            rule = acti[1];
        }
	if(reco[0]!=NULL) {
            rule = reco[0];
        }
	if(reco[1]!=NULL) {
            rule = reco[1];
        }
        if(rule->type!=ERROR) {
            rule->type = ERROR;
        }
        *nodeRef=rule;
        *numberOfRules=1;
	return;
}

void parseRuleName(Pointer *e, Node **name, Region *r) {
	Token res;
	Label start;
        getFPos(&start, e);
	nextTokenRuleGen(e, &res, 1);
	if(res.type!=TEXT) {
		*name = createErrorNode("",&start,r);
		return;
	} else {
		*name = createTextNode(res.text,&start,r);
	}
	nextTokenRuleGen(e, &res, 1);
	int n = 0;
	Node *params[MAX_COND_LEN];

	if(res.type==2 && strcmp(res.text,"(")==0) {
		/* parse parameter list */
		while(1) {
			Token token, token2;
			Label es;
                        getFPos(&es, e);
			nextTokenRuleGen(e, &token, 1);
			ERROR(token.type!= TEXT);
			params[n] = createTextNode(token.text,&es,r);
			n++;
			ERROR(params[n-1] == NULL || params[n-1]->type != TEXT);
			nextTokenRuleGen(e, &token2, 1);
			ERROR(token2.type!=TEXT);
			if(strcmp(token2.text,",")==0) {
				continue;
			} else if(strcmp(token2.text,")")==0) {
				break;
                        } else if(strcmp(token2.text,"}")==0) {
				break;
			} else {
				goto error;
			}
		}
		Node** subtrees = setDegree(*name,n,r);
		ERROR(subtrees == NULL);
		memcpy(subtrees, params, n*sizeof(Node *));
	} else {
            pushback(e, &res);
        }
	return;
error:
	if(*name!=NULL) {
		(*name)->type=ERROR;
	}
}

void parseRuleActions(Pointer *e, Node **node, int rulegen, int backwardCompatible, Node** node2,rError_t *errmsg,Region *r) {
	Node *actions[MAX_COND_LEN];
	Node *recoveries[MAX_COND_LEN];
	int n=0;

        Token token2;
	Node *value = NULL, *value2 = NULL;

	while(1) {
            if(!backwardCompatible) {
		nextTermRuleGen(e, &value, MIN_PREC, rulegen,errmsg,r);
            } else {
                nextActionBackwardCompatible(e, &value, errmsg, r);
            }
		if(value==NULL || value->type == ERROR) {

                    *node = value;
                    goto error;
		}
		actions[n++]=value;
                Label pos;
        recoveries[n-1] = createTextNode("nop", getFPos(&pos, e),r);
        value = NULL;
		nextTokenRuleGen(e, &token2, rulegen);
		if(token2.type==EOS) {
			break;
		}
        ERROR(token2.type!=TEXT);
        if(strcmp(token2.text,":::")==0) {
            nextTermRuleGen(e, &value2, MIN_PREC, rulegen,errmsg,r);
            ERROR(value2 == NULL || value2->type == ERROR);
            /*deleteTree(recoveries[n-1]); */
            recoveries[n-1] = value2;
            value2 = NULL;
            nextTokenRuleGen(e, &token2, rulegen);
        }
        if(strcmp(token2.text,"##")==0) {
			continue;
        } else if(strcmp(token2.text,";")==0) {
            Token token3;
            nextTokenRuleGen(e, &token3, rulegen);
            if(token3.type==TEXT &&
                    (strcmp(token3.text, ")") == 0
                    || strcmp(token3.text, "}") == 0
                    || strcmp(token3.text, "|") == 0
                    || strcmp(token3.text, ",") == 0)) {
                pushback(e, &token3);
                break;
            } else {
                pushback(e, &token3);
                continue;
            }
		} else if(strcmp(token2.text,"|")==0
                        ||strcmp(token2.text,")")==0
                        ||strcmp(token2.text,"}")==0
                        ||strcmp(token2.text,",")==0) {
			pushback(e, &token2);
			break;
		} else {
                        pushback(e, &token2);
                    if(rulegen) {
                        continue;
                    } else {
                        /* possible beginning of a new rule */
                        break;
                    }
		}
	}
        Label pos;
	*node = createActionsNode(actions, n, getFPos(&pos, e),r);
	*node2 = createActionsNode(recoveries, n, getFPos(&pos, e),r);
	return;
error:
        if(*node == NULL || (*node)->type!=ERROR) {
            *node = createErrorNode("see errmsg for error details",getFPos(&pos, e),r);
            char errbuf[ERR_MSG_LEN];
            snprintf(errbuf, ERR_MSG_LEN, "error at %ld", getFPos(&pos, e)->exprloc);
            addRErrorMsg(errmsg, PARSER_ERROR, errbuf);
            *node2 = NULL;
        }
	return;
}
void nextActionBackwardCompatible(Pointer *e, Node **node, rError_t *errmsg, Region *r) {
	Token token;
        Label pos;
	Node* value=NULL, *reco=NULL;
	Node *params[MAX_COND_LEN];
	int n=0;
        char *fn;
        
	nextTokenRuleGen(e, &token, 0);
	ERROR(token.type != TEXT);
        Label estart;
        getFPos(&estart, e);
        fn = token.text;
        if(strcmp(fn, "ifExec") == 0) {
            nextTermRuleGen(e, &(params[0]), MIN_PREC, 0, errmsg, r);
            ERROR(params[0]->type == ERROR);
            parseRuleActions(e, &(params[1]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[1]->type == ERROR);
            parseRuleActions(e, &(params[2]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[2]->type == ERROR);
            parseRuleActions(e, &(params[3]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[3]->type == ERROR);
            parseRuleActions(e, &(params[4]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[4]->type == ERROR);
            *node = createFunctionNode(fn, params, 5, &estart,r);
            return;
        } else if(strcmp(fn, "whileExec") == 0) {
            nextTermRuleGen(e, &(params[0]), MIN_PREC, 0, errmsg, r);
            ERROR(params[0]->type == ERROR);
            parseRuleActions(e, &(params[1]), 0, 1, &reco,errmsg,r);
            ERROR(params[1]->type == ERROR);
            parseRuleActions(e, &(params[2]), 0, 1, &reco,errmsg,r);
            ERROR(params[2]->type == ERROR);
            *node = createFunctionNode(fn, params, 3, &estart,r);
            return;
        } else if(strcmp(fn, "forEachExec") == 0) {
            nextTermRuleGen(e, &(params[0]), MIN_PREC, 0, errmsg, r);
            ERROR(params[0]->type == ERROR);
            Token collVar;
            nextTokenRuleGen(e, &collVar, 0);
            ERROR(collVar.type == ERROR);
            pos.base = e->ruleBaseName;
            pos.exprloc = collVar.exprloc;
            params[1] = createTextNode(collVar.text, &pos, r);
            ERROR(params[1] == NULL);
            parseRuleActions(e, &(params[2]), 0, 1, &reco,errmsg,r);
            ERROR(params[2]->type == ERROR);
            *node = createFunctionNode(fn, params, 3, &estart,r);
            return;
        } else if(strcmp(fn, "assign") == 0) {
            Token collVar;
            nextTokenRuleGen(e, &collVar, 0);
            ERROR(collVar.type == ERROR);
            pos.base = e->ruleBaseName;
            pos.exprloc = collVar.exprloc;
            params[0] = createTextNode(collVar.text, &pos, r);
            ERROR(params[0] == NULL);
            nextTermRuleGen(e, &(params[1]), MIN_PREC, 0, errmsg, r);
            ERROR(params[1]->type == ERROR);
            *node = createFunctionNode(fn, params, 3, &estart,r);
            return;
        } else if(strcmp(fn, "forExec")==0) {
            nextTermRuleGen(e, &(params[0]), MIN_PREC, 0, errmsg, r);
            ERROR(params[0]->type == ERROR);
            parseRuleActions(e, &(params[1]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[1]->type == ERROR);
            parseRuleActions(e, &(params[2]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[2]->type == ERROR);
            parseRuleActions(e, &(params[3]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[3]->type == ERROR);
            parseRuleActions(e, &(params[4]), 0 /*rulegen*/, 1, &reco,errmsg,r);
            ERROR(params[4]->type == ERROR);
            *node = createFunctionNode(fn, params, 5, &estart,r);
            return;
        } else {
            Token token2;
            nextTokenRuleGen(e, &token2, 0);
            ERROR(token2.type == ERROR);
            if(!(token2.type == TEXT && strcmp(token2.text,"(") == 0)) {
                pushback(e, &token2);
                *node = createTextNode(token.text, &estart,r);
                return;
            }
            nextTokenRuleGen(e, &token2, 0);
            if(token2.type != TEXT || strcmp(token2.text, ")")!=0) {
                pushback(e, &token2);
                while(1) {
                    parseActionArgumentBackwardCompatible(e, &value, errmsg, r);
                    *node = value;

                    ERROR(value==NULL || value->type == ERROR);
                    params[n++]=value;
                    nextTokenRuleGen(e, &token2, 0);
                    ERROR(token2.type!=TEXT);
                    if(strcmp(token2.text,",")==0) {
                        continue;
                    } else if(strcmp(token2.text,")")==0) {
                        break;
                    } else {
                        ERROR(1);
                    }
                }
            }

            *node = createFunctionNode(fn, params, n, &estart,r);
            ERROR((*node)->type== ERROR);
            return;
        }
        error:
        if(*node == NULL || (*node)->type!=ERROR) {
            *node = createErrorNode("see errmsg for error details", getFPos(&pos, e),r);
            char errbuf[ERR_MSG_LEN];
            snprintf(errbuf, ERR_MSG_LEN, "error at %ld", getFPos(&pos, e)->exprloc);
            addRErrorMsg(errmsg, PARSER_ERROR, errbuf);
        }
	return;
}

void parseActionArgumentBackwardCompatible(Pointer *e, Node **node, rError_t *errmsg, Region *r) {
    Token token;
    nextActionArgumentStringBackwardCompatible(e, &token);
    Label pos;
    if(token.type != STRING) {
        *node = createErrorNode("reached the end of stream while parsing an action argument", getFPos(&pos, e),r);
        return;
    }
    convertStringToExpression(&token, e->ruleBaseName, node, r);
    

}
void nextTermRuleGen(Pointer *e, Node **node, int prec, int rulegen /* = 0 */, rError_t *errmsg, Region *r) {
/*	Label * start = getFPos(e); */
	Token token;
	Node *term = NULL;
	Node *value = NULL;
	Node *value2 = NULL;
        *node = NULL;
	nextValueRuleGen(e, &value, rulegen, errmsg,r);
	ERROR (value==NULL || value->type==ERROR);
	while (1) {
		nextTokenRuleGen(e, &token, rulegen);
		if (token.type==ERROR || token.type == EOS) {
                    *node = value;
                    ERROR(value == NULL);
                    return;
                } else if((token.type==TEXT &&
                        (strcmp(token.text, ")")==0 ||
			strcmp(token.text, "}")==0 ||
			strcmp(token.text, ",")==0 ||
			strcmp(token.text, ":")==0 ||
			strcmp(token.text, ":::")==0 ||
			strcmp(token.text, "|")==0 ||
			strcmp(token.text, ";")==0 ||
			strcmp(token.text, "##")==0
		))) {
                    pushback(e, &token);
                    *node = value;
                    ERROR(value == NULL);
                    return;
		}
		if(prec>=getPrecedence(&token)) {
			pushback(e, &token);
			*node = value;
                        ERROR(value == NULL);
			return;
		}
		nextTermRuleGen(e, &value2, getPrecedence(&token), rulegen,errmsg,r);
                *node = value2;
		ERROR(value2==NULL || value2->type==ERROR);
                if(strcmp(token.text, "=") == 0) {
                    strcpy(token.text, "assign");
                }
                Label pos;
                pos.base = e->ruleBaseName;
                pos.exprloc = value->expr;
		term = createBinaryFunctionNode(token.text,value, value2,&pos,r);
                *node = term;
		ERROR(term == NULL || term->type == ERROR);
		value = term;
	}
	*node = value;
	return;
error:
        if(*node==NULL || (*node)->type != ERROR) {
            Label pos;
            *node = createErrorNode("see errmsg for error details",getFPos(&pos, e),r);
        }
	return;
}
void convertStringToExpression(Token *token, char *base, Node **node, Region *r) {
            int i = 0, k = 0;
            Label pos;
            pos.base = base;
            long start = token->exprloc;
            char *str = token->text;
            int st[100];
            int end[100];
            int noi = 1;
            st[0] = 0;
            end[0] = strlen(str);
            while(token->vars[i]!=-1) {
                /* this string contains reference to vars */
                int vs = token->vars[i];
                i++;
                if(!isalpha(str[vs+1]) && str[vs+1]!='_') {
                    continue;
                }
                int ve;
                ve = vs+2;
                while(isalnum(str[ve]) || str[ve]=='_') ve++;
                end[noi]=end[noi-1];
                end[noi-1]=vs;
                st[noi]=ve;
                noi++;
            }
            char sbuf[MAX_COND_LEN];
            char delim[1];
            delim[0] = '\0';
            strncpy(sbuf, str+st[0], end[0]-st[0]);
            strcpy(sbuf+end[0]-st[0], delim);
            pos.exprloc = start + st[0];
            *node = createStringNode(sbuf, &pos,r);
            ERROR(*node == NULL || (*node)->type == ERROR);
            for(k=1;k<noi;k++) {
                strncpy(sbuf, str+end[k-1], st[k]-end[k-1]); /* var */
                strcpy(sbuf+st[k]-end[k-1], delim);
                Node *node0 = *node;
                pos.exprloc = start + end[k-1];
                *node = createTextNode(sbuf, &pos,r);
                ERROR(*node == NULL || (*node)->type == ERROR);
                pos.exprloc = (*node)->expr;
                *node = createUnaryFunctionNode("str", *node, &pos,r);
                ERROR(*node == NULL || (*node)->type == ERROR);

                *node = createBinaryFunctionNode("++",  node0, *node, &pos, r);
                ERROR(*node == NULL || (*node)->type == ERROR);
                strncpy(sbuf, str+st[k], end[k]-st[k]);
                strcpy(sbuf+end[k]-st[k], delim);
                node0 = *node;
                pos.exprloc = start + st[k];
                *node = createStringNode(sbuf, &pos,r);
                ERROR(*node == NULL || (*node)->type == ERROR);
                pos.exprloc = (*node)->expr;
                *node = createBinaryFunctionNode("++", node0, *node,
                        &pos,r);
                ERROR(*node == NULL || (*node)->type == ERROR);

            }
            return;
        error:
        if(*node == NULL || (*node)->type!=ERROR) {
                pos.exprloc = start;
            *node = createErrorNode("see errmsg for error details", &pos,r);
        }
	return;

}
void nextValueRuleGen(Pointer* e, Node** node, int rulegen /* = rulegen */, rError_t *errmsg, Region *r) {
	Label start;
        getFPos(&start, e);
	Token token;
	Node* value=NULL;
	Node *params[MAX_COND_LEN];
	int n=0;

	nextTokenRuleGen(e, &token, rulegen);
	ERROR(token.type == ERROR || (token.type == TEXT && strcmp(token.text, "|")==0));
	if(token.type == TEXT && ((token.text[0]=='*' && strlen(token.text)>1) ||
	   (token.text[0]=='$' && strlen(token.text)>1) ||
	   strcmp(token.text, "nop") == 0)) { /* variables or nop */
		*node = createTextNode(token.text, &start,r);
		ERROR(*node == NULL);
		return;
        } else if (strcmp(token.text, "(")==0) {
            char *etok;
            etok = ")";
            nextTermRuleGen(e, node, 0, rulegen,errmsg, r);
            ERROR(*node ==NULL || (*node)->type==ERROR);
            ERROR(skip(e, etok, &token, rulegen)==0);
            return;

        } else if(strcmp(token.text, "{")==0 ) {
            char *etok;
            etok = "}";
            Node *reco;
            parseRuleActions(e, node, rulegen, 0, &reco,errmsg, r);
            ERROR(*node ==NULL || (*node)->type==ERROR);
            /*deleteTree(reco); */
            ERROR(skip(e, etok, &token, rulegen)==0);
            return;
	} else if (isUnaryOp(&token)) {
		nextValueRuleGen(e, &value, rulegen,errmsg, r);
                *node = value; /* copy value to node so because error handling works on node only */
		ERROR (value == NULL || value->type==ERROR);
		*node = createUnaryFunctionNode(token.text, value, &start,r);
		ERROR(*node == NULL || (*node)->type == ERROR);
		return;
	} else if (token.type == INT) {
		*node = createIntNode(token.text, &start,r);
                ERROR(*node == NULL || (*node)->type == ERROR);
		return;
	} else if (token.type == DOUBLE) {
		*node = createDoubleNode(token.text, &start,r);
                ERROR(*node == NULL || (*node)->type == ERROR);
		return;
	} else if(token.type == STRING) {
            convertStringToExpression(&token, e->ruleBaseName, node, r);
            return;
	} else { /* function */
            ERROR(! isalpha(token.text[0]));
		Label estart;
                getFPos(&estart, e);
                char *fn = token.text;
                if(rulegen && strcmp(fn, "if") == 0) {
                    Token tk;
                    Node *reco;
                    n = 0;
                    ERROR(skip(e, "(", &tk, rulegen)==0);
                    nextTermRuleGen(e, &(params[0]), MIN_PREC, rulegen, errmsg, r);
                    ERROR(skip(e, ")", &tk, rulegen)==0);
                    nextTokenRuleGen(e, &tk, rulegen);
                    ERROR(strcmp(tk.text, "then")!=0 && strcmp(tk.text, "{")!=0);
                    if(strcmp(tk.text, "then")==0) {
                        ERROR(skip(e, "{", &tk, rulegen)==0);
                    }
                    parseRuleActions(e, &value, rulegen, 0, &reco,errmsg,r);
                    *node = value;
                    ERROR(value->type == ERROR);
                    if(value->degree==1) { /* single action sequences are converted back to expressions */
                        value = value->subtrees[0];
                    }
                    params[1] = value;
                    params[3] = reco;
                    ERROR(skip(e, "}", &tk, rulegen)==0);
                    Label expr;
                    getFPos(&expr, e);
                    nextTokenRuleGen(e, &tk, rulegen);
                    if(tk.type!=TEXT || strcmp(tk.text, "else")!=0) {
                        pushback(e, &tk);
                        Node *nop = createTextNode("nop", &expr,r);
                        params[2] = params[4] = createActionsNode(&(nop), 1, &expr,r);
                        *node = params[2];
                        ERROR(params[2]->type == ERROR);
                    } else {
                        nextTokenRuleGen(e, &tk, rulegen);
                        int brace;
                        ERROR(tk.type != TEXT || (
                                (brace = strcmp(tk.text, "{")!=0) && strcmp(tk.text, "if")!=0));
                        if(brace == 0) {
                            parseRuleActions(e, &value, rulegen, 0, &reco,errmsg,r);
                            *node = value;
                            ERROR(value->type == ERROR);
                            if(value->degree==1) { /* single action sequences are converted back to expressions */
                                value = value->subtrees[0];
                            }
                            params[2] = value;
                            params[4] = reco;
                            ERROR(skip(e, "}", &tk, rulegen)==0);
                        } else { /* if */
                            pushback(e, &tk);
                            nextValueRuleGen(e, &(params[2]), rulegen, errmsg, r);
                            ERROR(params[2]==NULL || params[2]->type == ERROR);
                            Label pos;
                            params[4] = createActionsNode(NULL, 0, getFPos(&pos, e),r);
                        }
                    }
                    *node = createFunctionNode(fn, params, 5, &estart,r);
                    return;
                } else if(rulegen && strcmp(fn, "while") == 0) {
                    Node *reco;
                    Token tk;
                    n = 0;
                    ERROR(skip(e, "(", &tk, rulegen)==0);
                    nextTermRuleGen(e, &(params[n++]), MIN_PREC, rulegen, errmsg, r);
                    ERROR(skip(e, ")", &tk, rulegen)==0);

                    ERROR(skip(e, "{", &tk, rulegen)==0);
                    parseRuleActions(e, &value, rulegen, 0, &reco,errmsg,r);
                    *node = value;
                    ERROR(value==NULL || value->type == ERROR);
                    params[n++] = value;
                    params[n++] = reco;
                    /*deleteTree(reco); */
                    ERROR(skip(e, "}", &tk, rulegen)==0);
                    *node = createFunctionNode(fn, params, n, &estart,r);
                    return;
                } else if(rulegen && strcmp(fn, "foreach") == 0) {
                    Node *reco;
                    Token tk;
                    n = 0;
                    ERROR(skip(e, "(", &tk, rulegen)==0);
                    nextTermRuleGen(e, &(params[n++]), MIN_PREC, rulegen, errmsg, r);
                    ERROR(skip(e, ")", &tk, rulegen)==0);

                    ERROR(skip(e, "{", &tk, rulegen)==0);
                    parseRuleActions(e, &value, rulegen, 0, &reco,errmsg, r);
                    *node = value;
                    ERROR(value->type == ERROR);
                    params[n++] = value;
                    params[n++] = reco;
                    /*deleteTree(reco); */
                    ERROR(skip(e, "}", &tk, rulegen)==0);
                    *node = createFunctionNode(fn, params, n, &estart,r);
                    return;
                } else if(rulegen && strcmp(fn, "for")==0) {
                    Node *reco;
                    Token tk;
                    n = 0;
                    ERROR(skip(e, "(", &tk, rulegen)==0);
                    nextTermRuleGen(e, &(params[n++]), MIN_PREC, rulegen, errmsg, r);
                    ERROR(skip(e, ";", &tk, rulegen)==0);
                    nextTermRuleGen(e, &(params[n++]), MIN_PREC, rulegen, errmsg, r);
                    ERROR(skip(e, ";", &tk, rulegen)==0);
                    nextTermRuleGen(e, &(params[n++]), MIN_PREC, rulegen, errmsg, r);
                    ERROR(skip(e, ")", &tk, rulegen)==0);
                    ERROR(skip(e, "{", &tk, rulegen)==0);
                    parseRuleActions(e, &value, rulegen, 0, &reco,errmsg,r);
                    *node = value;
                    ERROR(value->type == ERROR);
                    params[n++] = value;
                    params[n++] = reco;
                    ERROR(skip(e, "}", &tk, rulegen)==0);
                    *node = createFunctionNode(fn, params, n, &estart,r);
                    return;


                } else {
                    Token token2;
                    nextTokenRuleGen(e, &token2, rulegen);
                    ERROR(token2.type == ERROR);
                    if(token2.type == TEXT && strcmp(token2.text,"(") == 0) {
                    } else {
			pushback(e, &token2);
			*node = createTextNode(token.text, &estart,r);
			return;
                    }
                    nextTokenRuleGen(e, &token2, rulegen);
                    if(token2.type != TEXT || strcmp(token2.text, ")")!=0) {
                        pushback(e, &token2);
                            while(1) {
                                Node *reco;
                                parseRuleActions(e, &value, rulegen, 0, &reco, errmsg, r);
                                *node = value;

                                ERROR(value==NULL || value->type == ERROR);
                                if(value->degree==1) { /* single action sequences are converted back to expressions */
                                    value = value->subtrees[0];
                                }
    /*                            deleteTree(reco); */
                                params[n++]=value;
                                nextTokenRuleGen(e, &token2, rulegen);
                                ERROR(token2.type!=TEXT);
                                if(strcmp(token2.text,",")==0) {
                                    continue;
                                } else if(strcmp(token2.text,")")==0) {
                                    break;
                                } else {
                                    ERROR(1);
                                }
                            }
                    }

                    *node = createFunctionNode(fn, params, n, &estart,r);
                    ERROR((*node)->type== ERROR);
                    return;
                }

	}
        error:
        if(*node == NULL || (*node)->type!=ERROR) {
            Label pos;
            getFPos(&pos, e);
            *node = createErrorNode("see errmsg for error details", &pos,r);
            char errbuf[ERR_MSG_LEN];
            snprintf(errbuf, ERR_MSG_LEN, "error at %ld", pos.exprloc);
            addRErrorMsg(errmsg, PARSER_ERROR, errbuf);
        }
	return;
}
/*
 * return number of vars or -1 if no string found
 */
int nextStringBase(Pointer *e, char *value, char* delim, int consumeDelim, char escape, int vars[]) {
	int mode=1; /* 1 string 3 escape */
        int nov = 0;
	char* value0=value;
/*        Label * start = getFPos(e); */
	*value = lookAhead(e, 0);
	value++;
	char ch = nextChar(e);
	while(ch!=-1) {
		*value = ch;
		switch(mode) {
                    case 1:
                        if(ch ==escape) {
                            value--;
                            mode = 3;
                        } else if(strchr(delim, ch)!=NULL) {
                            if(consumeDelim) {
                                value[1]='\0';
                                trimquotes(value0);
                                nextChar(e);
                            } else {
                                value[0]='\0';
                            }
                            vars[nov] = -1;
                            return nov;
                        } else if((ch =='*' || ch=='$') &&
                                isalpha(lookAhead(e, 1))) {
                            vars[nov++] = value - value0 - 1;
                        }


                        break;
                    case 3:
                        if(ch=='n') {
                            *value = '\n';
                        } else if(ch == 't') {
                            *value = '\t';
                        } else if(ch =='r') {
                            *value = '\r';
                        } else if(ch =='0') {
                            *value = '\0';
                        } else {
                            *value = ch;
                        }
                        mode -= 2;
		}
		ch = nextChar(e);
		value ++;
	}
	return -1;
}
int nextString(Pointer *e, char *value, int vars[]) {
	return nextStringBase(e, value, "\"", 1, '\\', vars);
}
int nextString2(Pointer *e, char *value, int vars[]) {
	return nextStringBase(e, value, "\'", 1, '\\', vars);
}

int getPrecedence(Token * token) {
	int i;
	for(i=0;i<num_ops;i++) {
		if(new_ops[i].arity!=1 && strcmp(new_ops[i].string,token->text)==0) {
			return new_ops[i].prec;
		}
	}
	return -1;
}
int isUnaryOp(Token *token) {
	int i;
	for (i=0;i<num_ops;i++) {
		if (strcmp(token->text, new_ops[i].string) == 0) {
			return new_ops[i].arity == 1;
		}
	}
	return 0;
}
int isOp(char *token) {
	int i;
	for (i=0;i<num_ops;i++) {
		if (strcmp(token, new_ops[i].string) == 0) {
			return 1;
		}
	}
	return 0;
}
char* trim(char* str) {
	char* trimmed = str;
	while(*trimmed =='\t' || *trimmed==' ') {
		trimmed ++;
	}
	int l = strlen(trimmed)-1;
	while(l>=0 && (trimmed[l] =='\t' || trimmed[l]==' ')) {
		l--;
	}
	trimmed[l+1] = '\0';
	return trimmed;

}
void trimquotes(char *string) {
	int len = strlen(string)-2;
	memmove(string, string+1, len*sizeof(char));
	string[len]='\0';
}

void printTree(Node *n, int indent) {
	printIndent(indent);
	printf("%s:%d->%s\n",n->text, n->type, n->coercion == NULL?"?":typeName_ExprType(n->coercion));
	int i;
	for(i=0;i<n->degree;i++) {
		printTree(n->subtrees[i],indent+1);
	}

}
void printTreeDeref(Node *n, int indent, Hashtable *var_types, Region *r) {
	printIndent(indent);
	printf("%s:%d->",n->text, n->type);
        printType(n->coercion, var_types);
        printf("\n");
	int i;
	for(i=0;i<n->degree;i++) {
		printTreeDeref(n->subtrees[i],indent+1, var_types, r);
	}

}

void printIndent(int n) {
	int i;
	for(i=0;i<n;i++) {
		printf("\t");
	}
}

int eqExprNodeSyntactic(Node *a, Node *b) {
    if(a->type == b->type &&
       strcmp(a->text, b->text) == 0 &&
       a->degree == b->degree) {
        int i;
        for(i=0;i<a->degree;i++) {
            if(!eqExprNodeSyntactic(a->subtrees[i], b->subtrees[i])) {
                return 0;
            }
        }
    }
    return 1;
}

StringList *getVarNamesInExprNode(Node *expr, Region *r) {
    return getVarNamesInExprNodeAux(expr, NULL, r);
}

StringList *getVarNamesInExprNodeAux(Node *expr, StringList *vars, Region *r) {
    int i;
    switch(expr->type) {
        case TEXT:
            if(expr->text[0] == '*') {
                StringList *nvars = (StringList*)region_alloc(r, sizeof(StringList));
                nvars->next = vars;
                nvars->str = expr->text;
                return nvars;
            }
            /* non var */
        default:
            for(i =0;i<expr->degree;i++) {
                vars = getVarNamesInExprNodeAux(expr->subtrees[i], vars, r);
            }
            return vars;
    }
}


/************** file/buffer pointer utilities ***************/
void nextChars(Pointer *p, int len) {
	int i;
        for(i=0;i<len;i++) {
		nextChar(p);
	}
}

/**
 * returns -1 if reached eof
 */
int nextChar(Pointer *p) {
    if(p->isFile) {
	int ch = lookAhead(p, 1);
	/*if(ch != -1) { */
		p->p++;
	/*} */
	return ch;
    } else {
        if(p->strbuf[p->strp] == '\0') {
            return -1;
        }
        int ch = p->strbuf[++p->strp];
        if(ch == '\0') {
            ch = -1; /* return -1 for eos */
	}
	return ch;
    }
}

Pointer *newPointer(FILE *fp, char *ruleBaseName) {
        Pointer *e = (Pointer *)malloc(sizeof(Pointer));
        initPointer(e, fp, ruleBaseName);
        return e;
}
Pointer *newPointer2(char* buf) {
        Pointer *e = (Pointer *)malloc(sizeof(Pointer));
        initPointer2(e, buf);
        return e;
}
void deletePointer(Pointer* buf) {
    if(buf->isFile) {
        fclose(buf->fp);
    }
    free(buf);

}

void initPointer(Pointer *p, FILE* fp, char* ruleBaseName /* = NULL */) {
    fseek(fp, 0, SEEK_SET);
    p->fp = fp;
    p->fpos = 0;
    p->len = 0;
    p->p = 0;
    p->isFile = 1;
    snprintf(p->ruleBaseName, MAX_NAME_LEN, "%s", ruleBaseName);
}

void initPointer2(Pointer *p, char *buf) {
    p->strbuf = buf;
    p->strp = 0;
    p->isFile = 0;
    p->ruleBaseName[0] = '\0';
}

void readToBuffer(Pointer *p) {
    if(p->isFile) {
	int move = (p->len+1)/2;
	move = move > p->p? p->p : move; /* prevent next char from being deleted */
	int startpos = p->len - move;
	int load = POINTER_BUF_SIZE - startpos;
	/* move remaining to the top of the buffer */
	memcpy(p->buf,p->buf+move,startpos*sizeof(char));
	/* load from file */
	int count = fread(p->buf+startpos, sizeof(char), load, p->fp);
	p->len = startpos + count;
	p->p -= move;
	p->fpos += move * sizeof(char);
   } else {
   }
}

void seekInFile(Pointer *p, long x) {
   if(p -> isFile) {
       if(p->fpos < x * sizeof(char) || p->fpos + p->len >= x * sizeof(char)) {
           fseek(p->fp, x * sizeof(char), SEEK_SET);
           clearBuffer(p);
           p->fpos = x * sizeof(char);
           readToBuffer(p);
       } else {
           p->p = x * sizeof(char) - p->fpos;
       }
   } else {
        p->strp = x;
   }
}

void clearBuffer(Pointer *p) {
   if(p->isFile) {
	p->fpos += p->len * sizeof(char);
	p->len = p->p = 0;
   } else {
   }
}

/* assume that n is less then POINTER_BUF_SIZE */
int dupString(Pointer *p, Label * start, int n, char *buf) {
   if(p->isFile) {
	Label curr;
        getFPos(&curr, p);
	seekInFile(p, start->exprloc);
        int len = 0;
        int ch;
        while(len < n && (ch=lookAhead(p, 0)) != -1) {
            buf[len++] = (char) ch;
            nextChar(p);
        }
	buf[len] = '\0';
	seekInFile(p, curr.exprloc);
	return len;
   } else {
	int len = strlen(p->strbuf + start->exprloc);
        len = len > n ? n : len;
        memcpy(buf, p->strbuf + start->exprloc, len * sizeof(char));
        buf[len] = '\0';
        return len;
   }
}

int dupLine(Pointer *p, Label * start, int n, char *buf) {
    Label pos;
    getFPos(&pos, p);
    seekInFile(p, 0);
    int len = 0;
    int i = 0;
    char ch = lookAhead(p, 0);
    while(ch != -1) {
        if(ch=='\n') {
            if(i<start->exprloc)
                len = 0;
            else {
                break;
            }
        } else {
            buf[len] = ch;
            len++;
            if(len == n - 1) {
                break;
            }
        }
        i++;
        ch = nextChar(p);
    }
    buf[len] = '\0';
    seekInFile(p, pos.exprloc);
    return len;
}

void getCoor(Pointer *p, Label * errloc, int coor[2]) {
    Label pos;
    getFPos(&pos, p);
    seekInFile(p, 0);
    coor[0] = coor[1] = 0;
    int i;
    char ch = lookAhead(p, 0);
    for(i =0;i<errloc->exprloc;i++) {
        if(ch=='\n') {
            coor[0]++;
            coor[1] = 0;
        } else {
            coor[1]++;
        }
        ch = nextChar(p);
    }
    seekInFile(p, pos.exprloc);
}

Label *getFPos(Label *l, Pointer *p) {
	l->exprloc = p->isFile? p->fpos/sizeof(char) + p->p : p->strp;
        l->base = p->ruleBaseName;
        return l;
}
/* assume that n is less then POINTER_BUF_SIZE/2 */
int lookAhead(Pointer *p, int n) {
	if(p->isFile) {
		if(p->p+n >= p->len) {
			readToBuffer(p);
			if(p->p+n >= p->len) {
				return -1;
			}
		}
		return (int)(p->buf[p->p+n]);
	} else {
		if(n >= strlen(p->strbuf+p->strp)) {
			return -1;
		}
		return (int)p->strbuf[p->strp+n];
	}

}

/* backward compatibility with the previous version */
char *functionParameters(char *e, char *value) {
	int mode=0; /* 0 params 1 string 2 string2 3 escape 4 escape2 */
	int l0 = 0;
	while(*e!=0) {
		*value = *e;
		switch(mode) {
			case 0:

				switch(*e) {
					case '(':
						l0++;
						break;
					case '\"':
						mode = 1;
						break;
					case '\'':
						mode = 2;
						break;
					case ')':
						l0--;
						if(l0==0) {
							value[1]='\0';
							return e+1;
						}
				}
				break;
			case 1:
				switch(*e) {
					case '\\':
						mode = 3;
						break;
					case '\"':
						mode = 0;
				}
				break;
			case 2:
				switch(*e) {
					case '\\':
						mode = 4;
						break;
					case '\'':
						mode = 0;
				}
				break;
			case 3:
			case 4:
				mode -= 2;
		}
		e++; value ++;
	}
	*value=0;
	return e;
}
char *nextStringString(char *e, char *value) {
	int mode=1; /* 1 string 3 escape */
	char* e0=e;
	char* value0=value;
			*value = *e;
	value++;
	e++;
	while(*e!=0) {
		*value = *e;
		switch(mode) {
			case 1:
				switch(*e) {
					case '\\':
					    value--;
						mode = 3;
						break;
					case '\"':
						value[1]='\0';
						trimquotes(value0);
						return e+1;
				}
				break;
			case 3:
				mode -= 2;
		}
		e++; value ++;
	}
	return e0;
}
char *nextString2String(char *e, char *value) {
	int mode=1; /* 1 string 3 escape */
	char* e0=e;
	char* value0 = value;
	*value = *e;
	value ++;
	e++;
	while(*e!=0) {
		*value = *e;
		switch(mode) {
			case 1:
				switch(*e) {
					case '\\':
						value--;
						mode = 3;
						break;
					case '\'':
						value[1]='\0';
						trimquotes(value0);
						return e+1;
				}
				break;
			case 3:
				mode -= 2;
		}
		e++; value ++;
	}
	return e0;
}


char * nextRuleSection(char* buf, char* value) {
	char* e=buf;
	int mode=0; /* 0 section 1 string 2 string2 3 escape 4 escape2 */
	while(*e!=0) {
		*value = *e;
		switch(mode) {
			case 0:

				switch(*e) {
					case '\"':
						mode = 1;
						break;
					case '\'':
						mode = 2;
						break;
					case '|':
						*value='\0';
						return e+1;
				}
				break;
			case 1:
				switch(*e) {
					case '\\':
						mode = 3;
						break;
					case '\"':
						mode = 0;
				}
				break;
			case 2:
				switch(*e) {
					case '\\':
						mode = 4;
						break;
					case '\'':
						mode = 0;
				}
				break;
			case 3:
			case 4:
				mode -= 2;
		}
		e++; value ++;
	}
	*value=0;
	return e;


}

void nextActionArgumentStringBackwardCompatible(Pointer *e, Token *token) {
    skipWhitespace(e);
    char ch = lookAhead(e, 0);
    if (ch==-1) { /* reach the end of stream */
        token->type = EOS;
        strcpy(token->text,"EOS");
    } else {
        ch = lookAhead(e, 0);
        if(ch == '\"') {
            nextStringBase(e, token->text, "\"", 1, '\\', token->vars);
            skipWhitespace(e);
        } else if( ch == '\'') {
            nextStringBase(e, token->text, "\'", 1, '\\', token->vars);
            skipWhitespace(e);
        } else {
            nextStringBase(e, token->text, ",)", 0, '\\', token->vars);
            /* remove trailing ws */
            int l0;
            l0 = strlen(token->text);
            while(isspace(token->text[l0-1])) {
                l0--;
            }
            token->text[l0++]='\0';
        }
        token->type = STRING;
    }
}

char* typeName_Res(Res *s) {
    return typeName_ExprType(s->type);
}

char* typeName_ExprType(ExprType *s) {
    if(s->t<0 || typeNameLen <= s->t)
    {
        switch(s->t) {
            case T_IRODS:
                return s->ext.irods.name;
            case T_VAR:
                return "TVAR";
            case T_DYNAMIC:
                return "DYNAMIC";
            default:
                return "OTHER";
        }
    }
    else
	return typeName[s->t];
}

char* typeName_TypeConstructor(TypeConstructor s) {
    if(s<0 || typeNameLen <= s)
    {
        switch(s) {
            case T_IRODS:
                return "IRODS";
            case T_VAR:
                return "VAR";
            case T_DYNAMIC:
                return "DYNAMIC";
            default:
                return "OTHER";
        }
    }
    else
	return typeName[s];
}

/**
 * delete the node, retaining its subtrees
 */
void deleteNode(Node *node) {
	if(node->subtrees!=NULL) {
/*
            int i;
            for(i=0;i<node->degree;i++)
		deleteNode(node->subtrees[i]);
*/
            free(node->subtrees);
	}
	free(node);
}
/**
 * delete the subtree at the node
 */
void deleteTree(Node *node) {
	int i;
	for(i=0;i<node->degree;i++) {
		deleteTree(node->subtrees[i]);
	}
	deleteNode(node);
}
