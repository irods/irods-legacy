//
//  Copyright (c) 2008  San Diego Supercomputer Center (SDSC),
//  University of California, San Diego (UCSD), San Diego, CA, USA.
//
//  Users and possessors of this source code are hereby granted a
//  nonexclusive, royalty-free copyright and design patent license
//  to use this code in individual software.  License is not granted
//  for commercial resale, in whole or in part, without prior written
//  permission from SDSC/UCSD.  This source is provided "AS IS"
//  without express or implied warranty of any kind.
//
//
//  FILE
//  Parameter.java  -  edu.sdsc.grid.io.irods.Parameter
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.irods.Parameter
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.lang.reflect.Array;

import edu.sdsc.grid.io.Base64;



/**
 * A microservice/rule parameter
 */
class Parameter 
{
  static final String INT_PI = "INT_PI";
  static final String myInt = "myInt";
  static final String STR_PI = "STR_PI";
  static final String myStr = "myStr";
  static final String BUF_LEN_PI = "BUF_LEN_PI";
  static final String BinBytesBuf_PI = "BinBytesBuf_PI";
  static final String ExecCmdOut_PI = "ExecCmdOut_PI";
  static final String buflen = "buflen";
  static final String buf = "buf";
  static final String NULL_PI = "NULL_PI";
  
  String uniqueName = "";
  
  Object value;  
  String type;
  
  Parameter( )
  {
    this( null, null, STR_PI );
  }  
  
  Parameter( int value )
  {
    this( null, new Integer(value), INT_PI );
  }  
  
  Parameter( String value )
  {
    this( null, value, STR_PI );
  }
  
  Parameter( byte[] value )
  {    
    this( null, value, BUF_LEN_PI );
  } 
  
  Parameter( String name, String value )
  {
    this( name, value, STR_PI );    
  }
  
  Parameter( String name, Object value, String type )
  {
    if (value == null) setNullValue();
    else {
      this.value = value;
      this.type = type;    
    }
    
    if (name != null) {
      uniqueName = name;
    }
    else {
      for (int i=0;i<8;i++)
        uniqueName += ((char) (65 + Math.random() * 25));
    }
  }
  
  /**
   * For parameters that do not have initial values.
   * Parameters that are not input values for the rule engine.
   */
  void setNullValue( )
  {
    this.value = "";
    type = NULL_PI;
  }
  
  void setIntValue( int value )
  {
    this.value = new Integer(value);
    type = INT_PI;
  }  
  
  void setStringValue( String value )
  {    
    this.value = value;
    type = STR_PI;
  }
  
  void setByteValue( byte[] value )
  {    
    this.value = value;
    type = BUF_LEN_PI;
  } 
  
  String getType()
  {
    return type;
  }
  
  int getIntValue()
  {
    if (value instanceof Integer)
      return ((Integer)value).intValue();
    else {
      //will fail on byte[]...
      return Integer.parseInt(value.toString());
    }
  }
  
	/**
	 * Get the value part of the parameter as a <code>String/code>.  Note that arrays are translated into Strings.
	 * @return <code>String</code> containing the value of the Parameter.
	 */
	String getStringValue() {
		if (value.getClass().isArray() && type.equals(ExecCmdOut_PI)) {

			StringBuilder stringValue = new StringBuilder();
			String msg = new String();
			int alength = Array.getLength(value);
			for (int ij = 0; ij < alength; ij++) {
				if (Array.get(value, ij) != null) {
					msg = Array.get(value, ij).toString();
					if (msg != null) {
						msg = new String(Base64.fromString(msg));
						if (msg != null) {
							stringValue.append(msg);
						}
					}
				}
			}
			return stringValue.toString();
		} else {
			return value.toString();
		}
	}
  
  byte[] getByteValue() 
  {
    if (value instanceof byte[])
      return (byte[])value;
    else
      return value.toString().getBytes();
  }
  
  public String toString()
  {
    return getStringValue();
  }
  
  
  String getUniqueName( )
  {
    return uniqueName;
  }
  
  Object getValue()
  {
    return value;
  }
  
  
  Tag createMsParamArray( )
  {
/*
<MsParamArray_PI>
<paramLen>2</paramLen>
<oprType>0</oprType>
<MsParam_PI>
<label>*A</label>
<type>STR_PI</type>
<STR_PI>
<myStr>getErrorStr</myStr>
</STR_PI>
</MsParam_PI>
<MsParam_PI>
<label>*B</label>
<type>STR_PI</type>
<STR_PI>
<myStr>513000</myStr>
</STR_PI>
</MsParam_PI>
</MsParamArray_PI>
*/
    Tag param = new Tag(IRODSConstants.MsParam_PI, new Tag[] {
        new Tag(IRODSConstants.label, getUniqueName()),
        new Tag(IRODSConstants.type, getType()),
    } );

    if (type.equals( INT_PI )) {
      param.addTag(
        new Tag(INT_PI, new Tag[] {
          //only one parameter, the int
          new Tag(myInt, getIntValue()),
        } )
      );
    }
    else if (type.equals(BUF_LEN_PI)) {
      param.addTag(
        new Tag(BUF_LEN_PI, new Tag[] {
          //send a byte buffer
          new Tag(buflen, getByteValue().length),
          //maybe convert to Base64?
          new Tag(buf, new String(getByteValue())),
        } )
      );
    }
    else {//STR_PI or NULL_PI
      param.addTag(
        new Tag(STR_PI, new Tag[] {
          //only one parameter, the string
          //if default, try sending the string value, might work...
          new Tag(myStr, getStringValue()),
        } )
      );
    }

    return param;
  }
}