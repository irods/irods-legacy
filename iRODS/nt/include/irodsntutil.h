#ifndef _IRODSNTUTIL_H_
#define _IRODSNTUTIL_H_

#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include "stdlib.h"
#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <conio.h>

#ifdef  __cplusplus
extern "C" {
#endif

int iRODSNtFileOpen(const char *filename,int oflag, int istextfile);
void iRODSPathToNtPath(char *ntpath,const char *srbpath);;
static void StrChangeChar(char* str,char from, char to);
void iRODSNTPathBackSlash(char *str);
void iRODSNTPathForwardSlash(char *str);
FILE *iRODSNT_fopen(const char *filename, const char *mode);
void iRODSPathToNtPath(char *ntpath,const char *srbpath);
int iRODSNtFileOpen(const char *filename,int oflag, int istextfile);
int iRODSNTFileBinaryOpen(const char *filename,int oflag);
int iRODSNTFileBinaryCreate(const char *filename);
int iRODSNTUnlinkFile(char *filename);
void iRODSNTCheckExecMode(int aargc,char **aargv);
int iRODSNTRunInConsoleMode();
void NtEmergencyMessage(char *msg);
void iRODSNTGetUserPasswdInputInConsole(char *buf, char *prompt);
long getppid(void);
int getopt(int argc, char* const argv[], const char* optstring);
int NTmkdir(const char *dirname);
#ifdef  __cplusplus
}
#endif

#endif _IRODSNTUTIL_H_