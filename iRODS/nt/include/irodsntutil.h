#ifndef _IRODSNTUTIL_H_
#define _IRODSNTUTIL_H_

#include <string.h>
/*
#include <io.h>
#include <fcntl.h>
#include <windows.h>
*/
#include "stdlib.h"
#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <conio.h>
#include <direct.h>

#ifdef  __cplusplus
extern "C" {
#endif

void iRODSPathToNtPath(char *ntpath,const char *srbpath);
int iRODSNtFileOpen(const char *filename,int oflag, int istextfile);
void iRODSNTPathBackSlash(char *str);
void iRODSNTPathForwardSlash(char *str);
FILE *iRODSNt_fopen(const char *filename, const char *mode);
int iRODSNt_open(const char *filename,int oflag, int istextfile);
int iRODSNt_bopen(const char *filename,int oflag);
int iRODSNt_bcreate(const char *filename);
int iRODSNt_unlink(char *filename);
int iRODSNt_stat(const char *filename,struct stat *stat_p);
int iRODSNt_mkdir(char *dir,int mode);

char *iRODSNt_gethome();
void iRODSNtGetUserPasswdInputInConsole(char *buf, char *prompt);

int getopt(int argc, char *const *argv, const char *shortopts);
long long atoll(const char *str);

#ifdef  __cplusplus
}
#endif

#endif _IRODSNTUTIL_H_