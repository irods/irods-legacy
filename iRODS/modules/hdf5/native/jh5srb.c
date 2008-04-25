#include <assert.h>
#include "jni.h"
#include "h5Object.h"
#include "h5File.h"
#include "h5Dataset.h"
#include "rcConnect.h"


#define NODEBUG
#define FILE_FIELD_SEPARATOR "::"

#ifdef _WIN32
#pragma comment(lib,"ws2_32")
#endif

extern int h5ObjRequest(rcComm_t *conn, void *obj, int objID);

#define THROW_JNI_ERROR(_ex, _msg) { \
    (*env)->ThrowNew(env, (*env)->FindClass(env, _ex), _msg); \
     ret_val = -1; \
     goto done; \
}

#define GOTO_JNI_ERROR() { \
     ret_val = -1; \
     goto done; \
}

/* count of open files */
static int file_count = 0;

rcComm_t *current_connection = NULL;
rcComm_t *make_connection(JNIEnv *env);
void     close_connection(rcComm_t *conn_t);

jint h5file_request(JNIEnv *env, jobject jobj);
jint h5dataset_request(JNIEnv *env, jobject jobj);
jint h5group_request(JNIEnv *env, jobject jobj);
jint j2c_h5file(JNIEnv *env, jobject jobj, H5File *cobj);
jint j2c_h5dataset(JNIEnv *env, jobject jdataset, H5Dataset *cobj);
jint j2c_h5group(JNIEnv *env, jobject jobj, H5Group *cobj);
jint c2j_h5file(JNIEnv *env, jobject jobj, H5File *cobj);
jint c2j_h5dataset_read(JNIEnv *env, jobject jdataset, H5Dataset *cobj);
jint c2j_h5dataset_read_attribute(JNIEnv *env, jobject jdataset, H5Dataset *cobj);
jint c2j_h5group(JNIEnv *env, jobject jfile, jobject jgroup, H5Group *cgroup);
jobject c2j_data_value (JNIEnv *env, void *value, unsigned int npoints, int tclass, int tsize);
jint c2j_h5group_read_attribute(JNIEnv *env, jobject jobj, H5Group *cobj);

int load_field_method_IDs(JNIEnv *env);
void set_field_method_IDs_scalar();
void set_field_method_IDs_compound();

/* for debug purpose */
void print_file(H5File *file);
void print_group(rcComm_t *conn, H5Group *pg);
void print_dataset(H5Dataset *d);
void print_dataset_value(H5Dataset *d);
void print_datatype(H5Datatype *type);
void print_dataspace(H5Dataspace *space);
void print_attribute(H5Attribute *a);


/**
 * Caching in the Defining Class's Initializer
 * For details, read http://java.sun.com/docs/books/jni/html/fldmeth.html#26855
 */

jclass    cls_file=NULL;
jfieldID  field_file_opID=NULL;
jfieldID  field_file_fid=NULL;
jfieldID  field_file_rootGroup=NULL;
jfieldID  field_file_fullFileName=NULL;

jfieldID  field_hobject_fullName=NULL;
jmethodID method_hobject_getFID=NULL;

jclass    cls_group=NULL;
jfieldID  field_group_opID=NULL;
jmethodID method_group_ctr=NULL;
jmethodID method_group_addToMemberList=NULL;
jmethodID method_group_addAttribute=NULL;

jclass    cls_dataset=NULL;
jfieldID  field_dataset_opID=NULL;
jfieldID  field_dataset_rank=NULL;
jfieldID  field_dataset_dims=NULL;
jfieldID  field_dataset_selectedDims=NULL;
jfieldID  field_dataset_startDims=NULL;
jfieldID  field_dataset_selectedIndex=NULL;
jfieldID  field_dataset_selectedStride=NULL;
jfieldID  field_dataset_datatype=NULL;
jmethodID method_dataset_ctr=NULL;
jmethodID method_dataset_setData=NULL;
jmethodID method_dataset_addAttribute=NULL;

jclass    cls_dataset_scalar=NULL;
jfieldID  field_dataset_scalar_opID=NULL;
jmethodID method_dataset_scalar_ctr=NULL;
jmethodID method_dataset_scalar_addAttribute=NULL;

/* unique for scalar dataset */
jmethodID method_dataset_scalar_setPalette=NULL;

jclass    cls_dataset_compound=NULL;
jfieldID  field_dataset_compound_opID=NULL;
jmethodID method_dataset_compound_ctr=NULL;
jmethodID method_dataset_compound_addAttribute=NULL;

/* unique for compound */
jfieldID  field_dataset_compound_memberNames=NULL;
jmethodID method_dataset_compound_setMemberCount=NULL;

jclass    cls_datatype=NULL;
jfieldID  field_datatype_class=NULL;
jfieldID  field_datatype_size=NULL;
jfieldID  field_datatype_order=NULL;
jfieldID  field_datatype_sign=NULL;

/*
 * Class:     ncsa_hdf_srb_H5SRB
 * Method:    getFileFieldSeparator
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ncsa_hdf_srb_H5SRB_getFileFieldSeparator
  (JNIEnv *env, jclass cls) 
{
    return (*env)->NewStringUTF(env,FILE_FIELD_SEPARATOR);
}
 
/*
 * Class:     ncsa_hdf_srb_H5SRB
 * Method:    listCollection
 * Signature: ([Ljava/lang/String;[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_srb_H5SRB_listCollection
  (JNIEnv *env, jclass cls, jobjectArray jfileList)
{
    jint ret_val = 0;


    return ret_val;
}

/*
 * Class:     ncsa_hdf_srb_H5SRB
 * Method:    h5ObjRequest
 * Signature: ([Ljava/lang/String;Ljava/lang/Object;I)I
 */
