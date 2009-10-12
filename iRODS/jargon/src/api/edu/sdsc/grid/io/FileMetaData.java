//  Copyright (c) 2007, Regents of the University of California
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
//  FileMetaData.java  -  edu.sdsc.grid.io.FileMetaData
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.StandardMetaData
//            |
//            +-.GeneralMetaData
//                |
//                +-.FileMetaData
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io;


/**
 * The metadata naming interface for file metadata.
 *
 * @author  Lucas Gilbert, San Diego Supercomputer Center
 */
public interface FileMetaData extends GeneralMetaData
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------

//public static final String FILE_DATA_ID = "File Identifier";

  public static final String FILE_COMMENTS = "File Comments";
  public static final String FILE_TYPE = "File Type";
  public static final String FILE_VERSION = "File Version";
  public static final String FILE_CHECKSUM = "File Checksum";
  public static final String FILE_REPLICA_NUM = "File Replica Number";
  public final static String PATH_NAME = "PATH_NAME";


/*TODO which are general enough?
  //srb
  public final static String OWNER_EMAIL = "OWNER_EMAIL";
  public final static String OWNER_DOMAIN = "OWNER_DOMAIN";
  public final static String FILE_IS_COMPRESSED = "FILE_IS_COMPRESSED";
  public final static String FILE_IS_ENCRYPTED = "FILE_IS_ENCRYPTED";
  public final static String IS_DIRTY = "IS_DIRTY";
  public final static String FILE_IS_DELETED = "FILE_IS_DELETED";
  public final static String FILE_HIDE = "FILE_HIDE";
  public final static String FILE_REPLICATION_ENUM = "FILE_REPLICATION_ENUM";
  public final static String FILE_ACCESS_LIST = "FILE_ACCESS_LIST";
  public final static String FILE_GROUP_ID = "FILE_GROUP_ID";
  public final static String FILE_AUDITFLAG = "FILE_AUDITFLAG";
  public final static String FILE_LOCK_EXPIRY = "FILE_LOCK_EXPIRY";
  public final static String FILE_PIN_VAL = "FILE_PIN_VAL";
  public final static String FILE_PIN_OWNER_NAME = "FILE_PIN_OWNER_NAME";
  public final static String FILE_PIN_OWNER_DOMAIN = "FILE_PIN_OWNER_DOMAIN";
  public final static String FILE_PIN_EXPIRY = "FILE_PIN_EXPIRY";
  public final static String FILE_EXPIRY_DATE = "FILE_EXPIRY_DATE";
  public final static String FILE_EXPIRE_DATE_2 = "FILE_EXPIRE_DATE_2";
  public final static String FILE_SEG_NUM = "FILE_SEG_NUM";
  public final static String FILE_CLASS_NAME = "FILE_CLASS_NAME";
  public final static String FILE_CLASS_TYPE = "FILE_CLASS_TYPE";
  public final static String FILE_LAST_ACCESS_TIMESTAMP = "FILE_LAST_ACCESS_TIMESTAMP";
  public final static String FILE_VER_NUM = "FILE_VER_NUM";
  public final static String FILE_LOCK_NUM = "FILE_LOCK_NUM";
  public final static String FILE_LOCK_DESCRIPTION = "FILE_LOCK_DESCRIPTION";
  public final static String FILE_LOCK_OWNER_NAME = "FILE_LOCK_OWNER_NAME";
  public final static String FILE_LOCK_OWNER_DOMAIN = "FILE_LOCK_OWNER_DOMAIN";
*/
  
  
  /**
   * File Access Data Identifier
   */
  public static final String FILE_ACCESS_DATA_ID = "File Access Data Identifier";

  /**
   * File Access User Identifier
   */
  public static final String FILE_ACCESS_USER_ID = "File Access User Identifier";

//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------


}

