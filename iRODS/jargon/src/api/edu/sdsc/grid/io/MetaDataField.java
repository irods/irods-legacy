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
//  MetaDataField.java  -  edu.sdsc.grid.io.MetaDataField
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.MetaDataField
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io;



/**
 * A "meta data field" is an attribute of a file. It could be the
 * name of the file, it's creation date, it's owner, or any of the
 * other types of metadata maintained by the SRB or whomever.
 *<P>
 * Metadata groups contain a list of metadata fields. Each field is
 * described by a MetaDataField object. The object may be queried
 * to get a description of the field, the data types it supports, etc.
 *<P>
 * There are no 'set' methods. Once constructed, the object cannot
 * be changed.
 *<P>
 * @author  Lucas Gilbert, San Diego Supercomputer Center
 */
public final class MetaDataField implements Comparable
{

  /**
   * The value type for the field.
   * Types include: INT, LONG, FLOAT, STRING, DATE, TABLE.
   */
  public static final int INT = 0;

  /**
   * The value type for the field.
   * Types include: INT, LONG, FLOAT, STRING, DATE, TABLE.
   */
  public static final int LONG = 1;

  /**
   * The value type for the field.
   * Types include: INT, LONG, FLOAT, STRING, DATE, TABLE.
   */
  public static final int FLOAT = 2;

  /**
   * The value type for the field.
   * Types include: INT, LONG, FLOAT, STRING, DATE, TABLE.
   */
  public static final int STRING = 3;

  /**
   * The value type for the field.
   * Types include: INT, LONG, FLOAT, STRING, DATE, TABLE.
   */
  public static final int DATE = 4;

  /**
   * The value type for the field.
   * Types include: INT, LONG, FLOAT, STRING, DATE, TABLE.
   */
  public static final int TABLE = MetaDataCondition.TABLE;


  /**
   * The attribute name
   */
  private String fieldName;


  /**
   * The human readable description for this field.
   */
  private String description;


  /**
   * The datatype of this field.
   */
  private int type;


  /**
   * Extensible metadata formats, such as the SRB extesible metadata,
   * need to know the extensible table name of such a field.
   * key = protocol, value = the extensible table name.
   */
  private String[] extensible;

  /**
   *
   */
  private Protocol[] protocols = new Protocol[1];


  /**
   * Constructs a field description.
   * If a type argument is given, the field is marked as
   * int, float, string, date or table, whichever is indicated.
   * If an int range pair is given, the field's data type
   * is int. If the range pair are floats, the data type
   * is set to float. If an enum is given, the data type
   * is always String.
   */
  public MetaDataField( String fieldName, String description, int type,
    Protocol protocol )
  {
    this.fieldName = fieldName;
    this.description = description;
    this.type = type;
    addProtocol( protocol );
  }

  /**
   * Constructs a field description.
   * If a type argument is given, the field is marked as
   * int, float, string, date or table, whichever is indicated.
   * If an int range pair is given, the field's data type
   * is int. If the range pair are floats, the data type
   * is set to float. If an enum is given, the data type
   * is always String.
   *
   * @param the extra extensible schema or table name used to make this
   *    value uniquely refer to a certain metadata attribute.
   */
  public MetaDataField( String fieldName, String description, int type,
    Protocol protocol, String extensibleName )
  {
    this.fieldName = fieldName;
    this.description = description;
    this.type = type;
    if (extensibleName != null) {
      addProtocol( protocol, extensibleName );
    }
    else {
      addProtocol( protocol );
    }
  }


  /**
   * Returns the name of the field.
   */
  public String getName( )
  {
    return fieldName;
  }


  /**
   * The description string may be displayed by a GUI. It has no
   * embedded carriage returns, so the application is
   * expected to insert line breaks to wrap the text
   * appropriately for its way of displaying it.
   *
   * @return A description string of the field.
   */
  public String getDescription( )
  {
    return description;
  }



  /**
   * Returns the data type code for the field. Data types
   * include: int, float, string, date, filepath, etc.
   */
  public int getType( )
  {
    return type;
  }