JNIEXPORT jint JNICALL Java_ncsa_hdf_srb_H5SRB_h5ObjRequest
  (JNIEnv *env, jclass cls, jobject jobj, jint obj_type)
{
    int ret_val=0;

    if (current_connection == NULL) {
        if ( (current_connection = make_connection(env)) == NULL )
            THROW_JNI_ERROR("java/lang/RuntimeException", "Cannot make connection to the server");
    }

    load_field_method_IDs(env);

    switch (obj_type) {
        case H5OBJECT_FILE:
            h5file_request(env, jobj);
            break;
        case H5OBJECT_DATASET:
            h5dataset_request(env, jobj);
            break;
        case H5OBJECT_GROUP:
            h5group_request(env, jobj);
            break;
        default:
            THROW_JNI_ERROR("java/lang/UnsupportedOperationException", "Unsupported client request");
            break;
    } /* end of switch */

done:
    return ret_val;
}

/* process HDF5-SRB file request: 
 * 1) decode Java message to C, 
 * 2) send request to the server
 * 3) encode server result to Java
 */
jint h5file_request(JNIEnv *env, jobject jobj)
{
    jint ret_val = 0;
    H5File h5file;

    assert(current_connection);

    H5File_ctor(&h5file);

    if ( (ret_val = j2c_h5file (env, jobj, &h5file)) < 0)
        goto done;

#ifdef DEBUG
    printf("opID = %d\n", h5file.opID);
    printf("fileName = %s\n", h5file.filename);
#endif

    /* send the request to the server to process */
    if (h5ObjRequest(current_connection, &h5file, H5OBJECT_FILE) < 0)
    {
        H5File_dtor(&h5file);
        THROW_JNI_ERROR("java/lang/RuntimeException", "file h5ObjRequest() failed");
    }

#ifdef DEBUG
    print_group(current_connection, h5file.root);fflush(stdout);
#endif

    if ( (ret_val = c2j_h5file (env, jobj, &h5file)) < 0)
        goto done;

    if (H5FILE_OP_OPEN == h5file.opID || H5FILE_OP_CREATE==h5file.opID)
        file_count++;
    else if (H5FILE_OP_CLOSE == h5file.opID)
        file_count--;

done:

    if (file_count <=0 ) {
        close_connection(current_connection);
    }

    H5File_dtor(&h5file);

    return ret_val;
}

/* process HDF5-SRB file request: 
 * 1) decode Java message to C, 
 * 2) send request to the server
 * 3) encode server result to Java
 */
jint h5dataset_request(JNIEnv *env, jobject jobj)
{
    jint ret_val=0, i=0;
    H5Dataset h5dataset;

    assert(current_connection);

    H5Dataset_ctor(&h5dataset);

    if ( (*env)->IsInstanceOf(env, jobj, (*env)->FindClass(env, "ncsa/hdf/srb/H5SrbScalarDS")) )
        set_field_method_IDs_scalar();
    else
        set_field_method_IDs_compound();

    if ( (ret_val = j2c_h5dataset(env, jobj, &h5dataset)) < 0)
        goto done;

    /* send the request to the server to process */
    if (h5ObjRequest(current_connection, &h5dataset, H5OBJECT_DATASET) < 0) {
        H5Dataset_dtor(&h5dataset);
        THROW_JNI_ERROR("java/lang/RuntimeException", "dataset h5ObjRequest() failed");
    }

    if (h5dataset.value && h5dataset.space.npoints==0) {
        h5dataset.space.npoints = 1;
        for (i=0; i<h5dataset.space.rank; i++)
            h5dataset.space.npoints *= h5dataset.space.count[i];
    }

    if ( h5dataset.opID == H5DATASET_OP_READ_ATTRIBUTE)
        ret_val = c2j_h5dataset_read_attribute (env, jobj, &h5dataset);
    else if (h5dataset.opID == H5DATASET_OP_READ)
        ret_val = c2j_h5dataset_read (env, jobj, &h5dataset);

done:

    H5Dataset_dtor(&h5dataset);

    return ret_val;
}

/* process HDF5-SRB file request: 
 * 1) decode Java message to C, 
 * 2) send request to the server
 * 3) encode server result to Java
 */
jint h5group_request(JNIEnv *env, jobject jobj)
{
    jint ret_val=0;
    H5Group h5group;

    assert(current_connection);

    H5Group_ctor(&h5group);

    if ( (ret_val = j2c_h5group(env, jobj, &h5group)) < 0)
        goto done;

    /* send the request to the server to process */
    if (h5ObjRequest(current_connection, &h5group, H5OBJECT_GROUP) < 0) {
        H5Group_dtor(&h5group);
        THROW_JNI_ERROR("java/lang/RuntimeException", "group h5ObjRequest() failed");
    }

    if ( h5group.opID == H5GROUP_OP_READ_ATTRIBUTE)
        ret_val = c2j_h5group_read_attribute (env, jobj, &h5group);

done:

    H5Group_dtor(&h5group);

    return ret_val;
}

/* construct C H5File structure from Java H5SrbFile object */
jint j2c_h5file(JNIEnv *env, jobject jobj, H5File *cobj)
{
    jint ret_val = 0;
    jstring jstr;
    const char *cstr;
    char jni_name[] = "j2c_h5file";

    assert(cobj);

    cobj->opID = (*env)->GetIntField(env, jobj, field_file_opID);
    cobj->fid = (*env)->GetIntField(env, jobj, field_file_fid);

    if (H5FILE_OP_OPEN == cobj->opID) {
         jstr = (*env)->GetObjectField(env, jobj, field_file_fullFileName);
         if (NULL == (cstr = (char *)(*env)->GetStringUTFChars(env, jstr, NULL)) )
            THROW_JNI_ERROR("java/lang/OutOfMemoryError", jni_name);

         cobj->filename = (char *)malloc(strlen(cstr)+1);
         strcpy(cobj->filename, cstr);
        (*env)->ReleaseStringUTFChars(env, jstr, cstr);
    }

done:
    return ret_val;
}

