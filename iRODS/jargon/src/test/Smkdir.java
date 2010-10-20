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
import java.net.*;

public class Smkdir
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

		GeneralFile srbFile = null;

		String[] list = null;

		try{
			if ((args.length == 2) && (args[0].equals("-uri"))) {
				srbFile = FileFactory.newFile( new URI( args[1] ) );
			}
			else if (args.length == 1) {
				srbFile = new SRBFile( new SRBFileSystem( ), args[0] );
			}
			else {
				throw new IllegalArgumentException(
					"\nUsage: Smkdir directory\nUsage: Smkdir -uri uriDirectory" );
			}

			srbFile.mkdir();

			if (list != null) {
				for (int i=0;i<list.length;i++) {
					System.out.println(list[i]);
				}
			}
		}
		catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
      System.exit(1);
		}

		System.exit(0);
	}
}
