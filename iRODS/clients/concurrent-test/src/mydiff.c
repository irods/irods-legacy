/*******************************************************************************
*
*  Mydiff.c Compares the output of two srb test result files, and makes sure that
*  there are no errors.
*
*  usage: ./mydiff file1 file2 n
*  
*  the "n" is optional, it is a number indicating
*  how many errors the user would like displayed. 
*  
*  07/09/2000
*  By: Arcot Rajasekar
*  Modified By: Roman Olschanowsky
*
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#if defined(PORTNAME_c90) || defined(PORTNAME_solaris)
#include <fcntl.h>
#endif
#include <sys/types.h>
#if defined(PORTNAME_c90)
#include <sys/jtab.h>
#endif

#define MAX_PHY_RESOURCE 10
char testid1[40],testid2[40];
char testdate1[40],testdate2[40];
char line1[300], line2[300];
char user1[200], user2[200];
char domain1[200],domain2[200];
char userdomain1[400],userdomain2[400];
char phyrsrc[2][MAX_PHY_RESOURCE][300];
char resource1[300],resource2[300];
char resourceShort1[20],resourceShort2[20];
char ticketHost1[100],ticketHost2[100];
char zone1[200],zone2[200];
int error;

/******************************************************************************
* Gets all physical resource names
******************************************************************************/
int
getPhyResources(char *fname, int ii)
{
  char *tmpPtr;
  int i = 0;
  int j = 0;
  int k = 0;
  char sysString[500];
  FILE *fd;

  sprintf(sysString,"grep 'phy_rsrc_name: ' %s  | sort -ur -o ../TMP/test1.sort.%d",fname, getpid());
  system(sysString);
  sprintf(sysString,"../TMP/test1.sort.%d", getpid());
  fd = fopen (sysString,"r");
  if (fd == NULL) {
     printf("error opening input file:%s\n",sysString);
      exit(1);
  }
  while (fgets(sysString, 498, fd) != NULL  && i < MAX_PHY_RESOURCE) {
    if ((tmpPtr = strstr(sysString,"phy_rsrc_name: ")) != NULL) {
      strcpy(phyrsrc[ii][i], (char *) (tmpPtr +strlen("phy_rsrc_name: ")));
      phyrsrc[ii][i][strlen(phyrsrc[ii][i]) - 1]  ='\0';
      i++;
    }
  }
  if (i == MAX_PHY_RESOURCE) {
     printf("error max resource limit of %i exceeded foe %s\n", MAX_PHY_RESOURCE, fname);
      exit(1);
  }
  strcpy(phyrsrc[ii][i],"ENDOFRESOURCES");
  return(0);
}
/******************************************************************************
* Gets the Date, User, Domain, Resource and Zone information from result files.
******************************************************************************/
int
gettestId_Date_User_Domain(FILE *file, char *testid, char *testdate, char* user, char *domain, char *resource, char *tHost, char *zone)
{
  char * tmpPtr;
  int i = 0;
  int j = 0;
  int k = 0;
  int l = 0;
  int m = 0;
  int p = 0;
  if (fgets(line1, 299, file) != NULL) {
    if ((tmpPtr = strstr(line1,"HOST=")) != NULL) {
      strcpy(tHost, (char *) (tmpPtr +strlen("HOST=")));
      tHost[strlen(tHost) - 1] ='\0';
    }
  }
  while (fgets(line1, 299, file) != NULL) {
    if (i == 0 && (tmpPtr = strstr(line1,"TESTID = ")) != NULL) {
      strcpy(testid, (char *) (tmpPtr +strlen("TESTID = ")));
      testid[strlen(testid) - 1] ='\0';
      i = 1;
    }
    if (j == 0 && (tmpPtr = strstr(line1,"TESTDATE = ")) != NULL) {
      strcpy(testdate, (char *) (tmpPtr +strlen("TESTDATE = ")));
      testdate[strlen(testdate) -1] ='\0';
      j = 1;
    }
    if (k == 0 && (tmpPtr = strstr(line1,"user_name: ")) != NULL) {
      strcpy(user, (char *) (tmpPtr +strlen("user_name: ")));
      user[strlen(user) -1] ='\0';
      k = 1;
    }
    if (l == 0 && m != 0 && (tmpPtr = strstr(line1,"domain_desc: ")) != NULL) {
      strcpy(domain, (char *) (tmpPtr +strlen("domain_desc: ")));
      domain[strlen(domain) -1] ='\0';
      l = 1;
    }
    if (p == 0 && (tmpPtr = strstr(line1,"data_grp_name: ")) != NULL) {
      strcpy(zone, (char *) (tmpPtr +strlen("data_grp_name: ")));
      if ((tmpPtr = strstr(zone,"/home/")) != NULL) {
	tmpPtr = tmpPtr+5;
	*tmpPtr = '\0';
      }
      zone[strlen(zone) -1] ='\0';
      p = 1;
    }
    if (m == 0 && (tmpPtr = strstr(line1,"phy_rsrc_name: ")) != NULL) {
      strcpy(resource, (char *) (tmpPtr +strlen("phy_rsrc_name: ")));
      resource[strlen(resource) -1] ='\0';
      m = 1;
      rewind(file);
    }
    if (i && j && k && l && m && p)
      return(0);
  }
  return -1;
}