/* construct C H5Dataset structure from Java Dataset object */
jint j2c_h5dataset(JNIEnv *env, jobject jobj, H5Dataset *cobj)
{
    jint ret_val = 0;
    jstring jstr;
    const char *cstr;
    char jni_name[] = "j2c_h5dataset";
    jobject jtype;
    jlongArray ja;
    jlong *jptr;
    int i=0;

    assert(cobj);

    cobj->opID = (*env)->GetIntField(env, jobj, field_dataset_opID);
    cobj->fid = (*env)->CallIntMethod(env, jobj, method_hobject_getFID);

    /* set the full path */
    jstr = (*env)->GetObjectField(env, jobj, field_hobject_fullName);
    if (NULL == (cstr = (char *)(*env)->GetStringUTFChars(env, jstr, NULL)) )
        THROW_JNI_ERROR("java/lang/OutOfMemoryError", jni_name);
    cobj->fullpath = (char *)malloc(strlen(cstr)+1);
    strcpy(cobj->fullpath, cstr);
    (*env)->ReleaseStringUTFChars(env, jstr, cstr);

    if (H5DATASET_OP_READ == cobj->opID)
    {
        /* set datatype information */
        jtype = (*env)->GetObjectField(env, jobj, field_dataset_datatype);
        cobj->type.class = (H5Datatype_class_t) (*env)->GetIntField(env, jtype, field_datatype_class);
        cobj->type.size = (unsigned int) (*env)->GetIntField(env, jtype, field_datatype_size);
        /*cobj->type.order = (H5Datatype_order_t) (*env)->GetIntField(env, jtype, field_datatype_order);*/
        cobj->type.sign = (H5Datatype_sign_t) (*env)->GetIntField(env, jtype, field_datatype_sign);
        cobj->type.order =get_machine_endian();

        /* set rank */
        cobj->space.rank = (int)(*env)->GetIntField(env, jobj, field_dataset_rank);

        /* set dim information */
        ja = (*env)->GetObjectField(env, jobj, field_dataset_dims);
        jptr = (*env)->GetLongArrayElements(env, ja, 0);
        if (jptr != NULL) {
            for (i=0; i<cobj->space.rank; i++) {
                cobj->space.dims[i] = (unsigned int)jptr[i];
               }
            (*env)->ReleaseLongArrayElements(env, ja, jptr, 0); 
        }

        /* set start information */
        ja = (*env)->GetObjectField(env, jobj, field_dataset_startDims);
        jptr = (*env)->GetLongArrayElements(env, ja, 0);
        if (jptr != NULL) {
            for (i=0; i<cobj->space.rank; i++) {
                cobj->space.start[i] = (unsigned int)jptr[i];
            }
            (*env)->ReleaseLongArrayElements(env, ja, jptr, 0); 
        }


        /* set stride information */
        ja = (*env)->GetObjectField(env, jobj, field_dataset_selectedStride);
        jptr = (*env)->GetLongArrayElements(env, ja, 0);
        if (jptr != NULL) {
            for (i=0; i<cobj->space.rank; i++)  {
                cobj->space.stride[i] = (unsigned int)jptr[i];
            }
            (*env)->ReleaseLongArrayElements(env, ja, jptr, 0); 
        }

        /* set stride information */
        ja = (*env)->GetObjectField(env, jobj, field_dataset_selectedDims);
        jptr = (*env)->GetLongArrayElements(env, ja, 0);
        if (jptr != NULL) {
            for (i=0; i<cobj->space.rank; i++) {
                cobj->space.count[i] = (unsigned int)jptr[i];
            }
            (*env)->ReleaseLongArrayElements(env, ja, jptr, 0);
        }
    }

done:
    return ret_val;
}

/* construct C H5Group structure from Java Group object */
jint j2c_h5group(JNIEnv *env, jobject jobj, H5Group *cobj)
{
    jint ret_val = 0;
    jstring jstr;
    const char *cstr;
    char jni_name[] = "j2c_h5group";

    assert(cobj);

    cobj->opID = (*env)->GetIntField(env, jobj, field_group_opID);
    cobj->fid = (*env)->CallIntMethod(env, jobj, method_hobject_getFID);

    /* set the full path */
    jstr = (*env)->GetObjectField(env, jobj, field_hobject_fullName);
    if (NULL == (cstr = (char *)(*env)->GetStringUTFChars(env, jstr, NULL)) )
        THROW_JNI_ERROR("java/lang/OutOfMemoryError", jni_name);
    cobj->fullpath = (char *)malloc(strlen(cstr)+1);
    strcpy(cobj->fullpath, cstr);
    (*env)->ReleaseStringUTFChars(env, jstr, cstr);

done:
    return ret_val;
}

/* construct Java H5SrbFile object from C H5File structure */
jint c2j_h5file(JNIEnv *env, jobject jobj, H5File *cobj)
{
    jint ret_val = 0;
    jobject jroot;
    char jni_name[] = "c2j_h5file";

    assert(cobj);

    if (H5FILE_OP_CLOSE == cobj->opID)
        goto  done;

    /* set file id */
    (*env)->SetIntField(env, jobj, field_file_fid, (jint)cobj->fid);

    /* retrieve the root group */
    if (NULL == (jroot = (*env)->GetObjectField(env, jobj, field_file_rootGroup)) )
        THROW_JNI_ERROR("java/lang/NoSuchFieldException", jni_name);

    if ( c2j_h5group(env, jobj, jroot, cobj->root) < 0)
        THROW_JNI_ERROR("java/lang/RuntimeException", jni_name);

done:
    return ret_val;
}

