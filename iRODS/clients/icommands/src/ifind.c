/* 
 * ifind - The irods find-like utility (searches for collections/files) 
*/

#include "rodsClient.h"
#include "parseCommandLine.h"
#include "rodsPath.h"
#include "rcMisc.h"
#include "lsUtil.h"

/* Should be big enough for some working strings needed: */
#define BIG_STR_LEN 1024

#define NUMERIC_BAD -3
#define NUMERIC_LT -1
#define NUMERIC_EQ 0
#define NUMERIC_GT 1

#define IFIND_SIZE_TYPE_BLOCKS 1
#define IFIND_SIZE_TYPE_CHARS 2
#define IFIND_SIZE_TYPE_WORDS 3
#define IFIND_SIZE_TYPE_KB 4
#define IFIND_SIZE_TYPE_MB 5
#define IFIND_SIZE_TYPE_GB 6

/* This are the ifind options that use a global flag.  These are
   set via parsing the input arguments. */
int ifind_option_ls=0;      /* if non-0, option -ls has been specified */
int ifind_option_lsctime=0; /* if non-0, option -lsctime has been specified */
int ifind_option_type=0;    /* if non-0, option -type has been specified */
int ifind_option_debug=0;   /* if non-0, option -debug has been specified */
char ifind_option_name[200]=""; /* if non-blank, -name or -iname string */
int ifind_option_name_flag=0;   /* if non-0, -name has been specified */
int ifind_option_iname_flag=0;  /* if non-0, -iname has been specified */
int ifind_option_metalike=0;    /* if non-0, -metalike has been specified */

int ifind_option_size_type=IFIND_SIZE_TYPE_CHARS; /* the type of size
                    if -size has been entered;IFIND_SIZE_TYPE_BLOCKS,
                    _CHARS, etc, from above defines */

int size_do_colls=0; /* flag for size option to include collections */

/* 
  These are the types for various command line options:
 */
#define CONDITION_OPTION_TYPE_MMIN 1
#define CONDITION_OPTION_TYPE_CMIN 2
#define CONDITION_OPTION_TYPE_MTIME 3
#define CONDITION_OPTION_TYPE_CTIME 4
#define CONDITION_OPTION_TYPE_SIZE 5
#define CONDITION_OPTION_TYPE_INUM 6

#define CONDITION_OPTION_ACCESS_USER_NAME 7
#define CONDITION_OPTION_ACCESS_TYPE 8

#define CONDITION_OPTION_META_A 9  /* The Attribute of AVUs */
#define CONDITION_OPTION_META_V 10  /* The Value of AVUs */
#define CONDITION_OPTION_META_U 11  /* The Unit of AVUs */

#define CONDITION_STR_MAX_LEN 60
#define MAX_CONDITIONS 100
int conditionIndex=0;
int conditionOptionType[MAX_CONDITIONS];
char *conditionString[MAX_CONDITIONS];


/*
 add a numeric Condtion value; i.e. set up strings for 'where' conditions
 to be passed to the general query.

 The condition strings are set up like this so that there can be
 multiple of the same input type, for example -mmin -10 -mmin +3 to
 find files that have a modify time less than 10 and greater than 3
 minutes.
 */
void
addNumericCondition(int numericType, int conditionType, int value) {
   char nowStr[TIME_LEN];
   time_t myTimeValue;
   time_t myTimeValue2;
   char *myStr;
   if (ifind_option_debug) { 
      fprintf(stderr,"ntype:%d, ctype:%d, value:%d\n",
              numericType, conditionType, value);
   }
   if (conditionIndex >= MAX_CONDITIONS) {
      printf("Too many conditions\n");
      exit(1);
   }
   getNowStr(nowStr);
   myTimeValue=atoll(nowStr);

   if (conditionType==CONDITION_OPTION_TYPE_SIZE) {
      rodsLong_t myValue;
      myValue = value;
      if (ifind_option_size_type==IFIND_SIZE_TYPE_BLOCKS) myValue=myValue*512;
      if (ifind_option_size_type==IFIND_SIZE_TYPE_CHARS); /* already OK */
      if (ifind_option_size_type==IFIND_SIZE_TYPE_WORDS) myValue=myValue*2;
      if (ifind_option_size_type==IFIND_SIZE_TYPE_KB) myValue=myValue*1024;
      if (ifind_option_size_type==IFIND_SIZE_TYPE_MB) myValue=myValue*1048576;
      if (ifind_option_size_type==IFIND_SIZE_TYPE_GB) myValue=myValue*1073741824;
      myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
      if (numericType==NUMERIC_LT) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n< '%lld'",myValue);
         if (myValue>0) size_do_colls++;
      }
      if (numericType==NUMERIC_EQ) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n= '%lld'",myValue);
         if (myValue==0) size_do_colls++;
      }
      if (numericType==NUMERIC_GT) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n> '%lld'",myValue);
         size_do_colls=-10; /* even if there are some of above, skip it */
      }
      conditionOptionType[conditionIndex]=conditionType;
      conditionString[conditionIndex]=myStr;
      conditionIndex++;
      if (ifind_option_debug) { 
         fprintf(stderr, "condition: %s\n",myStr);
      }
      return;
   }
   if (conditionType==CONDITION_OPTION_TYPE_INUM) {
      rodsLong_t myValue;
      myValue = value;
      myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
      if (numericType==NUMERIC_LT) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n< '%lld'",myValue);
      }
      if (numericType==NUMERIC_EQ) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n= '%lld'",myValue);
      }
      if (numericType==NUMERIC_GT) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n> '%lld'",myValue);
      }
      conditionOptionType[conditionIndex]=conditionType;
      conditionString[conditionIndex]=myStr;
      conditionIndex++;
      if (ifind_option_debug) { 
         fprintf(stderr, "condition: %s\n",myStr);
     }
      return;
   }
   if (conditionType==CONDITION_OPTION_TYPE_MMIN) {
      myTimeValue=myTimeValue-(value*60); /* to seconds ago */
      myTimeValue2=myTimeValue-60;
   }
   if (conditionType==CONDITION_OPTION_TYPE_CMIN) {
      myTimeValue=myTimeValue-(value*60); /* to seconds ago */
      myTimeValue2=myTimeValue-60;
   }
   if (conditionType==CONDITION_OPTION_TYPE_MTIME) {
      myTimeValue=myTimeValue-(value*60*60*24); /* to seconds ago */
      myTimeValue2=myTimeValue-(60*60*24);
   }
   if (conditionType==CONDITION_OPTION_TYPE_CTIME) {
      myTimeValue=myTimeValue-(value*60*60*24); /* to seconds ago */
      myTimeValue2=myTimeValue-(60*60*24);
   }
   if (numericType==NUMERIC_LT) {
      myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
      snprintf(myStr,CONDITION_STR_MAX_LEN,
              "n> '%ld'",myTimeValue); /* LT N minutes ago means GT the val*/
   }
   if (numericType==NUMERIC_EQ) {
      /* For -amin (and similar) N (equal), we need to do the
         truncation and round-off like find does.  So we add 1 to
         the value can check that it's in that range.
      */
      myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
      snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "n< '%ld'",myTimeValue);
      conditionOptionType[conditionIndex]=conditionType;
      conditionString[conditionIndex]=myStr;
      conditionIndex++;
      if (ifind_option_debug) { 
         fprintf(stderr, "condition: %s\n",myStr);
      }

      myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
      snprintf(myStr,CONDITION_STR_MAX_LEN,
               "n> '%ld'",myTimeValue2);
   }
   if (numericType==NUMERIC_GT) {
      myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
      snprintf(myStr,CONDITION_STR_MAX_LEN,
               "n< '%ld'",myTimeValue); 
   }
   conditionOptionType[conditionIndex]=conditionType;
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition: %s\n",myStr);
   }
}

