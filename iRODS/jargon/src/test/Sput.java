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

import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;
import java.io.*;
import java.net.*;

public class Sput
{
	static boolean overwrite = true;

//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
	/**
	 * Testing
	 */
	public static void main(String args[])
	{
		SRBFileSystem srbFileSystem = null;

		GeneralFile source = null;
		try{
			if ( args[0].equals( "-uri" ) ) {
				if (args.length == 3) {
					FileFactory.newFile( new URI( args[1] ) ).copyTo(
						FileFactory.newFile( new URI( args[2] ) ), overwrite
					);
				}
				else if (args.length == 2) {
					srbFileSystem = new SRBFileSystem( );

					source = FileFactory.newFile( new URI( args[1] ) );
					source.copyTo( new SRBFile( srbFileSystem, source.getName() ),
						overwrite );
				}
				return;
			}
			else if (args.length == 2) {
				srbFileSystem = new SRBFileSystem( );

				new LocalFile( args[0] ).copyTo(
					new SRBFile( srbFileSystem, args[1] ), overwrite );
				return;
			}
			else if (args.length == 1) {
				srbFileSystem = new SRBFileSystem( );

				source = new LocalFile( args[0] );
				source.copyTo( new SRBFile( srbFileSystem, source.getName() ),
					overwrite );
				return;
			}


			throw new IllegalArgumentException(
				"\nUsage: Sput localFileName/directory srbFileName/directory"+
				"\nUsage: Sput localFileName/directory"+
				"\nUsage: Sput -uri uri [destinationUri]");
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
