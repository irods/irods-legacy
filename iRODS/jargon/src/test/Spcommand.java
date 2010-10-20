//	Copyright (c) 2005, Regents of the University of California
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

import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;
import java.io.*;
import java.net.*;

public class Spcommand
{

//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
	/**
	 * Testing
	 */
	public static void main(String args[])
	{
		SRBFileSystem srbFileSystem = null;
		String host = null;
		String command = null;
		String commandArgs = null;
    String uri = null;

		try{

			if ((args.length >= 3) && (args[0].equals( "-host" ))) {
				host = args[1];
				command = args[2];

				if (args.length >= 4) {
					commandArgs = "";
					for (int i=3;i<args.length;i++) {
						commandArgs += args[i]+" ";
					}
				}
			}
      else if ((args.length >= 3) && (args[0].equals( "-uri" ))) {
				uri = args[1];
				command = args[2];

				if (args.length >= 4) {
					commandArgs = "";
					for (int i=3;i<args.length;i++) {
						commandArgs += args[i]+" ";
					}
				}
			}
			else if (args.length >= 1) {
				command = args[0];

				if (args.length >= 2) {
					commandArgs = "";
					for (int i=1;i<args.length;i++) {
						commandArgs += args[i]+" ";
					}
				}
			}
			else {
				throw new IllegalArgumentException(
					"\nUsage: Spcommand [-host hostServer] command");// [commandArg commandArg...]" );
			}      
      if (uri != null) {
  			srbFileSystem = (SRBFileSystem) FileFactory.newFileSystem( new URI( uri ) );
      }
      else {
        srbFileSystem = new SRBFileSystem( );
      }
      
			InputStream in = srbFileSystem.executeProxyCommand(
				command, commandArgs, host, -1 );
			int result = in.read();
			while (result != -1) {
				System.out.print((char)result);
				result = in.read();
			}
		}
		catch( SRBException e ) {
			//Get the standard error string
			String bar = e.getStandardMessage( );
			System.out.println( "Standardized SRB Server Error Message: "+ bar );

			//The original error message is still available through
			System.out.println( "\nJava Error Message: "+ e.getMessage() );


			e.printStackTrace();
		}
		catch ( Throwable e ) {
			e.printStackTrace();
		}


		System.exit(0);
	}
}
