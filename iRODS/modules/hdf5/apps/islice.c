/* 
 * islice -- Extract a slice, perpendicular to one of the coordinate axes,
 * from a FLASH output file stored at iRODS server. Selection of a slice is
 * based on the tool, extract_slice_from_chkpnt, developed by Paul Ricker 
 * (UIUC/NCSA).
 *
 * Typical usage from the command line might be:
 *      "islice infile outfile pos var coor", where
 *          infile  -- the name of the FLASH file to read
 *          outfile -- the name of the output file to create
 *          pos     -- the orientation of the slice (0 = xy, 1 = xz, 2 = yz)
 *          var     -- the name of the FLASH mesh variable
 *          coor    -- the value of the coordinate perpendicular to the slice
 *
 * Output is an unformatted file containing the slice data, uniformly
 * sampled at the highest resolution in the dataset.  Blocks at lower
 * levels of refinement are simply injected to the highest level.
 *
 * This program must be linked against the HDF5/iRODS client module. Since
 * all file access is done at the server side, HDF5 library is not needed
 * to compile and run this program.
 *
 * Peter Cao (THG, xcao@hdfgroup.org)
 * Date 7/7/08 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "clH5Handler.h"
#include "h5File.h"
#include "h5Dataset.h"
#include "h5Group.h"

#define DEBUG

#define DNAME_REFINE_LEVEL "refine level"
#define DNAME_NODE_TYPE "node type"
#define DNAME_BOUNDING_BOX "bounding box"

/*
 * Name:
 *      help
 *
 * Purpose:
 *      Print a helpful summary of command usage and features.
 */
static void 
help(char *name)
{
    (void) printf("NAME:\n\thislice -- Extract a slice from a FLASH output file\n\n");

    (void) printf("DESCRIPTION:\n\tislice -- Extract a slice, perpendicular to one of the coordinate axes,\n");
    (void) printf(              "\tfrom a FLASH output file stored at iRODS server. Selection of a slice is\n");
    (void) printf(              "\tbased on the tool, extract_slice_from_chkpnt, developed by Paul Ricker\n");
    (void) printf(              "\t(UIUC/NCSA).\n\n");

    (void) fprintf (stdout, "SYNOPSIS:");
    (void) fprintf (stdout, "\n\t%s -h[elp], OR", name);
    (void) fprintf (stdout, "\n\t%s infile outfile pos var coor", name);

    (void) fprintf (stdout, "\n\n\t-h[elp]:");
    (void) fprintf (stdout, "\n\t\tPrint this summary of usage, and exit.");

    (void) fprintf (stdout, "\n\t\t");
    (void) fprintf (stdout, "\n\n\tinfile:");
    (void) fprintf (stdout, "\n\t\tName of the FLASH file to read");

    (void) fprintf (stdout, "\n\n\toutfile:");
    (void) fprintf (stdout, "\n\t\tName of the output file to create");

    (void) fprintf (stdout, "\n\n\tpos:");
    (void) fprintf (stdout, "\n\t\tOrientation of the slice (0 = xy, 1 = xz, 2 = yz)");

    (void) fprintf (stdout, "\n\n\tvar:");
    (void) fprintf (stdout, "\n\t\tName of the FLASH mesh variable");

    (void) fprintf (stdout, "\n\n\tcoor:");
    (void) fprintf (stdout, "\n\t\tValue of the coordinate perpendicular to the slice\n\n");
}

/*
 * Name:
 *      usage
 *
 * Purpose:
 *      Print a summary of command usage.
 */
static void 
usage(char *name)
{
    (void) fprintf(stderr, "\nUsage:\t%s -h[elp], OR\n", name);
    (void) fprintf(stderr, "\t%s infile outfile pos var coor\n", name);
}

/*
 * Name:
 *      trim
 *
 * Purpose:
 *     Trim white spaces in a string.
 */
