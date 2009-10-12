//  Copyright (c) 2008, Regents of the University of California
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
//  IRODSMetaDataRecordList.java  -  edu.sdsc.grid.io.irods.IRODSMetaDataRecordList
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.MetaDataRecordList
//              |
//              +-.IRODSMetaDataRecordList
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.*;

import java.io.IOException;

/**
 * Results of long queries will only return a partial list
 * to save on bandwidth which can be iterated through by further calls to
 * the server.
 *<P>
 * IRODSMetaDataRecordList works closely with the file server to do a
 * multi-step query that does not have to return everything immediately.
 * This class, for instance, works with partial query
 * results and, on need, issues a query for the next batch of results.
 * If some of the results are never asked for, they are never retreived.
 * To the caller, some requests for a record are immediate while others
 * pause while the partial query is sent.
 *
 * @author  Lucas Gilbert, San Diego Supercomputer Center
 * @since   JARGON2.0
 */
public class IRODSMetaDataRecordList extends MetaDataRecordList
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------





//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
  /**
   * If the query returned a partial list, this value is used to obtain
   * more records from the server.
   */
  private int continuationIndex = -1;

  /**
   * Contains an the number of user definable metadata rows attached to
   * each record in the query. Used to get the number of results expected
   * and all their user definable metadata.
   */
//  private IRODSMetaDataRecordList[] metaDataNums;
//  private int metaDataNumAt;
  
  
  private IRODSCommands irodsConnection;

//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
  /**
   * Load the IRODSMetaDataRecordList internal field list
   * with the selectFields passed to the constructor.
   *<P>
   * Then copy the queryReturn into records[][].
   * The order has to be switched to match the order of fields[],
   * which is the same order as the selectArray initially sent.
   */
  IRODSMetaDataRecordList( IRODSCommands irodsConnection,
    MetaDataField[] fields, Object[] recordValues, int continuationIndex )
  {
    super(fields, recordValues);

    this.irodsConnection = irodsConnection;
    this.continuationIndex = continuationIndex;
  }

  /**
   * Create a new MetaDataRecordList with this <code>field</code> and
   * <code>recordValue</code>.
   */
  public IRODSMetaDataRecordList( MetaDataField field, int recordValue )
  {
    super(field, recordValue);
  }

  /**
   * Create a new MetaDataRecordList with this <code>field</code> and
   * <code>recordValue</code>.
   */
  public IRODSMetaDataRecordList( MetaDataField field, float recordValue )
  {
    super(field, recordValue);
  }


  /**
   * Create a new MetaDataRecordList with this <code>field</code> and
   * <code>recordValue</code>.
   */
  public IRODSMetaDataRecordList( MetaDataField field, Integer recordValue )
  {
    super(field, recordValue);
  }

  /**
   * Create a new MetaDataRecordList with this <code>field</code> and
   * <code>recordValue</code>.
   */
  public IRODSMetaDataRecordList( MetaDataField field, Float recordValue )
  {
    super(field, recordValue);
  }


  /**
   * Create a new MetaDataRecordList with this <code>field</code> and
   * <code>recordValue</code>.
   */
  public IRODSMetaDataRecordList( MetaDataField field, String recordValue )
  {
    super(field, recordValue);
  }


  /**
   * Finalizes the object by explicitly letting go of each of
   * its internally held values.
   */
  protected void finalize( )
  {
    if (irodsConnection != null)
      irodsConnection = null;
  }



//----------------------------------------------------------------------
//  Methods
//----------------------------------------------------------------------
  /**
   * Used by IRODSCommands during a query return.
   * if all the fields that this and <code>recordList</code> have in common
   * have matching values, then this.addRecord( recordList )
   * The values in this IRODSMetaDataRecordList will be overwritten by
   * <code>recordList</code>. (Though in the case of IRODSCommands the
   * values will always be equal.)
   */
  boolean combineRecordLists( MetaDataRecordList recordList )
  {
    //TODO er, not actually used by iRODS? just copied over from SRB
    if (recordList != null) {
      for (int i=0;i<fields.length;i++) {
        for (int j=0;j<recordList.getFieldCount();j++) {
          if (fields[i].equals(recordList.getField(j))) {
            if ((records[i] == null) || (recordList.getValue(j) == null)) {
              if (records[i] != recordList.getValue(j)) {
                return false;
              }
            }
            else if (!records[i].equals(recordList.getValue(j))) {
              //both RecordLists have the same field but with different values
              return false;
            }
          }
        }
      }


        for (int j=0;j<recordList.getFieldCount();j++) {
            addRecord( recordList.getField(j), recordList.getValue(j) );
        }

      return true;
    }
    else {
      return false;
    }
  }


  /**
   * Sets the int used by the IRODS server to refer to a query.
   * (Kind of like a file descriptor.)
   */
  void setContinuationIndex( int continuationIndex )
  {
    this.continuationIndex = continuationIndex;
  }

  /**
   * Returns the int used by the IRODS server to refer to a query.
   * (Kind of like a file descriptor.)
   */
  int getContinuationIndex( )
  {
    return continuationIndex;
  }



  /**
   * Tests if this IRODSMetaDataRecordList can return more values from the
   * query.
   */
  public boolean isQueryComplete()
  {
    if (continuationIndex >= 0) {
      return false;
    }

    return true;
  }


  /**
   * Gets further results from the query. If and only if the
   * query returned a partial list and there are more results
   * which matched the query that haven't been returned. Otherwise null.
   * By default a query will only return 300 values at a time which match the
   * query, see also <code>IRODSFileSystem.DEFAULT_RECORDS_WANTED</code>.
   */
  public MetaDataRecordList[] getMoreResults()
    throws IOException
  {
    return getMoreResults(IRODSFileSystem.DEFAULT_RECORDS_WANTED);
  }


  /**
   * Gets further results from the query. If and only if the
   * query returned a partial list and there are more results
   * which matched the query that haven't been returned. Otherwise null.
   */
  public MetaDataRecordList[] getMoreResults( int numOfResults )
    throws IOException
  {
    if (continuationIndex < 0 ) {
      return null;
    }
    if (irodsConnection == null) {
      //Maybe more query is left and something went wrong
      //but irodsConnection wasn't loaded.
      return null;
    }
    
    return irodsConnection.getMoreResults(continuationIndex, numOfResults);
  }
}