/* construct Java group object from C group structure */
jint c2j_h5group(JNIEnv *env, jobject jfile, jobject jgroup, H5Group *cgroup)
{
    jint ret_val = 0;
    char jni_name[] = "c2j_h5group";
    int i=0,j=0;
    jstring jpath;
    jlongArray joid;
    jlongArray jdims;
    H5Group *cg;
    H5Dataset *cd;
    jobject jg;
    jobject jd;
    jlong *jptr;

    if (NULL==jfile || NULL == jgroup || NULL == cgroup)
        THROW_JNI_ERROR("java/lang/NullPointerException", jni_name);

    if (cgroup->groups && cgroup->ngroups>0) {
        for (i=0; i<cgroup->ngroups; i++) {
            cg = &cgroup->groups[i];
            if (NULL == cg) continue;

            /* get full path */
            jpath = (*env)->NewStringUTF(env,cg->fullpath);

            /* get the oid */
            joid = (*env)->NewLongArray(env, 1);
            jptr = (*env)->GetLongArrayElements(env, joid, 0);
            jptr[0] = (jlong)cg->objID[0];
            (*env)->ReleaseLongArrayElements(env, joid, jptr, 0); 

            /* create a new group */
            jg = (*env)->NewObject(env, cls_group, method_group_ctr, jfile, NULL, jpath, jgroup, joid);

            /* add the new group into its parant */
            (*env)->CallVoidMethod(env, jgroup, method_group_addToMemberList, jg);

            /* recursively call c2j_h5group to contruct the subtree */
            c2j_h5group(env, jfile, jg, cg);
        }
    }

    if (cgroup->datasets && cgroup->ndatasets>0) {
        for (i=0; i<cgroup->ndatasets; i++) {
            cd = &cgroup->datasets[i];
            if (NULL == cd) continue;

            if (H5DATATYPE_COMPOUND == cd->type.class)
                set_field_method_IDs_compound();
            else
                set_field_method_IDs_scalar();            

            /* get full path */
            jpath = (*env)->NewStringUTF(env,cd->fullpath);

            /* get the oid */
            joid = (*env)->NewLongArray(env, 1);
            jptr = (*env)->GetLongArrayElements(env, joid, 0);
            jptr[0] = (jlong)cd->objID[0];
            (*env)->ReleaseLongArrayElements(env, joid, jptr, 0); 

            /* create a new dataset */
            jd = (*env)->NewObject(env, cls_dataset, method_dataset_ctr, jfile, NULL, jpath, joid);

            /* for compound only */
            if (H5DATATYPE_COMPOUND == cd->type.class && cd->type.nmembers>0 && cd->type.mnames) {
                jobjectArray jmnames;
                jstring jname;

                /* set the names of the compound fields (the order of the calls are very important */
                (*env)->CallVoidMethod(env, jd, method_dataset_compound_setMemberCount, (jint)cd->type.nmembers);
                jmnames = (jobjectArray)(*env)->GetObjectField(env, jd, field_dataset_compound_memberNames);
                for (j=0; j<cd->type.nmembers; j++) {
	            jname = (*env)->NewStringUTF(env, cd->type.mnames[j]);
	            (*env)->SetObjectArrayElement(env, jmnames, j, jname);
	        }
            }
            else if (cd->attributes && 
                strcmp(PALETTE_VALUE, (cd->attributes)[0].name)==0 &&
                (cd->attributes)[0].value)
            {
                unsigned char *value;             
                jbyte *jbptr;
                jbyteArray jbytes;

                // setup palette
                value = (unsigned char *)(cd->attributes)[0].value;
                jbytes = (*env)->NewByteArray(env, 768);
                jbptr = (*env)->GetByteArrayElements(env, jbytes, 0);
                for (j=0; j<768; j++)
                {
                    jbptr[j] = (jbyte)value[j];
                }
                (*env)->ReleaseByteArrayElements(env, jbytes, jbptr, 0); 
                (*env)->CallVoidMethod(env, jd, method_dataset_scalar_setPalette, jbytes);
            }

            /* get the dims */
            jdims = (*env)->NewLongArray(env, cd->space.rank);
            jptr = (*env)->GetLongArrayElements(env, jdims, 0);
            for (j=0; j<cd->space.rank; j++) jptr[j] = (jlong)cd->space.dims[j];
            (*env)->ReleaseLongArrayElements(env, jdims, jptr, 0);

            /* add the dataset into its parant */
            (*env)->CallVoidMethod(env, jgroup, method_group_addToMemberList, jd);
        }
    }

done:

    return ret_val;
}

/* construct  Java Dataset object from C H5Dataset structure*/
jint c2j_h5dataset_read(JNIEnv *env, jobject jobj, H5Dataset *cobj)
{
    jint ret_val = 0;
    jobject jdata;

    assert(cobj);

    jdata = c2j_data_value (env, cobj->value, cobj->space.npoints, cobj->type.class, cobj->type.size);
    
    if (NULL == jdata)
        GOTO_JNI_ERROR();

    (*env)->CallVoidMethod(env, jobj, method_dataset_setData, jdata);

done:
    return ret_val;
}

/* construct  Java Dataset object from C H5Dataset structure for attributes*/
jint c2j_h5dataset_read_attribute(JNIEnv *env, jobject jobj, H5Dataset *cobj)
{
    jint ret_val = 0;
    int i=0,j=0;
    jlongArray jdims;
    jlong *jptr;
    H5Attribute attr;
    jstring attr_name;
    jobject attr_value;

    assert(cobj);

    if (cobj->nattributes <=0 )
        goto done;
    
    if (NULL == cobj->attributes)
        GOTO_JNI_ERROR();

    for (i=0; i<cobj->nattributes; i++)
    {
        attr = cobj->attributes[i];

        attr_name = (*env)->NewStringUTF(env, attr.name);
        attr_value = c2j_data_value (env, attr.value, attr.space.npoints, attr.type.class, attr.type.size);

        /* get the dims */
        jdims = (*env)->NewLongArray(env, attr.space.rank);
        jptr = (*env)->GetLongArrayElements(env, jdims, 0);
        for (j=0; j<attr.space.rank; j++) jptr[j] = (jlong)attr.space.dims[j];
        (*env)->ReleaseLongArrayElements(env, jdims, jptr, 0);

        /* add the attribute */
        (*env)->CallVoidMethod(env, jobj, method_dataset_addAttribute, 
            attr_name, attr_value, jdims, attr.type.class, attr.type.size, 
            attr.type.order, attr.type.sign);
     }


done:
    return ret_val;
}

