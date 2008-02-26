
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "hdf5MS.h"
#include "objMetaOpr.h"
#include "miscServerFunct.h"
#include "rsGlobalExtern.h"

struct openedH5File *OpenedH5FileHead = NULL;

/* msiH5File_open - msi for opening a H5File.
 * inpH5FileParam - The input H5File to open. Must be h5File_MS_T.
 * inpFlagParam - Input flag - INT_MS_T
 * outFidParam - a INT_MS_T containing the descriptor of the create.
 * outH5FileParam - the output H5File - Must be h5File_MS_T.
 *
 */

int
msiH5File_open (msParam_t *inpH5FileParam, msParam_t *inpFlagParam,
msParam_t *outFidParam, msParam_t *outH5FileParam, ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    H5File *inf = 0;
    H5File *outf;
    int inpFlag;
    int l1descInx;
    dataObjInp_t dataObjInp;
    dataObjCloseInp_t dataObjCloseInp;
    dataObjInfo_t *dataObjInfo, *tmpDataObjInfo;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    int fid;

    RE_TEST_MACRO ("    Calling msiH5File_open")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiH5File_open: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if (inpH5FileParam == NULL || inpFlagParam == NULL ||
     outFidParam == NULL || outH5FileParam == NULL) {
        rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_open: NULL input/output Param");
        return (rei->status);
    }

    if (strcmp (inpH5FileParam->type, h5File_MS_T) == 0) {
        inf = inpH5FileParam->inOutStruct;
    } else {
        rei->status = USER_PARAM_TYPE_ERR;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_open: input rei or rsComm is NULL");
        return (rei->status);
    }     

    inpFlag = parseMspForPosInt (inpFlagParam);
    if (inpFlag < 0) {
	inpFlag = 0;
    }

    /* see if we need to do it remotely based on the objPath. Open the
     * obj but don't do it physically */

    memset (&dataObjInp, 0, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, inf->filename, MAX_NAME_LEN);
    dataObjInp.openFlags = O_RDONLY;
    rei->status = l1descInx = 
      _rsDataObjOpen (rsComm, &dataObjInp, DO_NOT_PHYOPEN);

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_open: _rsDataObjOpen of %s error",
	  dataObjInp.objPath);
        return (rei->status);
    }

    dataObjCloseInp.l1descInx = l1descInx;
    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    if ((inpFlag & LOCAL_H5_OPEN) != 0) {
	/* need to find a local copy */
	tmpDataObjInfo = dataObjInfo;
	while (tmpDataObjInfo != NULL) {
	    remoteFlag = resolveHostByDataObjInfo (tmpDataObjInfo,
	      &rodsServerHost);
	    if (remoteFlag == LOCAL_HOST) {
		requeDataObjInfoByReplNum (&dataObjInfo, 
		  tmpDataObjInfo->replNum);
		/* have to update L1desc */
		L1desc[l1descInx].dataObjInfo = dataObjInfo;
		break;
	    }
	}
	if (remoteFlag != LOCAL_HOST) {
	    /* there is no local copy */
	    rei->status = SYS_COPY_NOT_EXIST_AT_RESC;
            rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
              "msiH5File_open: _local copy of %s does not exist",
	      dataObjInp.objPath);
            rsDataObjClose (rsComm, &dataObjCloseInp);
            return (rei->status);
	}
    } else {
	remoteFlag = resolveHostByDataObjInfo (dataObjInfo,
          &rodsServerHost);
    }

    if (remoteFlag == LOCAL_HOST) {
	outf = malloc (sizeof (H5File));
        /* replace iRods file with local file */
        if (inf->filename != NULL) {
            free (inf->filename);
        }
        inf->filename = strdup (dataObjInfo->filePath);
        fid = H5File_open(inf, outf);
    } else {
	/* do the remote open */
        if ((rei->status = svrToSvrConnect (rsComm, rodsServerHost)) < 0) {
            rsDataObjClose (rsComm, &dataObjCloseInp);
            return rei->status;
        }

	outf = NULL;
	fid = _clH5File_open (rodsServerHost->conn, inf, &outf, LOCAL_H5_OPEN);
    }

    if (fid >= 0) {
	L1desc[l1descInx].l3descInx = fid;
    } else {
	rei->status = fid;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_open: H5File_open %s error",
          dataObjInp.objPath);
        rsDataObjClose (rsComm, &dataObjCloseInp);
	return rei->status;
    }

    /* prepare the output */

    fillIntInMsParam (outFidParam, l1descInx);
    outf->fid = l1descInx;
    fillMsParam (outH5FileParam, NULL, h5File_MS_T, outf, NULL);
    
    if (inf) H5File_dtor(inf);

    return (rei->status);
}

