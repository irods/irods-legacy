/* This is a very simple test of the irods standard IO emulation
  library.  When using standard i/o, include stdio.h; to change to
  using isio, change stdio.h to isio.h. */

/* include <stdio.h> */
#include "isio.h"   /* the irods standard IO emulation library */
#include <stdlib.h>
#include <string.h>
main(argc, argv)
int argc;
char **argv;
{
    FILE *FI, *FO;
    int i0,i1,i2,i3;
    int count, rval, wval;
    char buf[1024*100];
 
    if (argc < 3) {
      printf("a.out file-in file-out [seek-position]\n");
      exit(-1);
    }

    FI = fopen(argv[1],"r");
    if (FI==0)
    {
        fprintf(stderr,"can't open input file %s\n",argv[1]);
        exit(-2);
    }

    FO = fopen(argv[2],"w");
    if (FO==0)
    {
        fprintf(stderr,"can't open output file %s\n",argv[2]);
        exit(-3);
    }

    if (argc == 4) {
       fseek(FO, atoi(argv[3]), SEEK_SET);
    }

    do {
      memset(buf, 0, sizeof(buf));
      i0 = sizeof(buf);
      printf("i0=%d\n",i0);
      rval = fread(&buf[0], 1, sizeof(buf), FI);
      printf("rval=%d\n",rval);
      if (rval < 0) {
	perror("read stream message");
      }
      if (rval > 0) {
	wval = fwrite(&buf[0], 1, rval, FO);
	if (wval < 0) {
	  perror("fwriting data:");
	}
      }
    } while (rval > 0);
    fclose(FI);
    fclose(FO);
    exit(0);
}