/*
 Parse an N argument,with optional + or - at the start, 
 and return the value and type.
 */
void
parseN(char *inStr,int *val, int *type) {
   *val=0;
   char c1;
   int myVal=0;
   if (inStr==NULL) {
      *type=NUMERIC_BAD;
      return;
   }
   c1=inStr[0];
   *type=NUMERIC_EQ;
   if (inStr[0]=='-') {*type=NUMERIC_LT; c1=inStr[1];}
   if (inStr[0]=='+') {*type=NUMERIC_GT; c1=inStr[1];}
   if (c1>'9' || c1 < '0') {
      *type=NUMERIC_BAD;
      return;
   }
   myVal=(atoi(inStr));
   *val=myVal;
   if (*type==-1)*val=-myVal;
}

void
printN(int val, int type) {
}

/*
  Parse and setup condition strings for an N argument
 */
void
parseAndSetupN(char *inputArg, int optionType, char *optionName) {
   int valueOfN;
   int numericType;
   parseN(inputArg, &valueOfN, &numericType);
   if (ifind_option_debug) {
      fprintf(stderr,"val=%d, type=%d\n",valueOfN,numericType);
   }
   if (numericType==NUMERIC_BAD) {
      fprintf(stderr, "Invalid %s argument\n", optionName);
      exit(1);
   }
   addNumericCondition(numericType, optionType, valueOfN);
}

/*
 Parse the optional suffix on -size and set a flag accordingly.
 */
void
parseSizeSuffix(char *inputArg) {
   char *cp;
   char *suffix;
   int isDigit;
   isDigit=0;
   suffix=NULL;
   for (cp=inputArg;*cp!='\0';cp++) {
      if (*cp>='0' && *cp<='9') {
         isDigit=1;
      }
      else {
         if (isDigit==1) {
            /* first non-digit character */
            suffix=cp;
            break;
         }
      }
   }
   if (suffix==NULL) return;
   if (*suffix=='b') ifind_option_size_type=IFIND_SIZE_TYPE_BLOCKS;
   if (*suffix=='c') ifind_option_size_type=IFIND_SIZE_TYPE_CHARS;
   if (*suffix=='w') ifind_option_size_type=IFIND_SIZE_TYPE_WORDS;
   if (*suffix=='k') ifind_option_size_type=IFIND_SIZE_TYPE_KB;
   if (*suffix=='M') ifind_option_size_type=IFIND_SIZE_TYPE_MB;
   if (*suffix=='G') ifind_option_size_type=IFIND_SIZE_TYPE_GB;
   if (ifind_option_debug) {
      fprintf(stderr,"size suffix:%c value:%d\n",*suffix,
              ifind_option_size_type);
   }
}

/*
  Parse and setup condition strings for an access argument
 */
void
parseAndSetupAccess(char *inputArg1, char *inputArg2) {
   char *myStr;
   if (inputArg1==NULL || inputArg2==NULL) {
      fprintf(stderr, "Invalid access argument\n");
      exit(1);
   }
   if (conditionIndex >= MAX_CONDITIONS-1) {
      printf("Too many conditions\n");
      exit(1);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg1);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_ACCESS_USER_NAME;
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition access user: %s\n",myStr);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   if (strncmp(inputArg2,"own",3)==0) {
      snprintf(myStr,CONDITION_STR_MAX_LEN,
               "= '%s'","own");
   }
   if (strncmp(inputArg2,"write",5)==0) {
      if (strncmp(inputArg2,"write+",6)==0) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s' || = '%s'","modify object", "own");
      }
      else {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'","modify object");
      }
   }
   if (strncmp(inputArg2,"read",4)==0) {
      if (strncmp(inputArg2,"read+",5)==0) {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s' || = '%s' || = '%s'",
                  "read object", "modify object", "own");
      }
      else {
         snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'", "read object");
      }
   }

   conditionOptionType[conditionIndex]=CONDITION_OPTION_ACCESS_TYPE;
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition access: %s\n",myStr);
   }
}

/*
  Parse and setup condition strings for a metaa argument
 */
void
parseAndSetupMetaA(char *inputArg1) {
   char *myStr;
   if (inputArg1==NULL) {
      fprintf(stderr, "Invalid metaa argument\n");
      exit(1);
   }
   if (conditionIndex >= MAX_CONDITIONS) { 
      printf("Too many conditions\n");
      exit(1);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg1);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_A; /* Attr of AVU*/
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaa Attribute: %s\n",myStr);
   }
}

/*
  Parse and setup condition strings for a metaav argument
 */
void
parseAndSetupMetaAV(char *inputArg1, char *inputArg2) {
   char *myStr;
   if (inputArg1==NULL || inputArg2==NULL) {
      fprintf(stderr, "Invalid metaav argument\n");
      exit(1);
   }
   if (conditionIndex >= MAX_CONDITIONS-1) { 
      printf("Too many conditions\n");
      exit(1);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg1);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_A; /* Attr of AVU*/
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaav Attribute: %s\n",myStr);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg2);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_V; /* V of AVU */
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaav Value: %s\n",myStr);
   }
}

/*
  Parse and setup condition strings for a metaavu argument
 */
