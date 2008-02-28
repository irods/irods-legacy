/* clH5Dataset.c
 */
#include "h5Dataset.h"
#include "h5Object.h"
#include "h5String.h"
#include <malloc.h>
#include <assert.h>

/*------------------------------------------------------------------------------
 * Purpose: Client siderRead data from file
 * Parameters: H5Dataset* d -- The dataset whose value to be retrieved from file
 *     to server: { opID, fid, fullpath, type, space }                      *
 *     to client: { value, error }                                          *
 * Return:  Returns a non-negative value if successful; otherwise returns a negative value.
 *------------------------------------------------------------------------------
 */
int clH5Dataset_read(rcComm_t *conn, H5Dataset* d)
{
    int ret_value = 0;
    H5Dataset *outd = NULL;

    assert(d);

    d->opID = H5DATASET_OP_READ;

    ret_value = _clH5Dataset_read (conn, d, &outd);

    if (ret_value < 0) 
	return (ret_value);

    /* psss on the value */
    d->nvalue = outd->nvalue;
    d->value = outd->value;
    d->error = outd->error;
    outd->value = NULL;
    outd->nvalue = 0;

    H5Dataset_dtor (outd);
    free (outd);

    return ret_value;
}

int _clH5Dataset_read (rcComm_t *conn, H5Dataset* d, H5Dataset** outd)
{
    int ret_value = 0;
#if 0   /* XXXXXX rm for iRods */
    ret_value = srbGenProxyFunct (conn, HDF5_OPR_TYPE, H5OBJECT_DATASET,
      0, NULL, NULL,
      (void *) d, h5Dataset_PF, (void **) &outd, h5Dataset_PF, Hdf5Def, 0);
#endif
    return (ret_value);
}

int clH5Dataset_read_attribute(rcComm_t *conn, H5Dataset* ind)
{
    int ret_value = 0;
    H5Dataset *outd = NULL;

    assert(ind);

    ind->opID = H5DATASET_OP_READ_ATTRIBUTE;

    ret_value = _clH5Dataset_read_attribute (conn, ind, &outd);

    if (ret_value < 0)
        return (ret_value);

    /* psss on the value */
    ind->class = outd->class;
    ind->error = outd->error;
    ind->nattributes = outd->nattributes;
    ind->attributes = outd->attributes;

    outd->attributes = NULL;
    outd->nattributes = 0;

    H5Dataset_dtor (outd);
    free (outd);

    return ret_value;
}

int _clH5Dataset_read_attribute (rcComm_t *conn, H5Dataset* d, H5Dataset** outd)
{
    int ret_value = 0;
#if 0   /* XXXXX rm for iRods */
    ret_value = srbGenProxyFunct (conn, HDF5_OPR_TYPE, H5OBJECT_DATASET,
      0, NULL, NULL,
      (void *) ind, h5Dataset_PF, (void **) &outd, h5Dataset_PF, Hdf5Def, 0);
#endif
    return (ret_value);
}

