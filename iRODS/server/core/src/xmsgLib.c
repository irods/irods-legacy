/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* xmsgLib.c - library routines for irodsXmsg
 */

#include "xmsgLib.h"

int
addXmsgStructToQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{

    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    xmsgStruct->next = xmsgStruct->prev = NULL;

    if (xmsgQue->head == NULL) {
	xmsgQue->head = xmsgQue->tail = xmsgStruct;
    } else {
	/* que it on top */
	xmsgQue->head->prev = xmsgStruct;
	xmsgStruct->next = xmsgQue->head;
	xmsgQue->head = xmsgStruct;
    }

    return (0);
}

int
rmXmsgStructFromQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{
    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (xmsgStruct->prev == NULL) {
	/* at head */
	xmsgQue->head = xmsgStruct->next;
    } else {
	xmsgStruct->prev->next = xmsgStruct->next;
    }

    if (xmsgStruct->next == NULL) {
	/* at tail */
        xmsgQue->tail = xmsgStruct->prev;
    } else {
	xmsgStruct->next->prev = xmsgStruct->prev;
    }
 
    xmsgStruct->prev = xmsgStruct->next = NULL;

    return (0);
}

int
addXmsgStructToHQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{

    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    xmsgStruct->hnext = xmsgStruct->hprev = NULL;

    if (xmsgQue->head == NULL) {
	xmsgQue->head = xmsgQue->tail = xmsgStruct;
    } else {
	/* que it on top */
	xmsgQue->head->hprev = xmsgStruct;
	xmsgStruct->hnext = xmsgQue->head;
	xmsgQue->head = xmsgStruct;
    }

    return (0);
}

int
rmXmsgStructFromHQue (xmsgStruct_t *xmsgStruct, xmsgQue_t *xmsgQue)
{
    if (xmsgStruct == NULL || xmsgQue == NULL) {
        rodsLog (LOG_ERROR,
          "addXmsgToQue: input xmsgStruct or xmsgQue is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    if (xmsgStruct->hprev == NULL) {
	/* at head */
	xmsgQue->head = xmsgStruct->hnext;
    } else {
	xmsgStruct->hprev->hnext = xmsgStruct->hnext;
    }

    if (xmsgStruct->hnext == NULL) {
	/* at tail */
        xmsgQue->tail = xmsgStruct->hprev;
    } else {
	xmsgStruct->hnext->hprev = xmsgStruct->hprev;
    }
 
    xmsgStruct->hprev = xmsgStruct->hnext = NULL;

    return (0);
}