/**************************************************************************************
* Checks to see if the difference between two files is just a User, Domain, or Resource
* difference
**************************************************************************************/
int
isUserOrDomain(int i, char *line, int j)
{

  char *tmpPtr;
  int k;
  tmpPtr = &line[j];
  if (i == 1) {
    if(strlen(userdomain1) > 0 && strstr(tmpPtr,userdomain1) == tmpPtr) 
      return (strlen(userdomain1) );
    if(strlen(zone1) > 0 && strstr(tmpPtr,zone1) == tmpPtr) 
      return (strlen(zone1) );
    k  = 0;
    while (strstr(phyrsrc[1][k],"ENDOFRESOURCES") == NULL) {
      if(strstr(tmpPtr,phyrsrc[1][k]) == tmpPtr)
	return(strlen(phyrsrc[1][k]));
      k++;
    }
    if(strlen(user1) > 0 && strstr(tmpPtr,user1) == tmpPtr) 
      return (strlen(user1) );
    if(strlen(domain1) > 0 && strstr(tmpPtr,domain1) == tmpPtr) 
      return (strlen(domain1) );
    if(strlen(resource1) > 0 && strstr(tmpPtr,resource1) == tmpPtr)
      return (strlen(resource1) );
    if(strlen(resourceShort1) > 0 && strstr(tmpPtr,resourceShort1) == tmpPtr)
      return (strlen(resourceShort1) );
    if(strlen(ticketHost1) > 0 && strstr(tmpPtr,ticketHost1) == tmpPtr)
      return (strlen(ticketHost1) );
    return (-1);
  }
  else if (i == 2) {
    if(strlen(userdomain2) > 0 && strstr(tmpPtr,userdomain2) == tmpPtr) 
      return (strlen(userdomain2) );
    if(strlen(zone2) > 0 && strstr(tmpPtr,zone2) == tmpPtr) 
      return (strlen(zone2) );
    k  = 0;
    while (strstr(phyrsrc[2][k],"ENDOFRESOURCES") == NULL) {
      if(strstr(tmpPtr,phyrsrc[2][k]) == tmpPtr)
	return(strlen(phyrsrc[2][k]));
      k++;
    }
    if(strlen(user2) > 0 && strstr(tmpPtr,user2) == tmpPtr) 
      return (strlen(user2) );
    if(strlen(domain2) > 0 && strstr(tmpPtr,domain2) == tmpPtr) 
      return (strlen(domain2) );
    if(strlen(resource2) > 0 && strstr(tmpPtr,resource2) == tmpPtr)
      return (strlen(resource2) );
    if(strlen(resourceShort2) > 0 && strstr(tmpPtr,resourceShort2) == tmpPtr)
      return (strlen(resourceShort2) );
    if(strlen(ticketHost2) > 0 && strstr(tmpPtr,ticketHost2) == tmpPtr)
      return (strlen(ticketHost2) );
    return (-1);
  }
  else
    return (-1);
  

}


/*************************************************************************************
* Checks to see if the difference is just from a nonIsolatedNumber,
* if however the difference is IsolatedNumber then error.
*************************************************************************************/
int
isIsolatedNumber(char *line, int j) 
{
 int i,k;
 i = 0;
 k = j;
 if (!isdigit(line[k]))
   return 0;

 while (isdigit(line[k]))
   k--;

 if (!isspace(line[k])) 
   return(0);
 k = j;
 while (isdigit(line[k]))
   k++;
 if (isspace(line[k])) 
   return (1);
 return(0);
}


