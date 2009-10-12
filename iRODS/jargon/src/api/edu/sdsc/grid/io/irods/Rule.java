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
//  Rule.java  -  edu.sdsc.grid.io.irods.Rule
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.irods.Rule
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalFileOutputStream;
import edu.sdsc.grid.io.local.LocalFileInputStream;

import java.io.*;
import java.util.StringTokenizer;
import java.util.HashMap;
import java.util.Vector;

/**
 * Java object for an IRODS rule. see https://www.irods.org/index.php/Rules
 *<P>
 * @author  Lucas Gilbert
 * @since   JARGON2.0
 */
class Rule
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
//TODO which class to keep these
  static final String INT_PI = "INT_PI";
  static final String myInt = "myInt";
  static final String STR_PI = "STR_PI";
  static final String myStr = "myStr";
  static final String BUF_LEN_PI = "BUF_LEN_PI";
  static final String BinBytesBuf_PI = "BinBytesBuf_PI";
  static final String ExecCmdOut_PI = "ExecCmdOut_PI";
  static final String DataObjInp_PI = "DataObjInp_PI";
  
  static final String CL_PUT_ACTION = "CL_PUT_ACTION";  
  static final String CL_GET_ACTION = "CL_GET_ACTION";  
  
  static final String buflen = "buflen";
  static final String buf = "buf";

  private static final int RULE_SIZE_BUFFER = 64000;
//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
  String label;

  String type;

  /**
   * Could be String, int, byte array or more?
   */
  Object parameter;

  String ruleName;
  Method[] methods;
  Parameter[] outputs;

//internal
Parameter[] inputs;

//----------------------------------------------------------------------
//  Constructor
//----------------------------------------------------------------------
//TODO no good?
  Rule( String label, String type, Object parameter )
  {
    
    this.label = label;
    this.type = type;
    this.parameter = parameter;
  }
  
  Rule( String ruleName, Method[] microservices, Parameter[] outputs )
  {
    this.ruleName = ruleName;
    methods = microservices;
    this.outputs = outputs;
  }


  //internal
  Rule( String ruleName, Parameter[] inputs, Parameter[] outputs )
  {
    this.ruleName = ruleName;
    this.inputs = inputs;
    this.outputs = outputs;
  }


//----------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------
/*
  String getRuleName( )
  {
    return ruleName;
  }

  String getMethodsStrings( )
  {
    String temp = "";
    for (Method method : methods)
    {
//      temp += method.getMethodsString() + 
    }
    return temp;
  }

  String getLabel( )
  {
    return label;
  }

  String getType( )
  {
    return type;
  }

  String getStringParameter( )
  {
    if (type.equals(INT_PI)) {
      return parameter+"";
    }
    else if (type.equals(BUF_LEN_PI)) {
      return new String((byte[])parameter);
    }
    else { //if String or anything else
      return parameter.toString();
    }
  }


  /**
   * @throws NumberFormatException If the value is not an integer
   *
  int getIntParameter( )
    throws NumberFormatException
  {
    if (type.equals(INT_PI)) {
      //TODO um, what will it be?
      if (parameter instanceof Integer) {
        ((Integer)parameter).intValue();
      }
      else {
      }
    }
    else if (type.equals(BUF_LEN_PI)) {
      //acceptable to just return the first? first 4? probably should fail
//      return ((byte[])parameter)[0];
    }

    //if String or anything else
    return Integer.parseInt(parameter.toString());
  }

*/

//----------------------------------------------------------------------
// Static Methods
//----------------------------------------------------------------------
  /**
   * @return the parameter value of the parameter tag.
   * Other values, like buffer length, can be derived from it,
   * if the type is known.
   */
  static Object getParameter( String type, Tag parameterTag ) {
    if (type.equals(INT_PI)) {
      return parameterTag.getTag(INT_PI).getTag(myInt).getIntValue()+"";
    }
    else if (type.equals(BUF_LEN_PI)) {
      return parameterTag.getTag(BinBytesBuf_PI).getTag(buf).getStringValue();
    }
    else if (type.equals(ExecCmdOut_PI)) {
      Tag exec = parameterTag.getTag(ExecCmdOut_PI);

      //last tag is status
      int length = exec.getLength() - 1;
      String[] results = new String[length];
      for (int i=0;i<length;i++) {
        //TODO not always BinBytesBuf_PI return?
        if (exec.getTag(BinBytesBuf_PI, i).getTag(buflen).getIntValue() > 0) {
          results[i] =
            exec.getTag(BinBytesBuf_PI, i).getTag(buf).getStringValue();
        }
      }
      return results;
    }
    else if (type.equals(STR_PI)) {
      return parameterTag.getTag(STR_PI).getTag(myStr).getStringValue();
    }
    else { //TODO return everything else as tag?
      return parameterTag.getTag(type);      
    }
  }

  static Parameter[] executeRule( 
    IRODSFileSystem fileSystem, InputStream ruleStream )
    throws IOException
  {
    String total = null;
    //Probably should just read the whole thing in one buffer, but what size?
    byte[] b = new byte[RULE_SIZE_BUFFER];
    int read;
    while((read = ruleStream.read(b)) != -1) {
      total += new String(b, 0, read);
    }
    
    StringTokenizer tokens = new StringTokenizer(total, "\n");
    String rule = "";
    Vector<Parameter> inputs = new Vector();
    Vector<Parameter> outputs = new Vector();
    int index = 0, index2 = 0;
    
    //if formatting error, such as only one line, below breaks
    if (!tokens.hasMoreTokens()) 
      throw new IllegalArgumentException("Rule stream is malformed");
    
    //Remove comments
    total = tokens.nextToken();
    while (total.startsWith("#")) {
      total = tokens.nextToken();
    }    
    //find the rule
    rule = total;
    
    //if formatting error, such as only one line, below breaks
    if (!tokens.hasMoreTokens()) 
      throw new IllegalArgumentException("Rule stream is malformed");
    //find the labels
    total = tokens.nextToken();
    index = total.indexOf('%');
    while (index >= 0) {      
      index2 = total.indexOf('=');
      if (index2 < 0)
        throw new IllegalArgumentException("Rule stream is malformed");
      inputs.add( new Parameter( 
        //label
        total.substring(0, index2), 
        //value
        total.substring(index2+1, total.indexOf('%', index2)))
      );
      total = total.substring( index+1 ); //TODO yes, I know.
      index = total.indexOf("%");
    } 
    index2 = total.indexOf('=');
    if (index2 < 0)
      throw new IllegalArgumentException("Rule stream is malformed");
    //add the final one     
    inputs.add( new Parameter( 
      //label
      total.substring(0, index2), 
      //value
      total.substring(index2+1) )
    );
    
    //if formatting error, such as only one line, below breaks
    if (!tokens.hasMoreTokens()) 
      throw new IllegalArgumentException("Rule stream is malformed");
    //find the outputs
    total = tokens.nextToken();
    index = total.indexOf('%');
    while (index >= 0) {
      outputs.add(new Parameter( total.substring(0, index), null, null ));
      total = total.substring( index+1 );
      index = total.indexOf("%");
    } 
    //add the final one
    outputs.add(new Parameter(total, null, null));
    
//TODO new Parameter[0]s
    return Rule.readResult( fileSystem, fileSystem.commands.executeRule( rule, 
      inputs.toArray(new Parameter[0]), outputs.toArray(new Parameter[0]) ));
  }
