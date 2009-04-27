/* iRODS unix stdio i/o functions; emulate a subset of stdio.h
 * calls */

#ifndef IRODS_IO_H
#define IRODS_IO_H

#define fopen(A,B) irodsfopen(A,B)
#define fread(A,B,C,D) irodsfread(A,B,C,D)
#define fclose(A) irodsfclose(A)
#define exit(A) irodsexit(A)
#define fwrite(A,B,C,D) irodsfwrite(A,B,C,D)
#define fseek(A,B,C) irodsfseek(A,B,C)
#if 0
#define fflush(A) irodsfflush(A)
#endif

#include <stdio.h>

#endif /* IRODS_IO_H */