jint c2j_h5group_read_attribute(JNIEnv *env, jobject jobj, H5Group *cobj)
{
    jint ret_val = 0;
    int i=0,j=0;
    jlongArray jdims;
    jlong *jptr;
    H5Attribute attr;
    jstring attr_name;
    jobject attr_value;

    assert(cobj);

    if (cobj->nattributes <=0 )
        goto done;
    
    if (NULL == cobj->attributes)
        GOTO_JNI_ERROR();

    for (i=0; i<cobj->nattributes; i++)
    {
        attr = cobj->attributes[i];

        attr_name = (*env)->NewStringUTF(env, attr.name);
        attr_value = c2j_data_value (env, attr.value, attr.space.npoints, attr.type.class, attr.type.size);

        /* get the dims */
        jdims = (*env)->NewLongArray(env, attr.space.rank);
        jptr = (*env)->GetLongArrayElements(env, jdims, 0);
        for (j=0; j<attr.space.rank; j++) jptr[j] = (jlong)attr.space.dims[j];
        (*env)->ReleaseLongArrayElements(env, jdims, jptr, 0);

        /* add the attribute */
        (*env)->CallVoidMethod(env, jobj, method_group_addAttribute, 
            attr_name, attr_value, jdims, attr.type.class, attr.type.size, 
            attr.type.order, attr.type.sign);
     }


done:
    return ret_val;
}