void
parseAndSetupMetaAVU(char *inputArg1, char *inputArg2, char *inputArg3) {
   char *myStr;
   if (inputArg1==NULL || inputArg2==NULL || inputArg3==NULL) {
      fprintf(stderr, "Invalid metaavu argument\n");
      exit(1);
   }
   if (conditionIndex >= MAX_CONDITIONS-2) { 
      printf("Too many conditions\n");
      exit(1);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg1);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_A; /* Attr of AVU*/
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaavu Attribute: %s\n",myStr);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg2);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_V; /* V of AVU */
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaavu Value: %s\n",myStr);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg3);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_U; /* U of AVU */
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaavu Value: %s\n",myStr);
   }
}

/*
  Parse and setup condition strings for a metaacomp argument
 */
void
parseAndSetupMetaComp(char *inputArg1, char *inputArg2, char *inputArg3) {
   char *myStr;
   if (inputArg1==NULL) {
      fprintf(stderr, "Invalid metacomp argument\n");
      exit(1);
   }
   if (conditionIndex >= MAX_CONDITIONS-2) { 
      printf("Too many conditions\n");
      exit(1);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "= '%s'",inputArg1);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_A; /* Attr of AVU*/
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaacomp Attribute: %s\n",myStr);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
            "%s '%s'",inputArg2, inputArg3);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_V; /* V of AVU */
   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metacomp Value: %s\n",myStr);
   }
}

/*
  Parse and setup condition strings for a metalike argument
 */
void
parseAndSetupMetaLike(char *inputArg1) {
   char *myStr;
   char *cptr;
   if (inputArg1==NULL) {
      fprintf(stderr, "Invalid metalike argument\n");
      exit(1);
   }
   if (conditionIndex >= MAX_CONDITIONS) { 
      printf("Too many conditions\n");
      exit(1);
   }

   myStr = (char *)malloc(CONDITION_STR_MAX_LEN);
   snprintf(myStr,CONDITION_STR_MAX_LEN,
                  "like '%s'",inputArg1);
   conditionOptionType[conditionIndex]=CONDITION_OPTION_META_A; /* Attr of AVU*/

   /* convert wildcards to SQL form */
   for (cptr=myStr;*cptr!='\0';cptr++) {
      if (*cptr=='*') {
         *cptr='%';
      }
      if (*cptr=='?') {
         *cptr='_';
      }
   }

   conditionString[conditionIndex]=myStr;
   conditionIndex++;
   if (ifind_option_debug) { 
      fprintf(stderr, "condition metaalike Attribute: %s\n",myStr);
   }
}


/*
 Convert a string to upper case.
 */
void
convertToUpper( char *inString) {
   char c;
   char *cp;
   if (inString==NULL) return;
   cp=inString;
   c = *cp;
   while (c != '\0') {
      if ( c >= 'a' && c <= 'z' ) {
         c = c + ('A'-'a');
         *cp=c;
      }
      cp++;
      c = *cp;
   }
   return;
}