/* msiH5File_close - msi for closing a H5File.
 * inpH5FileParam - The input H5File to close. Must be h5File_MS_T.
 * outParam - a INT_MS_T containing the status.
 *
 */

int
msiH5File_close (msParam_t *inpH5FileParam, msParam_t *outParam,
ruleExecInfo_t *rei)
{
    rsComm_t *rsComm;
    H5File *inf = 0;
    H5File outf;
    dataObjCloseInp_t dataObjCloseInp;
    dataObjInfo_t *dataObjInfo;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;
    int l1descInx;

    RE_TEST_MACRO ("    Calling msiH5File_close")

    if (rei == NULL || rei->rsComm == NULL) {
        rodsLog (LOG_ERROR,
          "msiH5File_close: input rei or rsComm is NULL");
        return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    rsComm = rei->rsComm;

    if (inpH5FileParam == NULL || outParam == NULL) {
        rei->status = SYS_INTERNAL_NULL_INPUT_ERR;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_close: NULL input/output Param");
        return (rei->status);
    }

    if (strcmp (inpH5FileParam->type, h5File_MS_T) == 0) {
        inf = inpH5FileParam->inOutStruct;
    } else {
        rei->status = USER_PARAM_TYPE_ERR;
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_open: input rei or rsComm is NULL");
        return (rei->status);
    }

    l1descInx = inf->fid;

    dataObjCloseInp.l1descInx = l1descInx;
    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    remoteFlag = resolveHostByDataObjInfo (dataObjInfo,
      &rodsServerHost);

    if (remoteFlag == LOCAL_HOST) {
	/* switch the fid */
	inf->fid = L1desc[l1descInx].l3descInx;
        rei->status = H5File_close (inf, &outf);
    } else {
        /* do the remote close */
        if ((rei->status = svrToSvrConnect (rsComm, rodsServerHost)) >= 0) {
            rei->status = clH5File_close (rodsServerHost->conn, inf);
	}
    }

    if (rei->status < 0) {
        rodsLogAndErrorMsg (LOG_ERROR, &rsComm->rError, rei->status,
          "msiH5File_close: H5File_close error for %d", l1descInx);
    }

    L1desc[l1descInx].l3descInx = 0;
    rsDataObjClose (rsComm, &dataObjCloseInp);

    /* prepare the output */

    fillIntInMsParam (outParam, rei->status);

    if (inf) H5File_dtor(inf);

    return rei->status;
}


#if 0	/* XXXXX take out for testing */
int h5ObjProcess(genFunctType_t functType, int objID, int intInput2, 
char *strInput1, char *strInput2, bytesBuf_t *bytesBufInp, 
bytesBuf_t *bytesBufOut, int maxOutSz)
{
    int ret_value=0;
    struct hostElement  *hostAddrPtr = NULL;
    int remoteFlag;
    int status;

    if (bytesBufInp == NULL) {
        elog (NOTICE,
         "h5ObjProcess: NULL bytesBufInp for functType %d, objID %d",
         functType, objID);
        return (SYS_ERR_NULL_INPUT);
    }

    if (H5OBJECT_FILE == objID)
    {
        H5File *inf=0;
        H5File outf;
	struct mdasInfoOut *infoOutHead = NULL;
 	int fid = 0;

#if 0
        ret_value = unpackMsg (VAROUTDATA (vPackedInput), (char **) &inf,
         h5File_PF, Hdf5Def);
#endif

        ret_value = unpackMsgFromBB (bytesBufInp, (char **) &inf,
         h5File_PF, Hdf5Def, 1);

        if (ret_value < 0) {
            elog (NOTICE,
             "h5ObjProcess: unpackMsgFromBB error for functType %d, objID %d",
             functType, objID);
            return (ret_value);
        }

        if (H5FILE_OP_OPEN == inf->opID) {
            /* see if we need to do it remotely */
            ret_value = getInfoOutByName (NULL, NULL, inf->filename, "read",
              &infoOutHead);

            if (ret_value < 0) 
                return (ret_value);

            remoteFlag = chkHostAndZone (infoOutHead->resourceLoc, 
	      &hostAddrPtr);

	    if (remoteFlag < 0) {
        	elog (NOTICE, "h5ObjProcess: Invalid host addr %s", 
		  infoOutHead->resourceLoc);
	        freeAllInfoOut (infoOutHead);
        	if (inf) {
                    H5File_dtor(inf);
                    free(inf);
                }
       		return INP_ERR_HOST_ADDR;               /* failure */
            } else if (remoteFlag == REMOTE_HOST) {
		fid = remoteGenProxyFunct (hostAddrPtr,
		  functType, objID, intInput2, strInput1, strInput2, 
		  bytesBufInp, bytesBufOut, maxOutSz); 
		if (fid >= 0) {
		    addOpenedHdf5File (fid, hostAddrPtr);
		}
	        freeAllInfoOut (infoOutHead);
                if (inf) {
                    H5File_dtor(inf);
                    free(inf);
                }
		return (fid);
	    }
		    
	    /* do it locally */

	    if (infoOutHead != NULL) {
		/* replace srb file with local file */ 
		if (inf->filename != NULL) {
		    free (inf->filename);
		}
		inf->filename = infoOutHead->dataPath;
		infoOutHead->dataPath = NULL;
		freeAllInfoOut (infoOutHead);
	    }

            fid = H5File_open(inf, &outf);
        } else if (H5FILE_OP_CLOSE == inf->opID) {
            hostAddrPtr = getHostByFid (inf->fid);

            if (hostAddrPtr != NULL) {
                /* do it remotely */
                fid = remoteGenProxyFunct (hostAddrPtr,
                  functType, objID, intInput2, strInput1, strInput2, 
		  bytesBufInp, bytesBufOut, maxOutSz);

		closeOpenedHdf5File (inf->fid);
        	if (inf) {
                    H5File_dtor(inf);
                    free(inf);
                }

                return (fid);
            }

	    /* do it locally */

            fid = H5File_close(inf, &outf);
	} else {
            elog (NOTICE,
             "h5ObjProcess: unknown opID %d, objID %d", inf->opID, objID);
	    return -1;	/* XXXX add meaningful error */
	}

	/* local falls through to here */

        if (inf) {
            H5File_dtor(inf);
            free(inf);
        }

        if (fid < 0) {
            elog (NOTICE,
              "h5ObjProcess:H5File_open err.functType %d,objID %d,stat=%d",
              functType, objID, fid);
	    ret_value = fid;
        } else {
#if 0
            ret_value = packMsg ((char *) &outf, outBuf, h5File_PF, Hdf5Def);
#endif
            ret_value = packMsgInBB ((char *) &outf, bytesBufOut, h5File_PF,
             Hdf5Def);
            if (ret_value < 0) {
                elog (NOTICE,
                  "h5ObjProcess: packMsg error. status = %d", ret_value);
                fid = ret_value;
            }
            H5File_dtor(&outf);
	}
        return (fid);

        /* TODO at h5Handler -- more actions needs to be added here */
    }
    else if (H5OBJECT_DATASET == objID)
    {
        H5Dataset *ind=NULL;
        H5Dataset outd;

#if 0
        ret_value = unpackMsg (VAROUTDATA (vPackedInput), (char **) &ind,
         h5Dataset_PF, Hdf5Def);
#endif

        ret_value = unpackMsgFromBB (bytesBufInp, (char **) &ind,
         h5Dataset_PF, Hdf5Def, 1);

        if (ret_value < 0) {
            elog (NOTICE,
             "h5ObjProcess: unpackMsgFromBB error for functType %d, objID %d",
             functType, objID);
            return (ret_value);
        }

        hostAddrPtr = getHostByFid (ind->fid);

        if (hostAddrPtr != NULL) {
	    /* do it remotely */
            ret_value = remoteGenProxyFunct (hostAddrPtr,
              functType, objID, intInput2, strInput1, strInput2, 
	      bytesBufInp, bytesBufOut, maxOutSz);

            if (ind) {
                H5Dataset_dtor (ind);
                free (ind);
            }

            return (ret_value);
	}

	/* do it locally */

        if (H5DATASET_OP_READ == ind->opID) {
            ret_value = H5Dataset_read (ind, &outd);

	    if (ind) {
                H5Dataset_dtor (ind);
                free (ind);
	    }

            if (ret_value < 0) {
                elog (NOTICE,
                  "h5ObjProcess: H5Dataset_read error, status = %d",
                  ret_value);
            }
#if 0
            status = packMsg ((char *) &outd, outBuf,
              h5Dataset_PF, Hdf5Def);
#endif
            status = packMsgInBB ((char *) &outd, bytesBufOut,
              h5Dataset_PF, Hdf5Def);

            if (status < 0) {
                elog (NOTICE,
                  "h5ObjProcess: packMsg err.functType %d,objID %d,stat=%d",
                  functType, objID, status);
            }

	    prepOutDataset (&outd);
            H5Dataset_dtor (&outd);

            return (ret_value);
        } else if (H5DATASET_OP_READ_ATTRIBUTE == ind->opID) {
            ret_value = H5Dataset_read_attribute (ind, &outd);

	    if (ind) {
                H5Dataset_dtor (ind);
                free (ind);
	    }

            if (ret_value < 0) {
                elog (NOTICE,
                  "h5ObjProcess: H5Dataset_read error, status = %d",
                  ret_value);
            }
            status = packMsgInBB ((char *) &outd, bytesBufOut,
              h5Dataset_PF, Hdf5Def);

            if (status < 0) {
                elog (NOTICE,
                  "h5ObjProcess: packMsg err.functType %d,objID %d,stat=%d",
                  functType, objID, status);
            }

	    prepOutDataset (&outd);
            H5Dataset_dtor (&outd);

            return (ret_value);
        } else {
            elog (NOTICE,
             "h5ObjProcess: unknown opID %d, objID %d", ind->opID, objID);
            return -1;  /* XXXX add meaningful error */

        }

        /* TODO at h5Handler -- more actions needs to be added here */
    } 
    else if (H5OBJECT_GROUP == objID) {
	H5Group *ing = NULL;
	H5Group outg;

#if 0
        ret_value = unpackMsg (VAROUTDATA (vPackedInput), (char **) &ing,
         h5Group_PF, Hdf5Def);
#endif
        ret_value = unpackMsgFromBB (bytesBufInp, (char **) &ing,
         h5Group_PF, Hdf5Def, 1);

        if (ret_value < 0) {
            elog (NOTICE,
             "h5ObjProcess: unpackMsgFromBB error for functType %d, objID %d",
             functType, objID);
            return (ret_value);
        }

        hostAddrPtr = getHostByFid (ing->fid);

        if (hostAddrPtr != NULL) {
            /* do it remotely */
            ret_value = remoteGenProxyFunct (hostAddrPtr,
              functType, objID, intInput2, strInput1, strInput2,
              bytesBufInp, bytesBufOut, maxOutSz);

            if (ing) {
                H5Group_dtor (ing);
                free (ing);
            }
            return (ret_value);
        }

        /* do it locally */

        if (H5GROUP_OP_READ_ATTRIBUTE == ing->opID) {
            ret_value = H5Group_read_attribute (ing, &outg);

            if (ing) {
                H5Group_dtor (ing);
                free (ing);
            }

            if (ret_value < 0) {
                elog (NOTICE,
                  "h5ObjProcess: H5Dataset_read error, status = %d",
                  ret_value);
            }
            status = packMsgInBB ((char *) &outg, bytesBufOut,
              h5Group_PF, Hdf5Def);

            if (status < 0) {
                elog (NOTICE,
                  "h5ObjProcess: packMsg err.functType %d,objID %d,stat=%d",
                  functType, objID, status);
            }

            H5Group_dtor (&outg);

            return (ret_value);
        } else {
            elog (NOTICE,
             "h5ObjProcess: unknown opID %d, objID %d", ing->opID, objID);
            return -1;  /* XXXX add meaningful error */

        }
    }

    return ret_value;
}
#endif	/* XXXXX taken out for testing */

int
addOpenedHdf5File (int fid, rodsHostAddr_t *hostAddrPtr) 
{
    struct openedH5File *myOopenedH5File;

    myOopenedH5File = malloc (sizeof (struct openedH5File));
    myOopenedH5File->fid = fid;
    myOopenedH5File->hostAddrPtr = hostAddrPtr;
    myOopenedH5File->next = NULL;

    /* queue it */ 

    if (OpenedH5FileHead == NULL) {
	OpenedH5FileHead = myOopenedH5File;
    } else {
	myOopenedH5File->next = OpenedH5FileHead;
	OpenedH5FileHead = myOopenedH5File;
    }

    return (0);
}

rodsHostAddr_t *
getHostByFid (int fid)
{
    struct openedH5File *tmpOpenedH5File;

    tmpOpenedH5File = OpenedH5FileHead;
    while (tmpOpenedH5File != NULL) {
	if (fid == tmpOpenedH5File->fid) {
	    return (tmpOpenedH5File->hostAddrPtr);
	}
	tmpOpenedH5File = tmpOpenedH5File->next;
    }
    return (NULL);
}

int
closeOpenedHdf5File (int fid)
{
    struct openedH5File *tmpOpenedH5File, *prevOpenedH5File = NULL;;

    tmpOpenedH5File = OpenedH5FileHead;
    while (tmpOpenedH5File != NULL) {
        if (fid == tmpOpenedH5File->fid) {
	    if (tmpOpenedH5File == OpenedH5FileHead) {
		/* the head */
		OpenedH5FileHead = OpenedH5FileHead->next;
	    } else {
		prevOpenedH5File->next = tmpOpenedH5File->next;
	    } 
	    free (tmpOpenedH5File);
            return (0);
        }
	prevOpenedH5File = tmpOpenedH5File;
        tmpOpenedH5File = tmpOpenedH5File->next;
    }

    /* no Match */
    rodsLog (LOG_ERROR,
      "closeOpenedHdf5File: unmatch fid %d", fid);
    return (-1);
} 

/* Prep the dataset so value won't be freed */

int
prepOutDataset (H5Dataset *d)
{
    if (d == NULL)
	return (0);

    if (d->value) {
        if (( H5DATATYPE_VLEN == d->type.class ) || 
	  ( H5DATATYPE_COMPOUND == d->type.class ) || 
	  ( H5DATATYPE_STRING == d->type.class )) {
	    return (0);
	} else {
	    d->value = NULL;
            return 0;
	}
    }
    return (0);
}

