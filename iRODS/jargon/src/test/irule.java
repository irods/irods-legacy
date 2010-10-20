//	Copyright (c) 2008, Regents of the University of California
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are
//	met:
//
//	  * Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//	  * Redistributions in binary form must reproduce the above copyright
//	notice, this list of conditions and the following disclaimer in the
//	documentation and/or other materials provided with the distribution.
//	  * Neither the name of the University of California, San Diego (UCSD) nor
//	the names of its contributors may be used to endorse or promote products
//	derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import edu.sdsc.grid.io.irods.*;
import edu.sdsc.grid.io.*;
import java.io.*;
import java.net.*;
import java.util.*;

/**
 * An example utility class based on the iRODS icommand, irule.
 */
public class irule
{
  /**
   * An example usage of iRODS rules
   */
  static void example()
    throws IOException
  {
    //---------------------
    // Usually the rule is stored in a file.
    // So have to turn it into an inputStream.
    //---------------------      
    String rule = 
      //rules/microservices
      "myTestRule||msiDataObjOpen(*A,*S_FD)##msiDataObjCreate(*B,null,*D_FD)" +
      "##msiDataObjLseek(*S_FD,10,SEEK_SET,*junk1)" +
      "##msiDataObjRead(*S_FD,10000,*R_BUF)" +
      "##msiDataObjWrite(*D_FD,*R_BUF,*W_LEN)" +
      "##msiDataObjClose(*S_FD,*junk2)" +
      "##msiDataObjClose(*D_FD,*junk3)" +
      "##msiDataObjCopy(*B,*C,null,*junk4)" +
      "##delayExec(<PLUSET>2m</PLUSET>,msiDataObjRepl(*C,*Resource,*junk5),nop)" +
      "|nop\n" +

      //input parameters
      "*A=/tempZone/home/rods/foo1%*B=/tempZone/home/rods/foo2%" +
      "*C=/tempZone/home/rods/foo3%*Resource=nvoReplResc\n" +

      //outputparameters
      "*R_BUF%*W_LEN%*A";

    java.io.ByteArrayInputStream inputStream =
      new java.io.ByteArrayInputStream(rule.getBytes());


    //---------------------
    // Execute the rule
    //---------------------
    java.util.HashMap outputParameters = 
      new IRODSFileSystem().executeRule( inputStream );


    //---------------------
    // Get the output of output variable *A
    // Note: Rules can return objects besides strings and primitives, 
    // but that's not so likely as an end result. 
    //---------------------
    System.out.println(outputParameters.get("*A").toString());    
  }
  
  static void printAll( HashMap map )
  {
    Object[] keys = map.keySet().toArray(); 
    Object[] values = map.values().toArray();
    for (int i=0;i<keys.length;i++)
    {
      System.out.println(keys[i]+" : "+values[i]);
    }
  }
  
  
	/**
	 * Testing
	 */
	public static void main(String args[])
	{
		GeneralFileSystem fileSystem = null;
    String path = "";
    
		try{
			if ((args.length == 3) && args[0].equals("-uri")) {
				fileSystem = FileFactory.newFileSystem( new URI( args[1] ) );
        path = args[2];
			}
			else if (args.length == 1) {
				fileSystem = new IRODSFileSystem();
        path = args[0];
			}
			else if (args.length == 0) {
        example();
        System.exit(0);        
			}
			else {
				throw new IllegalArgumentException(
					"\nUsage: irule rule_filepath" +
          "\nUsage: irule -uri irods://... rule_filepath" );
			}

      java.util.HashMap outputParameters = 
        new IRODSFileSystem().executeRule( new FileInputStream(path) );
      
      printAll(outputParameters);
		}
		catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
      System.exit(1);
		}

		System.exit(0);
	}
}