void
usage () {
   char *msgs[]={
"ifind - search for data-objects (files) in a directory (collection) hierarchy",
" ",
"Synopsis",
"     ifind [-h] [-t ticket] [path...] [expression...]",
" ",
"'ifind' is similar to the Linux/Unix 'find' command but operates in",
"the iRODS system to search the iRODS name space.  'ifind' implements a",
"subset of the extensive 'find' capabilities, but is a powerful tool",
"for searching iRODS space, particularly if you are familiar with",
"'find'.  Except for the use of tickets, it is also possible to do",
"many of these searches via 'iquest' (SQL-like syntax) or 'imeta'.",
"but 'ifind' can be an easy way to find iRODS files with particular",
"attributes.",
" ",
"If the iRODS instance ('zone') being used is running with 'strict'",
"access control (ACL) mode enabled, you will be prevented from viewing",
"data-object information for those that you do not have access to.",
"Access can be provided via 'ichmod' and/or via tickets.",
" ",
"Unlike 'find' the order of the expressions on the command line do not",
"matter in 'ifind'.  ",
" ",
"Also, there is no '-exec' in 'ifind' to execute commands on the found",
"data-objects.  Instead, it is recommended that you create a list of",
"objects via 'ifind', verify they are correct, and then use that as",
"input to another script or command.",
" ",
"Options:",
" ",
"-t ticket  (iRODS option)",
"   Use the specified ticket to access information for objects that",
"   have access provided by this ticket.  When you use -t, you will",
"   only see data-objects and collections to which access is provided",
"   via the ticket, so you may want to run ifind twice, once with -t",
"   and again without.",
" ",
"-h this help (iRODS option)",
" ",
"-ctime n ",
"   Find iRODS data-objects with a create time of n.  ",
"   Use:",
"       +n for greater than n",
"       -n for less than n",
"        n for exactly n",
" ",
"   This is similar to the 'find' -ctime option for file status change",
"   time, but is limited to create-time in iRODS.  This searches for",
"   objects (data-objects or collections) that were created n*24 hours",
"   ago.  Like 'find', when 'ifind' figures out how many 24-hour",
"   periods ago the object (data-object or collection) was created, any",
"   fractional part is ignored, so to match -ctime +1, an object has to",
"   have been created at least two days ago.",
" ",
"-cmin n",
"   Objects's create time was minutes ago.",
"   See comments for -ctime for meaning of n, +n, and -n.",
" ",
"-mtime n",
"   Objects's data was last modified n*24 hours ago.  In most cases the",
"   iRODS the modify-time is the time the data was written, however if",
"   the collection is moved (imv) that's considered a modification and",
"   the modify-time is updated.  In this sense, the -mtime is similar",
"   to 'find's -ctime.  But the -mtime option searches on the iRODS",
"   modify-time.  See the comments for -ctime to understand how",
"   rounding affects the interpretation of modification times.",
" ",
"   Also see comments for -ctime for meaning of n, +n, and -n.",
" ",
"-mmin n",
"   Object's data was last modified n minutes ago.",
"   See comments for -ctime for meaning of n, +n, and -n.",
" ",
"-name \"pattern\"",
"   Base of data-object name (the path with the leading collection names",
"   removed. The meta-characters '*' and '?', can be used to match any",
"   characters.  '*' matches any number of characters and each '?'",
"   matches any one character.  The collection names are matched more",
"   easily as any collection name that ends with \"pattern\" will be",
"   found.",
" ",
"-iname \"pattern\"",
"   Like -name, but the match is case insensitive.  For example, the",
"   patterns 'fo*' and 'F\?\?' match the data-object names 'Foo', 'FOO',",
"   'foo', 'fOo', etc.  When this is used, a case insensitive match is",
"   also done on the base collection (path) being searched so if there",
"   happens to be others that match this way, they will be included",
"   (this is due to a limitation in the general-query API function).",
"   Also because of this, the full path will be listed instead of the",
"   form entered on the command line.",
" ",
"-print print just the path/object-name and then the linefeed",
"   for objects (collections or data-objects).  This is the default.",
" ",
"-ls print a longer line of info and then the path/object-name.",
"   Columns displayed are object-id, 'coll' or 'd-obj' to indicate collection",
"   or data-object, size (for data-objects), modify-time, and path.",
"   If '-lsctime' is used then '-ls' displays the create-time instead of",
"   the modify-time.",
" ",
"-type [c|d] limit the results to just collections ('c') or ",
"   data-objects ('d').",
" ",
"-size n[cwbkMG]",
"   Find objects that are of size n.  The following suffixes can be used:",
"      'b'    for  512-byte blocks ",
"      'c'    for bytes (or characters); this is the default if no suffix",
"                  is used",
"      'w'    for two-byte words",
"      'k'    for Kilobytes (units of 1024 bytes)",
"      'M'    for Megabytes (units of 1048576 bytes)",
"      'G'    for Gigabytes (units of 1073741824 bytes)",
" ",
"   For the purposes of -size, collections are of size 0, so will be ",
"   found up for '-n' and '0' but not 'n' (other than 0) or '+n'. ",
"   See comments for -ctime for meaning of n, +n, and -n.",
" ",
"-access user-or-group read[+]|write[+]|own",
"   Find object to which the user or group has read, write or own",
"   access permissions.  If the type is followed by a '+' it means",
"   'or better', i.e. read+ means read, write or own and write+ means",
"   write or own.  Due to a limitation in the general-query",
"   functionality, only one -access option can be used in 'ifind'",
"   at a time.",
" ",
"-inum n   Find objects that are of object-id n. Object-ids within the",
"   instance (zone) increase each time an object is created (data-object, ",
"   collection, user, group, resource, etc) so it can be used to locate",
"   data-objects and collections that were created before or after others.",
"   See comments for -ctime for meaning of n, +n, and -n.",
" ",
"-metaa A    Find objects with an Attribute name of 'A' (whatever is input).",
"            The Value and Units of the AVU (user-defined meta-data) can be",
"            any values, the search is only on the Attribute name.",
"            For example, -metaa Distance.",
" ",
"-metaav A V",
"            Find objects with Attribute name 'A' and a Value of 'V' (input).",
"            For example, -metaav Distance 78",
" ",
"-metaavu A V U",
"            Find objects with Attribute name 'A', Value of 'V', and Units",
"            of 'U'.  For example, -metaavu Distance 78 kilometers",
" ",
"-metacomp A op string",
"            Find objects with Attribute name 'A' and a value that matches",
"            the 'op string' condition.  'op' can be < > n< n> <= >= n<= n>=",
"            or = (altho for = you can use -metaav).  The 'n' types ('n<' ",
"            etc) are for numeric comparisons instead of string.",
"            For example: -metacomp Distance 'n<' 100",
" ",
"-metalike string",
"            Find objects with Attributes with names that match the string",
"            using wildcards.  '*' means any number of any characters ",
"            and  '?' means any one character.  For example:",
"            -metalike 'Dist*'",
"            Normally -metalike will do a case sensitive match, but if",
"            -iname is used -metalike will do a case-insensitive match (this",
"            is necessary since -iname is doing a case-insensitive search).",
" ",
"-debug (non-standard option)  ",
"   Print some debug information to stderr.  This is primarily",
"   of use to developers but also prints time-values in integer form.",
" ",
"-lsctime (non-standard option)  ",
"   If -ls is used, -lsctime will cause ifind to list the object's",
"   create-time instead of the default modify-time. This can be",
"   useful when using -ctime or -cmin.",
" ",
"The input collection name can be . (currently collection), '..' for up",
"one level from the current path, or a full collection path.  More than",
"one path can be entered.  If no collection is entered, '.' is assumed.",
" ",
"Examples:",
" ",
" ifind . -name \"foo*\"     List all objects (data-objects or collections)",
"                          under the current collection with a name that",
"                          starts with \"foo\".",
" ",
" ifind . -type d -name \"foo*\"",
"                          As above, but just find data-objects.",
" ",
" ifind /tempZone/home/rods -mtime -2",
"                          Find objects (collections and data-objects) under",
"                          /tempZone/home/rods that have been modified within",
"                          the last 2 days.",
" ",
" ifind . -ctime +50 -ls",
"                          Find objects under the current collection that were",
"                          created more than 50 days ago.  List them in the",
"                          longer format.",
" ",
" ifind . -ctime +50 -name \"foo*\" -ls",
"                          As above but only for objects with names that begin",
"                          with \"foo\".",
" ",
" ifind -t myTicket1 /tempZone/home/rods",
"                          List all the collections and data-objects",
"                          under /tempZone/home/rods to which myTicket1",
"                          provides access.  If the ticket is for a",
"                          collection, this will list all the",
"                          data-objects in that collection, although",
"                          you may not have read access to the",
"                          data-object itself. See",
"                          http://wiki.irods.org/index.php/Ticket-based_Access",
"                          for more.",
" ",
" ifind . -size +100000000 ",
"                          Find data-objects that are more than 10,000,000",
"                          bytes in size.",
" ",
" ifind . -size +90000000 -size -10000000 -ls",
"                          Find data-objects that are more than 9,000,000 bytes ",
"                          and less than 10,000,000 bytes in size.",
" ",
" ifind . -size +9M -ctime -3 -ls",
"                          Find data-objects larger than 9 MegaBytes",
"                          and that were created less than 3 days ago.",
" ",
" ifind . -ctime +50 -ctime -52 -ls",
"                          Find objects that are between 50 and 52 days old.",
" ",
" ifind . -ctime -1 -access rods write+ -ls",
"                          Find objects created in the last day that user",
"                          'rods' has write or better access to.",
" ",
" ifind . -ctime -50 -metalike Distance 'n<' 30 -ls",
"                          Find objects that are less than 50 days old that ",
"                          also have AVUs called 'Distance' with a value",
"                          less than 30.",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("ifind");
}

void
printFormatted(FILE *fd, char *format, char *args[], int nargs) {
   if (nargs==1) {
      fprintf(fd,format, args[0]);
   }
   if (nargs==2) {
      fprintf(fd,format, args[0], args[1]);
   }
   if (nargs==3) {
      fprintf(fd,format, args[0], args[1], args[2]);
   }
   if (nargs==4) {
      fprintf(fd,format, args[0], args[1], args[2], args[3]);
   }
   if (nargs==5) {
      fprintf(fd,format, args[0], args[1], args[2], args[3], args[4]);
   }
   if (nargs==6) {
      fprintf(fd,format, args[0], args[1], args[2], args[3], args[4],
             args[5]);
   }
   if (nargs==7) {
      fprintf(fd,format, args[0], args[1], args[2], args[3], args[4],
             args[5], args[6]);
   }
   if (nargs==8) {
      fprintf(fd,format, args[0], args[1], args[2], args[3], args[4],
             args[5], args[6], args[7]);
   }
   if (nargs==9) {
      fprintf(fd,format, args[0], args[1], args[2], args[3], args[4],
             args[5], args[6], args[7], args[8]);
   }
   if (nargs==10) {
      fprintf(fd,format, args[0], args[1], args[2], args[3], args[4],
             args[5], args[6], args[7], args[8], args[9]);
   }
}


/*
 Print the GenQueryOut with a few conversions/formatting for ifind.

 For the first column, if it matches expandedCollName it is converted
 to the unexpandedCollName.  This is to match 'find's behaviour.  If
 the user enters '.' (or blank) for the collection name, that's what's
 echoed back as part of the path for each object found.

 For items that are time-values (the timeCol define indicates which
 one), it is converted to date-time form.

 And for numbers, pad a with some blanks to make them line up better
 most of the time.
*/
#define timeCol 4  /* The column that is a time value  */
int 
printGenQueryInFindFormat(FILE *fd, char *format, 
                                          char *expandedCollName,
                                          char *unexpandedCollName,
                                          genQueryOut_t *genQueryOut) {
   int i, j, k;

   int padColOne;
   int padColThree;
   /* First, go thru the results and determine how much padding is needed
      for the two numeric columns that we pad (0 and 3)
    */
   padColOne=0;
   padColThree=0;
   if (genQueryOut->attriCnt>3)  {  /* doing a long line */
      int maxLenColOne=0;
      int maxLenColThree=0;
      for (i=0;i<genQueryOut->rowCnt;i++) {
         char *tResult;
         for (j=0;j<genQueryOut->attriCnt;j++) {
            tResult = genQueryOut->sqlResult[j].value;
            tResult += i*genQueryOut->sqlResult[j].len;
            if (j==0) {
               int tResultLen;
               tResultLen=strlen(tResult);
               if (tResultLen>maxLenColOne) maxLenColOne=tResultLen;
            }
            if (j==3) {
               int tResultLen;
               tResultLen=strlen(tResult);
               if (tResultLen>maxLenColThree) maxLenColThree=tResultLen;
            }
         }
      }
      padColOne=maxLenColOne;
      padColThree=maxLenColThree;
   }

   if (ifind_option_debug) {
      fprintf(stderr,"ex:%s unex:%s\n",expandedCollName,unexpandedCollName);
   }

   for (i=0;i<genQueryOut->rowCnt;i++) {
      char *results[20];
      k=0;
      for (j=0;j<genQueryOut->attriCnt;j++) {
         char *tResult;
         char modifiedName[BIG_STR_LEN];
         int lenExp;
         char *cp2;
         char localTime[20];
         char paddedStr1[200];
         char paddedStr2[200];
         int lenResult;
         lenExp = strlen(expandedCollName);
         tResult = genQueryOut->sqlResult[j].value;
         tResult += i*genQueryOut->sqlResult[j].len;
         results[k]=tResult;
         if (strncmp(tResult,expandedCollName, lenExp)==0) {
            strncpy(modifiedName, unexpandedCollName, BIG_STR_LEN);
            cp2 = tResult+lenExp;
            strncat(modifiedName, cp2, BIG_STR_LEN);
            results[k]=modifiedName;
         }
         lenResult=strlen(tResult);
         if (lenResult==11 && j==timeCol) {
            getLocalTimeFromRodsTime(tResult, localTime);
            results[k]=localTime;
            if (ifind_option_debug) { 
               fprintf(stderr,"time ivalue:%s localTime:%s\n", 
                       tResult, localTime);
            }
         }
         if (j==0 && padColOne>0) {
            int nblanks;
            strncpy(paddedStr1,"                                        ", 40);
            nblanks = padColOne-lenResult;
            if (nblanks<0) nblanks=0;
            if (nblanks>25) nblanks=25;
            strncpy(paddedStr1,"                                        ", 40);
            cp2=paddedStr1+nblanks;
            strncpy(cp2,tResult,lenResult+1);
            results[k]=paddedStr1;
         }
         if (j==3 && padColThree>0) {
            int nblanks;
            nblanks = padColThree-lenResult;
            if (nblanks<0) nblanks=0;
            if (nblanks>25) nblanks=25;
            strncpy(paddedStr2,"                                        ", 40);
            cp2=paddedStr2+nblanks;
            strncpy(cp2,tResult,lenResult+1);
            results[k]=paddedStr2;
         }
         k++;
      }
      printFormatted(fd, format, results, k);
   }
   return(0);
}

/*
 Query and show the results for collections.
 */
int
queryAndShowColls(rcComm_t *conn, char *collection, char *unexpandedColl)
{

  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut = NULL;

  char format[100]="%s\n";

  memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   int selectIndexes[10];
   int selectValues[10];
   int conditionIndexes[10];
   char *conditionValues[10];
   char conditionString1[BIG_STR_LEN];
   char conditionStringName[BIG_STR_LEN];
   int status;

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));

   selectIndexes[0]=COL_COLL_NAME;
   selectValues[0]=0; 
   genQueryInp.selectInp.len = 1;
   genQueryInp.selectInp.inx = selectIndexes;
   genQueryInp.selectInp.value = selectValues;

   if (ifind_option_ls) {
      int i=0;
      selectIndexes[i]=COL_COLL_ID;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_COLL_OWNER_NAME;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_COLL_OWNER_ZONE;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_COLL_INHERITANCE;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_COLL_MODIFY_TIME;
      if (ifind_option_lsctime) {
         selectIndexes[i]=COL_COLL_CREATE_TIME;
      }
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_COLL_NAME;
      selectValues[i]=0; 
      i++;
      genQueryInp.selectInp.len = i;
      strncpy(format,"%s coll  %s#%s %s %s %s\n", sizeof(format));
   }

   snprintf(conditionString1,sizeof(conditionString1),
            "= '%s' || like '%s/%s'", collection, collection, "%");

   conditionIndexes[0]=COL_COLL_NAME;

   conditionValues[0]=conditionString1;

   genQueryInp.sqlCondInp.inx = conditionIndexes;
   genQueryInp.sqlCondInp.value = conditionValues;
   genQueryInp.sqlCondInp.len=1;

   if (ifind_option_name_flag) {
      int i;
      i = genQueryInp.sqlCondInp.len;
      snprintf(conditionStringName,sizeof(conditionStringName),
               "like '%s%s'","%",ifind_option_name);

      if (ifind_option_iname_flag) {
         genQueryInp.options = UPPER_CASE_WHERE;
      }
      conditionIndexes[1]=COL_COLL_NAME;
      conditionValues[1]=conditionStringName;
      genQueryInp.sqlCondInp.len++;
   }

   int ix;
   for (ix=0;ix<conditionIndex;ix++) {
      int i;
      i = genQueryInp.sqlCondInp.len;
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_MMIN) {
         conditionIndexes[i]=COL_COLL_MODIFY_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_CMIN) {
         conditionIndexes[i]=COL_COLL_CREATE_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_MTIME) {
         conditionIndexes[i]=COL_COLL_MODIFY_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_CTIME) {
         conditionIndexes[i]=COL_COLL_CREATE_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_INUM) {
         conditionIndexes[i]=COL_COLL_ID;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_ACCESS_USER_NAME) {
         conditionIndexes[i]=COL_COLL_USER_NAME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_ACCESS_TYPE) {
         conditionIndexes[i]=COL_COLL_ACCESS_NAME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_META_A) {
         conditionIndexes[i]=COL_META_COLL_ATTR_NAME;
         if (ifind_option_metalike>0 && ifind_option_iname_flag>0) {
            /* if we're doing metalike and iname, need to upper-case
               the metalike conditionString since the query will be
               done in UPPER_CASE_WHERE mode.  The help text mentions
               this (metalike will be case-insensitive too). */
            convertToUpper(conditionString[ix]);
         }
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_META_V) {
         conditionIndexes[i]=COL_META_COLL_ATTR_VALUE;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_META_U) {
         conditionIndexes[i]=COL_META_COLL_ATTR_UNITS;
      }

      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_SIZE) {
         if (size_do_colls<=0) return(0); /* Flag indicates a size option
                                          that would exclude collections
                                          so just return */
         break; /* can't check size on collections so break out of loop 
                    to exclude it */
      }
      conditionValues[i]=conditionString[ix];
      genQueryInp.sqlCondInp.len++;
   }

   genQueryInp.maxRows= MAX_SQL_ROWS;
   genQueryInp.continueInx=0;
   status = rcGenQuery (conn, &genQueryInp, &genQueryOut);
   if (status < 0) {
      return(status);
   }
   status = printGenQueryInFindFormat(stdout, format, 
                                      collection, 
                                      unexpandedColl,
                                      genQueryOut);
   if (status < 0) {
      return(status);
   }
   while (status==0 && genQueryOut->continueInx > 0) {
      genQueryInp.continueInx=genQueryOut->continueInx;
      status = rcGenQuery (conn, &genQueryInp, &genQueryOut);
      if (status < 0) {
         return(status);
      }
      status = printGenQueryInFindFormat(stdout, format, 
                                                      collection, 
                                                      unexpandedColl,
                                                      genQueryOut);
      if (status < 0) {
         return(status);
      }
  }
  return(0);
}