int load_field_method_IDs(JNIEnv *env) 
{
    int ret_val = -1;
    jclass cls;

    ///////////////////////////////////////////////////////////////////////////
    //                              FileFormat                               //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/object/FileFormat");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/object/FileFormat");
    
    /* fields and methods for FileFormat */    
    field_file_fid = (*env)->GetFieldID(env, cls, "fid", "I");
    if (!field_file_fid)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/FileFormat.fid");
 
    ///////////////////////////////////////////////////////////////////////////
    //                              H5SrbFile                                //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/srb/H5SrbFile");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/srb/H5SrbFile");

    cls_file = cls;

    field_file_opID = (*env)->GetFieldID(env, cls, "opID", "I");
    if (!field_file_opID)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/srb/H5SrbFile.opID");

    field_file_fullFileName = (*env)->GetFieldID(env, cls, "fullFileName", "Ljava/lang/String;");
    if (!field_file_fullFileName)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/srb/H5SrbFile.fullFileName");

    field_file_rootGroup = (*env)->GetFieldID(env, cls, "rootGroup", "Lncsa/hdf/srb/H5SrbGroup;");
    if (!field_file_rootGroup)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/srb/H5SrbFile.rootGroup");

    ///////////////////////////////////////////////////////////////////////////
    //                              HObject                                  //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/object/HObject");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/object/HObject");

    field_hobject_fullName = (*env)->GetFieldID(env, cls, "fullName", "Ljava/lang/String;");
    if (!field_hobject_fullName)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/HObject.fullName");

    method_hobject_getFID = (*env)->GetMethodID(env, cls, "getFID", "()I");
    if (!method_hobject_getFID)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException","ncsa/hdf/oject/HObject.getFID");

    ///////////////////////////////////////////////////////////////////////////
    //                              H5SrbGroup                               //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/srb/H5SrbGroup");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/srb/H5SrbGroup");

    cls_group = cls;

    field_group_opID = (*env)->GetFieldID(env, cls, "opID", "I");
    if (!field_group_opID)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/srb/H5SrbGroup.opID");

    method_group_ctr = (*env)->GetMethodID(env, cls, "<init>", 
        "(Lncsa/hdf/object/FileFormat;Ljava/lang/String;Ljava/lang/String;Lncsa/hdf/object/Group;[J)V");
     if (!method_group_ctr)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException","ncsa/hdf/srb/H5SrbGroup.<init>");

    method_group_addToMemberList = (*env)->GetMethodID(env, cls, "addToMemberList", "(Lncsa/hdf/object/HObject;)V");
    if (!method_group_addToMemberList)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException","ncsa/hdf/srb/H5SrbGroup.addToMemberList()");

    method_group_addAttribute = (*env)->GetMethodID(env, cls, "addAttribute", "(Ljava/lang/String;Ljava/lang/Object;[JIIII)V");
    if (!method_group_addAttribute)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException","ncsa/hdf/srb/H5SrbGroup.addAttribute()");

    ///////////////////////////////////////////////////////////////////////////
    //                                 Dataset                               //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/object/Dataset");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/object/Dataset");
    
    field_dataset_rank = (*env)->GetFieldID(env, cls, "rank", "I");
    if (!field_dataset_rank)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.rank");

    field_dataset_dims = (*env)->GetFieldID(env, cls, "dims", "[J");
    if (!field_dataset_dims)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.dims");

    field_dataset_selectedDims = (*env)->GetFieldID(env, cls, "selectedDims", "[J");
    if (!field_dataset_selectedDims)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.selectedDims");

    field_dataset_startDims = (*env)->GetFieldID(env, cls, "startDims", "[J");
    if (!field_dataset_startDims)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.startDims");

    field_dataset_selectedIndex = (*env)->GetFieldID(env, cls, "selectedIndex", "[I");
    if (!field_dataset_selectedIndex)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.selectedIndex");

    field_dataset_selectedStride = (*env)->GetFieldID(env, cls, "selectedStride", "[J");
    if (!field_dataset_selectedStride)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.selectedStride");

    field_dataset_datatype = (*env)->GetFieldID(env, cls, "datatype", "Lncsa/hdf/object/Datatype;");
    if (!field_dataset_datatype)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Dataset.datatype");

    method_dataset_setData = (*env)->GetMethodID(env, cls, "setData", "(Ljava/lang/Object;)V");
    if (!method_dataset_setData)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/object/Dataset.setData");

    ///////////////////////////////////////////////////////////////////////////
    //                               H5SrbScalarDS                           //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/srb/H5SrbScalarDS");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/srb/H5SrbScalarDS");

    cls_dataset_scalar = cls;

    field_dataset_scalar_opID = (*env)->GetFieldID(env, cls, "opID", "I");
    if (!field_dataset_scalar_opID)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/srb/H5SrbScalarDS.opID");

    method_dataset_scalar_ctr = (*env)->GetMethodID(env, cls, "<init>", 
        "(Lncsa/hdf/object/FileFormat;Ljava/lang/String;Ljava/lang/String;[J)V");
    if (!method_dataset_scalar_ctr)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/srb/H5SrbScalarDS.<init>");
    
    method_dataset_scalar_addAttribute = (*env)->GetMethodID(env, cls, "addAttribute", 
        "(Ljava/lang/String;Ljava/lang/Object;[JIIII)V");
    if (!method_dataset_scalar_addAttribute)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/srb/H5SrbScalarDS.addAttribute()");

    method_dataset_scalar_setPalette = (*env)->GetMethodID(env, cls, "setPalette", "([B)V"); 
    if (!method_dataset_scalar_setPalette)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/srb/H5SrbScalarDS.setPalette()");

    ///////////////////////////////////////////////////////////////////////////
    //                                  CompoundDS                           //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/object/CompoundDS");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/object/CompoundDS");

    field_dataset_compound_memberNames = (*env)->GetFieldID(env, cls, "memberNames", "[Ljava/lang/String;");
    if (!field_dataset_compound_memberNames)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/CompoundDS.memberNames");

    ///////////////////////////////////////////////////////////////////////////
    //                               H5SrbCompoundDS                         //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/srb/H5SrbCompoundDS");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/srb/H5SrbCompoundDS");

    cls_dataset_compound = cls;

    field_dataset_compound_opID = (*env)->GetFieldID(env, cls, "opID", "I");
    if (!field_dataset_compound_opID)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/srb/H5SrbCompoundDS.opID");

    method_dataset_compound_ctr = (*env)->GetMethodID(env, cls, "<init>", 
        "(Lncsa/hdf/object/FileFormat;Ljava/lang/String;Ljava/lang/String;[J)V");
    if (! method_dataset_compound_ctr)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/srb/H5SrbCompoundDS.<init>");

    method_dataset_compound_addAttribute = (*env)->GetMethodID(env, cls, "addAttribute", 
        "(Ljava/lang/String;Ljava/lang/Object;[JIIII)V");
    if (!method_dataset_compound_addAttribute)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/srb/H5SrbCompoundDS.addAttribute");

    method_dataset_compound_setMemberCount = (*env)->GetMethodID(env, cls_dataset_compound,"setMemberCount", "(I)V");
    if (!method_dataset_compound_setMemberCount)
        THROW_JNI_ERROR ("java/lang/NoSuchMethodException", "ncsa/hdf/srb/H5SrbCompoundDS.setMemberCount");

    ///////////////////////////////////////////////////////////////////////////
    //                                    Datatype                           //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/object/Datatype");
    if (!cls)
        THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/object/Datatype");

    field_datatype_class = (*env)->GetFieldID(env, cls, "datatypeClass", "I");
    if (!field_datatype_class)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Datatype.datatypeClass");

    field_datatype_size = (*env)->GetFieldID(env, cls, "datatypeSize", "I");
    if (!field_datatype_size)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Datatype.datatypeSize");

    field_datatype_order = (*env)->GetFieldID(env, cls, "datatypeOrder", "I");
    if (!field_datatype_order)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Datatype.datatypeOrder");

    field_datatype_sign = (*env)->GetFieldID(env, cls, "datatypeSign", "I");
    if (!field_datatype_sign)
        THROW_JNI_ERROR ("java/lang/NoSuchFieldException", "ncsa/hdf/object/Datatype.datatypeSign");

    ///////////////////////////////////////////////////////////////////////////
    //                               H5SrbDatatype                           //
    ///////////////////////////////////////////////////////////////////////////
    cls = (*env)->FindClass(env, "ncsa/hdf/srb/H5SrbDatatype");
    if (!cls)
         THROW_JNI_ERROR ("java/lang/ClassNotFoundException", "ncsa/hdf/srb/H5SrbDatatype");

    cls_datatype = cls;

    ret_val = 0;

done:

    return ret_val;
}

void set_field_method_IDs_scalar()
{
    cls_dataset = cls_dataset_scalar;
    field_dataset_opID = field_dataset_scalar_opID;
    method_dataset_ctr = method_dataset_scalar_ctr;
    method_dataset_addAttribute = method_dataset_scalar_addAttribute;
}

void set_field_method_IDs_compound()
{
    cls_dataset = cls_dataset_compound;
    field_dataset_opID = field_dataset_compound_opID;
    method_dataset_ctr = method_dataset_compound_ctr;
    method_dataset_addAttribute = method_dataset_compound_addAttribute;
}

rcComm_t *make_connection(JNIEnv *env) {
    rcComm_t *conn_t;
    rodsEnv rEnv;
    rErrMsg_t errMsg;
    int status;

    status = getRodsEnv(&rEnv);
    if (status<0) {
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "getRodsEnv() failed");
        return NULL;
    }

    conn_t = rcConnect (rEnv.rodsHost, rEnv.rodsPort, rEnv.rodsUserName, rEnv.rodsZone, 1, &errMsg);

    if ( (NULL == conn_t) ) {
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), errMsg.msg);
        return NULL;
    }

    status = clientLogin(conn_t);

    if (status != 0) {
        rcDisconnect(conn_t);
        (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "Client login failed");
        return NULL;
    }

    return conn_t;
}

