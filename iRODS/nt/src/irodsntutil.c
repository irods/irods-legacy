/*******************************************************************
 iRODSNtUtil.c
 Author: Bing Zhu
         San Diego Supercomputer Center
 Date of last modifictaion:  10-12-2005
 Modified: Charles Cowart
 Date of last modification: 11/15/2006
 *******************************************************************/
#include "iRODSNtutil.h"
#include <io.h>
#include <conio.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <errno.h>

/* The function is used to convert unix path delimiter,slash, to
   windows path delimiter, back slash. 
 */
static void StrChangeChar(char* str,char from, char to)
{
	int n,i;

	if(str == NULL)
		return;

	n = strlen(str);

	for(i=0;i<n;i++)
	{
		if(str[i] == from)
			str[i] = to;
	}
}

void iRODSNtPathBackSlash(char *str)
{
	StrChangeChar(str,'/','\\');
}

void iRODSNtPathForwardSlash(char *str)
{
	StrChangeChar(str,'\\','/');
}

void iRODSPathToNtPath(char *ntpath,const char *srbpath)
{
	char buff[2048];

	if(strlen(srbpath) <= 3)
	{
		strcpy(buff,srbpath);
	}
	else if((srbpath[0] == '/')&&(srbpath[2] == ':'))  /* the path has form /D:/tete/... */
	{
		strcpy(buff,&(srbpath[1]));
	}
	else if((srbpath[0] == '\\')&&(srbpath[2] == ':'))
	{
		strcpy(buff,&(srbpath[1]));
	}
	else
	{
		strcpy(buff,srbpath);
	}

	iRODSNtPathBackSlash(buff);
	strcpy(ntpath,buff);
}

FILE *iRODSNt_fopen(const char *filename, const char *mode)
{
	char ntfp[2048];
	iRODSPathToNtPath(ntfp,filename);
	return fopen(ntfp,mode);
}

int iRODSNt_open(const char *filename,int oflag, int istextfile)
{
	int New_Oflag;
	char ntfp[2048];

	iRODSPathToNtPath(ntfp,filename);

	if(istextfile)
		New_Oflag = oflag;
	else
		New_Oflag = _O_BINARY | oflag;

	if(New_Oflag & _O_CREAT)
	{
		return _open(ntfp,New_Oflag,_S_IREAD|_S_IWRITE);
	}

	return _open(ntfp,New_Oflag);
}

/* open a file in binary mode. */
int iRODSNt_bopen(const char *filename,int oflag)
{
	int New_Oflag;
	char ntfp[2048];

	iRODSPathToNtPath(ntfp,filename);

	New_Oflag = _O_BINARY | oflag;
	return _open(ntfp,New_Oflag);
}

/* create a file in binary mode */
int iRODSNt_bcreate(const char *filename)
{
	char ntfp[2048];

	iRODSPathToNtPath(ntfp,filename);

	return _open(ntfp,_O_RDWR|_O_CREAT|_O_EXCL|_O_BINARY, _S_IREAD|_S_IWRITE);
}

int iRODSNt_unlink(char *filename)
{
	char ntfp[2048];

	iRODSPathToNtPath(ntfp,filename);

	return _unlink(ntfp);
}

int iRODSNt_stat(const char *filename,struct stat *stat_p)
{
        char ntfp[2048];
        iRODSPathToNtPath(ntfp,filename);
        return stat(ntfp,stat_p);
}

int iRODSNt_mkdir(char *dir,int mode)
{
        char ntfp[2048];
        iRODSPathToNtPath(ntfp,dir);
        return _mkdir(ntfp);
}

/* the caller needs to free the memory. */
char *iRODSNt_gethome()
{
	char *s1, *s2;
	char tmpstr[1024];
	s1 = getenv("HOMEDRIVE");
	s2 = getenv("HOMEPATH");

	if((s1==NULL)||(strlen(s1)==0))
		return NULL;

	if((s2==NULL)||(strlen(s2)==0))
		return NULL;

	sprintf(tmpstr, "%s%s", s1, s2);
	return strdup(tmpstr);
}

/* The function is used in Windows console app, especially S-commands. */
void iRODSNtGetUserPasswdInputInConsole(char *buf, char *prompt)
{
   char *p = buf;
   char c;
   char star='*';

   if((prompt != NULL) && (strlen(prompt) > 0))
   {
      printf("%s", prompt);
      fflush(stdout);
   }

   while(1)
   {
      c = _getch();

      if(c == 8)   /* a backspace, we currently ignore it. i.e. treat it as doing nothing. User can alway re-do it. */
      {
      }
      else if(c == 13)  /* 13 is a return char */
      {
         _putch(c);
         break;
      }
      else
      {
         _putch(star);
         p[0] = c;
         ++p;
      }

      /* extra protection */
      if((c == '\n') || (c == '\f') || (c == 13))
        break;

   }

   p[0] = '\0';

   printf("\n");
}

long long atoll(const char *str)
{
	return _atoi64(str);
}