/*
 Query and show the results for data-objects.
 */
int
queryAndShowDataObjs(rcComm_t *conn, char *collection, char *unexpandedColl)
{
  genQueryInp_t genQueryInp;
  genQueryOut_t *genQueryOut = NULL;
  char format[100]="%s/%s\n";

  memset (&genQueryInp, 0, sizeof (genQueryInp_t));
  int selectIndexes[10];
  int selectValues[10];
  int conditionIndexes[10];
  char *conditionValues[10];
  char conditionString1[BIG_STR_LEN];
  char conditionStringName[BIG_STR_LEN];
  int status;
  int ix;
  
  memset (&genQueryInp, 0, sizeof (genQueryInp_t));

  genQueryInp.selectInp.inx = selectIndexes;
  genQueryInp.selectInp.value = selectValues;
  if (ifind_option_ls) {
     int i=0;
     selectIndexes[i]=COL_D_DATA_ID;
     selectValues[i]=0; 
     i++;
     selectIndexes[i]=COL_D_OWNER_NAME;
     selectValues[i]=0; 
     i++;
      selectIndexes[i]=COL_D_OWNER_ZONE;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_DATA_SIZE;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_D_MODIFY_TIME;
      if (ifind_option_lsctime) {
         selectIndexes[i]=COL_D_CREATE_TIME;
      }
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_COLL_NAME;
      selectValues[i]=0; 
      i++;
      selectIndexes[i]=COL_DATA_NAME;
      selectValues[i]=0;
      i++;
      genQueryInp.selectInp.len = i;
      strncpy(format,"%s d-obj %s#%s %s %s %s/%s\n", sizeof(format));
   }
   else {
      selectIndexes[0]=COL_COLL_NAME;
      selectValues[0]=0; 
      selectIndexes[1]=COL_DATA_NAME;
      selectValues[1]=0;
      genQueryInp.selectInp.len = 2;
   }

   conditionIndexes[0]=COL_COLL_NAME;

   snprintf(conditionString1,sizeof(conditionString1),"= '%s' || like '%s/%s'",
            collection, collection, "%");

   conditionValues[0]=conditionString1;

   genQueryInp.sqlCondInp.inx = conditionIndexes;
   genQueryInp.sqlCondInp.value = conditionValues;
   genQueryInp.sqlCondInp.len=1;

   for (ix=0;ix<conditionIndex;ix++) {
      int i;
      i = genQueryInp.sqlCondInp.len;
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_SIZE) {
         conditionIndexes[i]=COL_DATA_SIZE;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_INUM) {
         conditionIndexes[i]=COL_D_DATA_ID;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_MMIN) {
         conditionIndexes[i]=COL_D_MODIFY_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_CMIN) {
         conditionIndexes[i]=COL_D_CREATE_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_MTIME) {
         conditionIndexes[i]=COL_D_MODIFY_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_TYPE_CTIME) {
         conditionIndexes[i]=COL_D_CREATE_TIME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_ACCESS_USER_NAME) {
         conditionIndexes[i]=COL_DATA_USER_NAME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_ACCESS_TYPE) {
         conditionIndexes[i]=COL_DATA_ACCESS_NAME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_META_A) {
         conditionIndexes[i]=COL_META_DATA_ATTR_NAME;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_META_V) {
         conditionIndexes[i]=COL_META_DATA_ATTR_VALUE;
      }
      if (conditionOptionType[ix]==CONDITION_OPTION_META_U) {
         conditionIndexes[i]=COL_META_DATA_ATTR_UNITS;
      }
      conditionValues[i]=conditionString[ix];
      genQueryInp.sqlCondInp.len++;
   }

   if (ifind_option_name_flag) {
      int i;
      i = genQueryInp.sqlCondInp.len;
      if (ifind_option_name_flag==1) {
         snprintf(conditionStringName,sizeof(conditionStringName),
                  "= '%s'",ifind_option_name);
      }
      else {
         snprintf(conditionStringName,sizeof(conditionStringName),
                  "like '%s'",ifind_option_name);
      }
      if (ifind_option_iname_flag) {
         genQueryInp.options = UPPER_CASE_WHERE;
      }
      i = genQueryInp.sqlCondInp.len;
      conditionIndexes[i]=COL_DATA_NAME;
      conditionValues[i]=conditionStringName;
      genQueryInp.sqlCondInp.len++;
   }

   genQueryInp.maxRows= MAX_SQL_ROWS;
   genQueryInp.continueInx=0;
   status = rcGenQuery (conn, &genQueryInp, &genQueryOut);
   if (status < 0) {
      return(status);
   }

   status = printGenQueryInFindFormat(stdout, format,
                                      collection, 
                                      unexpandedColl,
                                      genQueryOut);
  if (status < 0)
    return(status);


  while (status==0 && genQueryOut->continueInx > 0) {
     genQueryInp.continueInx=genQueryOut->continueInx;
     status = rcGenQuery (conn, &genQueryInp, &genQueryOut);
     if (status < 0)
        return(status);
     status = printGenQueryInFindFormat(stdout, format,
                                        collection, 
                                        unexpandedColl,
                                        genQueryOut);

     if (status < 0)
        return(status);
  }

  return(0);

}

