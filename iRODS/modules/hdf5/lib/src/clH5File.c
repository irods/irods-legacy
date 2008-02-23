/* clH5File.c
 */
#include "h5File.h"
#include <malloc.h>
#include <assert.h>

/*------------------------------------------------------------------------------
 * Purpose: Client open an existing file and retrieve the file structure 
 * (root group)
 * Parameters: H5File* f -- The file to open
 *     to server: { opID, filename }                                        *
 *     from server : { fid, root, error }                                      *
 * Return:  Returns a non-negative value if successful; otherwise returns a negative value.
 *------------------------------------------------------------------------------
 */
int clH5File_open(rcComm_t *conn, H5File* f)
{
    int ret_value = 0;
    H5File *outf = NULL;


    assert(f);

#if 0	/* XXXXXX rm for iRods */
    ret_value = srbGenProxyFunct (conn, HDF5_OPR_TYPE, H5OBJECT_FILE, 0, 
      f->filename, NULL, (void *) f, h5File_PF, (void **) &outf, h5File_PF, 
      Hdf5Def, 0);
#endif
    if (ret_value < 0) 
	return (ret_value);

    f->fid = outf->fid;
    f->root = outf->root;
    outf->root = NULL;
    f->error = outf->error;

    H5File_dtor(outf);
    if (outf)
        free(outf);

    return ret_value;
}

/*------------------------------------------------------------------------------
 * Purpose: Client Close an opened H5file.
 * Parameters: H5File* f -- The opend H5file
 *     to server: { opID, fid }                                             *
 *     to client: { error }                                                 *
 * Return:  Returns a non-negative value if successful; otherwise returns a negative value.
 *------------------------------------------------------------------------------
 */
int clH5File_close (rcComm_t *conn, H5File* f)
{
    int ret_value = 0;
    H5File inf, *outf = NULL;


    assert(f);

    memset (&inf, 0, sizeof (inf));

    inf.fid = f->fid;
    inf.opID = f->opID;
 
#if 0	/* XXXXXX rm for iRods */
    ret_value = srbGenProxyFunct (conn, HDF5_OPR_TYPE, H5OBJECT_FILE, 0,
      f->filename, NULL, (void *) &inf, h5File_PF, (void **) &outf, h5File_PF, 
      Hdf5Def, 0);
#endif
    if (ret_value < 0) 
	return (ret_value);

    f->error = outf->error;

    H5File_dtor(outf);
    if (outf)
        free(outf);

    return ret_value;
}

