/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* definitions for obf routines */

#ifdef  __cplusplus
extern "C" {
#endif

int obfGetPw(char *pw);
void obfDecodeByKey(char *in, char *key, char *out);

#ifdef  __cplusplus
}
#endif

