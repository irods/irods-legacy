
#ifndef _WIN32
#define _WIN32
#endif

#include <assert.h>
#include "h5Object.h"
#include "h5File.h"
#include "h5Dataset.h"


#define NODEBUG

#define MAX_CONNECTIONS 50

#ifdef _WIN32
#pragma comment(lib,"ws2_32")
#endif

extern int h5ObjRequest(rcComm_t *conn, void *obj, int objID);

#define THROW_JNI_ERROR(_ex, _msg) { \
    (*env)->ThrowNew(env, (*env)->FindClass(env, _ex), _msg); \
     ret_val = -1; \
     goto done; \
}

#define GOTO_JNI_ERROR() { \
     ret_val = -1; \
     goto done; \
}

/* count of open files */
static int file_count = 0;

typedef struct rodsConn {
    rcComm_t *connection;
    char host[MAX_NAME_LEN];
    char port[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    char user[MAX_NAME_LEN];
    char domain[MAX_NAME_LEN];
} rodsConn;

typedef struct rodsConn_list {
    rodsConn connections[MAX_CONNECTIONS];
    int count;      /* count of total connectins */
} rodsConn_list;