  /**
   * Returns the protocols for this field.
   */
  Protocol[] getProtocols( )
  {
    return protocols;
  }

  /**
   * Returns the protocol for this field at the specified index.
   */
  Protocol getProtocol( int index )
  {
    return protocols[index];
  }

  /**
   * @return The extra extensible schema or table name used to make this
   *    value uniquely refer to a certain metadata attribute.
   */
  public String getExtensibleName( Protocol protocol )
  {
    if (extensible != null) {
      if (protocol != null) {
        for (int i=0;i<protocols.length;i++) {
          if (protocol.equals(protocols[i])) {
            return extensible[i];
          }
        }
      }
      else {
        return MetaDataSet.DEFINABLE_METADATA; 
      }
    }

    return null;
  }


  /**
   * Returns true if and only if
   * this field is an extensible field under the given protocol.
   */
  public boolean isExtensible( Protocol protocol )
  {
    if (extensible == null)
      return false;

    for (int i=0; i<protocols.length; i++ ) {
      if ( protocols[i].equals( protocol ) ) {
        if (extensible[i] != null)
          return true;
      }
    }

    return false;
  }


  /**
   * Test if this field uses the given protocol.
   */
  public boolean usesProtocol( Protocol protocol )
  {
    for (int i=0; i<protocols.length; i++ ) {
      if ( protocols[i].equals( protocol ) ) {
        return true;
      }
    }
    return false;
  }




  /**
   * Adds a metadata protocol for this field.
   */
  public void addProtocol( Protocol protocol )
  {
    //if first one
    if (protocols[0] == null) {
      protocols[0] = protocol;
      return;
    }

    //check see if already exists
    for (int i=0; i<protocols.length; i++ ) {
      if ( protocols[i].equals( protocol ) ) {
        return;
      }
    }

    Protocol[] temp = protocols;
    protocols = new Protocol[temp.length+1];
    System.arraycopy( temp, 0, protocols, 0, temp.length );

    protocols[temp.length] = protocol;
  }



  /**
   * Adds an extensible name value, associated with a metadata protocol.
   */
  public void addProtocol( Protocol protocol, String extensibleName )
  {
    addProtocol( protocol );
    if (extensible == null) {
      extensible = new String[1];
      extensible[0] = extensibleName;
    }
    else {
      for (int i=0; i<protocols.length; i++ ) {
        if ( protocols[i].equals( protocol ) ) {
          extensible[i] = extensibleName;
          return;
        }
      }
    }
  }


  /**
   * Tests this MetaDataField object for equality with the given object.
   * Returns <code>true</code> if and only if the argument is not
   * <code>null</code> and both are MetaDataField objects with equal
   * name and description values;
   *
   * @param   obj   The object to be compared with this abstract pathname
   *
   * @return  <code>true</code> if and only if the objects are the same;
   *          <code>false</code> otherwise
   */
  public boolean equals( Object obj )
  {
    MetaDataField field = null;

    if (obj == null)
      return false;

    try {
      field = (MetaDataField) obj;
    } catch (ClassCastException e) {
      return false;
    }

    Protocol[] protocol = field.getProtocols();
    String extensibleName = null, extensibleName2 = null;

    if (fieldName == field.getName()) {
      if (description == field.getDescription()) {
        if (type == field.getType()) {
          if (extensible != null) {
            for (int i=0;i<protocol.length;i++) {
              extensibleName = getExtensibleName(protocol[i]);
              extensibleName2 = field.getExtensibleName(protocol[i]);
              if (extensibleName != null) {
                if (!extensibleName.equals(extensibleName2)) {
                  return false;
                }
              }
              else if ((extensibleName2 != null) &&
                       (extensibleName == null))
              {
                return false;
              }
            }
            //all the protocols and extensible match
          }
          return true;
        }
      }
    }
    return false;
  }



  /**
   * Returns a string representation of the object.
   */
  public String toString()
  {
    return fieldName+": "+description;
  }


  public int compareTo( Object obj )
  {
  
    return toString().compareTo( obj.toString() );
  }
}