void close_connection(rcComm_t *conn_t) 
{
    if (conn_t != NULL) {
        rcDisconnect (conn_t);
        conn_t = NULL;
    }
}   

/* write data value from c to java */
jobject c2j_data_value (JNIEnv *env, void *value, unsigned int npoints, int tclass, int tsize)
{
    jobject jvalue = NULL;
    unsigned int i=0;
    jstring jstr;
    char **strs;
    jobjectArray jobj_a;

    if (NULL == value)
        return NULL;

    switch (tclass) {
        case H5DATATYPE_INTEGER:
        case H5DATATYPE_REFERENCE:
            if (1 == tsize) {
                jbyte *ca = (jbyte *)value;
                jbyteArray ja = (*env)->NewByteArray(env, npoints);
                jbyte *jptr = (*env)->GetByteArrayElements(env, ja, 0);
                for (i=0; i<npoints; i++) jptr[i] = ca[i];
                (*env)->ReleaseByteArrayElements(env, ja, jptr, 0); 
                jvalue = ja;
            }
            else if (2 == tsize) {
                jshort *ca = (jshort *)value;
                jshortArray ja = (*env)->NewShortArray(env, npoints);
                jshort *jptr = (*env)->GetShortArrayElements(env, ja, 0);
                for (i=0; i<npoints; i++) jptr[i] = ca[i];
                (*env)->ReleaseShortArrayElements(env, ja, jptr, 0); 
                jvalue = ja;
            }
            else if (4 == tsize) {
                jint *ca = (jint *)value;
                jintArray ja = (*env)->NewIntArray(env, npoints);
                jint *jptr = (*env)->GetIntArrayElements(env, ja, 0);
                for (i=0; i<npoints; i++)
                {
                    jptr[i] = ca[i];
                }
                (*env)->ReleaseIntArrayElements(env, ja, jptr, 0); 
                jvalue = ja;
            }
            else {
                jlong *ca = (jlong *)value;
                jlongArray ja = (*env)->NewLongArray(env, npoints);
                jlong *jptr = (*env)->GetLongArrayElements(env, ja, 0);
                for (i=0; i<npoints; i++) jptr[i] = ca[i];
                (*env)->ReleaseLongArrayElements(env, ja, jptr, 0); 
                jvalue = ja;
            }
            break;
        case H5DATATYPE_FLOAT:
            if (4 == tsize) {
                jfloat *ca = (jfloat *)value;
                jfloatArray ja = (*env)->NewFloatArray(env, npoints);
                jfloat *jptr = (*env)->GetFloatArrayElements(env, ja, 0);
                for (i=0; i<npoints; i++) jptr[i] = ca[i];
                (*env)->ReleaseFloatArrayElements(env, ja, jptr, 0); 
                jvalue = ja;
            }
            else {
                jdouble *ca = (jdouble *)value;
                jdoubleArray ja = (*env)->NewDoubleArray(env, npoints);
                jdouble *jptr = (*env)->GetDoubleArrayElements(env, ja, 0);
                for (i=0; i<npoints; i++) jptr[i] = ca[i];
                (*env)->ReleaseDoubleArrayElements(env, ja, jptr, 0); 
                jvalue = ja;
            }
            break;
        case H5DATATYPE_STRING:
        case H5DATATYPE_VLEN:
        case H5DATATYPE_COMPOUND:
            strs = (char **)value;
            jobj_a = (*env)->NewObjectArray(env, npoints, 
                (*env)->FindClass(env, "java/lang/String"), (*env)->NewStringUTF(env, ""));
	        for (i=0; i<npoints; i++) {
		        jstr = (*env)->NewStringUTF(env, strs[i]);
		        (*env)->SetObjectArrayElement(env, jobj_a, i, jstr);
	        }; 
            jvalue = jobj_a;
            break;
        default:
            jvalue = NULL;
            break;
    }

    return jvalue;

}


/**********************************************************************
 *                                                                    *
 *                       Debug Rountines                              *
 *                                                                    *
 **********************************************************************/

void print_file(H5File *file)
{
    assert(file);
    
    printf("\nFile name = %s\n", file->filename);
    printf("opID = %d\n", file->opID);
}

void print_group(rcComm_t *conn, H5Group *pg)
{
    int i=0;
    H5Group *g=0;
    H5Dataset *d=0;

    assert(pg);

    for (i=0; i<pg->ngroups; i++)
    {
        g = (H5Group *) &pg->groups[i];
        printf("%s\n", g->fullpath);
        print_group(conn, g);
    }

    for (i=0; i<pg->ndatasets; i++)
    {
        d = (H5Dataset *) &pg->datasets[i];
        d->opID = H5DATASET_OP_READ;
        printf("%s\n", d->fullpath);

        if (h5ObjRequest(conn, d, H5OBJECT_DATASET) < 0) {
            fprintf (stderr, "H5DATASET_OP_READ failed\n");
            goto done;
        }

        print_dataset(d); /* print_dataset_value(d); */
    }

done:
    return;
}

void print_datatype(H5Datatype *type)
{
    int i = 0;

    assert(type);

    printf("\ntype class = %d\n", type->class);
    printf(  "type order = %d\n", type->order);
    printf(  "type sign  = %d\n", type->sign);
    printf(  "type size  = %d\n", type->size);

    printf(  "nmembers   = %d\n", type->nmembers);
    if (type->nmembers > 0 && type->mnames != NULL)
    {
        for (i=0; i<type->nmembers; i++)
            printf("member[%d] = %s\n", i, type->mnames[i]);
    }

}

