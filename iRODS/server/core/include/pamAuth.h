/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* pamAuth.h - header file for pamAuth.c
 */

#ifndef PAM_AUTH_H
#define PAM_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

int
pamAuthenticate(char *username, char *password);

#ifdef __cplusplus
}
#endif

#endif	/* PAM_AUTH_H */
