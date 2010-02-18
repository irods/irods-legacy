//  Copyright (c) 2005, Regents of the University of California
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//    * Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of California, San Diego (UCSD) nor
//  the names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//  SRBMetaDataSet.java  -  edu.sdsc.grid.io.srb.SRBMetaDataSet
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.MetaDataSet
//          |
//          +-.SRBMetaDataSet
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import java.io.IOException;
import java.util.HashMap;

import edu.sdsc.grid.io.DirectoryMetaData;
import edu.sdsc.grid.io.DublinCoreMetaData;
import edu.sdsc.grid.io.FileMetaData;
import edu.sdsc.grid.io.GeneralFileSystem;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataGroup;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.grid.io.UserMetaData;
import edu.sdsc.grid.io.ZoneMetaData;

/**
 * SRBMetaDataSet registers all the SRB metadata fields. The fields are divided
 * among various groups to make listing and using the metadata fields more
 * convienent.
 *<P>
 * This class is basically a listing of all the queriable SRB metadata
 * fieldnames. A few hundred static final Strings which equal some more readable
 * name. These strings are then used when forming a MetaDataCondition or
 * MetaDataSelect object.
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 */
/*
 * When adding new metadata fields. Four items need to be created for each field
 * added.
 * 
 * DON'T FORGET THESE. If you forget them, it could take a long time to figure
 * out the problem or even notice it.
 * 
 * 1) The static string name which equals a short but readable name. - If you
 * forget this, the compiler will fail.
 * 
 * 2) The addition of that name and its related srb ID integer to the
 * jargonToSRBID. - If you forget this, it can only be caught at runtime. Such
 * values will be treated as extensible then error only when that extensible
 * value cannot be found on the SRB server. A NullPointerException will be
 * thrown stating this occurance.
 * "Extensible metadata or invalid metadata attribute: "+
 * selects[i].getFieldName()+ "not supported with this Jargon version"
 * 
 * 3) The addition to srbNameToGeneralName of the return string sent by the srb
 * and the name used in 1). - If you forget this, it can only be caught at
 * runtime. A NullPointerException will occur when the field is used as a select
 * in a query. "This srb return value: "+srbTableName+" : "+srbAttributeName+
 * ", is not supported in this version." - see also getGeneralMetaData( String
 * srbName )
 * 
 * 4) The addition of a MetaDataField( name, description, value type ) to the
 * group hashmap. An appropriate group for this field must also be chosen. - If
 * you forget this, it can only be caught at runtime. A NullPointerException,
 * where the fieldName will not be found to correspond with any field object,
 * will occur when the field is used as a condition or a select in a query.
 * "field cannot be null." - see also MetaDataSet.getField( String fieldName )
 * and - the MetaDataCondition and MetaDataSelect constructors.
 */