void print_dataspace(H5Dataspace *space)
{
    int i = 0;

    assert (space);

    printf("\nspace rank = %d\n", space->rank);
    printf(  "space npoints = %d\n", space->npoints);

    if (space->rank > 0 && space->dims != NULL)
    {
        for (i=0; i<space->rank; i++) {
            printf("dims[%d] = %d\n", i, space->dims[i]);
	    }
    }

    if (space->rank > 0 && space->start != NULL)
    {
        for (i=0; i<space->rank; i++) {
            printf("start[%d] = %d\n", i, space->start[i]);
	    }
    }


    if (space->rank > 0 && space->stride != NULL)
    {
        for (i=0; i<space->rank; i++) {
            printf("stride[%d] = %d\n", i, space->stride[i]);
	    }
    }


    if (space->rank > 0 && space->count != NULL)
    {
        for (i=0; i<space->rank; i++) {
            printf("count[%d] = %d\n", i, space->count[i]);
	    }
    }
}

void print_dataset(H5Dataset *d)
{
    assert(d);

    printf("\nDataset fullpath = %s\n", d->fullpath);
    printf("fid = %d\n", d->fid);
    print_datatype(&(d->type));
    print_dataspace(&(d->space));
    print_dataset_value(d);
}


void print_dataset_value(H5Dataset *d)
{
    int size=0, i=0;
    char* pv;
    char **strs;

    assert(d);

    printf("\nThe total size of the value buffer = %d", d->nvalue);
    printf("\nPrinting the first 10 values of %s\n", d->fullpath);
    if (d->value)
    {
        size = 1;
        pv = (char*)d->value;
        for (i=0; i<d->space.rank; i++) size *= d->space.count[i];
        if (size > 10 ) size = 10; /* print only the first 10 values */
        if (d->type.class == H5DATATYPE_VLEN
            || d->type.class == H5DATATYPE_COMPOUND
            || d->type.class == H5DATATYPE_STRING)
            strs = (char **)d->value;

        for (i=0; i<size; i++)
        {
            if (d->type.class == H5DATATYPE_INTEGER)
            {
                if (d->type.sign == H5DATATYPE_SGN_NONE)
                {
                    if (d->type.size == 1)
                        printf("%u\t", *((unsigned char*)(pv+i)));
                    else if (d->type.size == 2)
                        printf("%u\t", *((unsigned short*)(pv+i*2)));
                    else if (d->type.size == 4)
                        printf("%u\t", *((unsigned int*)(pv+i*4)));
                    else if (d->type.size == 8)
                        printf("%u\t", *((unsigned long*)(pv+i*8)));
                }
                else
                {
                    if (d->type.size == 1)
                        printf("%d\t", *((char*)(pv+i)));
                    else if (d->type.size == 2)
                        printf("%d\t", *((short*)(pv+i*2)));
                    else if (d->type.size == 4)
                        printf("%d\t", *((int*)(pv+i*4)));
                    else if (d->type.size == 8)
                        printf("%d\t", *((long*)(pv+i*8)));
                }
            } else if (d->type.class == H5DATATYPE_FLOAT)
            {
                if (d->type.size == 4)
                    printf("%f\t", *((float *)(pv+i*4)));
                else if (d->type.size == 8)
                    printf("%f\t", *((double *)(pv+i*8)));
            } else if (d->type.class == H5DATATYPE_VLEN
                    || d->type.class == H5DATATYPE_COMPOUND)
            {
                if (strs[i])
                    printf("%s\t", strs[i]);
            } else if (d->type.class == H5DATATYPE_STRING)
            {
                if (strs[i])
                    printf("%s\t", strs[i]);
            }
        }
        printf("\n\n");
    }

    return;
}

void print_attribute(H5Attribute *a)
{
    int size=0, i=0;
    char* pv;
    char **strs;

    assert(a);

    printf("\n\tThe total size of the attribute value buffer = %d\n", a->nvalue);
    printf("\n\tPrinting the first 10 values of %s\n", a->name);
    if (a->value)
    {
        size = 1;
        pv = (char*)a->value;
        for (i=0; i<a->space.rank; i++) size *= a->space.count[i];
        if (size > 10 ) size = 10; /* print only the first 10 values */
        if (a->type.class == H5DATATYPE_VLEN
            || a->type.class == H5DATATYPE_COMPOUND
            || a->type.class == H5DATATYPE_STRING)
            strs = (char **)a->value;

        for (i=0; i<size; i++)
        {
            if (a->type.class == H5DATATYPE_INTEGER)
            {
                if (a->type.sign == H5DATATYPE_SGN_NONE)
                {
                    if (a->type.size == 1)
                        printf("%u\t", *((unsigned char*)(pv+i)));
                    else if (a->type.size == 2)
                        printf("%u\t", *((unsigned short*)(pv+i*2)));
                    else if (a->type.size == 4)
                        printf("%u\t", *((unsigned int*)(pv+i*4)));
                    else if (a->type.size == 8)
                        printf("%u\t", *((unsigned long*)(pv+i*8)));
                }
                else
                {
                    if (a->type.size == 1)
                        printf("%d\t", *((char*)(pv+i)));
                    else if (a->type.size == 2)
                        printf("%d\t", *((short*)(pv+i*2)));
                    else if (a->type.size == 4)
                        printf("%d\t", *((int*)(pv+i*4)));
                    else if (a->type.size == 8)
                        printf("%d\t", *((long *)(pv+i*8)));
                }
            } else if (a->type.class == H5DATATYPE_FLOAT)
            {
                if (a->type.size == 4)
                    printf("%f\t", *((float *)(pv+i*4)));
                else if (a->type.size == 8)
                    printf("%f\t", *((double *)(pv+i*8)));
            } else if (a->type.class == H5DATATYPE_VLEN
                    || a->type.class == H5DATATYPE_COMPOUND)
            {
                if (strs[i])
                    printf("%s\t", strs[i]);
            } else if (a->type.class == H5DATATYPE_STRING)
            {
                if (strs[i]) printf("%s\t", strs[i]);
            }
        }
        printf("\n\n");
    }
}


