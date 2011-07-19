/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef DATETIME_H
#define DATETIME_H
/*#define __USE_XOPEN*/
#include <time.h>
#include "debug.h"
#ifndef DEBUG
#include "rodsType.h"
#endif
int strttime(char* timestr, char* timeformat, rodsLong_t* t);
int ttimestr(char* buf, int n, char* timeformat, rodsLong_t* t);
#endif