static char *
trim(char *str, int len)
{
    int idx=0, n=len-1;
    char*  str_idx;

    str_idx = str;
    idx = 0;
    while (((int) *str_idx > 32) && (idx < n)) {
        idx++;
        str_idx++;
    }
    
    *(str+idx) = '\0';

    return str;
}

/*
 * Name:
 *      rodsConnect
 *
 * Purpose:
 *     Make connection to iRODS server.
 */
static rcComm_t *
rodsConnect()
{
    int status = 0;
    rodsEnv env;
    rcComm_t *conn=NULL;
    rErrMsg_t errMsg;

    status = getRodsEnv (&env);

    if (status < 0) {
        rodsLogError (LOG_ERROR, status, "main: getRodsEnv error. ");
        return NULL;
    }

    conn = rcConnect (env.rodsHost, env.rodsPort, env.rodsUserName, env.rodsZone, 1, &errMsg);

    if (conn == NULL) {
        rodsLogError (LOG_ERROR, errMsg.status, "rcConnect failure %s", errMsg.msg);
        return NULL;
    }

    status = clientLogin(conn);
    if (status != 0) {
        rcDisconnect(conn);
        return NULL;
    }

    return conn;
}

int main(int argc, char* argv[])
{
    /* input parameters from commandline */
    char   infile[80];
    char   outfile[80];
    int    pos;
    char   var[5];
    float  coor;

    /* slcice parameter */
    int    c, b, select;
    int    i1, i2, j1, j2, i, j, k, ii, jj;
    int    tot_blocks, lrefmin, lrefmax;
    int    Nx, Ny, Nz, N1, N2, N1b, N2b, N1r, N2r, nxb, nyb, nzb;
    float  dx1, dx2, xm1, xm2;
    float  xmin, xmax, ymin, ymax, zmin, zmax;
    float  dx, dy, dz;
    int    rebin_factor;
    int    *lrefine, *nodetype;
    float  *slice=NULL;
    double *data, *bnd_box;

    FILE*  outfilePtr;
    size_t *dims, *start, *stride, *count;

    /* iRODS connection */
    rcComm_t *conn = NULL;

    /* HDF5 objects */
    H5File *h5file=NULL;
    H5Dataset *h5dset=NULL;

    int ret_val = 0;

    conn = rodsConnect();
    if (conn == NULL) {
        (void) fprintf(stderr, "Failed to make connection to iRODS server.\n");
        exit(1);
    }

    if (argc==2 && strncmp(argv[1], "-h", 2) == 0) {
        help(argv[0]);
        exit(1);
    }

    /*
     * validate the number of command line arguments
     */
    if (argc < 6) {
        (void) fprintf(stderr, "Invalid command line arguments.\n");
        usage(argv[0]);
        exit(1);
    }

    /* input parameters */
    strcpy(infile, argv[1]);
    strcpy(outfile, argv[2]);
    pos = atoi(argv[3]);
    strncpy(var, argv[4], 5);
    coor = atof(argv[5]);

    trim(infile, 80);
    trim(outfile, 80);
    trim(var, 80);

    /* initialize file object */
    h5file = (H5File*)malloc(sizeof(H5File));
    H5File_ctor(h5file);
    h5file->filename = (char*)malloc(strlen(infile)+1);
    strcpy(h5file->filename, infile);

    /* open the file at the server and get the file structure */
    h5file->opID = H5FILE_OP_OPEN;
    ret_val = h5ObjRequest(conn, h5file, H5OBJECT_FILE);
    if (ret_val < 0 || h5file->fid < 0) {
        fprintf (stderr, "H5FILE_OP_OPEN failed, status = %d\n", ret_val);
        goto done;
    }

    /* get refine level */
    for (i=0; i<h5file->root->ndatasets; i++) {
        h5dset = (H5Dataset *) &(h5file->root->datasets[i]);
        if (strcmp(h5dset->fullpath+1, DNAME_REFINE_LEVEL) == 0)
            break;
        else 
            h5dset = NULL;
    }
    if (h5dset == NULL) {
        fprintf(stderr, "Failed to open dataset: %s.\n", DNAME_REFINE_LEVEL);
        goto done;
    }
    h5dset->opID = H5DATASET_OP_READ;
    ret_val = h5ObjRequest(conn, h5dset, H5OBJECT_DATASET);
    if (ret_val < 0) {
        fprintf (stderr, "H5DATASET_OP_READ failed, status = %d\n", ret_val);
        goto done;
    }
    tot_blocks = h5dset->space.dims[0];
    lrefine   = (int *)h5dset->value;

#ifdef DEBUG
    printf("refine level:\n");
    for (i = 0; i < tot_blocks; i++)
        printf("%d, ", lrefine[i]);
    puts("\n");
#endif

    /* get node type */
    for (i=0; i<h5file->root->ndatasets; i++) {
        h5dset = (H5Dataset *) &(h5file->root->datasets[i]);
        if (strcmp(h5dset->fullpath+1, DNAME_NODE_TYPE) == 0)
            break;
        else 
            h5dset = NULL;
    }
    if (h5dset == NULL) {
        fprintf(stderr, "Failed to open dataset: %s.\n", DNAME_NODE_TYPE);
        goto done;
    }
    h5dset->opID = H5DATASET_OP_READ;
    ret_val = h5ObjRequest(conn, h5dset, H5OBJECT_DATASET);
    if (ret_val < 0) {
        fprintf (stderr, "H5DATASET_OP_READ failed, status = %d\n", ret_val);
        goto done;
    }
    nodetype   = (int *)h5dset->value;
    
#ifdef DEBUG
    printf("node type:\n");
    for (i = 0; i < tot_blocks; i++)
        printf("%d, ", nodetype[i]);
    puts("\n");
#endif

    /* get bounding box */
    for (i=0; i<h5file->root->ndatasets; i++) {
        h5dset = (H5Dataset *) &(h5file->root->datasets[i]);
        if (strcmp(h5dset->fullpath+1, DNAME_BOUNDING_BOX) == 0)
            break;
        else 
            h5dset = NULL;
    }
    if (h5dset == NULL) {
        fprintf(stderr, "Failed to open dataset: %s.\n", DNAME_BOUNDING_BOX);
        goto done;
    }
    h5dset->opID = H5DATASET_OP_READ;
    ret_val = h5ObjRequest(conn, h5dset, H5OBJECT_DATASET);
    if (ret_val < 0) {
        fprintf (stderr, "H5DATASET_OP_READ failed, status = %d\n", ret_val);
        goto done;
    }
    bnd_box   = (double *)h5dset->value;

    xmin =  1.E99;
    xmax = -1.E99;
    ymin =  1.E99;
    ymax = -1.E99;
    zmin =  1.E99;
    zmax = -1.E99;

    for (i = 0; i < 6*tot_blocks; i += 6) {
        if (bnd_box[i  ] < xmin) { xmin = bnd_box[i  ]; }
        if (bnd_box[i+1] > xmax) { xmax = bnd_box[i+1]; }
        if (bnd_box[i+2] < ymin) { ymin = bnd_box[i+2]; }
        if (bnd_box[i+3] > ymax) { ymax = bnd_box[i+3]; }
        if (bnd_box[i+4] < zmin) { zmin = bnd_box[i+4]; }
        if (bnd_box[i+5] > zmax) { zmax = bnd_box[i+5]; }
    }

#ifdef DEBUG
    printf("bounding box:\n");
    printf("  x = %e ... %e\n", xmin, xmax);
    printf("  y = %e ... %e\n", ymin, ymax);
    printf("  z = %e ... %e\n", zmin, zmax);
#endif

    lrefmin = 100000;
    lrefmax = 0;

    for (i = 0; i < tot_blocks; i++) {
        if (lrefine[i] < lrefmin) { lrefmin = lrefine[i]; }
        if (lrefine[i] > lrefmax) { lrefmax = lrefine[i]; }
    }

    printf("refinement levels: %d ... %d\n", lrefmin, lrefmax);

    for (i=0; i<h5file->root->ndatasets; i++) {
        h5dset = (H5Dataset *) &(h5file->root->datasets[i]);
        if (strcmp(h5dset->fullpath+1, var) == 0)
            break;
        else 
            h5dset = NULL;
    }
    if (h5dset == NULL) {
        fprintf(stderr, "Failed to open dataset: %s.\n", var);
        goto done;
    }

    dims = h5dset->space.dims;
    start = h5dset->space.start;
    stride = h5dset->space.stride;
    count = h5dset->space.count;

    nxb  = (int)dims[3];
    nyb  = (int)dims[2];
    nzb  = (int)dims[1];

    start[0]  = 0;
    start[1]  = 0;
    start[2]  = 0;
    start[3]  = 0;
    stride[0] = 1;
    stride[1] = 1;
    stride[2] = 1;
    stride[3] = 1;
    count[0]  = 1;
    count[1]  = dims[1];
    count[2]  = dims[2];
    count[3]  = dims[3];

    Nx = dims[3];
    Ny = dims[2];
    Nz = dims[1];
    for (i = 0; i < lrefmax-1; i++) {
        Nx *= 2;
        Ny *= 2;
        Nz *= 2;
    }

    if (pos == 0) {
        slice = (float *)malloc(Nx*Ny*sizeof(float));
        N1    = Nx;
        N2    = Ny;
        N1b   = dims[3];
        N2b   = dims[2];
        dx1   = (xmax - xmin) / ((float)N1);
        xm1   = xmin;
        dx2   = (ymax - ymin) / ((float)N2);
        xm2   = ymin;
    } else if (pos == 1) {
        slice = (float *)malloc(Nx*Nz*sizeof(float));
        N1    = Nx;
        N2    = Nz;
        N1b   = dims[3];
        N2b   = dims[1];
        dx1   = (xmax - xmin) / ((float)N1);
        xm1   = xmin;
        dx2   = (zmax - zmin) / ((float)N2);
        xm2   = zmin;
    } else {
        slice = (float *)malloc(Ny*Nz*sizeof(float));
        N1    = Ny;
        N2    = Nz;
        N1b   = dims[2];
        N2b   = dims[1];
        dx1   = (ymax - ymin) / ((float)N1);
        xm1   = ymin;
        dx2   = (zmax - zmin) / ((float)N2);
        xm2   = zmin;
    }

#ifdef DEBUG
    printf("slice grid size:  %d x %d\n", N1, N2);
    printf("min zone spacing: %e x %e\n", dx1, dx2);
#endif

    c = 0;
    for (b = 0; b < tot_blocks; b++) {
        c = 6*b;

        if (nodetype[b] != 1)
            continue;
/*
#ifdef DEBUG
    printf("block %d, level %d\n", b, lrefine[b]);
    printf(" x --> %e  %e\n", bnd_box[c  ], bnd_box[c+1]);
    printf(" y --> %e  %e\n", bnd_box[c+2], bnd_box[c+3]);
    printf(" z --> %e  %e\n", bnd_box[c+4], bnd_box[c+5]);
#endif
*/

        dx = (float)(bnd_box[c+1] - bnd_box[c  ]) / ((float)dims[3]);
        dy = (float)(bnd_box[c+3] - bnd_box[c+2]) / ((float)dims[2]);
        dz = (float)(bnd_box[c+5] - bnd_box[c+4]) / ((float)dims[1]);

        select = 0;
        switch (pos) {
            case 0: 
                if ((bnd_box[c+4] <= coor) && (bnd_box[c+5] > coor)) select = 1;
                break;
            case 1: 
                if ((bnd_box[c+2] <= coor) && (bnd_box[c+3] > coor)) select = 1;
                break;
            case 2: 
                if ((bnd_box[c  ] <= coor) && (bnd_box[c+1] > coor)) select = 1;
                break;
        } /* switch (pos) */

        if (select != 1) 
            continue;

        rebin_factor = 1;
        for (i = 0; i < lrefmax-lrefine[b]; i++) {
            rebin_factor *= 2;
        }
        N1r = N1b * rebin_factor;
        N2r = N2b * rebin_factor;
    
        start[0] = b;
        h5dset->opID = H5DATASET_OP_READ;
        ret_val = h5ObjRequest(conn, h5dset, H5OBJECT_DATASET);
        if (ret_val < 0) {
            fprintf (stderr, "H5DATASET_OP_READ failed, status = %d\n", ret_val);
            goto done;
        }
        data = (double *)h5dset->value;
    
        switch (pos) {
            case 0: 
                i1 = (int)((bnd_box[c  ] - xm1)/dx1);
                i2 = (int)((bnd_box[c+1] - xm1)/dx1);
                j1 = (int)((bnd_box[c+2] - xm2)/dx2);
                j2 = (int)((bnd_box[c+3] - xm2)/dx2);
                k  = (int)((coor - bnd_box[c+4])/dz);
    
                for (j = j1; j < j2; j++) {
                    for (i = i1; i < i2; i++) {
                        ii = (i-i1)/rebin_factor;
                        jj = (j-j1)/rebin_factor;
                        slice[i+j*N1] = (float)data[ii+jj*N1b+k*N1b*N2b];
                    }
                }
                break;
            case 1: 
                i1 = (int)((bnd_box[c  ] - xm1)/dx1);
                i2 = (int)((bnd_box[c+1] - xm1)/dx1);
                j1 = (int)((bnd_box[c+4] - xm2)/dx2);
                j2 = (int)((bnd_box[c+5] - xm2)/dx2);
                k  = (int)((coor - bnd_box[c+2])/dy);
                for (j = j1; j < j2; j++) {
                    for (i = i1; i < i2; i++) {
                        ii = (i-i1)/rebin_factor;
                        jj = (j-j1)/rebin_factor;
                        slice[i+j*N1] = (float)data[ii+k*N1b+jj*N1b*N2b];
                    }
                }
                break;
            case 2: 
                i1 = (int)((bnd_box[c+2] - xm1)/dx1);
                i2 = (int)((bnd_box[c+3] - xm1)/dx1);
                j1 = (int)((bnd_box[c+4] - xm2)/dx2);
                j2 = (int)((bnd_box[c+5] - xm2)/dx2);
                k  = (int)((coor - bnd_box[c  ])/dx);
                for (j = j1; j < j2; j++) {
                    for (i = i1; i < i2; i++) {
                        ii = (i-i1)/rebin_factor;
                        jj = (j-j1)/rebin_factor;
                        slice[i+j*N1] = (float)data[k+ii*N1b+jj*N1b*N2b];
                    }
                }
                break;
        } /* switch (pos) */
    } /* for (b = 0; b < tot_blocks; b++) */

#ifdef DEBUG
    puts("slice data:");
    for (i=0; i<10; i++) {
        for (j=0; j<10; j++) {
            printf("%f, ", slice[N2*i+j]);
        }
        printf("\n");
     }
     puts("\n");
#endif

    outfilePtr = fopen(outfile, "w");
    fwrite(slice, N1*N2*sizeof(float), 1, outfilePtr);
    fclose(outfilePtr);

done:

    /* close remote file and clean memeory space held by the file object */
    if (h5file) {
        if (h5file->fid > 0) {
            h5file->opID = H5FILE_OP_CLOSE;
            h5ObjRequest(conn, h5file, H5OBJECT_FILE);
        }
        H5File_dtor(h5file);
        free(h5file);
    }

    if (slice)
        free (slice);

    rcDisconnect(conn);

    return ret_val;

}
