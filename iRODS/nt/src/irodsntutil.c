/*******************************************************************
 iRODSNtUtil.c
 Author: Bing Zhu
         San Diego Supercomputer Center
 Date of last modifictaion:  10-12-2005
 Modified: Charles Cowart
 Date of last modification: 11/15/2006
 *******************************************************************/
#include "iRODSNtutil.h"
#include "direct.h"

int nRunInConsole=0;
void iRODSPathToNtPath(char *ntpath,const char *srbpath);
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

FILE *iRODSNt_fopen(const char *filename, const char *mode)
{
	char ntfp[1024];

	iRODSPathToNtPath(ntfp,filename);

	return fopen(ntfp,mode);
}

void iRODSPathToNtPath(char *ntpath,const char *srbpath)
{
	char buff[1024];

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

int iRODSNtFileOpen(const char *filename,int oflag, int istextfile)
{
	int New_Oflag;
	char ntfp[1024];

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

int iRODSNtFileBinaryOpen(const char *filename,int oflag)
{
	int New_Oflag;
	char ntfp[1024];

	iRODSPathToNtPath(ntfp,filename);

	New_Oflag = _O_BINARY | oflag;
	return _open(ntfp,New_Oflag);
}

int iRODSNtFileBinaryCreate(const char *filename)
{
	char ntfp[1024];

	iRODSPathToNtPath(ntfp,filename);

	return _open(ntfp,_O_RDWR|_O_CREAT|_O_EXCL|_O_BINARY, _S_IREAD|_S_IWRITE);
}

int iRODSNtUnlinkFile(char *filename)
{
	char ntfp[1024];

	iRODSPathToNtPath(ntfp,filename);

	return _unlink(ntfp);
}

void iRODSNtCheckExecMode(int aargc,char **aargv)
{
	int i;

	nRunInConsole = 0;
	for(i=0;i<aargc;i++)
	{
		if(strcmp(aargv[i],"console") == 0)
		{
			nRunInConsole = 1;
			return;
		}
	}
}

int iRODSNtRunInConsoleMode()
{
	return nRunInConsole;
}

void NtEmergencyMessage(char *msg)
{
   int fd;
   fd = open("c:\\ntsrblog.txt",_O_CREAT|_O_APPEND|_O_WRONLY, _S_IREAD | _S_IWRITE);

   if(fd < 0)
     return;

   _write(fd,msg,strlen(msg));
   _close(fd);
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
      c = getch();

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

long getppid(void)
{
	return NULL;
}

int getopt(int argc, char* const argv[], const char* optstring)
{
	return 0;
}


int NTmkdir(const char *dirname)
{
	char buf[2048];
	int i,len;

	len = strlen(dirname);

	for(i = 0; i < len; i++)
	{
		if(dirname[i] == '/')
			buf[i] = '\\';
		else
			buf[i] = dirname[i];
	}
	buf[i] = '\0';

	len = mkdir(buf);

	return mkdir(buf);
}