public final class SRBMetaDataSet extends MetaDataSet implements
		DublinCoreMetaData, DirectoryMetaData, FileMetaData, ResourceMetaData,
		UserMetaData, ZoneMetaData {

	/**
	 * Total SRB metadata attributes, includes System and the spaces for
	 * definable and extensible.
	 */
	private static final int TOTAL_METADATA_FIELDS = 500;

	/**
	 * The SRB expects a integer, this hashmap is used to convert an extensible
	 * String name into that integer. Extensible IDs are different because they
	 * can change each query. First a query must be done to inform the SRB which
	 * extensible attributes will be queried. This is used to fill the 420-500
	 * values of the catalogNames array on the SRB server.
	 */
	private static HashMap jargonToSRBID = new HashMap(TOTAL_METADATA_FIELDS);

	private static final int EXTENSIBLE_START_NUM = 420;

	/**
	 * The SRB return the table and attribute name, this hashmap is used to
	 * convert thosee to Jargon fieldname.
	 */
	private static HashMap srbToJargon = new HashMap(TOTAL_METADATA_FIELDS);

	/** the reverse of srbToJargon */
	private static HashMap jargonToSRB = new HashMap(TOTAL_METADATA_FIELDS);

	final static char nll = 0;

	/** Define the query protocol used */
	static SRBProtocol protocol;

	// Special attributes
	/** return the query values sorted according to another attribute */
	final static String ORDERBY = "order by";

	/** nondistinct */
	final static String NONDISTINCT = "nondistinct";

	/**
	 * Group name for metadata relating to User-defined metadata attributes
	 * These are the GROUP_DEFINABLE_METADATA attributes using a
	 * non-MetaDataTable access.
	 */
	final static String GROUP_UDMD = "User-defined metadata";

	// **********************************************************
	// MetaDataField Names
	// **********************************************************
	// When updating for new SRB versions, and
	// adding metadata attribute pairs, need to change
	// SRBFileSystem.TOTAL_METADATA_ATTRIBUTES
	// to match, 1 + last DCS-ATTRIBUTE-INDEX
	// see also c client, catalog/include/mdasC_db2_externs.h
	// and SRBCommands class.

	/** Informs the server which zone you intend to use */
	public final static String CURRENT_ZONE = "current zone";

	/** internal data id */
	final static String FILE_ID = "file id";

	/** replica copy number */
	public final static String FILE_REPLICATION_ENUM = FILE_REPLICA_NUM;

	/** name of user group */
	public final static String USER_GROUP_NAME = USER_GROUP;

	/** file or data type name */
	public final static String FILE_TYPE_NAME = FILE_TYPE;

	/** user type */
	public final static String USER_TYPE_NAME = USER_TYPE;

	/** user domain name */
	public final static String USER_DOMAIN = "user domain";

	/** net address of resource */
	public final static String RESOURCE_ADDRESS_NETPREFIX = "resource address netprefix";

	/** logical resource type */
	public final static String RESOURCE_TYPE_NAME = RESOURCE_TYPE;

	/** audit on user time stamp */
	public final static String USER_AUDIT_TIME_STAMP = "user audit time stamp";

	/** audit on user comments */
	public final static String USER_AUDIT_COMMENTS = "user audit comments";

	/** audited action on data */
	public final static String AUDIT_ACTION_DESCRIPTION = "audit action description";

	/** audit time stamp for data */
	public final static String AUDIT_TIMESTAMP = "audit timestamp";

	/** audit comments for data */
	public final static String AUDIT_COMMENTS = "audit comments";

	/** collection name for access in DIRECTORY_ACCESS_CONSTRAINT */
	public final static String ACCESS_DIRECTORY_NAME = "access directory name";

	/** default path in logical resource */
	public final static String RESOURCE_DEFAULT_PATH = "resource default path";

	/** default path in physical resource */
	public final static String PHYSICAL_RESOURCE_DEFAULT_PATH = "physical resource default path";

	/** physical resource name */
	public final static String PHYSICAL_RESOURCE_NAME = "physical resource name";

	/** physical resource type */
	public final static String PHYSICAL_RESOURCE_TYPE_NAME = "physical resource type name";

	/** index of physical resource in logical resource */
	public final static String RESOURCE_REPLICATION_ENUM = "resource replication enum";

	/** access list for data */
	public final static String FILE_ACCESS_LIST = "file access list";

	/** access list for resource */
	public final static String RESOURCE_ACCESS_LIST = "resource access list";

	/** data liveness */
	public final static String FILE_IS_DELETED = "file is deleted";

	/** identifier for ticket given for data */
	public final static String TICKET_D = "ticket data";

	/** data ticket validity start time */
	public final static String TICKET_BEGIN_TIME_D = "ticket begin time data";

	/** data ticket validity end time */
	public final static String TICKET_END_TIME_D = "ticket end time data";

	/** valid number of opens allowed on data ticket */
	public final static String TICKET_ACC_COUNT_D = "ticket acc count data";

	/** valid access allowed on data ticket currently readonly */
	public final static String TICKET_ACC_LIST_D = "ticket acc list data";

	/** data ticket creator */
	public final static String TICKET_OWNER_D = "ticket owner data";

	/** allowed ticket user or user group */
	public final static String TICKET_USER_D = "ticket user data";

	/** identifier for ticket given for collection and sub collections */
	public final static String TICKET_C = "ticket collection";

	/** collection ticket validity start time */
	public final static String TICKET_BEGIN_TIME_C = "ticket begin time collection";

	/** collection ticket validity end time */
	public final static String TICKET_END_TIME_C = "ticket end time collection";

	/** valid number of opens allowed on data in collection */
	public final static String TICKET_ACC_COUNT_C = "ticket acc count collection";

	/** valid access allowed on data in collection currently readonly */
	public final static String TICKET_ACC_LIST_C = "ticket acc list collection";

	/** collection ticket creator */
	public final static String TICKET_OWNER_C = "ticket owner collection";

	/** allowed collection ticket user */
	public final static String TICKET_USER_C = "ticket user collection";

	/** collection ticket user domain */
	public final static String TICKET_USER_DOMAIN_C = "ticket user domain collection";

	/** data ticket user domain */
	public final static String TICKET_USER_DOMAIN_D = "ticket user domain data";

	/** collection ticket creator domain */
	public final static String TICKET_OWNER_DOMAIN_C = "ticket owner domain collection";

	/** data ticket creator domain */
	public final static String TICKET_OWNER_DOMAIN_D = "ticket owner domain data";

	/** location of resource name */
	public final static String LOCATION_NAME = "location name";

	/**
	 * data has been changed compared to other copies currently used only for
	 * containered data
	 */
	public final static String IS_DIRTY = "is dirty";

	/** maximum size of container */
	public final static String CONTAINER_MAX_SIZE = "container max size";

	/** name of container */
	public final static String CONTAINER_NAME = "container name";

	/** classification of resource */
	public final static String RESOURCE_CLASS = "resource class";

	/** maximum size of data object allowed in resource not enforced by MCAT */
	public final static String MAX_OBJ_SIZE = "max object size";

	/** position of data in container */
	public final static String OFFSET = "offset";

	/** name of physical resource of container */
	public final static String CONTAINER_RESOURCE_NAME = "container resource name";

	/** class of physical resource of container */
	public final static String CONTAINER_RESOURCE_CLASS = "container resource class";

	/** current size of container */
	public final static String CONTAINER_SIZE = "container size";

	/** logical resource associated with a container */
	public final static String CONTAINER_LOG_RESOURCE_NAME = "container log resource name";

	/** domain of data creator */
	public final static String OWNER_DOMAIN = "owner domain";

	/** distinguished name of user used by authentication systems */
	public final static String USER_DISTINGUISHED_NAME = "user distinguished name";

	/** user authentication scheme in USER_DISTINGUISHED_NAME */
	public final static String USER_AUTHENTICATION_SCHEME = "user authentication scheme";

	/** location of SRB server */
	public final static String SERVER_LOCATION = "server location";

	/** net address of SRB server */
	public final static String SERVER_NETPREFIX = "server netprefix";

	/** container copy number */
	public final static String CONTAINER_REPLICATION_ENUM = "container replication enum";

	/** name of annotator */
	public final static String FILE_ANNOTATION_USERNAME = "file annotation username";

	/** domain of annotator */
	public final static String FILE_ANNOTATION_USERDOMAIN = "file annotation userdomain";

	/** annotation on data */
	public final static String FILE_ANNOTATION = "file annotation";

	/** time of annotation */
	public final static String FILE_ANNOTATION_TIMESTAMP = "file annotation timestamp";

	/** time stamp for comments on data */
	public final static String FILE_COMMENTS_TIMESTAMP = "file comments timestamp";

	/** some user-defined location for the annotation */
	public final static String FILE_ANNOTATION_POSITION = "file annotation position";

	/** access privilege on data */
	public final static String FILE_ACCESS_PRIVILEGE = "file access privilege";

	/** physical resource estimated latency max */
	public final static String RESOURCE_MAX_LATENCY = "resource max latency";

	/** physical resource estimated latency min */
	public final static String RESOURCE_MIN_LATENCY = "resource min latency";

	/** physical resource estimated bandwidth */
	public final static String RESOURCE_BANDWIDTH = "resource bandwidth";

	/** physical resource maximum concurrent requests */
	public final static String RESOURCE_MAX_CONCURRENCY = "resource max concurrency";

	/** number of hierarchies in the physical resource */
	public final static String RESOURCE_NUM_OF_HIERARCHIES = "resource num of hierarchies";

	/** number of striping of data in the physical resource */
	public final static String RESOURCE_NUM_OF_STRIPES = "resource num of stripes";

	/** capacity of the physical resource */
	public final static String RESOURCE_CAPACITY = "resource capacity";

	/** comments on the resource */
	public final static String RESOURCE_DESCRIPTION = "resource description";

	/** classifcation name for data different from FILE_TYPE_NAME */
	public final static String FILE_CLASS_NAME = "file class name";

	/** classification type */
	public final static String FILE_CLASS_TYPE = "file class type";

	/**
	 * type of user-inserted structured metadata for data in
	 * STRUCTURED_METADATA_FILE_NAME or INTERNAL_STRUCTURED_METADATA
	 */
	public final static String STRUCTURED_METADATA_TYPE = "structured metadata type";

	/**
	 * comments on the structured metadata in STRUCTURED_METADATA_FILE_NAME or
	 * INTERNAL_STRUCTURED_METADATA
	 */
	public final static String STRUCTURED_METADATA_COMMENTS = "structured metadata comments";

	/**
	 * data name of user-inserted structured metadata stored as another data
	 * object inside SRB see also STRUCTURED_METADATA_DIRECTORY_NAME
	 */
	public final static String STRUCTURED_METADATA_FILE_NAME = "structured metadata file name";

	/** user-inserted structured metadata stored as a string inside MCAT */
	public final static String INTERNAL_STRUCTURED_METADATA = "internal structured metadata";

	/** data name of index on data */
	public final static String INDEX_NAME_FOR_FILE = "index name for file";

	/** data name of index on data type */
	public final static String INDEX_NAME_FOR_DATATYPE = "index name for datatype";

	/** data name of index on collection */
	public final static String INDEX_NAME_FOR_DIRECTORY = "index name for directory";

	/** data name of method on data */
	public final static String METHOD_NAME_FOR_FILE = "method name for file";

	/** data name of method on data type */
	public final static String METHOD_NAME_FOR_DATATYPE = "method name for datatype";

	/** data name of method on collection */
	public final static String METHOD_NAME_FOR_DIRECTORY = "method name for directory";

	/** collection name of index on data in FILE_TYPE_NAME */
	public final static String IX_DIRECTORY_NAME_FOR_FILE = "ix directory name for file";

	/** collection name of index on data type in INDEX_NAME_FOR_DATATYPE */
	public final static String IX_DIRECTORY_NAME_FOR_DATATYPE = "ix directory name for datatype";

	/** collection name of index on collection in INDEX_NAME_FOR_DIRECTORY */
	public final static String IX_DIRECTORY_NAME_FOR_DIRECTORY = "ix directory name for directory";

	/** collection name of method on data in METHOD_NAME_FOR_FILE */
	public final static String METHOD_DIRECTORY_NAME_FOR_FILE = "method directory name for file";

	/** collection name of method on data type in METHOD_NAME_FOR_DATATYPE */
	public final static String METHOD_DIRECTORY_NAME_FOR_DATATYPE = "method directory name for datatype";

	/** collection name of method on collection in METHOD_NAME_FOR_DIRECTORY */
	public final static String METHOD_DIRECTORY_NAME_FOR_DIRECTORY = "method directory name for directory";

	/** index type for INDEX_NAME_FOR_FILE */
	public final static String IX_DATATYPE_FOR_FILE = "ix datatype for file";

	/** index type for INDEX_NAME_FOR_DATATYPE */
	public final static String IX_DATATYPE_FOR_DATATYPE = "ix datatype for datatype";

	/** index type for INDEX_NAME_FOR_DIRECTORY */
	public final static String IX_DATATYPE_FOR_DIRECTORY = "ix datatype for directory";

	/** method type for METHOD_NAME_FOR_FILE */
	public final static String METHOD_DATATYPE_FOR_FILE = "method datatype for file";

	/** method type for METHOD_NAME_FOR_DATATYPE */
	public final static String METHOD_DATATYPE_FOR_DATATYPE = "method datatype for datatype";

	/** method type for METHOD_NAME_FOR_DIRECTORY */
	public final static String METHOD_DATATYPE_FOR_DIRECTORY = "method datatype for directory";

	/**
	 * collection name of user-inserted structured metadata stored as another
	 * data object inside SRB see also STRUCTURED_METADATA_FILE_NAME
	 */
	public final static String STRUCTURED_METADATA_DIRECTORY_NAME = "structured metadata directory name";

	/** path name of index in INDEX_NAME_FOR_FILE */
	public final static String IX_LOCATION_FOR_FILE = "ix location for file";

	/** path name of index in INDEX_NAME_FOR_DATATYPE */
	public final static String IX_LOCATION_FOR_DATATYPE = "ix location for datatype";

	/** path name of index in INDEX_NAME_FOR_DIRECTORY */
	public final static String IX_LOCATION_FOR_DIRECTORY = "ix location for directory";

	// definable file
	public final static String DEFINABLE_METADATA_FOR_FILES = "definable metadata for files";

	/** metadata num */
	public final static String METADATA_NUM = "metadata number";

	/** user-defined string metadata */
	public final static String DEFINABLE_METADATA_FOR_FILES0 = "definable metadata file0";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES1 = "definable metadata file1";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES2 = "definable metadata file2";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES3 = "definable metadata file3";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES4 = "definable metadata file4";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES5 = "definable metadata file5";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES6 = "definable metadata file6";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES7 = "definable metadata file7";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES8 = "definable metadata file8";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES9 = "definable metadata file9";

	/** user-defined integer metadata */
	final static String INTEGER_DEFINABLE_METADATA0 = "integer definable metadata file0";
	/** user-defined integer metadata */
	final static String INTEGER_DEFINABLE_METADATA1 = "integer definable metadata file1";

	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES0_0 = DEFINABLE_METADATA_FOR_FILES
			+ "0_0";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES0_1 = DEFINABLE_METADATA_FOR_FILES
			+ "0_1";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES0_2 = DEFINABLE_METADATA_FOR_FILES
			+ "0_2";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES0_3 = DEFINABLE_METADATA_FOR_FILES
			+ "0_3";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES0_4 = DEFINABLE_METADATA_FOR_FILES
			+ "0_4";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES1_0 = DEFINABLE_METADATA_FOR_FILES
			+ "1_0";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES1_1 = DEFINABLE_METADATA_FOR_FILES
			+ "1_1";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES1_2 = DEFINABLE_METADATA_FOR_FILES
			+ "1_2";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES1_3 = DEFINABLE_METADATA_FOR_FILES
			+ "1_3";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_FILES1_4 = DEFINABLE_METADATA_FOR_FILES
			+ "1_4";
	// end definable

	/** email of data creator */
	public final static String OWNER_EMAIL = "owner email";

	/** audit user */
	public final static String AUDIT_USER = "audit user";

	/** audit user domain */
	public final static String AUDIT_USER_DOMAIN = "audit user domain";

	/** default container for collection */
	public final static String CONTAINER_FOR_DIRECTORY = "container for directory";

	// definable dir
	public final static String DEFINABLE_METADATA_FOR_DIRECTORIES = "definable metadata for directories";

	/** user-defined string metadata for collection */
	public final static String METADATA_NUM_DIRECTORY = "metadata number directory";

	/** user-defined string metadata */
	public final static String DEFINABLE_METADATA_FOR_DIRECTORIES0 = "definable metadata directory0";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES1 = "definable metadata directory1";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES2 = "definable metadata directory2";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES3 = "definable metadata directory3";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES4 = "definable metadata directory4";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES5 = "definable metadata directory5";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES6 = "definable metadata directory6";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES7 = "definable metadata directory7";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES8 = "definable metadata directory8";
	/** user-defined string metadata */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES9 = "definable metadata directory9";
	/** user-defined integer metadata */
	final static String INTEGER_DEFINABLE_METADATA_FOR_DIRECTORIES0 = "integer definable metadata directory0";
	/** user-defined integer metadata */
	final static String INTEGER_DEFINABLE_METADATA_FOR_DIRECTORIES1 = "integer definable metadata directory1";

	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES0_0 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "0_0";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES0_1 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "0_1";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES0_2 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "0_2";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES0_3 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "0_3";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES0_4 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "0_4";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES1_0 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "1_0";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES1_1 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "1_1";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES1_2 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "1_2";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES1_3 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "1_3";
	/** user-defined string metadata for collections */
	final static String DEFINABLE_METADATA_FOR_DIRECTORIES1_4 = DEFINABLE_METADATA_FOR_DIRECTORIES
			+ "1_4";

	// end definable dir

	/** MCAT's internal Id for Access Type */
	public final static String FILE_ACCESS_ID = "file access id";

	/** MCAT's internal Id for the User */
	final static String USER_ID = "user id";

	/** data last access time stamp */
	public final static String FILE_LAST_ACCESS_TIMESTAMP = "file last access timestamp";

	/** collection owner name */
	public final static String DIRECTORY_OWNER_NAME = DIRECTORY_OWNER;

	/** directory access id */
	public final static String COLL_ACCS_ID = "directory access id";

	/** directory access user id */
	public final static String COLL_ACCS_USER_ID = "directory access user id";

	/** data access id */
	public final static String DATA_ACCS_ID = FILE_ACCESS_DATA_ID;

	/** data access user id */
	public final static String DATA_ACCS_USER_ID = FILE_ACCESS_USER_ID;

	/** recursive flag for C_TICKET */
	public final static String TICKET_RECURSIVE = "c ticket recursive";

	/** tape number */
	public final static String TAPE_NUMBER = "tape number";

	/** tape owner */
	public final static String TAPE_OWNER = "tape owner";

	/** TAPE_TYPE_VAL */
	public final static String TAPE_TYPE_VAL = "tape type val";

	/** tape libinx */
	public final static String TAPE_LIBINX = "tape libinx";

	/** tape fullflag */
	public final static String TAPE_FULLFLAG = "tape fullflag";

	/** tape current file seqnum */
	public final static String TAPE_CURRENT_FILE_SEQNUM = "tape current file seqnum";

	/** tape current abs position */
	public final static String TAPE_CURRENT_ABS_POSITION = "tape current abs position";

	/** tape bytes written */
	public final static String TAPE_BYTES_WRITTEN = "tape bytes written";

	/** dataset version number */
	public final static String FILE_VER_NUM = "file ver num";

	/** dataset lock */
	public final static String FILE_LOCK_NUM = "file lock num";

	public final static String FILE_LOCK_DESCRIPTION = "file lock description";

	/** dataset lock owner name */
	public final static String FILE_LOCK_OWNER_NAME = "file lock owner name";

	/** dataset lock owner domain */
	public final static String FILE_LOCK_OWNER_DOMAIN = "file lock owner domain";

	/** collection owner domain */
	public final static String DIRECTORY_OWNER_DOMAIN = "directory owner domain";

	/** collection for ticket */
	public final static String TICKET_DIRECTORY_NAME = "ticket directory name";

	/** ticket collction timestamp */
	public final static String TICKET_DIRECTORY_CREATE_TIMESTAMP = "ticket directory create timestamp";

	/** ticket collction comments */
	public final static String TICKET_DIRECTORY_COMMENTS = "ticket directory comments";

	/** name of annotator */
	public final static String DIRECTORY_ANNOTATION_USERNAME = "directory annotation username";

	/** domain of annotator */
	public final static String DIRECTORY_ANNOTATION_USERDOMAIN = "directory annotation userdomain";

	/** annotation on data */
	public final static String DIRECTORY_ANNOTATION = "directory annotation";

	/** time of annotation */
	public final static String DIRECTORY_ANNOTATION_TIMESTAMP = "directory annotation timestamp";

	/** some user-defined type for the annotation */
	public final static String DIRECTORY_ANNOTATION_TYPE = "directory annotation type";

	/** internal collection id */
	public final static String FILE_GROUP_ID = "file group id";

	/** setting it more than 0 hides the data */
	public final static String FILE_HIDE = "file hide";

	/** flag to see whether the data needs to be audited */
	public final static String FILE_AUDITFLAG = "file auditflag";

	/** lock expory dtimestamp */
	public final static String FILE_LOCK_EXPIRY = "file lock expiry";

	/** data replica pinned from being moved if set greater than 0 */
	public final static String FILE_PIN_VAL = "file pin val";

	/** owner of the pin */
	public final static String FILE_PIN_OWNER_NAME = "file pin owner name";

	/** domain of the pin owner */
	public final static String FILE_PIN_OWNER_DOMAIN = "file pin owner domain";

	/** expiry timestamp for the pin */
	public final static String FILE_PIN_EXPIRY = "file pin expiry";

	/** expiry timestamp for the data replica itself */
	public final static String FILE_EXPIRY_DATE = "file expiry date";

	/** if data is compressed tell here how? */
	public final static String FILE_IS_COMPRESSED = "file is compressed";

	/** if data is encrypted tell here how? not the password */
	public final static String FILE_IS_ENCRYPTED = "file is encrypted";

	/** another expiry time_stamp for data as per george req */
	public final static String FILE_EXPIRE_DATE_2 = "file expire date 2";

	/** new audit action description */
	public final static String NEW_AUDIT_ACTION_DESCRIPTION = "new audit action description";

	/** new audit timestamp */
	public final static String NEW_AUDIT_TIMESTAMP = "new audit timestamp";

	/** new audit comments */
	public final static String NEW_AUDIT_COMMENTS = "new audit comments";

	/** definable metadata for resources */
	public final static String DEFINABLE_METADATA_FOR_RESOURCES = "definable metadata for resources";

	public final static String METADATA_NUM_RESOURCE = "metadata number for resources";

	public final static String DEFINABLE_METADATA_FOR_RESOURCES0 = "definable metadata resource0";
	final static String DEFINABLE_METADATA_FOR_RESOURCES1 = "definable metadata resource1";
	final static String DEFINABLE_METADATA_FOR_RESOURCES2 = "definable metadata resource2";
	final static String DEFINABLE_METADATA_FOR_RESOURCES3 = "definable metadata resource3";
	final static String DEFINABLE_METADATA_FOR_RESOURCES4 = "definable metadata resource4";
	final static String DEFINABLE_METADATA_FOR_RESOURCES5 = "definable metadata resource5";
	final static String DEFINABLE_METADATA_FOR_RESOURCES6 = "definable metadata resource6";
	final static String DEFINABLE_METADATA_FOR_RESOURCES7 = "definable metadata resource7";
	final static String DEFINABLE_METADATA_FOR_RESOURCES8 = "definable metadata resource8";
	final static String DEFINABLE_METADATA_FOR_RESOURCES9 = "definable metadata resource9";
	final static String DEFINABLE_INTEGER_METADATA_RESOURCE0 = "definable integer metadata resource0";

	/** definable resource metadata 0_0 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES0_0 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "0_0";
	/** definable resource metadata 0_1 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES0_1 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "0_1";
	/** definable resource metadata 0_2 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES0_2 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "0_2";
	/** definable resource metadata 0_3 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES0_3 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "0_3";
	/** definable resource metadata 1_1 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES1_0 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "1_0";
	/** definable resource metadata 1_1 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES1_1 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "1_1";
	/** definable resource metadata 1-2 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES1_2 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "1_2";
	/** definable resource metadata 1_3 */
	final static String DEFINABLE_METADATA_FOR_RESOURCES1_3 = DEFINABLE_METADATA_FOR_RESOURCES
			+ "1_3";

	// end resource

	/** compound resource name */
	public final static String COMPOUND_RESOURCE_NAME = "compound resource name";

	/** file seg num */
	public final static String FILE_SEG_NUM = "file seg num";

	/** int replication enum */
	public final static String INT_REPLICATION_ENUM = "int replication enum";

	/** int seg num */
	public final static String INT_SEG_NUM = "int seg num";

	/** int path name */
	public final static String INT_PATH_NAME = "int path name";

	/** int resource name */
	public final static String INT_RESOURCE_NAME = "int resource name";

	/** int resource address netprefix */
	public final static String INT_RESOURCE_ADDRESS_NETPREFIX = "int resource address netprefix";

	/** int resource type name */
	public final static String INT_RESOURCE_TYPE_NAME = "int resource type name";

	/** int is dirty */
	public final static String INT_IS_DIRTY = "int is dirty";

	/** int resource class */
	public final static String INT_RESOURCE_CLASS = "int resource class";

	/** int offset */
	public final static String INT_OFFSET = "int offset";

	/** int size */
	public final static String INT_SIZE = "int size";

	/** comp obj user name */
	public final static String COMP_OBJ_USER_NAME = "comp obj user name";

	/** comp obj user domain */
	public final static String COMP_OBJ_USER_DOMAIN = "comp obj user domain";

	/** int path name for repls */
	public final static String INT_PATH_NAME_FOR_REPL = "int path name for repl";

	/** ACL_INHERITANCE_BIT */
	public final static String DIRECTORY_LINK_NUMBER = "directory link number";
	public final static String ACL_INHERITANCE_BIT = DIRECTORY_LINK_NUMBER;

	/*** fake ****/
	public final static String REAL_DIRECTORY_NAME = "real directory name";

	/*** fake ****/
	public final static String FILE_CONTAINER_NAME = "file container name";

	/*** fake ****/
	public final static String DIRECTORY_CONTAINER_NAME = "directory container name";

	/** definable user metadata */
	public final static String DEFINABLE_METADATA_FOR_USERS = "definable metadata for users";

	public final static String METADATA_NUM_USER = "metadata number for users";

	/** definable user metadata string 0 */
	public final static String DEFINABLE_METADATA_FOR_USERS0 = "definable metadata user0";
	/** definable user metadata string 1 */
	final static String DEFINABLE_METADATA_FOR_USERS1 = "definable metadata user1";
	/** definable user metadata string 2 */
	final static String DEFINABLE_METADATA_FOR_USERS2 = "definable metadata user2";
	/** definable user metadata string 3 */
	final static String DEFINABLE_METADATA_FOR_USERS3 = "definable metadata user3";
	/** definable user metadata string 4 */
	final static String DEFINABLE_METADATA_FOR_USERS4 = "definable metadata user4";
	/** definable user metadata string 5 */
	final static String DEFINABLE_METADATA_FOR_USERS5 = "definable metadata user5";
	/** definable user metadata string 6 */
	final static String DEFINABLE_METADATA_FOR_USERS6 = "definable metadata user6";
	/** definable user metadata string 7 */
	final static String DEFINABLE_METADATA_FOR_USERS7 = "definable metadata user7";
	/** definable user metadata string 8 */
	final static String DEFINABLE_METADATA_FOR_USERS8 = "definable metadata user8";
	/** definable user metadata string 9 */
	final static String DEFINABLE_METADATA_FOR_USERS9 = "definable metadata user9";

	/** definable user metadata */
	final static String UDIMD_USER0 = "definable user integer metadata 0";

	/** definable user metadata 0_0 */
	final static String DEFINABLE_METADATA_FOR_USERS0_0 = DEFINABLE_METADATA_FOR_USERS
			+ "0_0";
	/** definable user metadata 0_1 */
	final static String DEFINABLE_METADATA_FOR_USERS0_1 = DEFINABLE_METADATA_FOR_USERS
			+ "0_1";
	/** definable user metadata 0_2 */
	final static String DEFINABLE_METADATA_FOR_USERS0_2 = DEFINABLE_METADATA_FOR_USERS
			+ "0_2";
	/** definable user metadata 0_3 */
	final static String DEFINABLE_METADATA_FOR_USERS0_3 = DEFINABLE_METADATA_FOR_USERS
			+ "0_3";
	/** definable user metadata 1_0 */
	final static String DEFINABLE_METADATA_FOR_USERS1_0 = DEFINABLE_METADATA_FOR_USERS
			+ "1_0";
	/** definable user metadata 1_1 */
	final static String DEFINABLE_METADATA_FOR_USERS1_1 = DEFINABLE_METADATA_FOR_USERS
			+ "1_1";
	/** definable user metadata 1_2 */
	final static String DEFINABLE_METADATA_FOR_USERS1_2 = DEFINABLE_METADATA_FOR_USERS
			+ "1_2";
	/** definable user metadata 1_3 */
	final static String DEFINABLE_METADATA_FOR_USERS1_3 = DEFINABLE_METADATA_FOR_USERS
			+ "1_3";
	// end definable

	/** definable function metadata 01 */
	final static String USERDEFFUNC01 = "definable function metadata 01";

	/** definable function metadata 02 */
	final static String USERDEFFUNC02 = "definable function metadata 02";

	/** definable function metadata 03 */
	final static String USERDEFFUNC03 = "definable function metadata 03";

	/** definable function metadata 04 */
	final static String USERDEFFUNC04 = "definable function metadata 04";

	/** definable function metadata 05 */
	final static String USERDEFFUNC05 = "definable function metadata 05";

	/** set to 1 for local zone, 0 otherwise */
	public final static String ZONE_LOCALITY = "zone locality";

	/** address where (remote) MCAT is */
	public final static String ZONE_NETPREFIX = "zone net prefix";

	/** port num to reach (remote) MCAT */
	public final static String ZONE_PORT_NUM = "zone port number";

	/** admin's auth scheme of rem MCAT */
	public final static String ZONE_ADMIN_AUTH_SCHEME_NAME = "zone admin's auth scheme";

	/** DN str of rem MCAT */
	public final static String ZONE_ADMIN_DISTIN_NAME = "zone admin distinguish name";

	/** 1 for a valid zone */
	public final static String ZONE_STATUS = "zone status";

	/** when the zone was created */
	public final static String ZONE_CREATE_DATE = "zone creation date";

	/** when the zone was modified */
	public final static String ZONE_MODIFY_DATE = "zone modification date";

	/** any comments about the zone */
	public final static String ZONE_COMMENTS = "zone comments";

	/** who is in charge of the zone */
	public final static String ZONE_CONTACT = "zone contact";

	/** zone admin's username */
	public final static String ZONE_ADMIN_NAME = "zone admin's username";

	/** zone admin's user domain name */
	public final static String ZONE_ADMIN_DOMAIN_NAME = "zone admin's user domain name";

	/** resource access privilege */
	public final static String RESOURCE_ACCESS_PRIVILEGE = "resource access privilege";
	public final static String RSRC_ACCESS_PRIVILEGE = RESOURCE_ACCESS_PRIVILEGE;

	/** resource access constraint name */
	public final static String RESOURCE_ACCESS_CONSTRAINT = "resource access constraint name";
	public final static String RSRC_ACCESS_CONSTRAINT = RESOURCE_ACCESS_CONSTRAINT;

	/** change MAX_USER_INTEGER_METADATA */
	public final static String UDIMD_USER1 = "change MAX_USER_INTEGER_METADATA";

	/** change MAX_RSRC_INTEGER_METADATA */
	public final static String UDIMD_RESOURCE1 = "change MAX_RSRC_INTEGER_METADATA";
	public final static String UDIMD_RSRC1 = UDIMD_RESOURCE1;

	/** group user info modify date */
	public final static String USER_GROUP_MODIFY_DATE = "group user info modify date";

	/** group user zone name */
	public final static String USER_GROUP_ZONE_NAME = "group user zone name";

	/** group user domain name */
	public final static String USER_GROUP_DOMAIN_DESC = "group user domain name";

	/** user-def metadata for data mod */
	public final static String DATA_UDEF_MDATA_MODIFY_DATE = "user-def metadata for data mod";

	/** user-def metadata for coll mod */
	public final static String COLL_UDEF_MDATA_MODIFY_DATE = "user-def metadata for coll mod";

	/** user-def metadata for user mod */
	public final static String USER_UDEF_MDATA_MODIFY_DATE = "user-def metadata for user mod";

	/** user-def metadata for resource mod */
	public final static String RESOURCE_UDEF_MDATA_MODIFY_DATE = "user-def metadata for resource mod";
	public final static String RSRC_UDEF_MDATA_MODIFY_DATE = RESOURCE_UDEF_MDATA_MODIFY_DATE;

	/** mime string for data type */
	public final static String DATA_TYPE_MIME_STRING = "mime string for data type";

	/** extender list for data type */
	public final static String DATA_TYPE_EXTENDERS = "extender list for data type";

	/** comments on log resource */
	public final static String RSRC_COMMENTS = RESOURCE_COMMENTS;

	/** log resource creation timestamp */
	public final static String RSRC_CREATE_DATE = RESOURCE_CREATE_DATE;

	/** log resource modify timestamp */
	public final static String RSRC_MODIFY_DATE = RESOURCE_MODIFY_DATE;

	/** log resource max obj size */
	public final static String RSRC_MAX_OBJ_SIZE = "log resource max obj size";

	/** owner of the log resource */
	public final static String RESOURCE_OWNER_NAME = "owner of the log resource";
	public final static String RSRC_OWNER_NAME = RESOURCE_OWNER_NAME;

	/** domain of the log resource */
	public final static String RESOURCE_OWNER_DOMAIN = "domain of the log resource";
	public final static String RSRC_OWNER_DOMAIN = "domain of the log resource";

	/** max latency of resource in millisec */
	public final static String RSRC_MLSEC_LATENCY_MAX = "max latency of resource in millisec";

	/** min latency of resource in millisec */
	public final static String RSRC_MLSEC_LATENCY_MIN = "min latency of resource in millisec";

	/** bandwidth of resource in mega bps */
	public final static String RSRC_MBPS_BANDWIDTH = "bandwidth of resource in mega bps";

	/** max concurrency allowed in resource */
	public final static String RSRC_CONCURRENCY_MAX = "max concurrency allowed in resource";

	/** num of staging levels in resource */
	public final static String RSRC_NUM_OF_HIERARCHIES = "num of staging levels in resource";

	/** num of striping done in resource */
	public final static String RSRC_NUM_OF_STRIPES = "num of striping done in resource";

	/** capacity of resource in megabytes */
	public final static String RSRC_MEGAB_CAPACITY = "capacity of resource in megabytes";

	/** user name used for resource access */
	public final static String RSRC_ACCS_USER_NAME = "user name used for resource access";

	/** user domn used for resource access */
	public final static String RSRC_ACCS_USER_DOMAIN = "user domn used for resource access";

	/** user zone used for resource access */
	public final static String RSRC_ACCS_USER_ZONE = "user zone used for resource access";

	/** zone for resource owner */
	public final static String RSRC_OWNER_ZONE = "zone for resource owner";

	/** zone for data owner */
	public final static String DATA_OWNER_ZONE = "zone for data owner";

	/** zone for data ticket owner */
	public final static String TICKET_OWNER_ZONE_D = "zone for data ticket owner";

	/** zone for data ticket user */
	public final static String TICKET_USER_ZONE_D = "zone for data ticket user";

	/** zone for coll ticket owner */
	public final static String TICKET_OWNER_ZONE_C = "zone for coll ticket owner";

	/** zone for coll ticket user */
	public final static String TICKET_USER_ZONE_C = "zone for coll ticket user";

	/** zone for data annotator */
	public final static String DATA_ANNOTATION_USER_ZONE = "zone for data annotator";

	/** zone for audited suer */
	public final static String AUDIT_USER_ZONE = "zone for audited suer";

	/** zone for collection owner */
	public final static String COLL_OWNER_ZONE = "zone for collection owner";

	/** zone for data lock owner */
	public final static String DATA_LOCK_OWNER_ZONE = "zone for data lock owner";

	/** zone for coll annotator */
	public final static String COLL_ANNOTATION_USER_ZONE = "zone for coll annotator";

	/** zone for data pin owner */
	public final static String DATA_PIN_OWNER_ZONE = "zone for data pin owner ";

	/** zone for composite obj owner */
	public final static String COMP_OBJ_USER_ZONE = "zone for composite obj owner";

	/** parent of a location */
	public final static String PARENT_SERVER_LOCATION = "parent of a location";

	/** access id number for resource access */
	public final static String RSRC_ACCESS_ID = "access id number for resource access";

	/** parent of a data type */
	public final static String PARENT_DATA_TYPE = "parent of a data type";

	/** location desc for zone */
	public final static String ZONE_LOCN_DESC = "location desc for zone";

	/** parent of a domain */
	public final static String PARENT_DOMAIN_DESC = "parent of a domain";

	/** parent of a user type */
	public final static String PARENT_USER_TYPE = "parent of a user type";

	/** parent of a resource type */
	public final static String PARENT_RSRC_TYPE = "parent of a resource type ";

	/** group user who has access to resource */
	public final static String RSRC_ACCS_GRPUSER_NAME = "group user who has access to resource";

	/** resource group user domain */
	public final static String RSRC_ACCS_GRPUSER_DOMAIN = "resource group user domain";

	/** resource group user zone */
	public final static String RSRC_ACCS_GRPUSER_ZONE = "resource group user zone";

	/** collection modify_timestamp */
	public final static String COLL_MODIFY_TIMESTAMP = "collection modify_timestamp";

	/** group user who has access to data */
	public final static String DATA_ACCS_GRPUSER_NAME = "group user who has access to data";

	/** data group user domain */
	public final static String DATA_ACCS_GRPUSER_DOMAIN = "data group user domain";

	/** data group user zone */
	public final static String DATA_ACCS_GRPUSER_ZONE = "data group user zone";

	/** group user who has access to coll */
	public final static String COLL_ACCS_GRPUSER_NAME = "group user who has access to coll";

	/** coll group user domain */
	public final static String COLL_ACCS_GRPUSER_DOMAIN = "coll group user domain";

	/** coll group user zone */
	public final static String COLL_ACCS_GRPUSER_ZONE = "coll group user zone";

	/** coll name for finding coll access */
	public final static String COLL_ACCS_COLLECTION_NAME = "coll name for finding coll access";

	/** accs cons for finding coll access */
	public final static String COLL_ACCS_ACCESS_CONSTRAINT = "accs cons for finding coll access";

	/** data type for container associated with collection */
	public final static String DATA_TYPE_FOR_CONTAINER_FOR_COLLECTION = "data type for container associated with collection";

	/** DATA_UDEF_MDATA_CREATE_DATE */
	public final static String DATA_UDEF_MDATA_CREATE_DATE = "DATA_UDEF_MDATA_CREATE_DATE";

	/** COLL_UDEF_MDATA_CREATE_DATE */
	public final static String COLL_UDEF_MDATA_CREATE_DATE = "COLL_UDEF_MDATA_CREATE_DATE";

	/** RSRC_UDEF_MDATA_CREATE_DATE */
	public final static String RSRC_UDEF_MDATA_CREATE_DATE = "RSRC_UDEF_MDATA_CREATE_DATE";

	/** USER_UDEF_MDATA_CREATE_DATE */
	public final static String USER_UDEF_MDATA_CREATE_DATE = "USER_UDEF_MDATA_CREATE_DATE";

	/**
	 * container id - deprecated, not for external use. Use CONTAINER_NAME
	 * instead.
	 * 
	 * @deprecate Use CONTAINER_NAME
	 */
	public final static String CONTAINER_ID = "container id";

	/** guid */
	public final static String GUID = "guid";

	/** guid flag */
	public final static String GUID_FLAG = "guid flag";

	/** guid timestamp */
	public final static String GUID_TIME_STAMP = "guid timestamp";

	// SRB3.1
	/** data_id of deleted data object */
	public final static String DELETE_FILE_ID = "delete " + FILE_ID;

	/** repl_enum of deleted data object */
	public final static String DELETE_FILE_REPLICATION_ENUM = "delete "
			+ FILE_REPLICATION_ENUM;

	/** data_name of deleted data object */
	public final static String DELETE_FILE_NAME = "delete " + FILE_NAME;

	/** timestamp of deleted data object */
	public final static String DELETE_MODIFICATION_DATE = "delete "
			+ MODIFICATION_DATE;

	/** collection_name of deleted data object */
	public final static String DELETE_DIRECTORY_NAME = "delete "
			+ DIRECTORY_NAME;
	// 391
	/** collection_id of deleted data object */
	public final static String DEL_COLLECTION_ID = "collection_id of deleted data object";
	// 392
	/** comments on deleted data object */
	public final static String DEL_DATA_COMMENT = "comments on deleted data object";

	/** free space */
	public final static String FREE_SPACE = "free space";

	/** fs timestamp */
	public final static String FS_TIMESTAMP = "fs timestamp";

	/** extensible schema name */
	public final static String EXTENSIBLE_SCHEMA_NAME = "extensible schema name";

	/** extensible table name */
	public final static String EXTENSIBLE_TABLE_NAME = "extensible table name";

	/** extensible attribute name */
	public final static String EXTENSIBLE_ATTR_NAME = "extensible attribute name";

	/** extensible attribute outside name */
	public final static String EXTENSIBLE_ATTR_OUTSIDE_NAME = "extensible attribute outside name";

	/** extensible attribute comments */
	public final static String EXTENSIBLE_ATTR_COMMENTS = "extensible attribute comments";

	/** resource status */
	public final static String RESOURCE_STATUS = "resource status";

	/*
	 * From srb C client, srbC_mdas_externs.h
	 * 
	 * The following definitions are used in the "retractionType" argument of
	 * functions that modify the metadata in MDAS catalog. These functions are:
	 * modify_dataset_info, modify_collection_info, modify_user_info and
	 * modify_resource_info. The definitions of those functions can be found in
	 * mdasPrototypes.h
	 */
	// I'd rather these were private but SRBAdmin is using them
	public final static int D_DELETE_ONE = 1;
	public final static int D_DELETE_DOMN = 2;
	public final static int D_INSERT_DOMN = 3;
	public final static int D_CHANGE_SIZE = 4;
	public final static int D_CHANGE_TYPE = 5;
	public final static int D_CHANGE_GROUP = 6;
	public final static int D_CHANGE_SCHEMA = 7;
	public final static int D_INSERT_ACCS = 8;
	public final static int D_DELETE_ACCS = 9;
	public final static int D_DELETE_ALIAS = 10;
	public final static int D_INSERT_ALIAS = 11;
	public final static int D_DELETE_COMMENTS = 12;
	public final static int D_INSERT_COMMENTS = 13;
	public final static int D_INSERT_AUDIT = 14;
	public final static int D_CHANGE_OWNER = 15;
	public final static int U_CHANGE_PASSWORD = 16;
	public final static int U_DELETE = 17;
	public final static int U_DELETE_DOMN = 18;
	public final static int U_INSERT_DOMN = 19;
	public final static int U_DELETE_GROUP = 20;
	public final static int U_INSERT_GROUP = 21;
	public final static int U_CHANGE_TYPE = 22;
	public final static int U_DELETE_ALIAS = 23;
	public final static int U_INSERT_ALIAS = 24;
	public final static int U_CHANGE_VERKEY = 25;
	public final static int U_CHANGE_ENCKEY = 26;
	public final static int D_INSERT_COLL_ACCS = 27;
	public final static int D_DELETE_COLL_ACCS = 28;
	public final static int D_CHANGE_DNAME = 29;
	public final static int D_CHANGE_COLL_NAME = 30;
	public final static int D_DELETE_COLL = 31;
	public final static int D_UPDATE_COMMENTS = 32;
	public final static int D_APPEND_COMMENTS = 33;
	public final static int SU_CHANGE_PASSWORD = 34;
	public final static int D_CHANGE_OFFSET = 35;
	public final static int D_CHANGE_DIRTY = 36;
	public final static int U_DELETE_AUTH_MAP = 37;
	public final static int U_INSERT_AUTH_MAP = 38;
	public final static int D_DELETE_ANNOTATIONS = 39;
	public final static int D_INSERT_ANNOTATIONS = 40;
	public final static int D_UPDATE_ANNOTATIONS = 41;
	public final static int D_CHANGE_MODIFY_TIMESTAMP = 42;
	public final static int D_DELETE_CLASS = 43;
	public final static int D_INSERT_CLASS = 44;
	public final static int D_DELETE_INDEX_FOR_DATA = 45;
	public final static int D_INSERT_INDEX_FOR_DATA = 46;
	public final static int D_DELETE_INDEX_FOR_DATATYPE = 47;
	public final static int D_INSERT_INDEX_FOR_DATATYPE = 48;
	public final static int D_DELETE_INDEX_FOR_COLLECTION = 49;
	public final static int D_INSERT_INDEX_FOR_COLLECTION = 50;
	public final static int D_DELETE_METHOD_FOR_DATA = 51;
	public final static int D_INSERT_METHOD_FOR_DATA = 52;
	public final static int D_DELETE_METHOD_FOR_DATATYPE = 53;
	public final static int D_INSERT_METHOD_FOR_DATATYPE = 54;
	public final static int D_DELETE_METHOD_FOR_COLLECTION = 55;
	public final static int D_INSERT_METHOD_FOR_COLLECTION = 56;
	public final static int D_DELETE_STRUCT_METADATA = 57;
	public final static int D_INSERT_EXTERNAL_STRUCT_METADATA = 58;
	public final static int D_INSERT_INTERNAL_STRUCT_METADATA = 59;
	public final static int D_CHANGE_USER_DEFINED_STRING_META_DATA = 60;
	public final static int D_CHANGE_USER_DEFINED_INTEGER_META_DATA = 61;
	public final static int D_INSERT_USER_DEFINED_STRING_META_DATA = 62;
	public final static int D_DELETE_USER_DEFINED_STRING_META_DATA = 63;
	public final static int D_INSERT_USER_DEFINED_INTEGER_META_DATA = 64;
	public final static int D_DELETE_USER_DEFINED_INTEGER_META_DATA = 65;
	public final static int C_INSERT_CONTAINER_FOR_COLLECTION = 66;
	public final static int C_DELETE_CONTAINER_FOR_COLLECTION = 67;
	public final static int C_CHANGE_USER_DEFINED_COLL_STRING_META_DATA = 68;
	public final static int C_CHANGE_USER_DEFINED_COLL_INTEGER_META_DATA = 69;
	public final static int C_INSERT_USER_DEFINED_COLL_STRING_META_DATA = 70;
	public final static int C_INSERT_USER_DEFINED_COLL_INTEGER_META_DATA = 71;
	public final static int C_DELETE_USER_DEFINED_COLL_STRING_META_DATA = 72;
	public final static int C_DELETE_USER_DEFINED_COLL_INTEGER_META_DATA = 73;
	public final static int R_DELETE_ACCS = 74;
	public final static int R_INSERT_ACCS = 75;
	public final static int D_CHANGE_LAST_ACCS_TIME = 76;
	public final static int D_DELETE_ATTR = 77;
	public final static int D_INSERT_ATTR = 78;
	public final static int D_MODIFY_ATTR = 79;
	public final static int D_DELETE_MULTI_ATTR = 80;
	public final static int D_INSERT_MULTI_ATTR = 81;
	public final static int D_MODIFY_MULTI_ATTR = 82;
	public final static int D_INSERT_LINK = 83;
	public final static int D_UPDATE_COLL_COMMENTS = 84;
	public final static int D_INSERT_LOCK = 85;
	public final static int D_UPGRADE_LOCK = 86;
	public final static int D_DELETE_LOCK = 87;
	public final static int D_CHANGE_VERSION = 88;
	public final static int D_DELETE_ALL_FROM_CONTAINER = 89;
	public final static int D_EXTRACT_TEMPLATE_METADATA = 90;
	public final static int C_COPY_META_DATA_FROM_COLL_TO_COLL = 91;
	public final static int D_COPY_META_DATA_FROM_COLL_TO_DATA = 92;
	public final static int C_COPY_META_DATA_FROM_DATA_TO_COLL = 93;
	public final static int D_COPY_META_DATA_FROM_DATA_TO_DATA = 94;
	public final static int D_COPY_ANNOTATIONS_FROM_DATA_TO_DATA = 95;
	public final static int D_READ = 96;
	public final static int D_WRITE = 97;
	public final static int D_CREATE = 98;
	public final static int D_REPLICATE = 99;
	public final static int D_MOVE = 100;
	public final static int C_CREATE = 101;
	public final static int D_TICKET = 102;
	public final static int C_TICKET = 103;
	public final static int D_LINK = 104;
	public final static int D_BULKINGEST = 105;
	public final static int D_DELETECONT = 106;
	public final static int D_AUDIT_SUCCESS = 107;
	public final static int D_AUDIT_FAILURE = 108;
	public final static int D_CONTAINER = 109;
	public final static int C_DELETE_ANNOTATIONS = 110;
	public final static int C_INSERT_ANNOTATIONS = 111;
	public final static int C_UPDATE_ANNOTATIONS = 112;
	public final static int C_COPY_ANNOTATIONS_FROM_COLL_TO_COLL = 113;
	public final static int D_COMPOUND_CREATE = 114;
	public final static int D_DELETE_COMPOUND = 115;
	public final static int D_DELETE_SUBCOMPOUND = 116;
	public final static int D_COMPOUND_MODIFY_CMPD_PATH_NAME = 117;
	public final static int D_COMPOUND_MODIFY_MODIFY_TIMESTAMP = 118;
	public final static int D_COMPOUND_MODIFY_LAST_ACCS_TIME = 119;
	public final static int D_COMPOUND_MODIFY_SIZE = 120;
	public final static int D_COMPOUND_MODIFY_OFFSET = 121;
	public final static int D_COMPOUND_MODIFY_IS_DIRTY = 122;
	public final static int D_COMPOUND_SET_MODIFY_TIMESTAMP = 123;
	public final static int D_COMPOUND_SET_LAST_ACCS_TIME = 124;
	public final static int D_COMPOUND_MODIFY_SIZE_AND_DIRTY = 125;
	public final static int D_CHANGE_DPATH = 126;
	public final static int D_INSERT_INCOLL_ACCS = 127;
	public final static int D_INSERT_INCOLL_ACCS_RECUR = 128;
	public final static int D_DELETE_INCOLL_ACCS = 129;
	public final static int D_DELETE_INCOLL_ACCS_RECUR = 130;
	public final static int C_INSERT_LINK = 131;
	public final static int D_CHANGE_REPLNUM_VERNUM = 132;
	public final static int T_INSERT_TAPE_INFO = 133;
	public final static int T_UPDATE_TAPE_INFO = 134;
	public final static int T_UPDATE_TAPE_INFO_2 = 135;
	public final static int T_DELETE_TAPE_INFO = 136;
	public final static int D_INSERT_DCHECKSUM = 137;
	public final static int D_INSERT_DHIDE = 138;
	public final static int D_INSERT_AUDITFLAG = 139;
	public final static int D_UPDATE_PIN = 140;
	public final static int D_UPDATE_DEXPIRE_DATE = 141;
	public final static int D_UPDATE_DEXPIRE_DATE_2 = 142;
	public final static int D_UPDATE_DCOMPRESSED = 143;
	public final static int D_UPDATE_DENCRYPTED = 144;
	public final static int D_COPY_META_DATA_FROM_CONTAINER_TO_NEW_CONTAINER = 145;
	public final static int D_UPDATE_CONTAINER_FOR_COLLECTION = 146;
	public final static int D_BULK_INSERT_UDEF_META_DATA_FOR_DATA = 147;
	public final static int C_BULK_INSERT_UDEF_META_DATA_FOR_COLL = 148;
	public final static int U_BULK_INSERT_UDEF_META_DATA_FOR_USER = 149;
	public final static int R_BULK_INSERT_UDEF_META_DATA_FOR_RSRC = 150;
	public final static int Z_INSERT_NEW_LOCAL_ZONE = 151;
	public final static int Z_INSERT_NEW_ALIEN_ZONE = 152;
	public final static int Z_MODIFY_ZONE_INFO = 153;
	public final static int Z_MODIFY_ZONE_FOR_USER = 154;
	public final static int Z_CHANGE_ZONE_NAME = 155;
	public final static int Z_MODIFY_ZONE_LOCAL_FLAG = 156;
	public final static int Z_MODIFY_ZONE_STATUS = 157;
	public final static int C_DELETE_MULTI_ATTR = 158;
	public final static int C_INSERT_MULTI_ATTR = 159;
	public final static int C_MODIFY_MULTI_ATTR = 160;
	public final static int C_DELETE_ATTR = 161;
	public final static int C_INSERT_ATTR = 162;
	public final static int C_MODIFY_ATTR = 163;
	public final static int D_BULK_INSERT_UDEF_META_DATA_FOR_MANY_DATA = 164;
	public final static int R_CHANGE_OWNER = 165;
	public final static int C_CHANGE_MODIFY_TIMESTAMP = 166;
	public final static int C_CHANGE_COLL_OWNER = 167;
	public final static int D_INSERT_GUID = 168;
	public final static int D_DELETE_GUID = 169;
	public final static int D_DELETE_FROM_EXTMD_TABLE = 170;
	public final static int D_INSERT_INTO_EXTMD_TABLE = 171;
	public final static int D_MODIFY_IN_EXTMD_TABLE = 172;
	public final static int D_MOVE_DATA_TO_NEWCOLL = 173;
	public final static int D_MOVE_DATA_TO_TRASH = 174;
	public final static int C_MOVE_COLL_TO_NEWCOLL = 175;
	public final static int C_MOVE_COLL_TO_TRASH = 176;
	public final static int D_CHANGE_INCOLL_OWNER = 177;
	public final static int D_CHANGE_INCOLL_OWNER_RECUR = 178;
	public final static int R_CHANGE_USER_DEFINED_STRING_META_DATA = 179;
	public final static int R_INSERT_USER_DEFINED_STRING_META_DATA = 180;
	public final static int R_DELETE_USER_DEFINED_STRING_META_DATA = 181;
	public final static int U_INSERT_USER_DEFINED_STRING_META_DATA = 182;
	public final static int U_DELETE_USER_DEFINED_STRING_META_DATA = 183;
	public final static int U_CHANGE_USER_DEFINED_STRING_META_DATA = 184;
	public final static int R_ADJUST_LATENCY_MAX_IN_MILLISEC = 185;
	public final static int R_ADJUST_LATENCY_MIN_IN_MILLISEC = 186;
	public final static int R_ADJUST_BANDWIDTH_IN_MBITSPS = 187;
	public final static int R_ADJUST_MAXIMUM_CONCURRENCY = 188;
	public final static int R_ADJUST_NUM_OF_STRIPES = 189;
	public final static int R_ADJUST_NUM_OF_HIERARCHIES = 190;
	public final static int R_ADJUST_CAPACITY_IN_GIGABYTES = 191;
	public final static int R_ADJUST_DESCRIPTION = 192;
	public final static int R_INSERT_RSRC_USAGE_TOTAL_BY_DATE = 193;
	public final static int R_INSERT_RSRC_USAGE_AND_QUOTA = 194;
	public final static int R_DELETE_PHY_RSRC_FROM_LOG_RSRC = 195;
	public final static int D_DELETE_USER_DEFINED_ATTR_VAL_META_DATA = 196;
	public final static int R_DELETE_USER_DEFINED_ATTR_VAL_META_DATA = 197;
	public final static int U_DELETE_USER_DEFINED_ATTR_VAL_META_DATA = 198;
	public final static int C_DELETE_USER_DEFINED_ATTR_VAL_META_DATA = 199;
	public final static int R_INSERT_PHY_RSRC_INTO_LOG_RSRC = 200;
	public final static int R_INSERT_FREE_SPACE = 201;
	public final static int R_ADJUST_RSRC_COMMENT = 202;
	public final static int U_CHANGE_INFO = 203;
	public final static int U_ADD_GROUP_OWNER = 204;
	public final static int U_REMOVE_GROUP_OWNER = 205;
	public final static int U_UPDATE_EMAIL = 206;
	public final static int U_UPDATE_PHONE = 207;
	public final static int U_UPDATE_ADDRESS = 208;
	public final static int BULK_PHY_MOVE = 209;
	public final static int BULK_PHY_MOVE_INTO_CONTAINER = 210;
	public final static int BULK_REGISTER_REPLICATE = 211;
	public final static int C_CHANGE_ACL_INHERITANCE_BIT = 212;
	public final static int C_CHANGE_ACL_INHERITANCE_BIT_RECUR = 213;
	public final static int R_RENAME = 214;
	public final static int R_ADJUST_LOCK = 215;
	public final static int R_CHANGE_LOCATION = 216;

	/* curator actions should have an equivalent in non-curator action!!! */
	public final static int CURATOR_ACTION_TYPE_MIN = 3000;
	public final static int D_INSERT_ACCS_BY_CURATOR = 3008;
	public final static int D_DELETE_ACCS_BY_CURATOR = 3009;
	public final static int D_CHANGE_OWNER_BY_CURATOR = 3015;
	public final static int D_INSERT_COLL_ACCS_BY_CURATOR = 3027;
	public final static int D_DELETE_COLL_ACCS_BY_CURATOR = 3028;
	public final static int D_INSERT_INCOLL_ACCS_BY_CURATOR = 3127;
	public final static int D_INSERT_INCOLL_ACCS_RECUR_BY_CURATOR = 3128;
	public final static int D_DELETE_INCOLL_ACCS_BY_CURATOR = 3129;
	public final static int D_DELETE_INCOLL_ACCS_RECUR_BY_CURATOR = 3130;
	public final static int C_CHANGE_COLL_OWNER_BY_CURATOR = 3167;
	public final static int D_CHANGE_INCOLL_OWNER_BY_CURATOR = 3177;
	public final static int D_CHANGE_INCOLL_OWNER_RECUR_BY_CURATOR = 3178;
	public final static int C_CHANGE_ACL_INHERITANCE_BIT_BY_CURATOR = 3212;
	public final static int C_CHANGE_ACL_INHERITANCE_BIT_RECUR_BY_CURATOR = 3213;
	public final static int CURATOR_ACTION_TYPE_MAX = 4000;

	public final static int SYS_ACTION_TYPE_MIN = 1000;
	public final static int D_SU_INSERT_ACCS = 1001;
	public final static int D_SU_DELETE_ACCS = 1002;
	public final static int C_SU_INSERT_COLL_ACCS = 1003;
	public final static int C_SU_DELETE_COLL_ACCS = 1004;
	public final static int C_SU_CHANGE_COLL_NAME = 1005;
	public final static int D_SU_DELETE_TRASH_ONE = 1006;
	public final static int C_SU_DELETE_TRASH_COLL = 1007;
	public final static int R_ENFORCE_QUOTAS = 1008;
	public final static int R_CHANGE_RESOURCE_STATUS = 1009;
	public final static int R_DELETE_PHY_RSRC_FROM_CMPD_RSRC = 1010;
	public final static int R_INSERT_PHY_RSRC_INTO_CMPD_RSRC = 1011;

	final static int SYS_ACTION_TYPE_MAX = 2000;

	/**
	 * The tableName, attributeNames, and catalogNames arrays are a consequence
	 * of the SRB return value. A tableName and attributeName will be returned
	 * by the SRB for every catalogName. No single tableName or attributeName is
	 * unique, so the combination of the two is used to find the catalogName.
	 * The catalogName corresponds to one of the public static final metadata
	 * names, e.g. FILE_NAME, COMPOUND_RESOURCE_NAME, USER_EMAIL.
	 */
	private static String catalogNames[] = {
			FILE_ID,
			FILE_REPLICATION_ENUM,
			FILE_NAME,
			USER_GROUP_NAME,
			FILE_TYPE_NAME,
			USER_TYPE_NAME,
			USER_ADDRESS,
			USER_NAME,
			ACCESS_CONSTRAINT,
			USER_DOMAIN,
			PATH_NAME, // 10
			RESOURCE_NAME,
			RESOURCE_ADDRESS_NETPREFIX,
			RESOURCE_TYPE_NAME,
			MODIFICATION_DATE,
			DIRECTORY_NAME,
			USER_PHONE,
			USER_EMAIL,
			SIZE,
			USER_AUDIT_TIME_STAMP,
			USER_AUDIT_COMMENTS, // 20
			FILE_COMMENTS,
			AUDIT_ACTION_DESCRIPTION,
			AUDIT_TIMESTAMP,
			AUDIT_COMMENTS,
			DIRECTORY_ACCESS_CONSTRAINT,
			ACCESS_DIRECTORY_NAME,
			RESOURCE_DEFAULT_PATH,
			PHYSICAL_RESOURCE_DEFAULT_PATH,
			PHYSICAL_RESOURCE_NAME,
			PHYSICAL_RESOURCE_TYPE_NAME, // 30
			RESOURCE_REPLICATION_ENUM,
			FILE_ACCESS_LIST,
			RESOURCE_ACCESS_LIST,
			FILE_IS_DELETED,
			OWNER,
			TICKET_D,
			TICKET_BEGIN_TIME_D,
			TICKET_END_TIME_D,
			TICKET_ACC_COUNT_D,
			TICKET_ACC_LIST_D, // 40
			TICKET_OWNER_D,
			TICKET_USER_D,
			TICKET_C,
			TICKET_BEGIN_TIME_C,
			TICKET_END_TIME_C,
			TICKET_ACC_COUNT_C,
			TICKET_ACC_LIST_C,
			TICKET_OWNER_C,
			TICKET_USER_C,
			TICKET_USER_DOMAIN_C, // 50
			TICKET_USER_DOMAIN_D,
			TICKET_OWNER_DOMAIN_C,
			TICKET_OWNER_DOMAIN_D,
			PARENT_DIRECTORY_NAME,
			LOCATION_NAME,
			IS_DIRTY,
			CONTAINER_MAX_SIZE,
			CONTAINER_NAME,
			RESOURCE_CLASS,
			MAX_OBJ_SIZE, // 60
			OFFSET,
			CONTAINER_RESOURCE_NAME,
			CONTAINER_RESOURCE_CLASS,
			CONTAINER_SIZE,
			CONTAINER_LOG_RESOURCE_NAME,
			OWNER_DOMAIN,
			USER_DISTINGUISHED_NAME,
			USER_AUTHENTICATION_SCHEME,
			SERVER_LOCATION,
			SERVER_NETPREFIX, // 70
			CONTAINER_REPLICATION_ENUM,
			FILE_ANNOTATION_USERNAME,
			FILE_ANNOTATION_USERDOMAIN,
			FILE_ANNOTATION,
			FILE_ANNOTATION_TIMESTAMP,
			CREATION_DATE,
			FILE_COMMENTS_TIMESTAMP,
			FILE_ANNOTATION_POSITION,
			FILE_ACCESS_PRIVILEGE,
			RESOURCE_MAX_LATENCY, // 80
			RESOURCE_MIN_LATENCY,
			RESOURCE_BANDWIDTH,
			RESOURCE_MAX_CONCURRENCY,
			RESOURCE_NUM_OF_HIERARCHIES,
			RESOURCE_NUM_OF_STRIPES,
			RESOURCE_CAPACITY,
			RESOURCE_DESCRIPTION,
			FILE_CLASS_NAME,
			FILE_CLASS_TYPE,
			STRUCTURED_METADATA_TYPE, // 90
			STRUCTURED_METADATA_COMMENTS,
			STRUCTURED_METADATA_FILE_NAME,
			INTERNAL_STRUCTURED_METADATA,
			INDEX_NAME_FOR_FILE,
			INDEX_NAME_FOR_DATATYPE,
			INDEX_NAME_FOR_DIRECTORY,
			METHOD_NAME_FOR_FILE,
			METHOD_NAME_FOR_DATATYPE,
			METHOD_NAME_FOR_DIRECTORY,
			IX_DIRECTORY_NAME_FOR_FILE, // 100
			IX_DIRECTORY_NAME_FOR_DATATYPE,
			IX_DIRECTORY_NAME_FOR_DIRECTORY,
			METHOD_DIRECTORY_NAME_FOR_FILE,
			METHOD_DIRECTORY_NAME_FOR_DATATYPE,
			METHOD_DIRECTORY_NAME_FOR_DIRECTORY,
			IX_DATATYPE_FOR_FILE,
			IX_DATATYPE_FOR_DATATYPE,
			IX_DATATYPE_FOR_DIRECTORY,
			METHOD_DATATYPE_FOR_FILE,
			METHOD_DATATYPE_FOR_DATATYPE, // 110
			METHOD_DATATYPE_FOR_DIRECTORY,
			STRUCTURED_METADATA_DIRECTORY_NAME,
			IX_LOCATION_FOR_FILE,
			IX_LOCATION_FOR_DATATYPE,
			IX_LOCATION_FOR_DIRECTORY,
			METADATA_NUM,
			DEFINABLE_METADATA_FOR_FILES0,
			DEFINABLE_METADATA_FOR_FILES1,
			DEFINABLE_METADATA_FOR_FILES2,
			DEFINABLE_METADATA_FOR_FILES3, // 120
			DEFINABLE_METADATA_FOR_FILES4,
			DEFINABLE_METADATA_FOR_FILES5,
			DEFINABLE_METADATA_FOR_FILES6,
			DEFINABLE_METADATA_FOR_FILES7,
			DEFINABLE_METADATA_FOR_FILES8,
			DEFINABLE_METADATA_FOR_FILES9,
			INTEGER_DEFINABLE_METADATA0,
			INTEGER_DEFINABLE_METADATA1,
			OWNER_EMAIL,
			AUDIT_USER, // 130
			AUDIT_USER_DOMAIN,
			CONTAINER_FOR_DIRECTORY,
			METADATA_NUM_DIRECTORY,
			DEFINABLE_METADATA_FOR_DIRECTORIES0,
			DEFINABLE_METADATA_FOR_DIRECTORIES1,
			DEFINABLE_METADATA_FOR_DIRECTORIES2,
			DEFINABLE_METADATA_FOR_DIRECTORIES3,
			DEFINABLE_METADATA_FOR_DIRECTORIES4,
			DEFINABLE_METADATA_FOR_DIRECTORIES5,
			DEFINABLE_METADATA_FOR_DIRECTORIES6, // 140
			DEFINABLE_METADATA_FOR_DIRECTORIES7,
			DEFINABLE_METADATA_FOR_DIRECTORIES8,
			DEFINABLE_METADATA_FOR_DIRECTORIES9,
			INTEGER_DEFINABLE_METADATA_FOR_DIRECTORIES0,
			INTEGER_DEFINABLE_METADATA_FOR_DIRECTORIES1,
			FILE_ACCESS_ID,
			USER_ID,
			FILE_LAST_ACCESS_TIMESTAMP,
			DIRECTORY_OWNER_NAME,
			DIRECTORY_CREATE_TIMESTAMP, // 150
			DIRECTORY_COMMENTS,
			DC_DATA_NAME,
			DC_COLLECTION,
			DC_CONTRIBUTOR_TYPE,
			DC_SUBJECT_CLASS,
			DC_DESCRIPTION_TYPE,
			DC_TYPE,
			DC_SOURCE_TYPE,
			DC_LANGUAGE,
			DC_RELATION_TYPE, // 160
			DC_COVERAGE_TYPE,
			DC_RIGHTS_TYPE,
			DC_TITLE,
			DC_CONTRIBUTOR_NAME,
			DC_SUBJECT_NAME,
			DC_DESCRIPTION,
			DC_PUBLISHER,
			DC_SOURCE,
			DC_RELATED_DATA_DESCRIPTION,
			DC_RELATED_DATA, // 170
			DC_RELATED_DIRECTORY,
			DC_COVERAGE,
			DC_RIGHTS,
			DC_PUBLISHER_ADDR,
			DC_CONTRIBUTOR_ADDR,
			DC_CONTRIBUTOR_EMAIL,
			DC_CONTRIBUTOR_PHONE,
			DC_CONTRIBUTOR_WEB,
			DC_CONTRIBUTOR_AFFILIATION,
			COLL_ACCS_ID, // 180
			COLL_ACCS_USER_ID,
			DATA_ACCS_ID,
			DATA_ACCS_USER_ID,
			DEFINABLE_METADATA_FOR_FILES0_1,
			DEFINABLE_METADATA_FOR_FILES0_2,
			DEFINABLE_METADATA_FOR_FILES0_3,
			DEFINABLE_METADATA_FOR_FILES0_4,
			DEFINABLE_METADATA_FOR_FILES1_1,
			DEFINABLE_METADATA_FOR_FILES1_2,
			DEFINABLE_METADATA_FOR_FILES1_3, // 190
			DEFINABLE_METADATA_FOR_FILES1_4,
			DEFINABLE_METADATA_FOR_DIRECTORIES0_1,
			DEFINABLE_METADATA_FOR_DIRECTORIES0_2,
			DEFINABLE_METADATA_FOR_DIRECTORIES0_3,
			DEFINABLE_METADATA_FOR_DIRECTORIES0_4,
			DEFINABLE_METADATA_FOR_DIRECTORIES1_1,
			DEFINABLE_METADATA_FOR_DIRECTORIES1_2,
			DEFINABLE_METADATA_FOR_DIRECTORIES1_3,
			DEFINABLE_METADATA_FOR_DIRECTORIES1_4,
			TICKET_RECURSIVE, // 200
			DEFINABLE_METADATA_FOR_RESOURCES0_1,
			DEFINABLE_METADATA_FOR_RESOURCES0_2,
			DEFINABLE_METADATA_FOR_RESOURCES0_3,
			DEFINABLE_METADATA_FOR_RESOURCES1_1,
			DEFINABLE_METADATA_FOR_RESOURCES1_2,
			DEFINABLE_METADATA_FOR_RESOURCES1_3,
			DEFINABLE_METADATA_FOR_USERS0_1,
			DEFINABLE_METADATA_FOR_USERS0_2,
			DEFINABLE_METADATA_FOR_USERS0_3,
			DEFINABLE_METADATA_FOR_USERS1_1, // 210
			DEFINABLE_METADATA_FOR_USERS1_2,
			DEFINABLE_METADATA_FOR_USERS1_3,
			TAPE_NUMBER,
			TAPE_OWNER,
			TAPE_TYPE_VAL,
			TAPE_LIBINX,
			TAPE_FULLFLAG,
			TAPE_CURRENT_FILE_SEQNUM,
			TAPE_CURRENT_ABS_POSITION,
			TAPE_BYTES_WRITTEN, // 220
			FILE_VER_NUM,
			FILE_LOCK_NUM,
			FILE_LOCK_DESCRIPTION,
			FILE_LOCK_OWNER_NAME,
			FILE_LOCK_OWNER_DOMAIN,
			DIRECTORY_OWNER_DOMAIN,
			TICKET_DIRECTORY_NAME,
			TICKET_DIRECTORY_CREATE_TIMESTAMP,
			TICKET_DIRECTORY_COMMENTS,
			DIRECTORY_ANNOTATION_USERNAME, // 230
			DIRECTORY_ANNOTATION_USERDOMAIN,
			DIRECTORY_ANNOTATION,
			DIRECTORY_ANNOTATION_TIMESTAMP,
			DIRECTORY_ANNOTATION_TYPE,
			FILE_GROUP_ID,
			FILE_HIDE,
			FILE_CHECKSUM,
			FILE_AUDITFLAG,
			FILE_LOCK_EXPIRY,
			FILE_PIN_VAL, // 240
			FILE_PIN_OWNER_NAME,
			FILE_PIN_OWNER_DOMAIN,
			FILE_PIN_EXPIRY,
			FILE_EXPIRY_DATE,
			FILE_IS_COMPRESSED,
			FILE_IS_ENCRYPTED,
			FILE_EXPIRE_DATE_2,
			NEW_AUDIT_ACTION_DESCRIPTION,
			NEW_AUDIT_TIMESTAMP,
			NEW_AUDIT_COMMENTS, // 250
			DEFINABLE_METADATA_FOR_RESOURCES0,
			DEFINABLE_METADATA_FOR_RESOURCES1,
			DEFINABLE_METADATA_FOR_RESOURCES2,
			DEFINABLE_METADATA_FOR_RESOURCES3,
			DEFINABLE_METADATA_FOR_RESOURCES4,
			DEFINABLE_METADATA_FOR_RESOURCES5,
			DEFINABLE_METADATA_FOR_RESOURCES6,
			DEFINABLE_METADATA_FOR_RESOURCES7,
			DEFINABLE_METADATA_FOR_RESOURCES8,
			DEFINABLE_METADATA_FOR_RESOURCES9, // 260
			DEFINABLE_INTEGER_METADATA_RESOURCE0,
			METADATA_NUM_RESOURCE,
			METADATA_NUM_USER,
			COMPOUND_RESOURCE_NAME,
			FILE_SEG_NUM,
			INT_REPLICATION_ENUM,
			INT_SEG_NUM,
			INT_PATH_NAME,
			INT_RESOURCE_NAME,
			INT_RESOURCE_ADDRESS_NETPREFIX, // 270
			INT_RESOURCE_TYPE_NAME,
			INT_IS_DIRTY,
			INT_RESOURCE_CLASS,
			INT_OFFSET,
			INT_SIZE,
			COMP_OBJ_USER_NAME,
			COMP_OBJ_USER_DOMAIN,
			INT_PATH_NAME_FOR_REPL,
			DIRECTORY_LINK_NUMBER, // ** MDAS_TD_DATA_GRP, **
			REAL_DIRECTORY_NAME, // 280
			FILE_CONTAINER_NAME,
			DIRECTORY_CONTAINER_NAME,
			DEFINABLE_METADATA_FOR_USERS0,
			DEFINABLE_METADATA_FOR_USERS1,
			DEFINABLE_METADATA_FOR_USERS2,
			DEFINABLE_METADATA_FOR_USERS3,
			DEFINABLE_METADATA_FOR_USERS4,
			DEFINABLE_METADATA_FOR_USERS5,
			DEFINABLE_METADATA_FOR_USERS6,
			DEFINABLE_METADATA_FOR_USERS7, // 290
			DEFINABLE_METADATA_FOR_USERS8,
			DEFINABLE_METADATA_FOR_USERS9,
			UDIMD_USER0,
			NONDISTINCT,
			USERDEFFUNC01,
			USERDEFFUNC02,
			USERDEFFUNC03,
			USERDEFFUNC04,
			USERDEFFUNC05,
			ZONE_NAME, // 300
			ZONE_LOCALITY,
			ZONE_NETPREFIX,
			ZONE_PORT_NUM,
			ZONE_ADMIN_AUTH_SCHEME_NAME,
			ZONE_ADMIN_DISTIN_NAME,
			ZONE_STATUS,
			ZONE_CREATE_DATE,
			ZONE_MODIFY_DATE,
			ZONE_COMMENTS,
			ZONE_CONTACT, // 310
			ZONE_ADMIN_NAME,
			ZONE_ADMIN_DOMAIN_NAME,
			RSRC_ACCESS_PRIVILEGE,
			RSRC_ACCESS_CONSTRAINT,
			UDIMD_USER1,
			UDIMD_RSRC1,
			USER_GROUP_MODIFY_DATE,
			USER_GROUP_ZONE_NAME,
			USER_GROUP_DOMAIN_DESC,
			DATA_UDEF_MDATA_MODIFY_DATE, // 320
			COLL_UDEF_MDATA_MODIFY_DATE,
			USER_UDEF_MDATA_MODIFY_DATE,
			RSRC_UDEF_MDATA_MODIFY_DATE,
			DATA_TYPE_MIME_STRING,
			DATA_TYPE_EXTENDERS,
			RSRC_COMMENTS,
			RSRC_CREATE_DATE,
			RSRC_MODIFY_DATE,
			USER_CREATE_DATE,
			USER_MODIFY_DATE, // 330
			RSRC_MAX_OBJ_SIZE,
			RSRC_OWNER_NAME,
			RSRC_OWNER_DOMAIN,
			RSRC_MLSEC_LATENCY_MAX,
			RSRC_MLSEC_LATENCY_MIN,
			RSRC_MBPS_BANDWIDTH,
			RSRC_CONCURRENCY_MAX,
			RSRC_NUM_OF_HIERARCHIES,
			RSRC_NUM_OF_STRIPES,
			RSRC_MEGAB_CAPACITY, // 340
			RSRC_ACCS_USER_NAME,
			RSRC_ACCS_USER_DOMAIN,
			RSRC_ACCS_USER_ZONE,
			RSRC_OWNER_ZONE,
			DATA_OWNER_ZONE,
			TICKET_OWNER_ZONE_D,
			TICKET_USER_ZONE_D,
			TICKET_OWNER_ZONE_C,
			TICKET_USER_ZONE_C,
			DATA_ANNOTATION_USER_ZONE, // 350
			AUDIT_USER_ZONE,
			COLL_OWNER_ZONE,
			DATA_LOCK_OWNER_ZONE,
			COLL_ANNOTATION_USER_ZONE,
			DATA_PIN_OWNER_ZONE,
			COMP_OBJ_USER_ZONE,
			PARENT_SERVER_LOCATION,
			RSRC_ACCESS_ID,
			PARENT_DATA_TYPE,
			ZONE_LOCN_DESC, // 360
			PARENT_DOMAIN_DESC,
			PARENT_USER_TYPE,
			PARENT_RSRC_TYPE,
			RSRC_ACCS_GRPUSER_NAME,
			RSRC_ACCS_GRPUSER_DOMAIN,
			RSRC_ACCS_GRPUSER_ZONE,
			COLL_MODIFY_TIMESTAMP,
			DATA_ACCS_GRPUSER_NAME,
			DATA_ACCS_GRPUSER_DOMAIN,
			DATA_ACCS_GRPUSER_ZONE, // 370
			COLL_ACCS_GRPUSER_NAME,
			COLL_ACCS_GRPUSER_DOMAIN,
			COLL_ACCS_GRPUSER_ZONE,
			COLL_ACCS_COLLECTION_NAME,
			COLL_ACCS_ACCESS_CONSTRAINT,
			DATA_TYPE_FOR_CONTAINER_FOR_COLLECTION,
			DATA_UDEF_MDATA_CREATE_DATE,
			COLL_UDEF_MDATA_CREATE_DATE,
			RSRC_UDEF_MDATA_CREATE_DATE,
			USER_UDEF_MDATA_CREATE_DATE, // 380
			CONTAINER_ID, GUID, GUID_FLAG, GUID_TIME_STAMP, ORDERBY,
			DELETE_FILE_ID,
			DELETE_FILE_REPLICATION_ENUM,
			DELETE_FILE_NAME,
			DELETE_MODIFICATION_DATE,
			DELETE_DIRECTORY_NAME, // 390
			DEL_COLLECTION_ID, DEL_DATA_COMMENT, FREE_SPACE, FS_TIMESTAMP,
			EXTENSIBLE_SCHEMA_NAME, EXTENSIBLE_TABLE_NAME,
			EXTENSIBLE_ATTR_NAME, EXTENSIBLE_ATTR_OUTSIDE_NAME,
			EXTENSIBLE_ATTR_COMMENTS, RESOURCE_STATUS, // 400
	};

	/**
	 * The unmodified SRB metadata attribute names, as string values. Known
	 * uses, ORDERBY, see also SRBMetaDataCommands.srbGenQuery
	 */
	private static String srbCatalogNames[] = { "DATA_ID", "DATA_REPL_ENUM",
			"DATA_NAME", "USER_GROUP_NAME", "DATA_TYP_NAME", "USER_TYP_NAME",
			"USER_ADDRESS", "USER_NAME", "ACCESS_CONSTRAINT", "DOMAIN_DESC",
			"PATH_NAME", "RSRC_NAME", "RSRC_ADDR_NETPREFIX", "RSRC_TYP_NAME",
			"REPL_TIMESTAMP", "DATA_GRP_NAME", "USER_PHONE", "USER_EMAIL",
			"SIZE", "USER_AUDIT_TIME_STAMP", "USER_AUDIT_COMMENTS",
			"DATA_COMMENTS", "AUDIT_ACTION_DESC", "AUDIT_TIMESTAMP",
			"AUDIT_COMMENTS", "COLLECTION_ACCESS_CONSTRAINT",
			"ACCESS_COLLECTION_NAME", "RSRC_DEFAULT_PATH",
			"PHY_RSRC_DEFAULT_PATH", "PHY_RSRC_NAME", "PHY_RSRC_TYP_NAME",
			"RSRC_REPL_ENUM", "DATA_ACCESS_LIST", "RSRC_ACCESS_LIST",
			"DATA_IS_DELETED", "DATA_OWNER", "TICKET_D", "TICKET_BEGIN_TIME_D",
			"TICKET_END_TIME_D", "TICKET_ACC_COUNT_D", "TICKET_ACC_LIST_D",
			"TICKET_OWNER_D", "TICKET_USER_D", "TICKET_C",
			"TICKET_BEGIN_TIME_C", "TICKET_END_TIME_C", "TICKET_ACC_COUNT_C",
			"TICKET_ACC_LIST_C", "TICKET_OWNER_C", "TICKET_USER_C",
			"TICKET_USER_DOMAIN_C", "TICKET_USER_DOMAIN_D",
			"TICKET_OWNER_DOMAIN_C", "TICKET_OWNER_DOMAIN_D",
			"PARENT_COLLECTION_NAME", "LOCATION_NAME", "IS_DIRTY",
			"CONTAINER_MAX_SIZE", "CONTAINER_NAME", "RSRC_CLASS",
			"MAX_OBJ_SIZE", "OFFSET", "CONTAINER_RSRC_NAME",
			"CONTAINER_RSRC_CLASS", "CONTAINER_SIZE",
			"CONTAINER_LOG_RSRC_NAME", "DATA_OWNER_DOMAIN", "USER_DISTIN_NAME",
			"USER_AUTH_SCHEME", "SERVER_LOCATION", "SERVER_NETPREFIX",
			"CONTAINER_REPL_ENUM", "DATA_ANNOTATION_USERNAME",
			"DATA_ANNOTATION_USERDOMAIN", "DATA_ANNOTATION",
			"DATA_ANNOTATION_TIMESTAMP", "DATA_CREATE_TIMESTAMP",
			"DATA_COMMENTS_TIMESTAMP", "DATA_ANNOTATION_POSITION",
			"DATA_ACCESS_PRIVILEGE", "RESOURCE_MAX_LATENCY",
			"RESOURCE_MIN_LATENCY", "RESOURCE_BANDWIDTH",
			"RESOURCE_MAX_CONCURRENCY", "RESOURCE_NUM_OF_HIERARCHIES",
			"RESOURCE_NUM_OF_STRIPES", "RESOURCE_CAPACITY",
			"RESOURCE_DESCRIPTION", "DATA_CLASS_NAME", "DATA_CLASS_TYPE",
			"STRUCTURED_METADATA_TYPE", "STRUCTURED_METADATA_COMMENTS",
			"STRUCTURED_METADATA_DATA_NAME", "INTERNAL_STRUCTURED_METADATA",
			"INDEX_NAME_FOR_DATASET", "INDEX_NAME_FOR_DATATYPE",
			"INDEX_NAME_FOR_COLLECTION", "METHOD_NAME_FOR_DATASET",
			"METHOD_NAME_FOR_DATATYPE", "METHOD_NAME_FOR_COLLECTION",
			"IX_COLL_NAME_FOR_DATASET", "IX_COLLNAME_FOR_DATATYPE",
			"IX_COLLNAME_FOR_COLLECTION", "MTH_COLLNAME_FOR_DATASET",
			"MTH_COLLNAME_FOR_DATATYPE", "MTH_COLLNAME_FOR_COLLECTION",
			"IX_DATATYPE_FOR_DATASET", "IX_DATATYPE_FOR_DATATYPE",
			"IX_DATATYPE_FOR_COLLECTION", "MTH_DATATYPE_FOR_DATASET",
			"MTH_DATATYPE_FOR_DATATYPE", "MTH_DATATYPE_FOR_COLLECTION",
			"STRUCTURED_METADATA_COLLNAME", "IX_LOCATION_FOR_DATASET",
			"IX_LOCATION_FOR_DATATYPE", "IX_LOCATION_FOR_COLLECTION",
			"METADATA_NUM", "UDSMD0", "UDSMD1", "UDSMD2", "UDSMD3", "UDSMD4",
			"UDSMD5", "UDSMD6", "UDSMD7", "UDSMD8", "UDSMD9", "UDIMD0",
			"UDIMD1", "DATA_OWNER_EMAIL", "AUDIT_USER", "AUDIT_USER_DOMAIN",
			"CONTAINER_FOR_COLLECTION", "METADATA_NUM_COLL", "UDSMD_COLL0",
			"UDSMD_COLL1", "UDSMD_COLL2", "UDSMD_COLL3", "UDSMD_COLL4",
			"UDSMD_COLL5", "UDSMD_COLL6", "UDSMD_COLL7", "UDSMD_COLL8",
			"UDSMD_COLL9", "UDIMD_COLL0", "UDIMD_COLL1", "DATA_ACCESS_ID",
			"USER_ID", "DATA_LAST_ACCESS_TIMESTAMP", "COLL_OWNER_NAME",
			"COLL_CREATE_TIMESTAMP", "COLL_COMMENTS", "DC_DATA_NAME",
			"DC_COLLECTION", "DC_CONTRIBUTOR_TYPE", "DC_SUBJECT_CLASS",
			"DC_DESCRIPTION_TYPE", "DC_TYPE", "DC_SOURCE_TYPE", "DC_LANGUAGE",
			"DC_RELATION_TYPE", "DC_COVERAGE_TYPE", "DC_RIGHTS_TYPE",
			"DC_TITLE", "DC_CONTRIBUTOR_NAME", "DC_SUBJECT_NAME",
			"DC_DESCRIPTION", "DC_PUBLISHER", "DC_SOURCE",
			"DC_RELATED_DATA_DESCR", "DC_RELATED_DATA", "DC_RELATED_COLL",
			"DC_COVERAGE", "DC_RIGHTS", "DC_PUBLISHER_ADDR",
			"DC_CONTRIBUTOR_ADDR", "DC_CONTRIBUTOR_EMAIL",
			"DC_CONTRIBUTOR_PHONE", "DC_CONTRIBUTOR_WEB",
			"DC_CONTRIBUTOR_CORPNAME", "COLL_ACCS_ID", "COLL_ACCS_USER_ID",
			"DATA_ACCS_ID", "DATA_ACCS_USER_ID", "UDSMD0_1", "UDSMD0_2",
			"UDSMD0_3", "UDSMD0_4", "UDSMD1_1", "UDSMD1_2", "UDSMD1_3",
			"UDSMD1_4", "UDSMD_COLL0_1", "UDSMD_COLL0_2", "UDSMD_COLL0_3",
			"UDSMD_COLL0_4", "UDSMD_COLL1_1", "UDSMD_COLL1_2", "UDSMD_COLL1_3",
			"UDSMD_COLL1_4", "C_TICKET_RECURSIVE", "UDSMD_RSRC1_0",
			"UDSMD_RSRC1_1", "UDSMD_RSRC1_2", "UDSMD_RSRC2_0", "UDSMD_RSRC2_1",
			"UDSMD_RSRC2_2", "UDSMD_USER1_0", "UDSMD_USER1_1", "UDSMD_USER1_2",
			"UDSMD_USER2_0", "UDSMD_USER2_1", "UDSMD_USER2_2", "TAPE_NUMBER",
			"TAPE_OWNER", "TAPE_TYPE_VAL", "TAPE_LIBINX", "TAPE_FULLFLAG",
			"TAPE_CURRENT_FILE_SEQNUM", "TAPE_CURRENT_ABS_POSITION",
			"TAPE_BYTES_WRITTEN", "DATA_VER_NUM", "DATA_LOCK_NUM",
			"DATA_LOCK_DESC", "DATA_LOCK_OWNER_NAME", "DATA_LOCK_OWNER_DOMAIN",
			"COLL_OWNER_DOMAIN", "C_TICKET_COLL_NAME",
			"C_TICKET_COLL_CREATE_TIMESTAMP", "C_TICKET_COLL_COMMENTS",
			"COLL_ANNOTATION_USERNAME", "COLL_ANNOTATION_USERDOMAIN",
			"COLL_ANNOTATION", "COLL_ANNOTATION_TIMESTAMP",
			"COLL_ANNOTATION_TYPE", "DATA_GRP_ID", "DATA_HIDE",
			"DATA_CHECKSUM", "DATA_AUDITFLAG", "DATA_LOCK_EXPIRY",
			"DATA_PIN_VAL", "DATA_PIN_OWNER_NAME", "DATA_PIN_OWNER_DOMAIN",
			"DATA_PIN_EXPIRY", "DATA_EXPIRY_DATE", "DATA_IS_COMPRESSED",
			"DATA_IS_ENCRYPTED", "DATA_EXPIRE_DATE_2", "NEW_AUDIT_ACTION_DESC",
			"NEW_AUDIT_TIMESTAMP", "NEW_AUDIT_COMMENTS", "UDSMD_RSRC0",
			"UDSMD_RSRC1", "UDSMD_RSRC2", "UDSMD_RSRC3", "UDSMD_RSRC4",
			"UDSMD_RSRC5", "UDSMD_RSRC6", "UDSMD_RSRC7", "UDSMD_RSRC8",
			"UDSMD_RSRC9", "UDIMD_RSRC0", "METADATA_NUM_RSRC",
			"METADATA_NUM_USER", "COMPOUND_RSRC_NAME", "DATA_SEG_NUM",
			"INT_REPL_ENUM", "INT_SEG_NUM", "INT_PATH_NAME", "INT_RSRC_NAME",
			"INT_RSRC_ADDR_NETPREFIX", "INT_RSRC_TYP_NAME", "INT_IS_DIRTY",
			"INT_RSRC_CLASS", "INT_OFFSET", "INT_SIZE", "COMP_OBJ_USER_NAME",
			"COMP_OBJ_USER_DOMAIN", "INT_PATH_NAME_FOR_REPL",
			"COLLECTION_LINK_NUM", "REAL_COLLECTION_NAME", "DATA_CONT_NAME",
			"COLLECTION_CONT_NAME", "UDSMD_USER0", "UDSMD_USER1",
			"UDSMD_USER2", "UDSMD_USER3", "UDSMD_USER4", "UDSMD_USER5",
			"UDSMD_USER6", "UDSMD_USER7", "UDSMD_USER8", "UDSMD_USER9",
			"UDIMD_USER0", "NONDISTINCT", "USERDEFFUNC01", "USERDEFFUNC02",
			"USERDEFFUNC03", "USERDEFFUNC04", "USERDEFFUNC05", "ZONE_NAME",
			"ZONE_LOCALITY", "ZONE_NETPREFIX", "ZONE_PORT_NUM",
			"ZONE_ADMIN_AUTH_SCHEME_NAME", "ZONE_ADMIN_DISTIN_NAME",
			"ZONE_STATUS", "ZONE_CREATE_DATE", "ZONE_MODIFY_DATE",
			"ZONE_COMMENTS", "ZONE_CONTACT", "ZONE_ADMIN_NAME",
			"ZONE_ADMIN_DOMAIN_NAME", "RSRC_ACCESS_PRIVILEGE",
			"RSRC_ACCESS_CONSTRAINT", "UDIMD_USER1", "UDIMD_RSRC1",
			"USER_GROUP_MODIFY_DATE", "USER_GROUP_ZONE_NAME",
			"USER_GROUP_DOMAIN_DESC", "DATA_UDEF_MDATA_MODIFY_DATE",
			"COLl_UDEF_MDATA_MODIFY_DATE", "USER_UDEF_MDATA_MODIFY_DATE",
			"RSRC_UDEF_MDATA_MODIFY_DATE", "DATA_TYPE_MIME_STRING",
			"DATA_TYPE_EXTENDERS", "RSRC_COMMENTS", "RSRC_CREATE_DATE",
			"RSRC_MODIFY_DATE", "USER_CREATE_DATE", "USER_MODIFY_DATE",
			"RSRC_MAX_OBJ_SIZE", "RSRC_OWNER_NAME", "RSRC_OWNER_DOMAIN",
			"RSRC_MLSEC_LATENCY_MAX", "RSRC_MLSEC_LATENCY_MIN",
			"RSRC_MBPS_BANDWIDTH", "RSRC_CONCURRENCY_MAX",
			"RSRC_NUM_OF_HIERARCHIES", "RSRC_NUM_OF_STRIPES",
			"RSRC_MEGAB_CAPACITY", "RSRC_ACCS_USER_NAME",
			"RSRC_ACCS_USER_DOMAIN", "RSRC_ACCS_USER_ZONE", "RSRC_OWNER_ZONE",
			"DATA_OWNER_ZONE", "TICKET_OWNER_ZONE_D", "TICKET_USER_ZONE_D",
			"TICKET_OWNER_ZONE_C", "TICKET_USER_ZONE_C",
			"DATA_ANNOTATION_USER_ZONE", "AUDIT_USER_ZONE", "COLL_OWNER_ZONE",
			"DATA_LOCK_OWNER_ZONE", "COLL_ANNOTATION_USER_ZONE",
			"DATA_PIN_OWNER_ZONE", "COMP_OBJ_USER_ZONE",
			"PARENT_SERVER_LOCATION", "RSRC_ACCESS_ID", "PARENT_DATA_TYPE",
			"ZONE_LOCN_DESC", "PARENT_DOMAIN_DESC", "PARENT_USER_TYPE",
			"PARENT_RSRC_TYPE", "RSRC_ACCS_GRPUSER_NAME",
			"RSRC_ACCS_GRPUSER_DOMAIN", "RSRC_ACCS_GRPUSER_ZONE",
			"COLL_MODIFY_TIMESTAMP", "DATA_ACCS_GRPUSER_NAME",
			"DATA_ACCS_GRPUSER_DOMAIN", "DATA_ACCS_GRPUSER_ZONE",
			"COLL_ACCS_GRPUSER_NAME", "COLL_ACCS_GRPUSER_DOMAIN",
			"COLL_ACCS_GRPUSER_ZONE", "COLL_ACCS_COLLECTION_NAME",
			"COLL_ACCS_ACCESS_CONSTRAINT",
			"DATA_TYPE_FOR_CONTAINER_FOR_COLLECTION",
			"DATA_UDEF_MDATA_CREATE_DATE", "COLL_UDEF_MDATA_CREATE_DATE",
			"RSRC_UDEF_MDATA_CREATE_DATE", "USER_UDEF_MDATA_CREATE_DATE",
			"CONTAINER_ID", "GUID", "GUID_FLAG", "GUID_TIME_STAMP", "ORDERBY",
			"DEL_DATA_ID", "DEL_DATA_REPL_ENUM", "DEL_DATA_NAME",
			"DEL_REPL_TIMESTAMP", "DEL_COLLECTION_NAME", "DEL_COLLECTION_ID",
			"DEL_DATA_COMMENT", "FREE_SPACE", "FS_TIMESTAMP",
			"EXTENSIBLE_SCHEMA_NAME", "EXTENSIBLE_TABLE_NAME",
			"EXTENSIBLE_ATTR_NAME", "EXTENSIBLE_ATTR_OUTSIDE_NAME",
			"EXTENSIBLE_ATTR_COMMENTS", "RESOURCE_STATUS", };

	static {
		// register all MetaDataGroup subclasses
		// Collection
		MetaDataGroup group = new MetaDataGroup(GROUP_DIRECTORY, "Core "
				+ "meta-information about directories");
		group.add(new MetaDataField(PARENT_DIRECTORY_NAME,
				"name of parent directory (" + DIRECTORY_NAME + ")",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_NAME, "directory name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_GROUP_ID, "internal directory id",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(DIRECTORY_OWNER_NAME, "directory owner",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_OWNER_DOMAIN,
				"directory owner domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_CREATE_TIMESTAMP,
				"directory create timestamp", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CONTAINER_FOR_DIRECTORY,
				"default container for coll", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_COMMENTS,
				"comments on directory", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(COLL_ACCS_ID, "directory access id",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(COLL_ACCS_USER_ID,
				"directory access user id", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_LINK_NUMBER,
				"ACL inheritance bit", MetaDataField.STRING, protocol));
		add(group);

		// Data
		group = new MetaDataGroup(GROUP_DATA,
				"Core meta-information about datasets.");
		group.add(new MetaDataField(DIRECTORY_NAME, "collection name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CONTAINER_NAME, "name of container",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_NAME, "file name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ID, "internal data id",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_REPLICATION_ENUM,
				"replica copy number", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_VER_NUM, "dataset version number",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_SEG_NUM, "dataset segment number",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PHYSICAL_RESOURCE_NAME,
				"physical resource name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PATH_NAME,
				"physical path name of data object", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(SIZE, "size of data", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(OFFSET, "position of data in container",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(FILE_TYPE_NAME, "data type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_CLASS_NAME,
				"classifcation name for data", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_CLASS_TYPE, "classification type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CREATION_DATE, "data creation time stamp",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(MODIFICATION_DATE,
						"data modification time stamp", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(FILE_LAST_ACCESS_TIMESTAMP,
				"last access time stamp", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(OWNER, "data creator name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(OWNER_DOMAIN, "domain of data creator",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_COMMENTS, "comments on data",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_IS_DELETED, "data liveness",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(IS_DIRTY,
				"data has changed compared to other copies",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_LOCK_NUM,
				"dataset lock type in numeric form", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(FILE_LOCK_DESCRIPTION,
				"lock type description", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_LOCK_OWNER_NAME,
				"dataset lock owner name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_LOCK_OWNER_DOMAIN,
				"dataset lock owner domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_HIDE,
				"setting it more than 0 hides the data", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(FILE_CHECKSUM, "check sum for data string",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(FILE_AUDITFLAG,
				"flag set if data has to be audited", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(FILE_LOCK_EXPIRY,
				"lock expory data timestamp", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_PIN_VAL,
				"data replica pinned from moved if>=0", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(FILE_PIN_OWNER_NAME, "owner of the pin",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_PIN_OWNER_DOMAIN,
				"domain of the pin owner", MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(FILE_PIN_EXPIRY,
						"expiry timestamp for the pin", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(FILE_EXPIRY_DATE,
				"expiry time for the data replica", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(FILE_IS_COMPRESSED,
				"data compressed or not and how", MetaDataField.STRING,
				protocol));
		group
				.add(new MetaDataField(FILE_IS_ENCRYPTED,
						"data encrypted or not and how", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(FILE_EXPIRE_DATE_2,
				"another expiry timestamp", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DATA_ACCS_ID, "file access id",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DATA_ACCS_USER_ID, "file access user id",
				MetaDataField.STRING, protocol));
		add(group);

		// User
		group = new MetaDataGroup(GROUP_USER,
				"Core information about SRB-registered users.");
		group.add(new MetaDataField(USER_NAME, "user name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_DOMAIN, "user domain name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_ID, "MCAT's internal Id for the User",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(USER_TYPE_NAME, "user type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_ADDRESS, "user address",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_PHONE, "user phone number",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_EMAIL, "user email",
				MetaDataField.STRING, protocol));
		add(group);

		// Physical resource
		group = new MetaDataGroup(GROUP_PHYSICAL_RESOURCE,
				"Core meta-information "
						+ "about physical resources including SRB servers.");
		group.add(new MetaDataField(PHYSICAL_RESOURCE_NAME,
				"physical resource name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PHYSICAL_RESOURCE_TYPE_NAME,
				"physical resource type", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PHYSICAL_RESOURCE_DEFAULT_PATH,
				"default path in physical resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(LOCATION_NAME,
				"location of physical resource name", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_ADDRESS_NETPREFIX,
				"net address of physical resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_DESCRIPTION,
				"comments on the resource", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(MAX_OBJ_SIZE,
				"max size of data allowed in resource", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(RESOURCE_MAX_LATENCY,
				"physical resource estimated latency (max)", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(RESOURCE_MIN_LATENCY,
				"physical resource estimated latency (min)", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(RESOURCE_BANDWIDTH,
				"physical resource estimated bandwidth", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(RESOURCE_MAX_CONCURRENCY,
				"physical resource max concurrent reqs", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(RESOURCE_NUM_OF_HIERARCHIES,
				"depth of hierarchy in resource", MetaDataField.INT, protocol));
		group.add(new MetaDataField(RESOURCE_NUM_OF_STRIPES,
				"number of striping in resource", MetaDataField.INT, protocol));
		group.add(new MetaDataField(RESOURCE_CAPACITY,
				"capacity of the physical resource", MetaDataField.INT,
				protocol));
		add(group);

		// Logical resource
		group = new MetaDataGroup(GROUP_LOGICAL_RESOURCE,
				"Core meta-information " + "about logical resources.");
		group.add(new MetaDataField(RESOURCE_NAME, "name of logical resource",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_TYPE_NAME,
				"logical resource type", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PHYSICAL_RESOURCE_NAME,
				"physical resource name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_REPLICATION_ENUM,
				"index of physical resource in logical resource",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_DEFAULT_PATH,
				"default path in logical resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RESOURCE_CLASS,
				"classification of resource", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_ACCESS_PRIVILEGE,
				"resource access privilege", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_ACCESS_CONSTRAINT,
				"resource access constraint namw", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_COMMENTS, "comments on log resource",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_CREATE_DATE,
				"log rsource creation timestamp", MetaDataField.STRING,
				protocol));
		group
				.add(new MetaDataField(RSRC_MODIFY_DATE,
						"log resource modify timestamp", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(RSRC_MAX_OBJ_SIZE,
				"log resource max obj size", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_OWNER_NAME,
				"owner of the log resource", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_OWNER_DOMAIN,
				"domain of the log resource", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_MLSEC_LATENCY_MAX,
				"max latency of resource in millisec", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_MLSEC_LATENCY_MIN,
				"min latency of resource in millisec", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_MBPS_BANDWIDTH,
				"bandwidth of resource in mega bps", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_CONCURRENCY_MAX,
				"max concurrency allowed in resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_NUM_OF_HIERARCHIES,
				"num of staging levels in resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_NUM_OF_STRIPES,
				"num of striping done in resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_MEGAB_CAPACITY,
				"capacity of resource in megabytes", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_ACCS_USER_NAME,
				"user name used for resource access", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_ACCS_USER_DOMAIN,
				"user domn used for resource access", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_ACCS_USER_ZONE,
				"user zone used for resource access", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_OWNER_ZONE, "zone for resource owner",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_ACCS_GRPUSER_NAME,
				"group user who has access to resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_ACCS_GRPUSER_DOMAIN,
				"resource group user domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_ACCS_GRPUSER_ZONE,
				"resource group user zone", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_ACCESS_ID,
				"access id number for resource access", MetaDataField.STRING,
				protocol));

		add(group);

		// Server
		group = new MetaDataGroup(GROUP_SERVER, "");
		group.add(new MetaDataField(SERVER_LOCATION, "location of SRB server",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(SERVER_NETPREFIX,
				"net address of SRB server", MetaDataField.STRING, protocol));
		add(group);

		// User group
		group = new MetaDataGroup(GROUP_USER_GROUP, "");
		group.add(new MetaDataField(USER_GROUP_NAME, "name of user group",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_NAME, "user name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_DOMAIN, "user domain name",
				MetaDataField.STRING, protocol));
		add(group);

		// Authentication
		group = new MetaDataGroup(GROUP_AUTHENTICATION, "");
		group.add(new MetaDataField(USER_NAME, "user name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_DOMAIN, "user domain name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_DISTINGUISHED_NAME,
				"distinguished name of user", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_AUTHENTICATION_SCHEME,
				"user authentication scheme (" + USER_DISTINGUISHED_NAME + ")",
				MetaDataField.STRING, protocol));
		add(group);

		// Authorization
		group = new MetaDataGroup(GROUP_AUTHORIZATION, "");
		group.add(new MetaDataField(USER_NAME, "user name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_DOMAIN, "user domain name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_D,
				"identifier for ticket given for data", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TICKET_C,
				"identifier for ticket given for coll", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DIRECTORY_NAME, "collection name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_NAME, "data name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PHYSICAL_RESOURCE_NAME,
				"physical resource name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ACCESS_CONSTRAINT,
				"access restriction on data", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ACCESS_CONSTRAINT,
				"access on directory (" + ACCESS_DIRECTORY_NAME + ")",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ACCESS_DIRECTORY_NAME,
				"directory name for access in (" + DIRECTORY_ACCESS_CONSTRAINT
						+ ")", MetaDataField.STRING, protocol));
		add(group);

		// Auditing
		group = new MetaDataGroup(GROUP_AUDIT,
				"Audit information on users and on datasets.");
		group.add(new MetaDataField(AUDIT_USER, "audited user name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(AUDIT_USER_DOMAIN, "audited user domain",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(AUDIT_ACTION_DESCRIPTION,
				"audit action description", MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(NEW_AUDIT_ACTION_DESCRIPTION,
						"new audit action description", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(NEW_AUDIT_TIMESTAMP, "audit timestamp",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(NEW_AUDIT_COMMENTS, "audit comments",
				MetaDataField.STRING, protocol));
		add(group);

		// Ticket
		group = new MetaDataGroup(
				GROUP_TICKET,
				"Information about ticket-based "
						+ "access control for datasets, collections as well as recursively under a collection.");
		group.add(new MetaDataField(TICKET_D,
				"identifier for ticket given for data", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DIRECTORY_NAME, "collection name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_NAME, "data name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_USER_D,
				"allowed ticket user or user group", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TICKET_USER_DOMAIN_D,
				"data ticket user domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_OWNER_D, "data ticket creator",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_OWNER_DOMAIN_D,
				"data ticket creator domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_BEGIN_TIME_D,
				"data ticket validity start time", MetaDataField.STRING,
				protocol));
		group
				.add(new MetaDataField(TICKET_END_TIME_D,
						"data ticket validity endtime", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(TICKET_ACC_COUNT_D,
				"number of opens allowed on data ticket", MetaDataField.INT,
				protocol));
		group
				.add(new MetaDataField(TICKET_ACC_LIST_D,
						"access allowed on data ticket", MetaDataField.STRING,
						protocol));

		group.add(new MetaDataField(TICKET_C,
				"identifier for ticket given for collection",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_DIRECTORY_NAME,
				"collection for ticket", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_RECURSIVE,
				"recursive flag for C_TICKET", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_USER_C,
				"allowed collection ticket user", MetaDataField.STRING,
				protocol));
		group
				.add(new MetaDataField(TICKET_USER_DOMAIN_C,
						"collection ticket user domain", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(TICKET_OWNER_C,
				"collection ticket creator", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_OWNER_DOMAIN_C,
				"collection ticket creator domain", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TICKET_BEGIN_TIME_C,
				"collection ticket validity start time", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TICKET_END_TIME_C,
				"collection ticket validity end time", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TICKET_ACC_COUNT_C,
				"number of opens allowed on collection ticket",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(TICKET_ACC_LIST_C,
				"access allowed on collection ticket", MetaDataField.STRING,
				protocol));

		group.add(new MetaDataField(TICKET_OWNER_ZONE_D,
				"zone for data ticket owner", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_USER_ZONE_D,
				"zone for data ticket user", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_OWNER_ZONE_C,
				"zone for coll ticket owner", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TICKET_USER_ZONE_C,
				"zone for coll ticket user", MetaDataField.STRING, protocol));
		add(group);

		// Container
		group = new MetaDataGroup(GROUP_CONTAINER,
				"Core meta-information about containers");
		group.add(new MetaDataField(CONTAINER_NAME,
				"name of container is a dataname", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(CONTAINER_REPLICATION_ENUM,
				"container copy number", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CONTAINER_MAX_SIZE,
				"maximum size of container", MetaDataField.INT, protocol));
		group.add(new MetaDataField(CONTAINER_SIZE,
				"current size of container", MetaDataField.INT, protocol));
		group.add(new MetaDataField(CONTAINER_LOG_RESOURCE_NAME,
				"logical resource associated with container",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CONTAINER_RESOURCE_NAME,
				"container resource name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CONTAINER_RESOURCE_CLASS,
				"class of physical resource of container",
				MetaDataField.STRING, protocol));
		add(group);

		// Annotator
		group = new MetaDataGroup(GROUP_ANNOTATIONS,
				"core meta-information on " + "annotating datasets. see also "
						+ ACCESS_CONSTRAINT + " attribute for access control");
		group.add(new MetaDataField(DIRECTORY_NAME, "collection name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_NAME, "data name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ANNOTATION_USERNAME,
				"name of annotator", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ANNOTATION_USERDOMAIN,
				"domain of annotator", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ANNOTATION, "annotation on data",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ANNOTATION_TIMESTAMP,
				"time of annotation", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FILE_ANNOTATION_POSITION,
				"location of the annotation", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ANNOTATION_USERNAME,
				"name of annotator", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ANNOTATION_USERDOMAIN,
				"domain of annotator", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ANNOTATION, "annotation on data",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ANNOTATION_TIMESTAMP,
				"time of annotation", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DIRECTORY_ANNOTATION_TYPE,
				"type for the annotation", MetaDataField.STRING, protocol));
		add(group);

		// Compound resource
		group = new MetaDataGroup(GROUP_COMPOUND_RESOURCE, "");
		group.add(new MetaDataField(COMPOUND_RESOURCE_NAME,
				"compound resource name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TAPE_NUMBER, "compound resource tape id",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(TAPE_OWNER,
				"compound resource tape owner id", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TAPE_TYPE_VAL,
				"compound resource tape type", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TAPE_LIBINX, "compound resource libinx",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(TAPE_FULLFLAG,
				"compound resource tape full flag", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(TAPE_CURRENT_FILE_SEQNUM,
				"file index in tape", MetaDataField.INT, protocol));
		group.add(new MetaDataField(TAPE_CURRENT_ABS_POSITION,
				"file position in tape", MetaDataField.INT, protocol));
		group.add(new MetaDataField(TAPE_BYTES_WRITTEN, "tape used space",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(INT_REPLICATION_ENUM,
				"replica number of file in compound resource",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(INT_SEG_NUM,
				"segment number of file in compound resource",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(INT_PATH_NAME,
				"path name of file in compound resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(INT_IS_DIRTY,
				"dirty flag of file in compound resource", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(INT_OFFSET,
				"offset of file in compound resource container",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(INT_SIZE,
				"size of file in compound resource", MetaDataField.INT,
				protocol));
		group.add(new MetaDataField(INT_PATH_NAME_FOR_REPL,
				"path of file in compound resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(INT_RESOURCE_NAME,
				"resource name of file in compound resource",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(INT_RESOURCE_ADDRESS_NETPREFIX,
				"net address of compound resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(INT_RESOURCE_TYPE_NAME,
				"type of resource in compound resource", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(INT_RESOURCE_CLASS,
				"class of resource in compound resource", MetaDataField.STRING,
				protocol));
		group
				.add(new MetaDataField(COMP_OBJ_USER_NAME,
						"user name of compound object", MetaDataField.STRING,
						protocol));
		group
				.add(new MetaDataField(COMP_OBJ_USER_DOMAIN,
						"user domn of compound object", MetaDataField.STRING,
						protocol));
		add(group);

		// Dublin Core
		group = new MetaDataGroup(
				GROUP_DUBLIN_CORE,
				"This set of metadata, even "
						+ "though part of MCAT core, is normally turned off in order to speed up "
						+ "processing. patches ned to be applied if this option needs to be used. "
						+ "For more information please check http://www.dublincore.org/");
		group.add(new MetaDataField(DC_DATA_NAME,
				"DC: Data Name same as data name", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DC_COLLECTION,
				"DC: Collection Name same as collection name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_TYPE,
				"DC: Contributor Type:eg.Author", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DC_SUBJECT_CLASS,
				"DC: Subject Classification", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_DESCRIPTION_TYPE,
				"DC: Type of Description", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_TYPE, "DC: Type of the Object",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_SOURCE_TYPE, "DC: Type of the Source",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_LANGUAGE, "DC: Language of the Object",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_RELATION_TYPE,
				"DC: Relation with another Object", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DC_COVERAGE_TYPE, "DC: Coverage Type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_RIGHTS_TYPE, "DC: Rights Type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_TITLE, "DC: Title of the Object",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_NAME,
				"DC: Contributor Name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_SUBJECT_NAME, "DC: Subject",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_DESCRIPTION, "DC: Description",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_PUBLISHER, "DC: Publisher Name",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_SOURCE, "DC: Source Name",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(DC_RELATED_DATA_DESCRIPTION,
						"DC: Related Data Description", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(DC_RELATED_DATA, "DC: Date Related to ("
				+ DC_DATA_NAME + "," + DC_COLLECTION + ")",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_RELATED_DIRECTORY,
				"DC: Collection related to (" + DC_DATA_NAME + ","
						+ DC_COLLECTION + ")", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_COVERAGE, "DC: Coverage Information",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_RIGHTS, "DC: Rights Information",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_PUBLISHER_ADDR, "DC: Publisher Address",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_ADDR,
				"DC: Contributor Address", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_EMAIL,
				"DC: Contributor Email", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_PHONE,
				"DC: Contributor Phone", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_WEB,
				"DC: Contributor Web Address", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DC_CONTRIBUTOR_AFFILIATION,
				"DC: Contributor Affiliation", MetaDataField.STRING, protocol));
		add(group);

		// Zone
		group = new MetaDataGroup(GROUP_ZONE, "Name of MCAT Zone");
		/** Name of MCAT Zone (char*) */
		group.add(new MetaDataField(ZONE_NAME, "Name of MCAT Zone (String)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ZONE_LOCALITY,
				"set to 1 for local zone, 0 otherwise", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(ZONE_NETPREFIX,
				"address where (remote) MCAT is", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(ZONE_PORT_NUM,
				"port num to reach (remote) MCAT", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(ZONE_ADMIN_AUTH_SCHEME_NAME,
				"admin's auth scheme of rem MCAT", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(ZONE_ADMIN_DISTIN_NAME,
				"DN str  of rem MCAT", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ZONE_STATUS, "1 for a valid zone",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ZONE_CREATE_DATE,
				"when the zone was created", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ZONE_MODIFY_DATE,
				"when the zone was modified", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(ZONE_COMMENTS,
				"any comments about the zone", MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(ZONE_CONTACT,
						"who is in charge of the zone", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(ZONE_ADMIN_NAME, "zone admin's username",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(ZONE_ADMIN_DOMAIN_NAME,
						"zone admin's user domain name", MetaDataField.STRING,
						protocol));
		group.add(new MetaDataField(ZONE_LOCN_DESC, "location desc for zone",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(CURRENT_ZONE, "current/working zone",
				MetaDataField.STRING, protocol));
		add(group);

		// User-defined string metadata for data
		group = new MetaDataGroup(
				GROUP_UDMD,
				"The special catagory of metadata known as user definable metadata. "
						+ "By using the MetaDataTable, the user can define their own "
						+ "key-pair relationships. Creating in some sense their own new metadata "
						+ "attributes. These definable metadata can be attached to the various "
						+ "kinds of system objects, eg. files/datasets, directories/collections, "
						+ "user, resources.");

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES,
				"definable metadata table for files", MetaDataField.TABLE,
				protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES,
				"definable metadata table for directories",
				MetaDataField.TABLE, protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS,
				"definable metadata table for users", MetaDataField.TABLE,
				protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES,
				"definable metadata table for resources", MetaDataField.TABLE,
				protocol));

		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_FILES0,
						"The first column of the definable "
								+ "metadata table for files. The standard usage keeps attribute names of "
								+ "the user defined metadata in this column. This field can be used in "
								+ "conjuction with the metadata number, which specifies a row in the table,"
								+ " to refer to a specific cell of the definable metadata table for files.",
						MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES1,
				"The  column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES2,
				"The third column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES3,
				"The fourth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES4,
				"The fifth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES5,
				"The sixth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_FILES6,
						"The seventh column of the definable metadata table for users.",
						MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES7,
				"The eigth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES8,
				"The ninth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES9,
				"The tenth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(INTEGER_DEFINABLE_METADATA0,
				"The first column of the user-defined integer metadata",
				MetaDataField.INT, protocol));
		group.add(new MetaDataField(INTEGER_DEFINABLE_METADATA1,
				"The second column of the user-defined integer metadata",
				MetaDataField.INT, protocol));

		group.add(new MetaDataField(METADATA_NUM,
				"Used to specify a certain row in the definable metadata "
						+ "table for files.", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES0_0,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES0_1,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES0_2,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES0_3,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES0_4,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES1_0,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES1_1,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES1_2,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES1_3,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_FILES1_4,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		// end definable files

		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES0,
						"The first column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES1,
						"The second column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES2,
						"The third column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES3,
						"The fourth column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES4,
						"The fifth column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES5,
						"The sixth column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES6,
						"The seventh column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES7,
						"The eigth column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES8,
						"The ninth column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_DIRECTORIES9,
						"The tenth column of the definable metadata table for directories.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						METADATA_NUM_DIRECTORY,
						"Used to specify a certain row in the definable metadata table for directories.",
						MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES0_0,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES0_1,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES0_2,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES0_3,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES0_4,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES1_0,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES1_1,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES1_2,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES1_3,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_DIRECTORIES1_4,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		// end definable directories

		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES0,
						"The first column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES1,
						"The second column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES2,
						"The third column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES3,
						"The fourth column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES4,
						"The fifth column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES5,
						"The sixth column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES6,
						"The seventh column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES7,
						"The eigth column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES8,
						"The ninth column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_RESOURCES9,
						"The tenth column of the definable metadata table for resources.",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						METADATA_NUM_RESOURCE,
						"Used to specify a certain row in the definable metadata table for resources.",
						MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES0_0,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES0_1,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES0_2,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES0_3,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES1_0,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES1_1,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES1_2,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_RESOURCES1_3,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		// end definable resources

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS0,
				"The first column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS1,
				"The second column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS2,
				"The third column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS3,
				"The fourth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS4,
				"The fifth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS5,
				"The sixth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						DEFINABLE_METADATA_FOR_USERS6,
						"The seventh column of the definable metadata table for users.",
						MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS7,
				"The eigth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS8,
				"The ninth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS9,
				"The tenth column of the definable metadata table for users.",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						METADATA_NUM_USER,
						"Used to specify a certain row in the definable metadata table for users.",
						MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS0_0,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS0_1,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS0_2,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS0_3,
				"Used for comparator queuries of the metadata, user attribute",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS1_0,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS1_1,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS1_2,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DEFINABLE_METADATA_FOR_USERS1_3,
				"Used for comparator queuries of the metadata, user value",
				MetaDataField.STRING, protocol));
		// end definable users
		add(group);

		group = new MetaDataGroup(
				GROUP_INDEX,
				"The user can index a dataset , "
						+ "datasets of given type or datasets in a collection. the index is "
						+ "treated as a SRB registered dataset. The user can download the index "
						+ "and search on it. The location can be collection-information (i.e., "
						+ "index is stored as several datasets inside a collection, or can be a "
						+ "URL!. Note that index is treated as a SRB registered dataset and hence "
						+ "inherits all meta information about datasets including structured "
						+ "metadata which can be used to store information about the index. see "
						+ "datacutter proxy for more information");
		group.add(new MetaDataField(INDEX_NAME_FOR_FILE,
				"data name of index on data", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(IX_DIRECTORY_NAME_FOR_FILE,
				"collection name of index on data (FILE_TYPE_NAME)",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(IX_DATATYPE_FOR_FILE, "index type for "
				+ INDEX_NAME_FOR_FILE, MetaDataField.STRING, protocol));
		group.add(new MetaDataField(IX_LOCATION_FOR_FILE,
				"path name of index in " + INDEX_NAME_FOR_FILE,
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(INDEX_NAME_FOR_DATATYPE,
				"data name of index on data type", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(IX_DIRECTORY_NAME_FOR_DATATYPE,
				"collection name of index on data type "
						+ INDEX_NAME_FOR_DATATYPE, MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(IX_DATATYPE_FOR_DATATYPE, "index type for "
				+ INDEX_NAME_FOR_DATATYPE, MetaDataField.STRING, protocol));
		group.add(new MetaDataField(IX_LOCATION_FOR_DATATYPE,
				"path name of index in " + INDEX_NAME_FOR_DATATYPE,
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(INDEX_NAME_FOR_DIRECTORY,
				"data name of index on collection", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(IX_DIRECTORY_NAME_FOR_DIRECTORY,
				"collection name of index on collection "
						+ INDEX_NAME_FOR_DIRECTORY, MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(IX_DATATYPE_FOR_DIRECTORY,
				"index type for " + INDEX_NAME_FOR_DIRECTORY,
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(IX_LOCATION_FOR_DIRECTORY,
				"path name of index in " + INDEX_NAME_FOR_DIRECTORY,
				MetaDataField.STRING, protocol));
		add(group);

		group = new MetaDataGroup(GROUP_STRUCTURED_METADATA,
				"The user can store structured (treated as a blob) metadata information.");
		group
				.add(new MetaDataField(
						STRUCTURED_METADATA_TYPE,
						"type of user-inserted structured metadata for data in (STRUCTURED_METADATA_FILE_NAME) or (INTERNAL_STRUCTURED_METADATA)",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						STRUCTURED_METADATA_COMMENTS,
						"comments on the structured metadata in (STRUCTURED_METADATA_FILE_NAME) or (INTERNAL_STRUCTURED_METADATA)",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						STRUCTURED_METADATA_FILE_NAME,
						"data name of user-inserted structured metadata stored as another data object inside SRB see also (STRUCTURED_METADATA_DIRECTORY_NAME)",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						INTERNAL_STRUCTURED_METADATA,
						"user-inserted structured metadata stored as a string inside MCAT",
						MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						STRUCTURED_METADATA_DIRECTORY_NAME,
						"collection name of user-inserted structured metadata stored as another data object inside SRB see also (92)",
						MetaDataField.STRING, protocol));
		add(group);

		group = new MetaDataGroup(
				GROUP_METHOD,
				"users can associate methods on "
						+ "dataset , datasets of given type or datasets in a collection. the "
						+ "method is treated as a SRB registered dataset and hence inherits all "
						+ "meta information about datasets including structured metadata which "
						+ "can be used to store information about the arguments and method "
						+ "return values. see datacutter proxy for more information.");
		group.add(new MetaDataField(METHOD_NAME_FOR_FILE,
				"data name of method on data", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(METHOD_NAME_FOR_DATATYPE,
				"data name of method on data type", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(METHOD_NAME_FOR_DIRECTORY,
				"data name of method on collection", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(METHOD_DIRECTORY_NAME_FOR_FILE,
				"collection name of method on data  (" + METHOD_NAME_FOR_FILE
						+ ")", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(METHOD_DIRECTORY_NAME_FOR_DATATYPE,
				"collection name of method on data type ("
						+ METHOD_NAME_FOR_DATATYPE + ")", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(METHOD_DIRECTORY_NAME_FOR_DIRECTORY,
				"collection name of method on collection ("
						+ METHOD_NAME_FOR_DIRECTORY + ")",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(METHOD_DATATYPE_FOR_FILE,
				"method type for (" + METHOD_NAME_FOR_FILE + ")",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(METHOD_DATATYPE_FOR_DATATYPE,
				"method type for (" + METHOD_NAME_FOR_DATATYPE + ")",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(METHOD_DATATYPE_FOR_DIRECTORY,
				"method type for (" + METHOD_NAME_FOR_DIRECTORY + ")",
				MetaDataField.STRING, protocol));
		add(group);

		group = new MetaDataGroup(GROUP_GUID,
				"Metadata attributes relating to GUIDs.");
		group.add(new MetaDataField(GUID, "guid", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(GUID_FLAG, "guid flag",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(GUID_TIME_STAMP, "guid timestamp",
				MetaDataField.STRING, protocol));
		add(group);

		group = new MetaDataGroup(GROUP_UNDEFINED,
				"Metadata attributes that have not yet been added to a group.");
		group.add(new MetaDataField(UDIMD_USER1,
				"change MAX_USER_INTEGER_METADATA", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(USER_UDEF_MDATA_MODIFY_DATE,
				"user-def metadata for user mod", MetaDataField.STRING,
				protocol));

		group.add(new MetaDataField(UDIMD_RSRC1,
				"change MAX_RSRC_INTEGER_METADATA", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(RSRC_UDEF_MDATA_MODIFY_DATE,
				"user-def metadata for resource mod", MetaDataField.STRING,
				protocol));

		group.add(new MetaDataField(USER_GROUP_MODIFY_DATE,
				"group user info modify date", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_GROUP_ZONE_NAME,
				"group user zone name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_GROUP_DOMAIN_DESC,
				"group user domain name", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_UDEF_MDATA_MODIFY_DATE,
				"user-def metadata for data mod", MetaDataField.STRING,
				protocol));

		group.add(new MetaDataField(COLL_UDEF_MDATA_MODIFY_DATE,
				"user-def metadata for coll mod", MetaDataField.STRING,
				protocol));

		group.add(new MetaDataField(DATA_TYPE_MIME_STRING,
				"mime string for data type", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DATA_TYPE_EXTENDERS,
				"extender list for data type", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(USER_CREATE_DATE,
				"user creation timestamp", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_MODIFY_DATE, "user modify timestamp",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_OWNER_ZONE, "zone for data owner",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_ANNOTATION_USER_ZONE,
				"zone for data annotator", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(AUDIT_USER_ZONE, "zone for audited suer",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(COLL_OWNER_ZONE,
				"zone for collection owner", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_LOCK_OWNER_ZONE,
				"zone for data lock owner", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(COLL_ANNOTATION_USER_ZONE,
				"zone for coll annotator", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_PIN_OWNER_ZONE,
				"zone for data pin owner", MetaDataField.STRING, protocol));

		group
				.add(new MetaDataField(COMP_OBJ_USER_ZONE,
						"zone for composite obj owner", MetaDataField.STRING,
						protocol));

		group.add(new MetaDataField(PARENT_SERVER_LOCATION,
				"parent of a location", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PARENT_DOMAIN_DESC, "parent of a domain",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PARENT_USER_TYPE, "parent of a user type",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PARENT_RSRC_TYPE,
				"parent of a resource type", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(PARENT_DATA_TYPE, "parent of a data type",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(ZONE_LOCN_DESC, "location desc for zone",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(COLL_MODIFY_TIMESTAMP,
				"collection modify_timestamp", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_ACCS_GRPUSER_NAME,
				"group user who has access to data", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DATA_ACCS_GRPUSER_DOMAIN,
				"data group user domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(DATA_ACCS_GRPUSER_ZONE,
				"data group user zone", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(COLL_ACCS_GRPUSER_NAME,
				"group user who has access to coll", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(COLL_ACCS_GRPUSER_DOMAIN,
				"coll group user domain", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(COLL_ACCS_GRPUSER_ZONE,
				"coll group user zone", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(COLL_ACCS_COLLECTION_NAME,
				"coll name for finding coll access", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(COLL_ACCS_ACCESS_CONSTRAINT,
				"accs cons for finding coll access", MetaDataField.STRING,
				protocol));

		group.add(new MetaDataField(DATA_TYPE_FOR_CONTAINER_FOR_COLLECTION,
				"data type for container associated with collection",
				MetaDataField.STRING, protocol));

		group.add(new MetaDataField(DATA_UDEF_MDATA_CREATE_DATE,
				"DATA_UDEF_MDATA_CREATE_DATE", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(COLL_UDEF_MDATA_CREATE_DATE,
				"COLL_UDEF_MDATA_CREATE_DATE", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RSRC_UDEF_MDATA_CREATE_DATE,
				"RSRC_UDEF_MDATA_CREATE_DATE", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(USER_UDEF_MDATA_CREATE_DATE,
				"USER_UDEF_MDATA_CREATE_DATE", MetaDataField.STRING, protocol));

		group.add(new MetaDataField(CONTAINER_ID, "container id",
				MetaDataField.STRING, protocol));
		group
				.add(new MetaDataField(
						ORDERBY,
						"return the query values sorted according to another "
								+ "attribute. Used in the condition as a comma seperated list: "
								+ "ORDER_BY = FILE_TYPE_NAME, FILE_NAME",
						MetaDataField.STRING, protocol));

		// SRB3.1
		group.add(new MetaDataField(DELETE_FILE_ID,
				"data_id of deleted data object", MetaDataField.INT, protocol));
		group
				.add(new MetaDataField(DELETE_FILE_REPLICATION_ENUM,
						"repl_enum of deleted data object", MetaDataField.INT,
						protocol));
		group.add(new MetaDataField(DELETE_FILE_NAME,
				"data_name of deleted data object", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DELETE_MODIFICATION_DATE,
				"timestamp of deleted data object", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DELETE_DIRECTORY_NAME,
				"collection_name of deleted data object", MetaDataField.STRING,
				protocol));

		// SRB3.3.1
		group.add(new MetaDataField(DEL_COLLECTION_ID,
				"collection_id of deleted data object", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(DEL_DATA_COMMENT,
				"comments on deleted data object", MetaDataField.STRING,
				protocol));
		group.add(new MetaDataField(FREE_SPACE, "free space",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(FS_TIMESTAMP, "fs timestamp",
				MetaDataField.STRING, protocol));
		group.add(new MetaDataField(RESOURCE_STATUS, "resource status",
				MetaDataField.STRING, protocol));
		add(group);

		// SRB3.3.1
		group = new MetaDataGroup(GROUP_EXTENSIBLE, "Extensible metadata");
		group.add(new MetaDataField(EXTENSIBLE_SCHEMA_NAME,
				"extensible schema name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(EXTENSIBLE_TABLE_NAME,
				"extensible table name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(EXTENSIBLE_ATTR_NAME,
				"extensible attribute name", MetaDataField.STRING, protocol));
		group.add(new MetaDataField(EXTENSIBLE_ATTR_OUTSIDE_NAME,
				"extensible attribute outside name", MetaDataField.STRING,
				protocol));
		group
				.add(new MetaDataField(EXTENSIBLE_ATTR_COMMENTS,
						"extensible attribute comments", MetaDataField.STRING,
						protocol));
		add(group);

		/**
		 * The tableName, attributeNames, and catalogNames arrays are a
		 * consequence of the SRB return value. A tableName and attributeName
		 * will be returned by the SRB for every catalogName. No single
		 * tableName or attributeName is unique, so the combination of the two
		 * is used to find the catalogName. The catalogName corresponds to one
		 * of the public static final metadata names, e.g. FILE_NAME,
		 * COMPOUND_RESOURCE_NAME, USER_EMAIL.
		 */
		srbToJargon.put("DC_AUTHCONTRIB" + nll + "dc_contr_address" + nll,
				catalogNames[175]);
		srbToJargon.put("DC_AUTHCONTRIB" + nll + "dc_contr_corp_name" + nll,
				catalogNames[179]);
		srbToJargon.put("DC_AUTHCONTRIB" + nll + "dc_contr_email" + nll,
				catalogNames[176]);
		srbToJargon.put("DC_AUTHCONTRIB" + nll + "dc_contr_name" + nll,
				catalogNames[164]);
		srbToJargon.put("DC_AUTHCONTRIB" + nll + "dc_contr_phone" + nll,
				catalogNames[177]);
		srbToJargon.put("DC_AUTHCONTRIB" + nll + "dc_contr_web" + nll,
				catalogNames[178]);
		srbToJargon.put("DC_COLLECTION" + nll + "data_grp_name" + nll,
				catalogNames[153]);
		srbToJargon.put("DC_COVERAGE" + nll + "coverage_desc" + nll,
				catalogNames[172]);
		srbToJargon.put("DC_DATA" + nll + "data_name" + nll, catalogNames[152]);
		srbToJargon.put("DC_DESCRIPTION" + nll + "dc_desc" + nll,
				catalogNames[166]);
		srbToJargon.put("DC_PUBLISHER" + nll + "dc_pub_addr" + nll,
				catalogNames[174]);
		srbToJargon.put("DC_PUBLISHER" + nll + "dc_pub_name" + nll,
				catalogNames[167]);
		srbToJargon.put("DC_RELATION" + nll + "related_data_desc" + nll,
				catalogNames[169]);
		srbToJargon.put("DC_RIGHTS" + nll + "rights_data_desc" + nll,
				catalogNames[173]);
		srbToJargon.put("DC_SOURCE" + nll + "source_desc" + nll,
				catalogNames[168]);
		srbToJargon.put("DC_SUBJECT" + nll + "dc_subject_name" + nll,
				catalogNames[165]);
		srbToJargon.put("DC_TD_CONTR_TYPE" + nll + "dc_contr_type_name" + nll,
				catalogNames[154]);
		srbToJargon.put("DC_TD_COVERAGE" + nll + "coverage_type_name" + nll,
				catalogNames[161]);
		srbToJargon.put("DC_TD_DESCR_TYPE" + nll + "dc_desc_type_name" + nll,
				catalogNames[156]);
		srbToJargon.put("DC_TD_LANGUAGE" + nll + "language_name" + nll,
				catalogNames[159]);
		srbToJargon.put(
				"DC_TD_RELATION_TYP" + nll + "relation_type_name" + nll,
				catalogNames[160]);
		srbToJargon.put("DC_TD_RIGHTS" + nll + "rights_type_name" + nll,
				catalogNames[162]);
		srbToJargon.put("DC_TD_SOURCE_TYPE" + nll + "source_type_name" + nll,
				catalogNames[158]);
		srbToJargon.put("DC_TD_SUBJ_CLASS" + nll + "dc_subj_class_name" + nll,
				catalogNames[155]);
		srbToJargon.put("DC_TD_TYPE" + nll + "dc_type_name" + nll,
				catalogNames[157]);
		srbToJargon.put("DC_TITLE" + nll + "dc_title" + nll, catalogNames[163]);
		srbToJargon.put("MDAS_AC_ANNOTATION" + nll + "anno_date" + nll,
				catalogNames[233]);
		srbToJargon.put("MDAS_AC_ANNOTATION" + nll + "anno_type" + nll,
				catalogNames[234]);
		srbToJargon.put("MDAS_AC_ANNOTATION" + nll + "annotations" + nll,
				catalogNames[232]);
		srbToJargon.put("MDAS_ADCONT_REPL" + nll + "data_size" + nll,
				catalogNames[64]);
		srbToJargon.put("MDAS_ADC_REPL" + nll + "data_name" + nll,
				catalogNames[281]);
		srbToJargon.put("MDAS_ADC_REPL" + nll + "path_name" + nll,
				catalogNames[10]);
		srbToJargon.put("MDAS_ADC_REPL" + nll + "repl_enum" + nll,
				catalogNames[1]);
		srbToJargon.put("MDAS_ADC_REPL" + nll + "seg_num" + nll,
				catalogNames[265]);
		srbToJargon.put("MDAS_ADIXCOLL_REPL" + nll + "data_name" + nll,
				catalogNames[96]);
		srbToJargon.put("MDAS_ADIXCOLL_REPL" + nll + "path_name" + nll,
				catalogNames[115]);
		srbToJargon.put("MDAS_ADIXDS_REPL" + nll + "data_name" + nll,
				catalogNames[94]);
		srbToJargon.put("MDAS_ADIXDS_REPL" + nll + "path_name" + nll,
				catalogNames[113]);
		srbToJargon.put("MDAS_ADIXDTP_REPL" + nll + "data_name" + nll,
				catalogNames[95]);
		srbToJargon.put("MDAS_ADIXDTP_REPL" + nll + "path_name" + nll,
				catalogNames[114]);
		srbToJargon.put("MDAS_ADMTHCOL_REPL" + nll + "data_name" + nll,
				catalogNames[99]);
		srbToJargon.put("MDAS_ADMTHDS_REPL" + nll + "data_name" + nll,
				catalogNames[97]);
		srbToJargon.put("MDAS_ADMTHDTP_REPL" + nll + "data_name" + nll,
				catalogNames[98]);
		srbToJargon.put("MDAS_ADSTRUCT_REPL" + nll + "data_name" + nll,
				catalogNames[92]);
		srbToJargon.put("MDAS_AD_1COLLMDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[192]);
		srbToJargon.put("MDAS_AD_1COLLMDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[196]);
		srbToJargon.put("MDAS_AD_1COMPOUND" + nll + "cmpd_path_name" + nll,
				catalogNames[278]);
		srbToJargon.put("MDAS_AD_1MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[184]);
		srbToJargon.put("MDAS_AD_1MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[188]);
		srbToJargon.put("MDAS_AD_2COLLMDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[193]);
		srbToJargon.put("MDAS_AD_2COLLMDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[197]);
		srbToJargon.put("MDAS_AD_2MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[185]);
		srbToJargon.put("MDAS_AD_2MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[189]);
		srbToJargon.put("MDAS_AD_3COLLMDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[194]);
		srbToJargon.put("MDAS_AD_3COLLMDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[198]);
		srbToJargon.put("MDAS_AD_3MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[186]);
		srbToJargon.put("MDAS_AD_3MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[190]);
		srbToJargon.put("MDAS_AD_4COLLMDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[195]);
		srbToJargon.put("MDAS_AD_4COLLMDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[199]);
		srbToJargon.put("MDAS_AD_4MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[187]);
		srbToJargon.put("MDAS_AD_4MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[191]);
		srbToJargon.put("MDAS_AD_ACCS" + nll + "access_id" + nll,
				catalogNames[182]);
		srbToJargon.put("MDAS_AD_ANNOTATION" + nll + "anno_date" + nll,
				catalogNames[75]);
		srbToJargon.put("MDAS_AD_ANNOTATION" + nll + "anno_position" + nll,
				catalogNames[78]);
		srbToJargon.put("MDAS_AD_ANNOTATION" + nll + "annotations" + nll,
				catalogNames[74]);
		srbToJargon.put("MDAS_AD_AUDIT" + nll + "comments" + nll,
				catalogNames[24]);
		srbToJargon.put("MDAS_AD_AUDIT" + nll + "time_stamp" + nll,
				catalogNames[23]);
		srbToJargon.put("MDAS_AD_CLASS" + nll + "class_name" + nll,
				catalogNames[88]);
		srbToJargon.put("MDAS_AD_CLASS" + nll + "class_type" + nll,
				catalogNames[89]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "metadatanum" + nll,
				catalogNames[133]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_creat_date" + nll,
				catalogNames[378]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metaint0" + nll,
				catalogNames[144]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metaint1" + nll,
				catalogNames[145]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[134]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[135]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr2" + nll,
				catalogNames[136]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr3" + nll,
				catalogNames[137]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr4" + nll,
				catalogNames[138]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr5" + nll,
				catalogNames[139]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr6" + nll,
				catalogNames[140]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr7" + nll,
				catalogNames[141]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr8" + nll,
				catalogNames[142]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_metastr9" + nll,
				catalogNames[143]);
		srbToJargon.put("MDAS_AD_COLLMDATA" + nll + "userdef_modif_date" + nll,
				catalogNames[321]);
		srbToJargon.put("MDAS_AD_COMMENTS" + nll + "com_date" + nll,
				catalogNames[77]);
		srbToJargon.put("MDAS_AD_COMPOUND" + nll + "cmpd_path_name" + nll,
				catalogNames[268]);
		srbToJargon.put("MDAS_AD_COMPOUND" + nll + "data_size" + nll,
				catalogNames[275]);
		srbToJargon.put("MDAS_AD_COMPOUND" + nll + "int_repl_num" + nll,
				catalogNames[266]);
		srbToJargon.put("MDAS_AD_COMPOUND" + nll + "int_seg_num" + nll,
				catalogNames[267]);
		srbToJargon.put("MDAS_AD_COMPOUND" + nll + "is_dirty" + nll,
				catalogNames[272]);
		srbToJargon.put("MDAS_AD_COMPOUND" + nll + "offset" + nll,
				catalogNames[274]);
		srbToJargon.put("MDAS_AD_DCRELREPL" + nll + "data_name" + nll,
				catalogNames[170]);
		srbToJargon.put("MDAS_AD_DEL_REPL" + nll + "d_comment" + nll,
				catalogNames[392]);
		srbToJargon.put("MDAS_AD_DEL_REPL" + nll + "data_grp_id" + nll,
				catalogNames[391]);
		srbToJargon.put("MDAS_AD_DEL_REPL" + nll + "data_id" + nll,
				catalogNames[386]);
		srbToJargon.put("MDAS_AD_DEL_REPL" + nll + "data_name" + nll,
				catalogNames[388]);
		srbToJargon.put("MDAS_AD_DEL_REPL" + nll + "repl_enum" + nll,
				catalogNames[387]);
		srbToJargon.put("MDAS_AD_DEL_REPL" + nll + "repl_timestamp" + nll,
				catalogNames[389]);
		srbToJargon.put("MDAS_AD_GRP_2ACCS" + nll + "access_id" + nll,
				catalogNames[180]);
		srbToJargon.put("MDAS_AD_GUID" + nll + "guid" + nll, catalogNames[382]);
		srbToJargon.put("MDAS_AD_GUID" + nll + "guid_flag" + nll,
				catalogNames[383]);
		srbToJargon.put("MDAS_AD_GUID" + nll + "guid_time_stamp" + nll,
				catalogNames[384]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "metadatanum" + nll,
				catalogNames[116]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_creat_date" + nll,
				catalogNames[377]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metaint0" + nll,
				catalogNames[127]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metaint1" + nll,
				catalogNames[128]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[117]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[118]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr2" + nll,
				catalogNames[119]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr3" + nll,
				catalogNames[120]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr4" + nll,
				catalogNames[121]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr5" + nll,
				catalogNames[122]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr6" + nll,
				catalogNames[123]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr7" + nll,
				catalogNames[124]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr8" + nll,
				catalogNames[125]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_metastr9" + nll,
				catalogNames[126]);
		srbToJargon.put("MDAS_AD_MDATA" + nll + "userdef_modif_date" + nll,
				catalogNames[320]);
		srbToJargon.put("MDAS_AD_MISC1" + nll + "dcompressed" + nll,
				catalogNames[245]);
		srbToJargon.put("MDAS_AD_MISC1" + nll + "dencrypted" + nll,
				catalogNames[246]);
		srbToJargon.put("MDAS_AD_MISC1" + nll + "dexpire_date" + nll,
				catalogNames[244]);
		srbToJargon.put("MDAS_AD_MISC1" + nll + "dexpire_date_2" + nll,
				catalogNames[247]);
		srbToJargon
				.put("MDAS_AD_MISC1" + nll + "dpin" + nll, catalogNames[240]);
		srbToJargon.put("MDAS_AD_MISC1" + nll + "dpinexpiry" + nll,
				catalogNames[243]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "auditflag" + nll,
				catalogNames[238]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "container_id" + nll,
				catalogNames[381]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "d_comment" + nll,
				catalogNames[21]);
		srbToJargon
				.put("MDAS_AD_REPL" + nll + "data_id" + nll, catalogNames[0]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "data_name" + nll,
				catalogNames[2]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "data_size" + nll,
				catalogNames[18]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "datalock" + nll,
				catalogNames[222]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "dchecksum" + nll,
				catalogNames[237]);
		srbToJargon
				.put("MDAS_AD_REPL" + nll + "dhide" + nll, catalogNames[236]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "dlockexpiry" + nll,
				catalogNames[239]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "is_deleted" + nll,
				catalogNames[34]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "is_dirty" + nll,
				catalogNames[56]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "last_accs_time" + nll,
				catalogNames[148]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "modify_timestamp" + nll,
				catalogNames[14]);
		srbToJargon
				.put("MDAS_AD_REPL" + nll + "offset" + nll, catalogNames[61]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "repl_enum" + nll,
				catalogNames[71]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "repl_timestamp" + nll,
				catalogNames[76]);
		srbToJargon.put("MDAS_AD_REPL" + nll + "version_num" + nll,
				catalogNames[221]);
		srbToJargon.put(
				"MDAS_AD_STRCT_BLOB" + nll + "internal_structure" + nll,
				catalogNames[93]);
		srbToJargon.put(
				"MDAS_AD_STRCT_BLOB" + nll + "structure_comments" + nll,
				catalogNames[91]);
		srbToJargon.put("MDAS_AD_STRCT_BLOB" + nll + "structure_type" + nll,
				catalogNames[90]);
		srbToJargon.put("MDAS_AR_1MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[201]);
		srbToJargon.put("MDAS_AR_1MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[204]);
		srbToJargon.put("MDAS_AR_1PHYSICAL" + nll + "phy_rsrc_name" + nll,
				catalogNames[62]);
		srbToJargon.put("MDAS_AR_1REPL" + nll + "rsrc_name" + nll,
				catalogNames[65]);
		srbToJargon.put("MDAS_AR_2MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[202]);
		srbToJargon.put("MDAS_AR_2MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[205]);
		srbToJargon.put("MDAS_AR_2PHYSICAL" + nll + "phy_rsrc_name" + nll,
				catalogNames[264]);
		srbToJargon.put("MDAS_AR_3MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[203]);
		srbToJargon.put("MDAS_AR_3MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[206]);
		srbToJargon.put("MDAS_AR_3PHYSICAL" + nll + "phy_rsrc_name" + nll,
				catalogNames[269]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "concurrency_max" + nll,
				catalogNames[337]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "concurrency_max" + nll,
				catalogNames[83]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "description" + nll,
				catalogNames[87]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "mbps_bandwidth" + nll,
				catalogNames[336]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "mbps_bandwidth" + nll,
				catalogNames[82]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "megab_capacity" + nll,
				catalogNames[340]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "megab_capacity" + nll,
				catalogNames[86]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "mlsec_latency_max" + nll,
				catalogNames[334]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "mlsec_latency_max" + nll,
				catalogNames[80]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "mlsec_latency_min" + nll,
				catalogNames[335]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "mlsec_latency_min" + nll,
				catalogNames[81]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "num_of_hierarchies" + nll,
				catalogNames[338]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "num_of_hierarchies" + nll,
				catalogNames[84]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "num_of_stripes" + nll,
				catalogNames[339]);
		srbToJargon.put("MDAS_AR_INFO" + nll + "num_of_stripes" + nll,
				catalogNames[85]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "metadatanum" + nll,
				catalogNames[262]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_creat_date" + nll,
				catalogNames[379]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metaint1" + nll,
				catalogNames[261]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metaint1" + nll,
				catalogNames[316]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[251]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[252]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr2" + nll,
				catalogNames[253]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr3" + nll,
				catalogNames[254]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr4" + nll,
				catalogNames[255]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr5" + nll,
				catalogNames[256]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr6" + nll,
				catalogNames[257]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr7" + nll,
				catalogNames[258]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr8" + nll,
				catalogNames[259]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_metastr9" + nll,
				catalogNames[260]);
		srbToJargon.put("MDAS_AR_MDATA" + nll + "userdef_modif_date" + nll,
				catalogNames[323]);
		srbToJargon.put("MDAS_AR_PHYSICAL" + nll + "free_space" + nll,
				catalogNames[393]);
		srbToJargon.put("MDAS_AR_PHYSICAL" + nll + "fs_timestamp" + nll,
				catalogNames[394]);
		srbToJargon.put("MDAS_AR_PHYSICAL" + nll + "max_obj_size" + nll,
				catalogNames[60]);
		srbToJargon.put("MDAS_AR_PHYSICAL" + nll + "phy_default_path" + nll,
				catalogNames[28]);
		srbToJargon.put("MDAS_AR_PHYSICAL" + nll + "phy_rsrc_name" + nll,
				catalogNames[29]);
		srbToJargon.put("MDAS_AR_PHYSICAL" + nll + "rsrc_lock" + nll,
				catalogNames[400]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "default_path" + nll,
				catalogNames[27]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "max_obj_size" + nll,
				catalogNames[331]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "repl_enum" + nll,
				catalogNames[31]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "rsrc_comments" + nll,
				catalogNames[326]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "rsrc_creation_date" + nll,
				catalogNames[327]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "rsrc_modify_date" + nll,
				catalogNames[328]);
		srbToJargon.put("MDAS_AR_REPL" + nll + "rsrc_name" + nll,
				catalogNames[11]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "bytesWritten" + nll,
				catalogNames[220]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "currentAbsPosition" + nll,
				catalogNames[219]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "currentFileSeqNum" + nll,
				catalogNames[218]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "fullFlag" + nll,
				catalogNames[217]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "tapeLibInx" + nll,
				catalogNames[216]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "tapeNumber" + nll,
				catalogNames[213]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "tapeOwner" + nll,
				catalogNames[214]);
		srbToJargon.put("MDAS_AR_TAPE_INFO" + nll + "tapeType" + nll,
				catalogNames[215]);
		srbToJargon.put("MDAS_AT_DATA_TYP_EXT" + nll + "data_typ_ext" + nll,
				catalogNames[325]);
		srbToJargon.put("MDAS_AT_DATA_TYP_EXT" + nll + "data_typ_mime" + nll,
				catalogNames[324]);
		srbToJargon.put("MDAS_AUDIT" + nll + "aud_comments" + nll,
				catalogNames[250]);
		srbToJargon.put("MDAS_AUDIT" + nll + "aud_timestamp" + nll,
				catalogNames[249]);
		srbToJargon.put("MDAS_AUDIT_DESC" + nll + "actionDesc" + nll,
				catalogNames[248]);
		srbToJargon.put("MDAS_AU_1MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[207]);
		srbToJargon.put("MDAS_AU_1MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[210]);
		srbToJargon.put("MDAS_AU_2MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[208]);
		srbToJargon.put("MDAS_AU_2MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[211]);
		srbToJargon.put("MDAS_AU_3MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[209]);
		srbToJargon.put("MDAS_AU_3MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[212]);
		srbToJargon.put("MDAS_AU_AUDIT" + nll + "comments" + nll,
				catalogNames[20]);
		srbToJargon.put("MDAS_AU_AUDIT" + nll + "time_stamp" + nll,
				catalogNames[19]);
		srbToJargon.put("MDAS_AU_AUTH_MAP" + nll + "distin_name" + nll,
				catalogNames[67]);
		srbToJargon.put("MDAS_AU_INFO" + nll + "user_address" + nll,
				catalogNames[6]);
		srbToJargon.put("MDAS_AU_INFO" + nll + "user_email" + nll,
				catalogNames[17]);
		srbToJargon.put("MDAS_AU_INFO" + nll + "user_phone" + nll,
				catalogNames[16]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "metadatanum" + nll,
				catalogNames[263]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_creat_date" + nll,
				catalogNames[380]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metaint0" + nll,
				catalogNames[293]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metaint1" + nll,
				catalogNames[315]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr0" + nll,
				catalogNames[283]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr1" + nll,
				catalogNames[284]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr2" + nll,
				catalogNames[285]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr3" + nll,
				catalogNames[286]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr4" + nll,
				catalogNames[287]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr5" + nll,
				catalogNames[288]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr6" + nll,
				catalogNames[289]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr7" + nll,
				catalogNames[290]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr8" + nll,
				catalogNames[291]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_metastr9" + nll,
				catalogNames[292]);
		srbToJargon.put("MDAS_AU_MDATA" + nll + "userdef_modif_date" + nll,
				catalogNames[322]);
		srbToJargon.put("MDAS_AU_OWNER_INFO" + nll + "user_email" + nll,
				catalogNames[129]);
		srbToJargon.put("MDAS_AU_TCKT_DATA" + nll + "access_count" + nll,
				catalogNames[39]);
		srbToJargon.put("MDAS_AU_TCKT_DATA" + nll + "begin_time" + nll,
				catalogNames[37]);
		srbToJargon.put("MDAS_AU_TCKT_DATA" + nll + "end_time" + nll,
				catalogNames[38]);
		srbToJargon.put("MDAS_AU_TCKT_DATA" + nll + "ticket_id" + nll,
				catalogNames[36]);
		srbToJargon.put("MDAS_AU_TCKT_GRP" + nll + "access_count" + nll,
				catalogNames[46]);
		srbToJargon.put("MDAS_AU_TCKT_GRP" + nll + "begin_time" + nll,
				catalogNames[44]);
		srbToJargon.put("MDAS_AU_TCKT_GRP" + nll + "end_time" + nll,
				catalogNames[45]);
		srbToJargon.put("MDAS_AU_TCKT_GRP" + nll + "is_recursive" + nll,
				catalogNames[200]);
		srbToJargon.put("MDAS_AU_TCKT_GRP" + nll + "ticket_id" + nll,
				catalogNames[43]);
		srbToJargon.put("MDAS_AU_ZN_ATHMAP" + nll + "distin_name" + nll,
				catalogNames[305]);
		srbToJargon.put("MDAS_CD_ANNO_USER" + nll + "user_name" + nll,
				catalogNames[72]);
		srbToJargon.put("MDAS_CD_ANNO_USER" + nll + "zone_id" + nll,
				catalogNames[350]);
		srbToJargon.put("MDAS_CD_CAGRP_USER" + nll + "user_name" + nll,
				catalogNames[371]);
		srbToJargon.put("MDAS_CD_CAGRP_USER" + nll + "zone_id" + nll,
				catalogNames[373]);
		srbToJargon.put("MDAS_CD_CANNO_USER" + nll + "user_name" + nll,
				catalogNames[230]);
		srbToJargon.put("MDAS_CD_CANNO_USER" + nll + "user_name" + nll,
				catalogNames[276]);
		srbToJargon.put("MDAS_CD_CANNO_USER" + nll + "zone_id" + nll,
				catalogNames[354]);
		srbToJargon.put("MDAS_CD_CANNO_USER" + nll + "zone_id" + nll,
				catalogNames[356]);
		srbToJargon.put("MDAS_CD_COOWN_USER" + nll + "user_name" + nll,
				catalogNames[149]);
		srbToJargon.put("MDAS_CD_COOWN_USER" + nll + "zone_id" + nll,
				catalogNames[352]);
		srbToJargon.put("MDAS_CD_CTKTOWNER" + nll + "user_name" + nll,
				catalogNames[48]);
		srbToJargon.put("MDAS_CD_CTKTOWNER" + nll + "zone_id" + nll,
				catalogNames[348]);
		srbToJargon.put("MDAS_CD_CTKTUSER" + nll + "user_name" + nll,
				catalogNames[49]);
		srbToJargon.put("MDAS_CD_CTKTUSER" + nll + "zone_id" + nll,
				catalogNames[349]);
		srbToJargon.put("MDAS_CD_DAGRP_USER" + nll + "user_name" + nll,
				catalogNames[368]);
		srbToJargon.put("MDAS_CD_DAGRP_USER" + nll + "zone_id" + nll,
				catalogNames[370]);
		srbToJargon.put("MDAS_CD_DAUDT_USER" + nll + "user_name" + nll,
				catalogNames[130]);
		srbToJargon.put("MDAS_CD_DAUDT_USER" + nll + "zone_id" + nll,
				catalogNames[351]);
		srbToJargon.put("MDAS_CD_DLOWN_USER" + nll + "user_name" + nll,
				catalogNames[224]);
		srbToJargon.put("MDAS_CD_DLOWN_USER" + nll + "zone_id" + nll,
				catalogNames[353]);
		srbToJargon.put("MDAS_CD_DTKTOWNER" + nll + "user_name" + nll,
				catalogNames[41]);
		srbToJargon.put("MDAS_CD_DTKTOWNER" + nll + "zone_id" + nll,
				catalogNames[346]);
		srbToJargon.put("MDAS_CD_DTKTUSER" + nll + "user_name" + nll,
				catalogNames[42]);
		srbToJargon.put("MDAS_CD_DTKTUSER" + nll + "zone_id" + nll,
				catalogNames[347]);
		srbToJargon.put("MDAS_CD_OWNER_USER" + nll + "user_name" + nll,
				catalogNames[35]);
		srbToJargon.put("MDAS_CD_OWNER_USER" + nll + "zone_id" + nll,
				catalogNames[345]);
		srbToJargon.put("MDAS_CD_PAR_USER" + nll + "user_modify_date" + nll,
				catalogNames[317]);
		srbToJargon.put("MDAS_CD_PAR_USER" + nll + "user_name" + nll,
				catalogNames[3]);
		srbToJargon.put("MDAS_CD_PAR_USER" + nll + "zone_id" + nll,
				catalogNames[318]);
		srbToJargon.put("MDAS_CD_PIN_USER" + nll + "user_name" + nll,
				catalogNames[241]);
		srbToJargon.put("MDAS_CD_PIN_USER" + nll + "zone_id" + nll,
				catalogNames[355]);
		srbToJargon.put("MDAS_CD_RAGRP_USER" + nll + "user_name" + nll,
				catalogNames[364]);
		srbToJargon.put("MDAS_CD_RAGRP_USER" + nll + "zone_id" + nll,
				catalogNames[366]);
		srbToJargon.put("MDAS_CD_RSAC_USER" + nll + "user_name" + nll,
				catalogNames[341]);
		srbToJargon.put("MDAS_CD_RSAC_USER" + nll + "zone_id" + nll,
				catalogNames[343]);
		srbToJargon.put("MDAS_CD_RS_USER" + nll + "user_name" + nll,
				catalogNames[332]);
		srbToJargon.put("MDAS_CD_RS_USER" + nll + "zone_id" + nll,
				catalogNames[344]);
		srbToJargon.put("MDAS_CD_USER" + nll + "user_creation_date" + nll,
				catalogNames[329]);
		srbToJargon.put("MDAS_CD_USER" + nll + "user_id" + nll,
				catalogNames[147]);
		srbToJargon.put("MDAS_CD_USER" + nll + "user_modify_date" + nll,
				catalogNames[330]);
		srbToJargon.put("MDAS_CD_USER" + nll + "user_name" + nll,
				catalogNames[7]);
		srbToJargon.put("MDAS_CD_ZN_USER" + nll + "user_name" + nll,
				catalogNames[311]);
		srbToJargon.put("MDAS_TD_1RSRC_CLASS" + nll + "rsrc_class_name" + nll,
				catalogNames[63]);
		srbToJargon.put("MDAS_TD_2RSRC_CLASS" + nll + "rsrc_class_name" + nll,
				catalogNames[273]);
		srbToJargon.put("MDAS_TD_ACTN" + nll + "action_desc" + nll,
				catalogNames[22]);
		srbToJargon.put("MDAS_TD_ANNO_DOMN" + nll + "domain_desc" + nll,
				catalogNames[73]);
		srbToJargon.put("MDAS_TD_AUTH_SCHM" + nll + "auth_scheme" + nll,
				catalogNames[68]);
		srbToJargon.put("MDAS_TD_CADATA_GRP" + nll + "data_grp_name" + nll,
				catalogNames[374]);
		srbToJargon.put("MDAS_TD_CAGRP_ACCS" + nll + "access_constraint" + nll,
				catalogNames[375]);
		srbToJargon.put("MDAS_TD_CAGRP_DOMN" + nll + "domain_desc" + nll,
				catalogNames[372]);
		srbToJargon.put("MDAS_TD_CANNO_DOMN" + nll + "domain_desc" + nll,
				catalogNames[231]);
		srbToJargon.put("MDAS_TD_CANNO_DOMN" + nll + "domain_desc" + nll,
				catalogNames[277]);
		srbToJargon.put("MDAS_TD_CCDATA_TYP" + nll + "data_typ_name" + nll,
				catalogNames[376]);
		srbToJargon.put("MDAS_TD_COLLCONT" + nll + "container_name" + nll,
				catalogNames[132]);
		srbToJargon.put("MDAS_TD_CONTAINER" + nll + "container_max_size" + nll,
				catalogNames[57]);
		srbToJargon.put("MDAS_TD_CONTAINER" + nll + "container_name" + nll,
				catalogNames[58]);
		srbToJargon.put("MDAS_TD_COOWN_DOMN" + nll + "domain_desc" + nll,
				catalogNames[226]);
		srbToJargon.put("MDAS_TD_CTKTO_DOMN" + nll + "domain_desc" + nll,
				catalogNames[52]);
		srbToJargon.put("MDAS_TD_CTKTU_DOMN" + nll + "domain_desc" + nll,
				catalogNames[50]);
		srbToJargon.put("MDAS_TD_DAGRP_DOMN" + nll + "domain_desc" + nll,
				catalogNames[369]);
		srbToJargon.put("MDAS_TD_DATA_2GRP" + nll + "data_grp_name" + nll,
				catalogNames[26]);
		srbToJargon.put("MDAS_TD_DATA_3GRP" + nll + "coll_comments" + nll,
				catalogNames[229]);
		srbToJargon.put("MDAS_TD_DATA_3GRP" + nll + "coll_cr_timestamp" + nll,
				catalogNames[228]);
		srbToJargon.put("MDAS_TD_DATA_3GRP" + nll + "data_grp_name" + nll,
				catalogNames[227]);
		srbToJargon.put("MDAS_TD_DATA_4GRP" + nll + "data_grp_name" + nll,
				catalogNames[280]);
		srbToJargon.put("MDAS_TD_DATA_5GRP" + nll + "data_grp_name" + nll,
				catalogNames[282]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "coll_comments" + nll,
				catalogNames[151]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "coll_cr_timestamp" + nll,
				catalogNames[150]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "coll_link" + nll,
				catalogNames[279]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "coll_mod_timestamp" + nll,
				catalogNames[367]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "data_grp_id" + nll,
				catalogNames[235]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "data_grp_name" + nll,
				catalogNames[15]);
		srbToJargon.put("MDAS_TD_DATA_GRP" + nll + "parent_grp_name" + nll,
				catalogNames[54]);
		srbToJargon.put("MDAS_TD_DATA_TYP" + nll + "data_typ_name" + nll,
				catalogNames[4]);
		srbToJargon.put("MDAS_TD_DAUDT_DOMN" + nll + "domain_desc" + nll,
				catalogNames[131]);
		srbToJargon.put("MDAS_TD_DCRELGRP" + nll + "data_grp_name" + nll,
				catalogNames[171]);
		srbToJargon.put("MDAS_TD_DEL_DATA_GRP" + nll + "data_grp_name" + nll,
				catalogNames[390]);
		srbToJargon.put("MDAS_TD_DGRP_IXCOL" + nll + "data_grp_name" + nll,
				catalogNames[102]);
		srbToJargon.put("MDAS_TD_DGRP_IXDS" + nll + "data_grp_name" + nll,
				catalogNames[100]);
		srbToJargon.put("MDAS_TD_DGRP_IXDTP" + nll + "data_grp_name" + nll,
				catalogNames[101]);
		srbToJargon.put("MDAS_TD_DGRP_MTCOL" + nll + "data_grp_name" + nll,
				catalogNames[105]);
		srbToJargon.put("MDAS_TD_DGRP_MTDTP" + nll + "data_grp_name" + nll,
				catalogNames[104]);
		srbToJargon.put("MDAS_TD_DGRP_MTHDS" + nll + "data_grp_name" + nll,
				catalogNames[103]);
		srbToJargon.put("MDAS_TD_DGRP_STRUC" + nll + "data_grp_name" + nll,
				catalogNames[112]);
		srbToJargon.put("MDAS_TD_DLOWN_DOMN" + nll + "domain_desc" + nll,
				catalogNames[225]);
		srbToJargon.put("MDAS_TD_DOMN" + nll + "domain_desc" + nll,
				catalogNames[9]);
		srbToJargon.put("MDAS_TD_DSTKT_ACCS" + nll + "access_constraint" + nll,
				catalogNames[40]);
		srbToJargon.put("MDAS_TD_DS_ACCS" + nll + "access_constraint" + nll,
				catalogNames[8]);
		srbToJargon.put("MDAS_TD_DS_ACCS" + nll + "access_id" + nll,
				catalogNames[146]);
		srbToJargon.put("MDAS_TD_DS_ACCS" + nll + "access_list" + nll,
				catalogNames[32]);
		srbToJargon.put("MDAS_TD_DS_ACCS" + nll + "access_privilege" + nll,
				catalogNames[79]);
		srbToJargon.put("MDAS_TD_DTKTO_DOMN" + nll + "domain_desc" + nll,
				catalogNames[53]);
		srbToJargon.put("MDAS_TD_DTKTU_DOMN" + nll + "domain_desc" + nll,
				catalogNames[51]);
		srbToJargon.put("MDAS_TD_DTYP_IXCOL" + nll + "data_typ_name" + nll,
				catalogNames[108]);
		srbToJargon.put("MDAS_TD_DTYP_IXDS" + nll + "data_typ_name" + nll,
				catalogNames[106]);
		srbToJargon.put("MDAS_TD_DTYP_IXDTP" + nll + "data_typ_name" + nll,
				catalogNames[107]);
		srbToJargon.put("MDAS_TD_DTYP_MTCOL" + nll + "data_typ_name" + nll,
				catalogNames[111]);
		srbToJargon.put("MDAS_TD_DTYP_MTDTP" + nll + "data_typ_name" + nll,
				catalogNames[110]);
		srbToJargon.put("MDAS_TD_DTYP_MTHDS" + nll + "data_typ_name" + nll,
				catalogNames[109]);
		srbToJargon.put("MDAS_TD_DTYP_PARENT" + nll + "data_typ_name" + nll,
				catalogNames[359]);
		srbToJargon.put("MDAS_TD_EXTAB_INFO" + nll + "ext_attr_comments" + nll,
				catalogNames[399]);
		srbToJargon.put("MDAS_TD_EXTAB_INFO" + nll + "ext_attr_name" + nll,
				catalogNames[397]);
		srbToJargon.put("MDAS_TD_EXTAB_INFO" + nll + "ext_attr_out_name" + nll,
				catalogNames[398]);
		srbToJargon.put("MDAS_TD_EXTAB_INFO" + nll + "ext_schema_name" + nll,
				catalogNames[395]);
		srbToJargon.put("MDAS_TD_EXTAB_INFO" + nll + "ext_table_name" + nll,
				catalogNames[396]);
		srbToJargon.put("MDAS_TD_GRP_ACCS" + nll + "access_constraint" + nll,
				catalogNames[25]);
		srbToJargon.put("MDAS_TD_GRTKT_ACCS" + nll + "access_constraint" + nll,
				catalogNames[47]);
		srbToJargon.put("MDAS_TD_IDGRPUSER1" + nll + "user_id" + nll,
				catalogNames[183]);
		srbToJargon.put("MDAS_TD_IDGRPUSER2" + nll + "user_id" + nll,
				catalogNames[181]);
		srbToJargon.put("MDAS_TD_INT_LOCN" + nll + "netprefix" + nll,
				catalogNames[270]);
		srbToJargon.put("MDAS_TD_LCK_ACCS" + nll + "access_constraint" + nll,
				catalogNames[223]);
		srbToJargon.put("MDAS_TD_LOCN" + nll + "locn_desc" + nll,
				catalogNames[55]);
		srbToJargon.put("MDAS_TD_LOCN" + nll + "netprefix" + nll,
				catalogNames[12]);
		srbToJargon.put("MDAS_TD_ORDERBY" + nll + "orderby" + nll,
				catalogNames[385]);
		srbToJargon.put("MDAS_TD_OWNER_DOMN" + nll + "domain_desc" + nll,
				catalogNames[66]);
		srbToJargon.put("MDAS_TD_PARDOMN" + nll + "domain_desc" + nll,
				catalogNames[361]);
		srbToJargon.put("MDAS_TD_PAR_DOMN" + nll + "domain_desc" + nll,
				catalogNames[319]);
		srbToJargon.put("MDAS_TD_PAR_LOCN" + nll + "locn_desc" + nll,
				catalogNames[357]);
		srbToJargon.put("MDAS_TD_PIN_DOMN" + nll + "domain_desc" + nll,
				catalogNames[242]);
		srbToJargon.put("MDAS_TD_RAGRP_DOMN" + nll + "domain_desc" + nll,
				catalogNames[365]);
		srbToJargon.put("MDAS_TD_RSAC_DOMN" + nll + "domain_desc" + nll,
				catalogNames[342]);
		srbToJargon.put("MDAS_TD_RSRC_2TYP" + nll + "rsrc_typ_name" + nll,
				catalogNames[30]);
		srbToJargon.put("MDAS_TD_RSRC_3TYP" + nll + "rsrc_typ_name" + nll,
				catalogNames[271]);
		srbToJargon.put("MDAS_TD_RSRC_ACCS" + nll + "access_constraint" + nll,
				catalogNames[314]);
		srbToJargon.put("MDAS_TD_RSRC_ACCS" + nll + "access_id" + nll,
				catalogNames[358]);
		srbToJargon.put("MDAS_TD_RSRC_ACCS" + nll + "access_list" + nll,
				catalogNames[33]);
		srbToJargon.put("MDAS_TD_RSRC_ACCS" + nll + "access_privilege" + nll,
				catalogNames[313]);
		srbToJargon.put("MDAS_TD_RSRC_CLASS" + nll + "rsrc_class_name" + nll,
				catalogNames[59]);
		srbToJargon.put("MDAS_TD_RSRC_PARENT" + nll + "rsrc_typ_name" + nll,
				catalogNames[363]);
		srbToJargon.put("MDAS_TD_RSRC_TYP" + nll + "rsrc_typ_name" + nll,
				catalogNames[13]);
		srbToJargon.put("MDAS_TD_RS_DOMN" + nll + "domain_desc" + nll,
				catalogNames[333]);
		srbToJargon.put("MDAS_TD_SRVR_LOCN" + nll + "locn_desc" + nll,
				catalogNames[69]);
		srbToJargon.put("MDAS_TD_SRVR_LOCN" + nll + "netprefix" + nll,
				catalogNames[70]);
		srbToJargon.put("MDAS_TD_UDF" + nll + "udf" + nll, catalogNames[295]);
		srbToJargon.put("MDAS_TD_UDF" + nll + "udf" + nll, catalogNames[296]);
		srbToJargon.put("MDAS_TD_UDF" + nll + "udf" + nll, catalogNames[297]);
		srbToJargon.put("MDAS_TD_UDF" + nll + "udf" + nll, catalogNames[298]);
		srbToJargon.put("MDAS_TD_UDF" + nll + "udf" + nll, catalogNames[299]);
		srbToJargon.put("MDAS_TD_USER_TYP" + nll + "user_typ_name" + nll,
				catalogNames[5]);
		srbToJargon.put("MDAS_TD_UTYP_PARENT" + nll + "user_typ_name" + nll,
				catalogNames[362]);
		srbToJargon.put("MDAS_TD_ZN_ATHSCHM" + nll + "auth_scheme" + nll,
				catalogNames[304]);
		srbToJargon.put("MDAS_TD_ZN_DOMN" + nll + "domain_desc" + nll,
				catalogNames[312]);
		srbToJargon.put("MDAS_TD_ZN_LOCN" + nll + "locn_desc" + nll,
				catalogNames[360]);
		srbToJargon.put("MDAS_TD_ZN_LOCN" + nll + "netprefix" + nll,
				catalogNames[302]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "local_zone_flag" + nll,
				catalogNames[301]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "port_number" + nll,
				catalogNames[303]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "zone_comments" + nll,
				catalogNames[309]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "zone_contact" + nll,
				catalogNames[310]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "zone_create_date" + nll,
				catalogNames[307]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "zone_id" + nll,
				catalogNames[300]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "zone_modify_date" + nll,
				catalogNames[308]);
		srbToJargon.put("MDAS_TD_ZONE" + nll + "zone_status" + nll,
				catalogNames[306]);
		srbToJargon.put("NONDISTINCT" + nll + "nondistinct" + nll,
				catalogNames[294]);

		jargonToSRB.put(catalogNames[175], "DC_AUTHCONTRIB" + nll
				+ "dc_contr_address" + nll);
		jargonToSRB.put(catalogNames[179], "DC_AUTHCONTRIB" + nll
				+ "dc_contr_corp_name" + nll);
		jargonToSRB.put(catalogNames[176], "DC_AUTHCONTRIB" + nll
				+ "dc_contr_email" + nll);
		jargonToSRB.put(catalogNames[164], "DC_AUTHCONTRIB" + nll
				+ "dc_contr_name" + nll);
		jargonToSRB.put(catalogNames[177], "DC_AUTHCONTRIB" + nll
				+ "dc_contr_phone" + nll);
		jargonToSRB.put(catalogNames[178], "DC_AUTHCONTRIB" + nll
				+ "dc_contr_web" + nll);
		jargonToSRB.put(catalogNames[153], "DC_COLLECTION" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[172], "DC_COVERAGE" + nll
				+ "coverage_desc" + nll);
		jargonToSRB.put(catalogNames[152], "DC_DATA" + nll + "data_name" + nll);
		jargonToSRB.put(catalogNames[166], "DC_DESCRIPTION" + nll + "dc_desc"
				+ nll);
		jargonToSRB.put(catalogNames[174], "DC_PUBLISHER" + nll + "dc_pub_addr"
				+ nll);
		jargonToSRB.put(catalogNames[167], "DC_PUBLISHER" + nll + "dc_pub_name"
				+ nll);
		jargonToSRB.put(catalogNames[169], "DC_RELATION" + nll
				+ "related_data_desc" + nll);
		jargonToSRB.put(catalogNames[173], "DC_RIGHTS" + nll
				+ "rights_data_desc" + nll);
		jargonToSRB.put(catalogNames[168], "DC_SOURCE" + nll + "source_desc"
				+ nll);
		jargonToSRB.put(catalogNames[165], "DC_SUBJECT" + nll
				+ "dc_subject_name" + nll);
		jargonToSRB.put(catalogNames[154], "DC_TD_CONTR_TYPE" + nll
				+ "dc_contr_type_name" + nll);
		jargonToSRB.put(catalogNames[161], "DC_TD_COVERAGE" + nll
				+ "coverage_type_name" + nll);
		jargonToSRB.put(catalogNames[156], "DC_TD_DESCR_TYPE" + nll
				+ "dc_desc_type_name" + nll);
		jargonToSRB.put(catalogNames[159], "DC_TD_LANGUAGE" + nll
				+ "language_name" + nll);
		jargonToSRB.put(catalogNames[160], "DC_TD_RELATION_TYP" + nll
				+ "relation_type_name" + nll);
		jargonToSRB.put(catalogNames[162], "DC_TD_RIGHTS" + nll
				+ "rights_type_name" + nll);
		jargonToSRB.put(catalogNames[158], "DC_TD_SOURCE_TYPE" + nll
				+ "source_type_name" + nll);
		jargonToSRB.put(catalogNames[155], "DC_TD_SUBJ_CLASS" + nll
				+ "dc_subj_class_name" + nll);
		jargonToSRB.put(catalogNames[157], "DC_TD_TYPE" + nll + "dc_type_name"
				+ nll);
		jargonToSRB.put(catalogNames[163], "DC_TITLE" + nll + "dc_title" + nll);
		jargonToSRB.put(catalogNames[233], "MDAS_AC_ANNOTATION" + nll
				+ "anno_date" + nll);
		jargonToSRB.put(catalogNames[234], "MDAS_AC_ANNOTATION" + nll
				+ "anno_type" + nll);
		jargonToSRB.put(catalogNames[232], "MDAS_AC_ANNOTATION" + nll
				+ "annotations" + nll);
		jargonToSRB.put(catalogNames[64], "MDAS_ADCONT_REPL" + nll
				+ "data_size" + nll);
		jargonToSRB.put(catalogNames[281], "MDAS_ADC_REPL" + nll + "data_name"
				+ nll);
		jargonToSRB.put(catalogNames[10], "MDAS_ADC_REPL" + nll + "path_name"
				+ nll);
		jargonToSRB.put(catalogNames[1], "MDAS_ADC_REPL" + nll + "repl_enum"
				+ nll);
		jargonToSRB.put(catalogNames[265], "MDAS_ADC_REPL" + nll + "seg_num"
				+ nll);
		jargonToSRB.put(catalogNames[96], "MDAS_ADIXCOLL_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[115], "MDAS_ADIXCOLL_REPL" + nll
				+ "path_name" + nll);
		jargonToSRB.put(catalogNames[94], "MDAS_ADIXDS_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[113], "MDAS_ADIXDS_REPL" + nll
				+ "path_name" + nll);
		jargonToSRB.put(catalogNames[95], "MDAS_ADIXDTP_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[114], "MDAS_ADIXDTP_REPL" + nll
				+ "path_name" + nll);
		jargonToSRB.put(catalogNames[99], "MDAS_ADMTHCOL_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[97], "MDAS_ADMTHDS_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[98], "MDAS_ADMTHDTP_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[92], "MDAS_ADSTRUCT_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[192], "MDAS_AD_1COLLMDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[196], "MDAS_AD_1COLLMDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[278], "MDAS_AD_1COMPOUND" + nll
				+ "cmpd_path_name" + nll);
		jargonToSRB.put(catalogNames[184], "MDAS_AD_1MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[188], "MDAS_AD_1MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[193], "MDAS_AD_2COLLMDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[197], "MDAS_AD_2COLLMDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[185], "MDAS_AD_2MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[189], "MDAS_AD_2MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[194], "MDAS_AD_3COLLMDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[198], "MDAS_AD_3COLLMDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[186], "MDAS_AD_3MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[190], "MDAS_AD_3MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[195], "MDAS_AD_4COLLMDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[199], "MDAS_AD_4COLLMDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[187], "MDAS_AD_4MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[191], "MDAS_AD_4MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[182], "MDAS_AD_ACCS" + nll + "access_id"
				+ nll);
		jargonToSRB.put(catalogNames[75], "MDAS_AD_ANNOTATION" + nll
				+ "anno_date" + nll);
		jargonToSRB.put(catalogNames[78], "MDAS_AD_ANNOTATION" + nll
				+ "anno_position" + nll);
		jargonToSRB.put(catalogNames[74], "MDAS_AD_ANNOTATION" + nll
				+ "annotations" + nll);
		jargonToSRB.put(catalogNames[24], "MDAS_AD_AUDIT" + nll + "comments"
				+ nll);
		jargonToSRB.put(catalogNames[23], "MDAS_AD_AUDIT" + nll + "time_stamp"
				+ nll);
		jargonToSRB.put(catalogNames[88], "MDAS_AD_CLASS" + nll + "class_name"
				+ nll);
		jargonToSRB.put(catalogNames[89], "MDAS_AD_CLASS" + nll + "class_type"
				+ nll);
		jargonToSRB.put(catalogNames[133], "MDAS_AD_COLLMDATA" + nll
				+ "metadatanum" + nll);
		jargonToSRB.put(catalogNames[378], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_creat_date" + nll);
		jargonToSRB.put(catalogNames[144], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metaint0" + nll);
		jargonToSRB.put(catalogNames[145], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metaint1" + nll);
		jargonToSRB.put(catalogNames[134], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[135], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[136], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr2" + nll);
		jargonToSRB.put(catalogNames[137], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr3" + nll);
		jargonToSRB.put(catalogNames[138], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr4" + nll);
		jargonToSRB.put(catalogNames[139], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr5" + nll);
		jargonToSRB.put(catalogNames[140], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr6" + nll);
		jargonToSRB.put(catalogNames[141], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr7" + nll);
		jargonToSRB.put(catalogNames[142], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr8" + nll);
		jargonToSRB.put(catalogNames[143], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_metastr9" + nll);
		jargonToSRB.put(catalogNames[321], "MDAS_AD_COLLMDATA" + nll
				+ "userdef_modif_date" + nll);
		jargonToSRB.put(catalogNames[77], "MDAS_AD_COMMENTS" + nll + "com_date"
				+ nll);
		jargonToSRB.put(catalogNames[268], "MDAS_AD_COMPOUND" + nll
				+ "cmpd_path_name" + nll);
		jargonToSRB.put(catalogNames[275], "MDAS_AD_COMPOUND" + nll
				+ "data_size" + nll);
		jargonToSRB.put(catalogNames[266], "MDAS_AD_COMPOUND" + nll
				+ "int_repl_num" + nll);
		jargonToSRB.put(catalogNames[267], "MDAS_AD_COMPOUND" + nll
				+ "int_seg_num" + nll);
		jargonToSRB.put(catalogNames[272], "MDAS_AD_COMPOUND" + nll
				+ "is_dirty" + nll);
		jargonToSRB.put(catalogNames[274], "MDAS_AD_COMPOUND" + nll + "offset"
				+ nll);
		jargonToSRB.put(catalogNames[170], "MDAS_AD_DCRELREPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[392], "MDAS_AD_DEL_REPL" + nll
				+ "d_comment" + nll);
		jargonToSRB.put(catalogNames[391], "MDAS_AD_DEL_REPL" + nll
				+ "data_grp_id" + nll);
		jargonToSRB.put(catalogNames[386], "MDAS_AD_DEL_REPL" + nll + "data_id"
				+ nll);
		jargonToSRB.put(catalogNames[388], "MDAS_AD_DEL_REPL" + nll
				+ "data_name" + nll);
		jargonToSRB.put(catalogNames[387], "MDAS_AD_DEL_REPL" + nll
				+ "repl_enum" + nll);
		jargonToSRB.put(catalogNames[389], "MDAS_AD_DEL_REPL" + nll
				+ "repl_timestamp" + nll);
		jargonToSRB.put(catalogNames[180], "MDAS_AD_GRP_2ACCS" + nll
				+ "access_id" + nll);
		jargonToSRB.put(catalogNames[382], "MDAS_AD_GUID" + nll + "guid" + nll);
		jargonToSRB.put(catalogNames[383], "MDAS_AD_GUID" + nll + "guid_flag"
				+ nll);
		jargonToSRB.put(catalogNames[384], "MDAS_AD_GUID" + nll
				+ "guid_time_stamp" + nll);
		jargonToSRB.put(catalogNames[116], "MDAS_AD_MDATA" + nll
				+ "metadatanum" + nll);
		jargonToSRB.put(catalogNames[377], "MDAS_AD_MDATA" + nll
				+ "userdef_creat_date" + nll);
		jargonToSRB.put(catalogNames[127], "MDAS_AD_MDATA" + nll
				+ "userdef_metaint0" + nll);
		jargonToSRB.put(catalogNames[128], "MDAS_AD_MDATA" + nll
				+ "userdef_metaint1" + nll);
		jargonToSRB.put(catalogNames[117], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[118], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[119], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr2" + nll);
		jargonToSRB.put(catalogNames[120], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr3" + nll);
		jargonToSRB.put(catalogNames[121], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr4" + nll);
		jargonToSRB.put(catalogNames[122], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr5" + nll);
		jargonToSRB.put(catalogNames[123], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr6" + nll);
		jargonToSRB.put(catalogNames[124], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr7" + nll);
		jargonToSRB.put(catalogNames[125], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr8" + nll);
		jargonToSRB.put(catalogNames[126], "MDAS_AD_MDATA" + nll
				+ "userdef_metastr9" + nll);
		jargonToSRB.put(catalogNames[320], "MDAS_AD_MDATA" + nll
				+ "userdef_modif_date" + nll);
		jargonToSRB.put(catalogNames[245], "MDAS_AD_MISC1" + nll
				+ "dcompressed" + nll);
		jargonToSRB.put(catalogNames[246], "MDAS_AD_MISC1" + nll + "dencrypted"
				+ nll);
		jargonToSRB.put(catalogNames[244], "MDAS_AD_MISC1" + nll
				+ "dexpire_date" + nll);
		jargonToSRB.put(catalogNames[247], "MDAS_AD_MISC1" + nll
				+ "dexpire_date_2" + nll);
		jargonToSRB
				.put(catalogNames[240], "MDAS_AD_MISC1" + nll + "dpin" + nll);
		jargonToSRB.put(catalogNames[243], "MDAS_AD_MISC1" + nll + "dpinexpiry"
				+ nll);
		jargonToSRB.put(catalogNames[238], "MDAS_AD_REPL" + nll + "auditflag"
				+ nll);
		jargonToSRB.put(catalogNames[381], "MDAS_AD_REPL" + nll
				+ "container_id" + nll);
		jargonToSRB.put(catalogNames[21], "MDAS_AD_REPL" + nll + "d_comment"
				+ nll);
		jargonToSRB
				.put(catalogNames[0], "MDAS_AD_REPL" + nll + "data_id" + nll);
		jargonToSRB.put(catalogNames[2], "MDAS_AD_REPL" + nll + "data_name"
				+ nll);
		jargonToSRB.put(catalogNames[18], "MDAS_AD_REPL" + nll + "data_size"
				+ nll);
		jargonToSRB.put(catalogNames[222], "MDAS_AD_REPL" + nll + "datalock"
				+ nll);
		jargonToSRB.put(catalogNames[237], "MDAS_AD_REPL" + nll + "dchecksum"
				+ nll);
		jargonToSRB
				.put(catalogNames[236], "MDAS_AD_REPL" + nll + "dhide" + nll);
		jargonToSRB.put(catalogNames[239], "MDAS_AD_REPL" + nll + "dlockexpiry"
				+ nll);
		jargonToSRB.put(catalogNames[34], "MDAS_AD_REPL" + nll + "is_deleted"
				+ nll);
		jargonToSRB.put(catalogNames[56], "MDAS_AD_REPL" + nll + "is_dirty"
				+ nll);
		jargonToSRB.put(catalogNames[148], "MDAS_AD_REPL" + nll
				+ "last_accs_time" + nll);
		jargonToSRB.put(catalogNames[14], "MDAS_AD_REPL" + nll
				+ "modify_timestamp" + nll);
		jargonToSRB
				.put(catalogNames[61], "MDAS_AD_REPL" + nll + "offset" + nll);
		jargonToSRB.put(catalogNames[71], "MDAS_AD_REPL" + nll + "repl_enum"
				+ nll);
		jargonToSRB.put(catalogNames[76], "MDAS_AD_REPL" + nll
				+ "repl_timestamp" + nll);
		jargonToSRB.put(catalogNames[221], "MDAS_AD_REPL" + nll + "version_num"
				+ nll);
		jargonToSRB.put(catalogNames[93], "MDAS_AD_STRCT_BLOB" + nll
				+ "internal_structure" + nll);
		jargonToSRB.put(catalogNames[91], "MDAS_AD_STRCT_BLOB" + nll
				+ "structure_comments" + nll);
		jargonToSRB.put(catalogNames[90], "MDAS_AD_STRCT_BLOB" + nll
				+ "structure_type" + nll);
		jargonToSRB.put(catalogNames[201], "MDAS_AR_1MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[204], "MDAS_AR_1MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[62], "MDAS_AR_1PHYSICAL" + nll
				+ "phy_rsrc_name" + nll);
		jargonToSRB.put(catalogNames[65], "MDAS_AR_1REPL" + nll + "rsrc_name"
				+ nll);
		jargonToSRB.put(catalogNames[202], "MDAS_AR_2MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[205], "MDAS_AR_2MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[264], "MDAS_AR_2PHYSICAL" + nll
				+ "phy_rsrc_name" + nll);
		jargonToSRB.put(catalogNames[203], "MDAS_AR_3MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[206], "MDAS_AR_3MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[269], "MDAS_AR_3PHYSICAL" + nll
				+ "phy_rsrc_name" + nll);
		jargonToSRB.put(catalogNames[337], "MDAS_AR_INFO" + nll
				+ "concurrency_max" + nll);
		jargonToSRB.put(catalogNames[83], "MDAS_AR_INFO" + nll
				+ "concurrency_max" + nll);
		jargonToSRB.put(catalogNames[87], "MDAS_AR_INFO" + nll + "description"
				+ nll);
		jargonToSRB.put(catalogNames[336], "MDAS_AR_INFO" + nll
				+ "mbps_bandwidth" + nll);
		jargonToSRB.put(catalogNames[82], "MDAS_AR_INFO" + nll
				+ "mbps_bandwidth" + nll);
		jargonToSRB.put(catalogNames[340], "MDAS_AR_INFO" + nll
				+ "megab_capacity" + nll);
		jargonToSRB.put(catalogNames[86], "MDAS_AR_INFO" + nll
				+ "megab_capacity" + nll);
		jargonToSRB.put(catalogNames[334], "MDAS_AR_INFO" + nll
				+ "mlsec_latency_max" + nll);
		jargonToSRB.put(catalogNames[80], "MDAS_AR_INFO" + nll
				+ "mlsec_latency_max" + nll);
		jargonToSRB.put(catalogNames[335], "MDAS_AR_INFO" + nll
				+ "mlsec_latency_min" + nll);
		jargonToSRB.put(catalogNames[81], "MDAS_AR_INFO" + nll
				+ "mlsec_latency_min" + nll);
		jargonToSRB.put(catalogNames[338], "MDAS_AR_INFO" + nll
				+ "num_of_hierarchies" + nll);
		jargonToSRB.put(catalogNames[84], "MDAS_AR_INFO" + nll
				+ "num_of_hierarchies" + nll);
		jargonToSRB.put(catalogNames[339], "MDAS_AR_INFO" + nll
				+ "num_of_stripes" + nll);
		jargonToSRB.put(catalogNames[85], "MDAS_AR_INFO" + nll
				+ "num_of_stripes" + nll);
		jargonToSRB.put(catalogNames[262], "MDAS_AR_MDATA" + nll
				+ "metadatanum" + nll);
		jargonToSRB.put(catalogNames[379], "MDAS_AR_MDATA" + nll
				+ "userdef_creat_date" + nll);
		jargonToSRB.put(catalogNames[261], "MDAS_AR_MDATA" + nll
				+ "userdef_metaint1" + nll);
		jargonToSRB.put(catalogNames[316], "MDAS_AR_MDATA" + nll
				+ "userdef_metaint1" + nll);
		jargonToSRB.put(catalogNames[251], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[252], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[253], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr2" + nll);
		jargonToSRB.put(catalogNames[254], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr3" + nll);
		jargonToSRB.put(catalogNames[255], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr4" + nll);
		jargonToSRB.put(catalogNames[256], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr5" + nll);
		jargonToSRB.put(catalogNames[257], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr6" + nll);
		jargonToSRB.put(catalogNames[258], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr7" + nll);
		jargonToSRB.put(catalogNames[259], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr8" + nll);
		jargonToSRB.put(catalogNames[260], "MDAS_AR_MDATA" + nll
				+ "userdef_metastr9" + nll);
		jargonToSRB.put(catalogNames[323], "MDAS_AR_MDATA" + nll
				+ "userdef_modif_date" + nll);
		jargonToSRB.put(catalogNames[393], "MDAS_AR_PHYSICAL" + nll
				+ "free_space" + nll);
		jargonToSRB.put(catalogNames[394], "MDAS_AR_PHYSICAL" + nll
				+ "fs_timestamp" + nll);
		jargonToSRB.put(catalogNames[60], "MDAS_AR_PHYSICAL" + nll
				+ "max_obj_size" + nll);
		jargonToSRB.put(catalogNames[28], "MDAS_AR_PHYSICAL" + nll
				+ "phy_default_path" + nll);
		jargonToSRB.put(catalogNames[29], "MDAS_AR_PHYSICAL" + nll
				+ "phy_rsrc_name" + nll);
		jargonToSRB.put(catalogNames[400], "MDAS_AR_PHYSICAL" + nll
				+ "rsrc_lock" + nll);
		jargonToSRB.put(catalogNames[27], "MDAS_AR_REPL" + nll + "default_path"
				+ nll);
		jargonToSRB.put(catalogNames[331], "MDAS_AR_REPL" + nll
				+ "max_obj_size" + nll);
		jargonToSRB.put(catalogNames[31], "MDAS_AR_REPL" + nll + "repl_enum"
				+ nll);
		jargonToSRB.put(catalogNames[326], "MDAS_AR_REPL" + nll
				+ "rsrc_comments" + nll);
		jargonToSRB.put(catalogNames[327], "MDAS_AR_REPL" + nll
				+ "rsrc_creation_date" + nll);
		jargonToSRB.put(catalogNames[328], "MDAS_AR_REPL" + nll
				+ "rsrc_modify_date" + nll);
		jargonToSRB.put(catalogNames[11], "MDAS_AR_REPL" + nll + "rsrc_name"
				+ nll);
		jargonToSRB.put(catalogNames[220], "MDAS_AR_TAPE_INFO" + nll
				+ "bytesWritten" + nll);
		jargonToSRB.put(catalogNames[219], "MDAS_AR_TAPE_INFO" + nll
				+ "currentAbsPosition" + nll);
		jargonToSRB.put(catalogNames[218], "MDAS_AR_TAPE_INFO" + nll
				+ "currentFileSeqNum" + nll);
		jargonToSRB.put(catalogNames[217], "MDAS_AR_TAPE_INFO" + nll
				+ "fullFlag" + nll);
		jargonToSRB.put(catalogNames[216], "MDAS_AR_TAPE_INFO" + nll
				+ "tapeLibInx" + nll);
		jargonToSRB.put(catalogNames[213], "MDAS_AR_TAPE_INFO" + nll
				+ "tapeNumber" + nll);
		jargonToSRB.put(catalogNames[214], "MDAS_AR_TAPE_INFO" + nll
				+ "tapeOwner" + nll);
		jargonToSRB.put(catalogNames[215], "MDAS_AR_TAPE_INFO" + nll
				+ "tapeType" + nll);
		jargonToSRB.put(catalogNames[325], "MDAS_AT_DATA_TYP_EXT" + nll
				+ "data_typ_ext" + nll);
		jargonToSRB.put(catalogNames[324], "MDAS_AT_DATA_TYP_EXT" + nll
				+ "data_typ_mime" + nll);
		jargonToSRB.put(catalogNames[250], "MDAS_AUDIT" + nll + "aud_comments"
				+ nll);
		jargonToSRB.put(catalogNames[249], "MDAS_AUDIT" + nll + "aud_timestamp"
				+ nll);
		jargonToSRB.put(catalogNames[248], "MDAS_AUDIT_DESC" + nll
				+ "actionDesc" + nll);
		jargonToSRB.put(catalogNames[207], "MDAS_AU_1MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[210], "MDAS_AU_1MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[208], "MDAS_AU_2MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[211], "MDAS_AU_2MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[209], "MDAS_AU_3MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[212], "MDAS_AU_3MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[20], "MDAS_AU_AUDIT" + nll + "comments"
				+ nll);
		jargonToSRB.put(catalogNames[19], "MDAS_AU_AUDIT" + nll + "time_stamp"
				+ nll);
		jargonToSRB.put(catalogNames[67], "MDAS_AU_AUTH_MAP" + nll
				+ "distin_name" + nll);
		jargonToSRB.put(catalogNames[6], "MDAS_AU_INFO" + nll + "user_address"
				+ nll);
		jargonToSRB.put(catalogNames[17], "MDAS_AU_INFO" + nll + "user_email"
				+ nll);
		jargonToSRB.put(catalogNames[16], "MDAS_AU_INFO" + nll + "user_phone"
				+ nll);
		jargonToSRB.put(catalogNames[263], "MDAS_AU_MDATA" + nll
				+ "metadatanum" + nll);
		jargonToSRB.put(catalogNames[380], "MDAS_AU_MDATA" + nll
				+ "userdef_creat_date" + nll);
		jargonToSRB.put(catalogNames[293], "MDAS_AU_MDATA" + nll
				+ "userdef_metaint0" + nll);
		jargonToSRB.put(catalogNames[315], "MDAS_AU_MDATA" + nll
				+ "userdef_metaint1" + nll);
		jargonToSRB.put(catalogNames[283], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr0" + nll);
		jargonToSRB.put(catalogNames[284], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr1" + nll);
		jargonToSRB.put(catalogNames[285], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr2" + nll);
		jargonToSRB.put(catalogNames[286], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr3" + nll);
		jargonToSRB.put(catalogNames[287], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr4" + nll);
		jargonToSRB.put(catalogNames[288], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr5" + nll);
		jargonToSRB.put(catalogNames[289], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr6" + nll);
		jargonToSRB.put(catalogNames[290], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr7" + nll);
		jargonToSRB.put(catalogNames[291], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr8" + nll);
		jargonToSRB.put(catalogNames[292], "MDAS_AU_MDATA" + nll
				+ "userdef_metastr9" + nll);
		jargonToSRB.put(catalogNames[322], "MDAS_AU_MDATA" + nll
				+ "userdef_modif_date" + nll);
		jargonToSRB.put(catalogNames[129], "MDAS_AU_OWNER_INFO" + nll
				+ "user_email" + nll);
		jargonToSRB.put(catalogNames[39], "MDAS_AU_TCKT_DATA" + nll
				+ "access_count" + nll);
		jargonToSRB.put(catalogNames[37], "MDAS_AU_TCKT_DATA" + nll
				+ "begin_time" + nll);
		jargonToSRB.put(catalogNames[38], "MDAS_AU_TCKT_DATA" + nll
				+ "end_time" + nll);
		jargonToSRB.put(catalogNames[36], "MDAS_AU_TCKT_DATA" + nll
				+ "ticket_id" + nll);
		jargonToSRB.put(catalogNames[46], "MDAS_AU_TCKT_GRP" + nll
				+ "access_count" + nll);
		jargonToSRB.put(catalogNames[44], "MDAS_AU_TCKT_GRP" + nll
				+ "begin_time" + nll);
		jargonToSRB.put(catalogNames[45], "MDAS_AU_TCKT_GRP" + nll + "end_time"
				+ nll);
		jargonToSRB.put(catalogNames[200], "MDAS_AU_TCKT_GRP" + nll
				+ "is_recursive" + nll);
		jargonToSRB.put(catalogNames[43], "MDAS_AU_TCKT_GRP" + nll
				+ "ticket_id" + nll);
		jargonToSRB.put(catalogNames[305], "MDAS_AU_ZN_ATHMAP" + nll
				+ "distin_name" + nll);
		jargonToSRB.put(catalogNames[72], "MDAS_CD_ANNO_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[350], "MDAS_CD_ANNO_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[371], "MDAS_CD_CAGRP_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[373], "MDAS_CD_CAGRP_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[230], "MDAS_CD_CANNO_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[276], "MDAS_CD_CANNO_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[354], "MDAS_CD_CANNO_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[356], "MDAS_CD_CANNO_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[149], "MDAS_CD_COOWN_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[352], "MDAS_CD_COOWN_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[48], "MDAS_CD_CTKTOWNER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[348], "MDAS_CD_CTKTOWNER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[49], "MDAS_CD_CTKTUSER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[349], "MDAS_CD_CTKTUSER" + nll + "zone_id"
				+ nll);
		jargonToSRB.put(catalogNames[368], "MDAS_CD_DAGRP_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[370], "MDAS_CD_DAGRP_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[130], "MDAS_CD_DAUDT_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[351], "MDAS_CD_DAUDT_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[224], "MDAS_CD_DLOWN_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[353], "MDAS_CD_DLOWN_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[41], "MDAS_CD_DTKTOWNER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[346], "MDAS_CD_DTKTOWNER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[42], "MDAS_CD_DTKTUSER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[347], "MDAS_CD_DTKTUSER" + nll + "zone_id"
				+ nll);
		jargonToSRB.put(catalogNames[35], "MDAS_CD_OWNER_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[345], "MDAS_CD_OWNER_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[317], "MDAS_CD_PAR_USER" + nll
				+ "user_modify_date" + nll);
		jargonToSRB.put(catalogNames[3], "MDAS_CD_PAR_USER" + nll + "user_name"
				+ nll);
		jargonToSRB.put(catalogNames[318], "MDAS_CD_PAR_USER" + nll + "zone_id"
				+ nll);
		jargonToSRB.put(catalogNames[241], "MDAS_CD_PIN_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[355], "MDAS_CD_PIN_USER" + nll + "zone_id"
				+ nll);
		jargonToSRB.put(catalogNames[364], "MDAS_CD_RAGRP_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[366], "MDAS_CD_RAGRP_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[341], "MDAS_CD_RSAC_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[343], "MDAS_CD_RSAC_USER" + nll
				+ "zone_id" + nll);
		jargonToSRB.put(catalogNames[332], "MDAS_CD_RS_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[344], "MDAS_CD_RS_USER" + nll + "zone_id"
				+ nll);
		jargonToSRB.put(catalogNames[329], "MDAS_CD_USER" + nll
				+ "user_creation_date" + nll);
		jargonToSRB.put(catalogNames[147], "MDAS_CD_USER" + nll + "user_id"
				+ nll);
		jargonToSRB.put(catalogNames[330], "MDAS_CD_USER" + nll
				+ "user_modify_date" + nll);
		jargonToSRB.put(catalogNames[7], "MDAS_CD_USER" + nll + "user_name"
				+ nll);
		jargonToSRB.put(catalogNames[311], "MDAS_CD_ZN_USER" + nll
				+ "user_name" + nll);
		jargonToSRB.put(catalogNames[63], "MDAS_TD_1RSRC_CLASS" + nll
				+ "rsrc_class_name" + nll);
		jargonToSRB.put(catalogNames[273], "MDAS_TD_2RSRC_CLASS" + nll
				+ "rsrc_class_name" + nll);
		jargonToSRB.put(catalogNames[22], "MDAS_TD_ACTN" + nll + "action_desc"
				+ nll);
		jargonToSRB.put(catalogNames[73], "MDAS_TD_ANNO_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[68], "MDAS_TD_AUTH_SCHM" + nll
				+ "auth_scheme" + nll);
		jargonToSRB.put(catalogNames[374], "MDAS_TD_CADATA_GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[375], "MDAS_TD_CAGRP_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[372], "MDAS_TD_CAGRP_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[231], "MDAS_TD_CANNO_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[277], "MDAS_TD_CANNO_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[376], "MDAS_TD_CCDATA_TYP" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[132], "MDAS_TD_COLLCONT" + nll
				+ "container_name" + nll);
		jargonToSRB.put(catalogNames[57], "MDAS_TD_CONTAINER" + nll
				+ "container_max_size" + nll);
		jargonToSRB.put(catalogNames[58], "MDAS_TD_CONTAINER" + nll
				+ "container_name" + nll);
		jargonToSRB.put(catalogNames[226], "MDAS_TD_COOWN_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[52], "MDAS_TD_CTKTO_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[50], "MDAS_TD_CTKTU_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[369], "MDAS_TD_DAGRP_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[26], "MDAS_TD_DATA_2GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[229], "MDAS_TD_DATA_3GRP" + nll
				+ "coll_comments" + nll);
		jargonToSRB.put(catalogNames[228], "MDAS_TD_DATA_3GRP" + nll
				+ "coll_cr_timestamp" + nll);
		jargonToSRB.put(catalogNames[227], "MDAS_TD_DATA_3GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[280], "MDAS_TD_DATA_4GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[282], "MDAS_TD_DATA_5GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[151], "MDAS_TD_DATA_GRP" + nll
				+ "coll_comments" + nll);
		jargonToSRB.put(catalogNames[150], "MDAS_TD_DATA_GRP" + nll
				+ "coll_cr_timestamp" + nll);
		jargonToSRB.put(catalogNames[279], "MDAS_TD_DATA_GRP" + nll
				+ "coll_link" + nll);
		jargonToSRB.put(catalogNames[367], "MDAS_TD_DATA_GRP" + nll
				+ "coll_mod_timestamp" + nll);
		jargonToSRB.put(catalogNames[235], "MDAS_TD_DATA_GRP" + nll
				+ "data_grp_id" + nll);
		jargonToSRB.put(catalogNames[15], "MDAS_TD_DATA_GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[54], "MDAS_TD_DATA_GRP" + nll
				+ "parent_grp_name" + nll);
		jargonToSRB.put(catalogNames[4], "MDAS_TD_DATA_TYP" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[131], "MDAS_TD_DAUDT_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[171], "MDAS_TD_DCRELGRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[390], "MDAS_TD_DEL_DATA_GRP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[102], "MDAS_TD_DGRP_IXCOL" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[100], "MDAS_TD_DGRP_IXDS" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[101], "MDAS_TD_DGRP_IXDTP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[105], "MDAS_TD_DGRP_MTCOL" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[104], "MDAS_TD_DGRP_MTDTP" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[103], "MDAS_TD_DGRP_MTHDS" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[112], "MDAS_TD_DGRP_STRUC" + nll
				+ "data_grp_name" + nll);
		jargonToSRB.put(catalogNames[225], "MDAS_TD_DLOWN_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[9], "MDAS_TD_DOMN" + nll + "domain_desc"
				+ nll);
		jargonToSRB.put(catalogNames[40], "MDAS_TD_DSTKT_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[8], "MDAS_TD_DS_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[146], "MDAS_TD_DS_ACCS" + nll
				+ "access_id" + nll);
		jargonToSRB.put(catalogNames[32], "MDAS_TD_DS_ACCS" + nll
				+ "access_list" + nll);
		jargonToSRB.put(catalogNames[79], "MDAS_TD_DS_ACCS" + nll
				+ "access_privilege" + nll);
		jargonToSRB.put(catalogNames[53], "MDAS_TD_DTKTO_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[51], "MDAS_TD_DTKTU_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[108], "MDAS_TD_DTYP_IXCOL" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[106], "MDAS_TD_DTYP_IXDS" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[107], "MDAS_TD_DTYP_IXDTP" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[111], "MDAS_TD_DTYP_MTCOL" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[110], "MDAS_TD_DTYP_MTDTP" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[109], "MDAS_TD_DTYP_MTHDS" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[359], "MDAS_TD_DTYP_PARENT" + nll
				+ "data_typ_name" + nll);
		jargonToSRB.put(catalogNames[399], "MDAS_TD_EXTAB_INFO" + nll
				+ "ext_attr_comments" + nll);
		jargonToSRB.put(catalogNames[397], "MDAS_TD_EXTAB_INFO" + nll
				+ "ext_attr_name" + nll);
		jargonToSRB.put(catalogNames[398], "MDAS_TD_EXTAB_INFO" + nll
				+ "ext_attr_out_name" + nll);
		jargonToSRB.put(catalogNames[395], "MDAS_TD_EXTAB_INFO" + nll
				+ "ext_schema_name" + nll);
		jargonToSRB.put(catalogNames[396], "MDAS_TD_EXTAB_INFO" + nll
				+ "ext_table_name" + nll);
		jargonToSRB.put(catalogNames[25], "MDAS_TD_GRP_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[47], "MDAS_TD_GRTKT_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[183], "MDAS_TD_IDGRPUSER1" + nll
				+ "user_id" + nll);
		jargonToSRB.put(catalogNames[181], "MDAS_TD_IDGRPUSER2" + nll
				+ "user_id" + nll);
		jargonToSRB.put(catalogNames[270], "MDAS_TD_INT_LOCN" + nll
				+ "netprefix" + nll);
		jargonToSRB.put(catalogNames[223], "MDAS_TD_LCK_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[55], "MDAS_TD_LOCN" + nll + "locn_desc"
				+ nll);
		jargonToSRB.put(catalogNames[12], "MDAS_TD_LOCN" + nll + "netprefix"
				+ nll);
		jargonToSRB.put(catalogNames[385], "MDAS_TD_ORDERBY" + nll + "orderby"
				+ nll);
		jargonToSRB.put(catalogNames[66], "MDAS_TD_OWNER_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[361], "MDAS_TD_PARDOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[319], "MDAS_TD_PAR_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[357], "MDAS_TD_PAR_LOCN" + nll
				+ "locn_desc" + nll);
		jargonToSRB.put(catalogNames[242], "MDAS_TD_PIN_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[365], "MDAS_TD_RAGRP_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[342], "MDAS_TD_RSAC_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[30], "MDAS_TD_RSRC_2TYP" + nll
				+ "rsrc_typ_name" + nll);
		jargonToSRB.put(catalogNames[271], "MDAS_TD_RSRC_3TYP" + nll
				+ "rsrc_typ_name" + nll);
		jargonToSRB.put(catalogNames[314], "MDAS_TD_RSRC_ACCS" + nll
				+ "access_constraint" + nll);
		jargonToSRB.put(catalogNames[358], "MDAS_TD_RSRC_ACCS" + nll
				+ "access_id" + nll);
		jargonToSRB.put(catalogNames[33], "MDAS_TD_RSRC_ACCS" + nll
				+ "access_list" + nll);
		jargonToSRB.put(catalogNames[313], "MDAS_TD_RSRC_ACCS" + nll
				+ "access_privilege" + nll);
		jargonToSRB.put(catalogNames[59], "MDAS_TD_RSRC_CLASS" + nll
				+ "rsrc_class_name" + nll);
		jargonToSRB.put(catalogNames[363], "MDAS_TD_RSRC_PARENT" + nll
				+ "rsrc_typ_name" + nll);
		jargonToSRB.put(catalogNames[13], "MDAS_TD_RSRC_TYP" + nll
				+ "rsrc_typ_name" + nll);
		jargonToSRB.put(catalogNames[333], "MDAS_TD_RS_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[69], "MDAS_TD_SRVR_LOCN" + nll
				+ "locn_desc" + nll);
		jargonToSRB.put(catalogNames[70], "MDAS_TD_SRVR_LOCN" + nll
				+ "netprefix" + nll);
		jargonToSRB.put(catalogNames[295], "MDAS_TD_UDF" + nll + "udf" + nll);
		jargonToSRB.put(catalogNames[296], "MDAS_TD_UDF" + nll + "udf" + nll);
		jargonToSRB.put(catalogNames[297], "MDAS_TD_UDF" + nll + "udf" + nll);
		jargonToSRB.put(catalogNames[298], "MDAS_TD_UDF" + nll + "udf" + nll);
		jargonToSRB.put(catalogNames[299], "MDAS_TD_UDF" + nll + "udf" + nll);
		jargonToSRB.put(catalogNames[5], "MDAS_TD_USER_TYP" + nll
				+ "user_typ_name" + nll);
		jargonToSRB.put(catalogNames[362], "MDAS_TD_UTYP_PARENT" + nll
				+ "user_typ_name" + nll);
		jargonToSRB.put(catalogNames[304], "MDAS_TD_ZN_ATHSCHM" + nll
				+ "auth_scheme" + nll);
		jargonToSRB.put(catalogNames[312], "MDAS_TD_ZN_DOMN" + nll
				+ "domain_desc" + nll);
		jargonToSRB.put(catalogNames[360], "MDAS_TD_ZN_LOCN" + nll
				+ "locn_desc" + nll);
		jargonToSRB.put(catalogNames[302], "MDAS_TD_ZN_LOCN" + nll
				+ "netprefix" + nll);
		jargonToSRB.put(catalogNames[301], "MDAS_TD_ZONE" + nll
				+ "local_zone_flag" + nll);
		jargonToSRB.put(catalogNames[303], "MDAS_TD_ZONE" + nll + "port_number"
				+ nll);
		jargonToSRB.put(catalogNames[309], "MDAS_TD_ZONE" + nll
				+ "zone_comments" + nll);
		jargonToSRB.put(catalogNames[310], "MDAS_TD_ZONE" + nll
				+ "zone_contact" + nll);
		jargonToSRB.put(catalogNames[307], "MDAS_TD_ZONE" + nll
				+ "zone_create_date" + nll);
		jargonToSRB.put(catalogNames[300], "MDAS_TD_ZONE" + nll + "zone_id"
				+ nll);
		jargonToSRB.put(catalogNames[308], "MDAS_TD_ZONE" + nll
				+ "zone_modify_date" + nll);
		jargonToSRB.put(catalogNames[306], "MDAS_TD_ZONE" + nll + "zone_status"
				+ nll);
		jargonToSRB.put(catalogNames[294], "NONDISTINCT" + nll + "nondistinct"
				+ nll);

		for (int i = 0; i < catalogNames.length; i++) {
			jargonToSRBID.put(catalogNames[i], new Integer(i));
		}
		// load the definable metadata
		jargonToSRBID.put(DEFINABLE_METADATA_FOR_FILES, new Integer(-1));
		jargonToSRBID.put(DEFINABLE_METADATA_FOR_DIRECTORIES, new Integer(-2));
		jargonToSRBID.put(DEFINABLE_METADATA_FOR_USERS, new Integer(-3));
		jargonToSRBID.put(DEFINABLE_METADATA_FOR_RESOURCES, new Integer(-4));
	} // end static

	SRBMetaDataSet(SRBProtocol protocol) {
		super();

		this.protocol = protocol;
	}

	static {
		if (protocol == null) {
			protocol = new SRBProtocol();
		}

		// *******************************************************************
		// API metadata field names related to the integers sent to SRB for
		// metadata
		// *******************************************************************

		// -1,-2,-3,-4,-5 Are used to set the
		// definable files, directories, users, resources, or extensible
		// metadata

		// *********************************************************************
		// The static variables defined above, used to translated the
		// SRBFieldName
		// into the api fieldName for interpreting query results, are also be
		// used
		// to relate the API fieldName to the SRB ID integer.
		//
		// For example,
		// create a new selection, MetaDataSet.newSelection( "FILE_NAME" );
		// and FILE_NAME= "FILE_NAME";
		// so the API gets the ID from the hashmap, FILE_NAME= 2;
		// then the SRB Server is asked for metadata value 2.
		// The server returns a attribute name string "data_name"
		// and the value string "foo".
		// *********************************************************************

	}

	/**
	 * Given one of the static field names this method returns the SRB integer
	 * id for that name.
	 */
	static final int getSRBID(String fieldName) {
		Integer srbID = ((Integer) jargonToSRBID.get(fieldName));
		if (srbID == null) {
			// Not a standard SRB metadata attribute, might be extensible
			return -5;
		}
		return srbID.intValue();
	}

	/**
	 * Given one of the static field names this method returns the SRB metadata
	 * attribute name, as a string.
	 */
	// only use as of SRB 3.1 is ORDERBY
	static final String getSRBName(String fieldName) {
		int id = getSRBID(fieldName);

		// why not use the catalogNames[]?
		// what would possibly use the defined names?

		// The original names must be sent to the SRB server,
		// which it uses to order the result.

		return srbCatalogNames[id];
	}

	/**
	 * From the results of a SRB query the MetaDataField is returned.
	 */
	static final MetaDataField getGeneralMetaData(String srbTableName,
			String srbAttributeName) {
		// table names are stored without the schema name, but are returned with
		// it
		// only want to look at the table name so we use substring here.
		String temp = (String) srbToJargon.get(srbTableName
				.substring(srbTableName.lastIndexOf(".") + 1)
				+ nll + srbAttributeName + nll);

		if (temp == null) {
			// extensible metadata is stored with the schema name,
			// so we have to redo it
			temp = (String) srbToJargon.get(srbTableName + nll
					+ srbAttributeName + nll);
		}

		MetaDataField field = getField(temp);

		if (field == null) {
			throw new NullPointerException("This srb return value: "
					+ srbTableName + " : " + srbAttributeName
					+ ", is not supported in this version.");
		}

		return field;
	}

	/**
	 * Turns the JARGON api metadata attribute name into the SRB database table
	 * name.
	 */
	static final String getSRBDatabaseName(String fieldName) {
		// really just need to somehow reverse lookup of srbToJargon
		String databaseName = (String) jargonToSRB.get(fieldName);

		if (databaseName == null)
			throw new NullPointerException(
					"Programming error, was this metadata value: "
							+ fieldName
							+ ", given an integer value in SRBMetaDataSet.jargonNameToSRBID?");
		return databaseName;
	}

	// for extensible metadata
	/**
	 * Get all the metadata groups, including the SRB extensible metadata for
	 * this <code>fileSystem</code>
	 * 
	 * @param fileSystem
	 *            SRB filesystem to gather the extensible metadata.
	 */
	public static MetaDataGroup[] getMetaDataGroups(GeneralFileSystem fileSystem)
			throws IOException {
		return getMetaDataGroups(fileSystem, null);
	}

	/**
	 * Get all the metadata groups, including the SRB extensible metadata for
	 * this <code>fileSystem</code>
	 * 
	 * @param fileSystem
	 *            A SRB filesystem to gather the extensible metadata.
	 * @param zoneName
	 *            Get the list of extensible metadata from this zone/SRB-MCAT.
	 *            Each zone has its own extensible metadata tables.
	 */
	public static MetaDataGroup[] getMetaDataGroups(
			GeneralFileSystem fileSystem, String zoneName) throws IOException {
		if (!(fileSystem instanceof SRBFileSystem))
			return getMetaDataGroups();

		String[] selectFieldNames = { SRBMetaDataSet.EXTENSIBLE_SCHEMA_NAME,
				SRBMetaDataSet.EXTENSIBLE_TABLE_NAME,
				SRBMetaDataSet.EXTENSIBLE_ATTR_NAME,
				SRBMetaDataSet.EXTENSIBLE_ATTR_OUTSIDE_NAME, };
		MetaDataSelect selects[] = MetaDataSet.newSelection(selectFieldNames);

		setExtensibleSchema(fileSystem.query(null, selects));

		return getMetaDataGroups();
	}

	public static void setExtensibleSchema(GeneralFileSystem fileSystem,
			String schema) throws IOException {
		setExtensibleSchema(fileSystem, new String[] { schema });
	}

	public static void setExtensibleSchema(GeneralFileSystem fileSystem,
			String[] schemas) throws IOException {
		if (schemas == null)
			return;

		String inValue = schemas[0];
		for (int i = 1; i < schemas.length; i++) { 
			inValue += "," + schemas[i];
		}

		MetaDataCondition[] conditions = new MetaDataCondition[] {
				MetaDataSet.newCondition(SRBMetaDataSet.EXTENSIBLE_SCHEMA_NAME,
						MetaDataCondition.IN, inValue),
				MetaDataSet.newCondition(
						SRBMetaDataSet.EXTENSIBLE_ATTR_OUTSIDE_NAME,
						MetaDataCondition.LIKE, "") };

		String[] selectFieldNames = { SRBMetaDataSet.EXTENSIBLE_SCHEMA_NAME,
				SRBMetaDataSet.EXTENSIBLE_TABLE_NAME,
				SRBMetaDataSet.EXTENSIBLE_ATTR_NAME,
				SRBMetaDataSet.EXTENSIBLE_ATTR_OUTSIDE_NAME, };
		MetaDataSelect selects[] = MetaDataSet.newSelection(selectFieldNames);

		setExtensibleSchema(fileSystem.query(conditions, selects));
	}

	private static void setExtensibleSchema(MetaDataRecordList[] rl) {
		if (rl == null)
			return;

		// prepare for creating extensible groups
		String groupName = rl[0].getStringValue(0); // SCHEMA_NAME
		MetaDataGroup group = new MetaDataGroup(groupName,
				"SRB Extensible Schema");

		for (int i = 0; i < rl.length; i++) {
			if (!groupName.equals(rl[i].getStringValue(0))) {
				add(group);
				groupName = rl[i].getStringValue(0); // SCHEMA_NAME
				group = new MetaDataGroup(groupName, "SRB Extensible Schema");
			}

			group.add(new MetaDataField(rl[i].getStringValue(3), "" /*
																	 * rl[i].getStringValue
																	 * (4)
																	 */,
					MetaDataField.STRING, protocol, rl[i].getStringValue(1)));
			Integer intX = new Integer(EXTENSIBLE_START_NUM + i);
			srbToJargon.put(rl[i].getStringValue(1) + nll
					+ rl[i].getStringValue(2) + nll, rl[i].getStringValue(3));
			jargonToSRBID.put(rl[i].getStringValue(3), intX);
		}
		if (group != null)
			add(group);
	}

	/**
   *
   */
	static MetaDataField getField(GeneralFileSystem fileSystem, String fieldName) {
		return null;
	}
}
