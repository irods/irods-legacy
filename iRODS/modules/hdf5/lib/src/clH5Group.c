/* clH5Group.c
 */
#include "h5Attribute.h"
#include "h5Object.h"
#include "h5Group.h"
#include "h5String.h"
#include <malloc.h>
#include <assert.h>

int clH5Group_read_attribute(rcComm_t *conn, H5Group* ing)
{
    int ret_value = 0;
    H5Group* outg = NULL;

    assert(ing);

    ing->opID = H5GROUP_OP_READ_ATTRIBUTE;

#if 0	/* XXXXX rm for iRods */
    ret_value = srbGenProxyFunct (conn, HDF5_OPR_TYPE, H5OBJECT_GROUP,
      0, NULL, NULL,
      (void *) ing, h5Group_PF, (void **) &outg, h5Group_PF, Hdf5Def, 0);
#endif
    if (ret_value < 0)
        return (ret_value);
    
    /* psss on the value */
    ing->error = outg->error;
    ing->nattributes = outg->nattributes;
    ing->attributes = outg->attributes;

    outg->attributes = NULL;
    outg->nattributes = 0;

    H5Group_dtor (outg);
    free (outg);

    return ret_value;
}