/**************************************************
* Compares two srb lines from two srb result files
**************************************************/
int
compLines(char *line1, char* line2) {

 int i,j,k;
 int u1,u2;
 int l1, l2;
 l1 = strlen(line1);
 l2 = strlen(line2);

 for (j=0,k=0; j <  l1 && k < l2; j++,k++ ) {
   if(line1[j] == '+') {
     while(line1[j] == '+')
        j++;
   }
   if (line2[k] == '+') {
     while(line2[k] == '+')
        k++;
   }

   if(isspace(line1[j])) {
     while(isspace(line1[j])) 
	j++;
   }
   if (isspace(line2[k])) {
     while(isspace(line2[k])) 
	k++;
   }

   if((strstr(line1,"path_name:") != NULL )&&(strstr(line2,"path_name:") != NULL ))
     return 0;

   if (line1[j] != line2[k]) {
     if (isalpha(line1[j]) || isalpha(line2[k])) {
       if(((u1 = isUserOrDomain(1,line1,j)) >= 0) &&
	  ((u2 = isUserOrDomain(2,line2,k)) >= 0)) {
	 j += u1;
	 k += u2;

	 if (isspace(line1[j])) {
	   while(isspace(line1[j])) 
	      j++;
	 }
	 if (isspace(line2[k])) {
	   while(isspace(line2[k])) 
              k++;
	 }
       }
       else {
	 error = 1;
         return -1;
       }
       j--;
       k--;
     }
     else {
       if (isIsolatedNumber(line1,j) || isIsolatedNumber(line2,k)) {
	 error = 1;
	 return -1;
       }

       if((isdigit(line1[j]) || line1[j] == '-' || line1[j] == '/' || line1[j] == '.')||(isdigit(line2[k]) || line2[k] == '-' || line2[k] == '/' || line2[k] == '.')) {
         while (isdigit(line1[j]) || line1[j] == '-' || line1[j] == '/' || line1[j] == '.') 
           j++; 
         while (isdigit(line2[k]) || line2[k] == '-' || line2[k] == '/' || line2[k] == '.') 
           k++;
         j--;
         k--;
       }

     } /* else isIsolatedNumber */
   } /* if char != char */
 } /* for loop */
 return 0;
} /* CompLines */