/*
 Process a path/collection that the user entered (or the default '.').
 The collection is the expended name (for example, '.' turning into
 the full path) and unexpandedCollection is is as the user provided it
 (which is then echoed back in the found sub-items).
 */
int
processPath(rcComm_t *conn, char *collection, char *unexpandedCollection) {
   int status;
   if (ifind_option_type!=1) {
      status = queryAndShowColls(conn, collection,
                                  unexpandedCollection);
      if (status == CAT_NO_ROWS_FOUND) status=0;
      if (status < 0) {
         return(status);
      }
   }
   if (ifind_option_type!=2) {
      status = queryAndShowDataObjs(conn, collection,
                                    unexpandedCollection);
      if (status == CAT_NO_ROWS_FOUND) status=0;
   }
   return(status);
}


#define MAX_ARGS 30
int
main(int argc, char **argv) {
    int status;
    rodsEnv myEnv;
    rErrMsg_t errMsg;
    rcComm_t *conn;
    rodsArguments_t myRodsArgs;
    char *optStr;

    int irodsArgc;
    char *irodsArgv[MAX_ARGS];
    int ifindArgc;
    char *ifindArgv[MAX_ARGS];
    int i, ix;

    rodsPathInp_t rodsPathInp;

    optStr = "ht:";
   
    /* Move the standard iRODS args into irodsArgv and parse that.  At
       the same time, set of ifileArgs for the remaining args.
    */
    irodsArgc = 0;
    ifindArgc = 0;
    int doIfind;
    for (i=0;i<argc&&i<MAX_ARGS;i++) {
       doIfind=1;
       if (i==0) {
          irodsArgv[irodsArgc]=argv[i];
          ifindArgv[irodsArgc]=argv[i];
          irodsArgc++;
          ifindArgc++;
          doIfind=0;
       }
       if (strncmp(argv[i], "-h", 2)==0) {
          irodsArgv[irodsArgc]=argv[i];
          irodsArgc++;
          doIfind=0;  /* don't do this one as an ifind argument as it
                         was an irods one */
       }
       if (strlen(argv[i])==2 && strncmp(argv[i], "-t", 2)==0) {
          irodsArgv[irodsArgc]=argv[i];
          irodsArgc++;
          if (irodsArgc>=MAX_ARGS) {
             printf("Invalid input, too many arguments\n");
             exit(6);
          }
          i++;
          irodsArgv[irodsArgc]=argv[i];
          irodsArgc++;
          doIfind=0;
       }
       if (doIfind==1) {
          int isOption;
          isOption=0;
          if (strncmp(argv[i],"-ls",3)==0) {
             ifind_option_ls=1;
             isOption=1;
          }
          if (strncmp(argv[i],"-lsctime",8)==0) {
             ifind_option_lsctime=1;
             isOption=1;
          }
          if (strncmp(argv[i],"-print",6)==0) {
             ifind_option_ls=0;
             isOption=1;
          }
          if (strncmp(argv[i],"-size",5)==0) {
             parseSizeSuffix(argv[i+1]);
             parseAndSetupN(argv[i+1], CONDITION_OPTION_TYPE_SIZE, "size");
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-inum",5)==0) {
             parseAndSetupN(argv[i+1], CONDITION_OPTION_TYPE_INUM, "inum");
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-cmin",5)==0) {
             parseAndSetupN(argv[i+1], CONDITION_OPTION_TYPE_CMIN, "cmin");
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-mmin",5)==0) {
             parseAndSetupN(argv[i+1], CONDITION_OPTION_TYPE_MMIN, "mmin");
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-ctime",6)==0) {
             parseAndSetupN(argv[i+1], CONDITION_OPTION_TYPE_CTIME, "ctime");
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-mtime",6)==0) {
             parseAndSetupN(argv[i+1], CONDITION_OPTION_TYPE_MTIME, "mtime");
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-access",7)==0) {
             static int accessOptCount=0;
             if (accessOptCount) {
                fprintf(stderr, "Multiple -access options are not allowed due to a\n");
                fprintf(stderr, "to a limitation in the general-query fucntionality\n");
                fprintf(stderr, "which would cause it to return incorrect results.\n");
                exit(1);
             }
             accessOptCount++;
             parseAndSetupAccess(argv[i+1], argv[i+2]);
             i++;
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-metaavu",8)==0) {
             parseAndSetupMetaAVU(argv[i+1], argv[i+2], argv[i+3]);
             i++;
             i++;
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-metaav",7)==0 && isOption==0) { 
                    /* skip this one if -metaavu above was matched*/
             parseAndSetupMetaAV(argv[i+1], argv[i+2]);
             i++;
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-metaa",6)==0 && isOption==0) { 
                    /* skip this one if one of the above meta's was matched*/
             parseAndSetupMetaA(argv[i+1]);
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-metacomp",9)==0) { 
             parseAndSetupMetaComp(argv[i+1], argv[i+2], argv[i+3]);
             i++;
             i++;
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-metalike",9)==0) { 
             parseAndSetupMetaLike(argv[i+1]);
             ifind_option_metalike++; 
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-name",5)==0 || strncmp(argv[i],"-iname",6)==0) {
             char *cptr;
             int isIname=0;
             if (strncmp(argv[i],"-iname",6)==0) { isIname=1; }
             strncpy(ifind_option_name, argv[i+1], sizeof(ifind_option_name));
             if (strlen(ifind_option_name)==0) {
                if (isIname) {
                   printf("Invalid iname argument\n");
                }
                else {
                   printf("Invalid name argument\n");
                }
                exit(1);
             }
             ifind_option_name_flag=1;
             for (cptr=ifind_option_name;*cptr!='\0';cptr++) {
                if (*cptr=='*') {
                   *cptr='%';
                   ifind_option_name_flag++;
                }
                if (*cptr=='?') {
                   *cptr='_';
                   ifind_option_name_flag++;
                }
             }
             if (isIname) {
                convertToUpper(ifind_option_name);
                ifind_option_iname_flag++;
             }
             if (ifind_option_debug) {
                fprintf(stderr,"name_flag:%d iname_flag:%d name:%s\n",
                        ifind_option_name_flag,
                        ifind_option_iname_flag, ifind_option_name);
             }
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-type",5)==0) {
             if (strncmp(argv[i+1],"d",1)==0) ifind_option_type=1;
             if (strncmp(argv[i+1],"c",1)==0) ifind_option_type=2;
             if (ifind_option_type==0) {
                status = SYS_INVALID_INPUT_PARAM;
                printf("invalid -type type, see ifind -h\n");
                exit(2);
             }
             i++;
             isOption=1;
          }
          if (strncmp(argv[i],"-debug",6)==0) {
             ifind_option_debug=1;
             isOption=1;
          }
          if (isOption==0) {
             char *cptr;
             cptr=argv[i];
             if (*cptr=='-') {
                printf("ifind: unknown option %s\n",argv[i]);
                exit(1);
             }
             ifindArgv[ifindArgc]=argv[i];
             ifindArgc++;
          }
       }
    }
    if (ifind_option_debug) {
       int k;
       for (k=0;k<irodsArgc;k++) {
          fprintf(stderr,"irods Arg:%s\n",irodsArgv[k]);
       }
    }

    status = parseCmdLineOpt (irodsArgc, irodsArgv, optStr, 1, &myRodsArgs);

    if (status < 0) {
        printf("Use -h for help\n");
        exit (1);
    }
    if (myRodsArgs.help==True) {
       usage();
       exit(0);
    }

    status = getRodsEnv (&myEnv);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        exit (1);
    }

    if (ifindArgc==1) {
       ifindArgv[1]=".";
       ifindArgc++;
    }

    if (ifind_option_debug) {
       int k;
       for (k=0;k<ifindArgc;k++) {
          fprintf(stderr,"ifind Arg:%s\n",ifindArgv[k]);
       }
       fprintf(stderr,"optind:%d\n",optind);
    }
    /* If the user enters multiple collection paths, parseCmdLinePath
       will return multiple 'source' paths and one 'destination', but
       we'll use them all as input paths.  But parseCmdLinePath handles
       cases like "d1", "/d2", "..", etc.
     */

    optind=1;  /* global variable where non-recognized options begin.
                  we set it to 1 here as the index into our ifindArgv array. */
    status = parseCmdLinePath (ifindArgc, ifindArgv, optind, &myEnv,
                               UNKNOWN_OBJ_T, UNKNOWN_OBJ_T, 0, &rodsPathInp);

    if (ifind_option_debug) {
       fprintf(stderr,"ifindArgv=%d\n",ifindArgc);
       for (i=0;i<rodsPathInp.numSrc;i++) {
          fprintf(stderr,"src %d: %s\n",i,rodsPathInp.srcPath[i].outPath);
       }
       fprintf(stderr,"dest: %s\n",rodsPathInp.destPath[0].outPath);
    }

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: parseCmdLinePath error. ");
        printf("Use -h for help.\n");
        exit (1);
    }

    conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
      myEnv.rodsZone, 0, &errMsg);

    if (conn == NULL) {
        exit (2);
    }

    status = clientLogin(conn);
    if (status != 0) {
       exit (3);
    }

    if (myRodsArgs.ticket == True) {
       if (myRodsArgs.ticketString == NULL) {
          rodsLog (LOG_ERROR,
                   "ifind: NULL ticketString error");
          return (USER__NULL_INPUT_ERR);
       } else {
          setSessionTicket(conn, myRodsArgs.ticketString);
       }
    }

    for (i=0;i<rodsPathInp.numSrc && status==0;i++) {
       if (ifind_option_iname_flag) {
          convertToUpper(rodsPathInp.srcPath[i].outPath);
       }
       status = processPath(conn, rodsPathInp.srcPath[i].outPath,
                      rodsPathInp.srcPath[0].inPath);
    }

    if (ifindArgc-1>rodsPathInp.numSrc && status==0) {
       if (ifind_option_iname_flag) {
          convertToUpper(rodsPathInp.destPath[0].outPath);
       }
       status = processPath(conn, rodsPathInp.destPath[0].outPath,
                      rodsPathInp.destPath[0].inPath);

    }

    /* It is not really necessary, but but go ahead and free the
       malloc'ed strings so it's all neat and tidy. */
    for (ix=0;ix<conditionIndex;ix++) {
       free(conditionString[ix]);
    }

    rcDisconnect(conn);

    if (status < 0) {
       rodsLogError(LOG_ERROR,status,"ifind Error: query failed");
       exit(5);
    }
    return(0);
}