/*
  static Parameter[] readRules( String ruleString )
    throws IOException
  {
    return readRules( IRODSCommands.readNextTag(ruleString.getBytes()) );
  }
*/
  static Parameter[] readResult( IRODSFileSystem fileSystem, Tag rulesTag )
    throws IOException
  {
/*  "<MsParamArray_PI><paramLen>2</paramLen><oprType>0</oprType>"+
"<MsParam_PI><label>*A</label><type>STR_PI</type>" +
"<STR_PI><myStr>getErrorStr</myStr></STR_PI></MsParam_PI>" +
"<MsParam_PI><label>*B</label><type>STR_PI</type>" +
"<STR_PI><myStr>513000</myStr></STR_PI></MsParam_PI></MsParamArray_PI>") );
*/
    int parametersLength = rulesTag.getTag(IRODSCommands.paramLen).getIntValue();

    if (parametersLength > 0) {
      String label = null;
      String type = null;
      Object value = null;
      Parameter[] parameters = new Parameter[parametersLength];

      for (int i=0;i<parametersLength;i++) {
        Tag msParam = rulesTag.getTag(IRODSCommands.MsParam_PI, i);
        label = msParam.getTag(IRODSCommands.label).getStringValue();
        type = msParam.getTag(IRODSCommands.type).getStringValue();
        value = getParameter(type, msParam);

        if (value instanceof Tag) {
//TODO? should check intInfo if ==  SYS_SVR_TO_CLI_MSI_REQUEST 99999995      
/*lib/core/include/rodsDef.h

// this is the return value for the rcExecMyRule call indicating the
// server is requesting the client to client to perform certain task 
#define SYS_SVR_TO_CLI_MSI_REQUEST 99999995
#define SYS_SVR_TO_CLI_COLL_STAT 99999996
#define SYS_CLI_TO_SVR_COLL_STAT_REPLY 99999997

// definition for iRods server to client action request from a microservice.
// these definitions are put in the "label" field of MsParam 

#define CL_PUT_ACTION   "CL_PUT_ACTION"
#define CL_GET_ACTION   "CL_GET_ACTION"
#define CL_ZONE_OPR_INX "CL_ZONE_OPR_INX"

*/          
          //server is requesting client action          
          Tag fileAction = msParam.getTag(type);
          
          IRODSFile irodsFile = new IRODSFile( 
            fileSystem, fileAction.getTag("objPath").getStringValue() );

          String otherFilePath = fileAction.getTag(
            "KeyValPair_PI").getTag("svalue").getStringValue();
          String otherFileType = fileAction.getTag(
            "KeyValPair_PI").getTag("keyWord").getStringValue();
          GeneralFile otherFile = null;
          if (otherFileType.equals("localPath")) {
            otherFile = new LocalFile(otherFilePath);
          }
          else if (false) {
            //TODO what is the irods type  also what other types are there?
          }
          else {            
            throw new IllegalArgumentException(
              "Rule requests tranfer from unknown protocol");
          }
          
          if (label.equals(CL_GET_ACTION)) {
            if (otherFile.exists()) {
              throw new IOException( otherFile+" already exists");
            }
            fileSystem.commands.get( irodsFile, otherFile );
            
            //return the destination? should return something...
            value = otherFile; 
          }
          else if (label.equals(CL_PUT_ACTION)) {  
            //can't check overwrite, causes problems search commands for RError
            fileSystem.commands.put( otherFile, irodsFile, false );
            value = irodsFile;
          }
          else {
            throw new UnsupportedOperationException(
              "Rule requests unknown action");
          }          
        }
        parameters[i] = new Parameter(label, value, type);
      }


      fileSystem.commands.operationComplete(0);
      return parameters;
    }
    return null;
  }
}