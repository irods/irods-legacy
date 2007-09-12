/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* reEraHelpers.h - header file for reEraHelpers.c
 */

#ifndef REERAHELPERS_H
#define REERAHELPERS_H

int appendStrToBBuf(bytesBuf_t *dest, size_t size, const char *format, ...);
int copyAVUMetadata(char *destPath, char *srcPath, rsComm_t *rsComm);
int getDataObjPSmeta(char *objPath, bytesBuf_t *mybuf, rsComm_t *rsComm);
int getCollectionPSmeta(char *objPath, bytesBuf_t *mybuf, rsComm_t *rsComm);
int loadMetadataFromFile(rsComm_t *rsComm, char *myPath);
int genQueryOutToXML(rsComm_t *Conn, int status, genQueryOut_t *genQueryOut, bytesBuf_t *mybuf, char **tags);
int extractPSQueryResults(rsComm_t *Conn, int status, genQueryOut_t *genQueryOut, bytesBuf_t *mybuf, char *fullName);

#endif	/* REERAHELPERS_H */

