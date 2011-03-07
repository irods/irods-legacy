/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef DATETIME_H
#define DATETIME_H
#define __USE_XOPEN
#include <time.h>
int strttime(char* timestr, char* timeformat, time_t* t);
int ttimestr(char* buf, int n, char* timeformat, time_t* t);
#endif