/*************************************************************************************
*  Compares two srb files to see if there are any errors.
*  usage: ./mydiff file1 file2 n
*  the "n" is a flag where "n" is a number indicating 
*  how many errors the user would like displayed.
*************************************************************************************/
int
main(int argc, char **argv)
{

    FILE *file1, *file2;
    int i,j,k,l,e;

    char linebuf1[20200], linebuf2[20200];

    if ((argc > 4) || (argc < 3)) {
      printf("wrong usage\nUse mydiff file1 file2\n");
      exit (1);
    }

    file1 = fopen(argv[1],"r");
    file2 = fopen(argv[2],"r");

    if (file1 == NULL || file2 == NULL) {
      printf("error opening input file(s)\n");
      exit(1);
    }

    l = 1;
    e = 1;
    linebuf1[0] ='\0';
    linebuf2[0] ='\0';
    testid1[0] ='\0';
    testdate1[0] ='\0';
    testid2[0] ='\0';
    testdate2[0] ='\0';
    user1[0] ='\0';
    user2[0] ='\0';
    domain1[0] ='\0';
    domain2[0] ='\0';
    userdomain1[0] ='\0';
    userdomain2[0] ='\0';
    resource1[0] ='\0';
    resource2[0] ='\0';
    resourceShort1[0] ='\0';
    resourceShort2[0] ='\0';
    ticketHost1[0] ='\0';
    ticketHost2[0] ='\0';
    error = 0;

    i = getPhyResources(argv[1],1);
    i = getPhyResources(argv[2],2);
    i = gettestId_Date_User_Domain(file1,testid1,testdate1,user1,domain1,resource1,ticketHost1,zone1);

    if (i != 0) {
      fseek(file1,0, SEEK_SET);
    }
    else {
      strcpy(resourceShort1,resource1);
      resourceShort1[12] ='\0';
    }
    
    i = gettestId_Date_User_Domain(file2,testid2,testdate2,user2,domain2,resource2,ticketHost2,zone2);

    if (i != 0) {
      fseek(file2,0, SEEK_SET);
    }
    else {
      strcpy(resourceShort2,resource2);
      resourceShort2[12] ='\0';
    }
    

    printf("\nTESTID1=%s       TESTDATE1=%s      TESTUSER1=%s@%s|%s      RESOURCE1=%s\n",
	   testid1,testdate1,user1,domain1,zone1,resource1);
    printf("\nTESTID2=%s       TESTDATE2=%s      TESTUSER2=%s@%s|%s      RESOURCE2=%s\n\n",
	   testid2,testdate2,user2,domain2,zone2,resource2);
/*
    printf("Host1=%s       Host2=%s\n",ticketHost1,ticketHost2);
*/
    sprintf(userdomain1,"%s.%s",user1,domain1);
    sprintf(userdomain2,"%s.%s",user2,domain2);

    for (i= 0; i <= strlen(user1) && i <= strlen(user2) ; i++) {

      if (i == strlen(user1) && i < strlen(user2)) {
	user1[0] = '\0';
	memmove(user2, &user2[i], strlen(&user2[i]) + 1);
      }
      else if (i == strlen(user2) && i < strlen(user1)) {
	user2[0] = '\0';
	memmove(user1, &user1[i], strlen(&user1[i]) + 1);
	
      }
      else if (user1[i] != user2[i]) {
	if (i != 0) {
	  memmove(user1, &user1[i], strlen(&user1[i]) + 1);
	  memmove(user2, &user2[i], strlen(&user2[i]) + 1);
	}
	break;
      }
    }

    for (i= 0; i <= strlen(domain1) && i <= strlen(domain2) ; i++) {
      if (i == strlen(domain1) && i < strlen(domain2)) {
	domain1[0] = '\0';
	memmove(domain2, &domain2[i], strlen(&domain2[i]) + 1);
      }
      else if (i == strlen(domain2) && i < strlen(domain1)) {
	domain2[0] = '\0';
	memmove(domain1, &domain1[i], strlen(&domain1[i]) + 1);
	
      }
      else if (domain1[i] != domain2[i]) {
	if (i != 0) {
	  memmove(domain1, &domain1[i], strlen(&domain1[i]) + 1);
	  memmove(domain2, &domain2[i], strlen(&domain2[i]) + 1);
	}
	break;
      }
    }

    for (i= 0; i <= strlen(userdomain1) && i <= strlen(userdomain2) ; i++) {
      if (i == strlen(userdomain1) && i < strlen(userdomain2)) {
	userdomain1[0] = '\0';
	memmove(userdomain2, &userdomain2[i], strlen(&userdomain2[i]) + 1);
      }
      else if (i == strlen(userdomain2) && i < strlen(userdomain1)) {
	userdomain2[0] = '\0';
	memmove(userdomain1, &userdomain1[i], strlen(&userdomain1[i]) + 1);
	
      }
      else if (userdomain1[i] != userdomain2[i]) {
	if (i != 0) {
	  memmove(userdomain1, &userdomain1[i], strlen(&userdomain1[i]) + 1);
	  memmove(userdomain2, &userdomain2[i], strlen(&userdomain2[i]) + 1);
	}
	break;
      }
    }

    for (i= 0; i <= strlen(zone1) && i <= strlen(zone2) ; i++) {
      if (i == strlen(zone1) && i < strlen(zone2)) {
	zone1[0] = '\0';
	memmove(zone2, &zone2[i], strlen(&zone2[i]) + 1);
      }
      else if (i == strlen(zone2) && i < strlen(zone1)) {
	zone2[0] = '\0';
	memmove(zone1, &zone1[i], strlen(&zone1[i]) + 1);
	
      }
      else if (zone1[i] != zone2[i]) {
	if (i != 0) {
	  memmove(zone1, &zone1[i], strlen(&zone1[i]) + 1);
	  memmove(zone2, &zone2[i], strlen(&zone2[i]) + 1);
	}
	break;
      }
    }
    while (fgets(line1, 299, file1) != NULL &&
	   fgets(line2, 299, file2)  != NULL ){

      i =  compLines(line1, line2);
      if ((i == -1) && (argc != 4)) {
        printf("Error #%d on Line #%d:\n*******1*******\n%s\n",e,l,line1);
	printf("*******2*******\n%s\n\n\n\n",line2);
	e++; 
	}
      else if ((i == -1) && (argc == 4) && (atoi(argv[3]) >= e)) {
        printf("Error #%d on Line #%d:\n*******1*******\n%s\n",e,l,line1);
        printf("*******2*******\n%s\n\n\n\n",line2);
        e++;
        }
      l++;
    }/* while */

    if( (line1 == NULL && line2 != NULL) || (line1 != NULL && line2 == NULL))
	  printf("\n                  The files are not the same length, there is most likely an error!\n\n");

    else if ( e == 1 )
	printf("\n\nTHERE WERE NO ERRORS!!! =)\n\n");
    fclose(file1);
    fclose(file2);
    if (error)
      exit(1);
    else 
      exit(0);
}




























































































































