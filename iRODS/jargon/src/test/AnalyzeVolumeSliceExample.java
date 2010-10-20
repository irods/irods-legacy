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
import java.io.*;

public class AnalyzeVolumeSliceExample
{

//----------------------------------------------------------------------
// This class is purely for testing 'anhdrinfo' and 'anslex' via SRB Proxy
// Written by: Roman Olschanowsky 2004
//
// anhdrinfo: Analyze Header Info
// anslex: Analyze Slice Extractor
// If you are interested in these programs, contact:
// Steve Lamont, UCSD NCMIR Lab
// spl@ncmir.ucsd.edu
// (858) 822-0760
//----------------------------------------------------------------------
	/**
	 * Testing
	 */
	public static void main(String args[])
	{
		SRBFileSystem srbFileSystem = null;

		SRBFile srbFile_anhdr = null;
		SRBFile srbFile_animg = null;

		String[] anArgs = null;
		// Common Naming of files, this is path up to file extension
		//(Testing hack??)
		String srbAnalyzeFiles = new String(args[0]);
                String localTestFile = new String(args[1]);

		if (args.length != 2) {
			throw new IllegalArgumentException(
			"\n\nUsage: getAnalyzeVolumeSliceExample <srb-In-Partial-Path> "+
			"local-Out-File\n\nSee Code for definition of <srb-In-Partial-Path>");
		}

		try{
			srbFileSystem = new SRBFileSystem( );
			srbFileSystem.setFirewallPorts(20000, 20199);

			// Common Naming of files, adding file extension here (Testing hack??)
			srbFile_anhdr = new SRBFile( srbFileSystem, srbAnalyzeFiles + ".hdr" );
			srbFile_animg = new SRBFile( srbFileSystem, srbAnalyzeFiles + ".img" );


//Add testing to see if files are really there (wrong path, etc..), exists(),
//makes program slower, you decide!

			//Get Header information with 1st SRB Proxy Operation
			InputStream in_hdr = srbFile_anhdr.executeProxyCommand( "anhdrinfo", "" );
			InputStreamReader isr = new InputStreamReader( in_hdr );
			BufferedReader br = new BufferedReader( isr );
			String hdr_info = br.readLine();
			hdr_info = hdr_info.trim();
			br.close();


//Get header info via SRB proxy
System.out.println("Header Info:\n " + hdr_info + "\n");
			//Parse header info, elements 0,1,2 are for '-d', 3,4,5 are for '-D'
			//(NO VALIDATION, OR var assignment, add it in your program)
				//anArgs = hdr_info.split("\s+"); //Don't know why won't work
			anArgs = hdr_info.split("[ \t\n\f\r]+");


//What anslex command is happening
//(Your program will need to handle arguments correctly!)
System.out.println("Command:\n anslex -q 1 0 0 0 -d " + anArgs[0] + " "
				+ anArgs[1] + " " + anArgs[2] + " -D "
				+ anArgs[3] + " " + anArgs[4] + " " + anArgs[5] + " -o -20 "
				+ srbFile_animg.getPath() + " " + localTestFile +"\n");


//Get slice from analyze image! via SRB proxy
			InputStream in = srbFile_animg.executeProxyCommand(
				"anslex", "-q 1 0 0 0 -d "
				+ anArgs[0] + " " + anArgs[1] + " " + anArgs[2] + " -D "
				+ anArgs[3] + " " + anArgs[4] + " " + anArgs[5] + " -o -20");


//Put output to local File
System.out.println("Storing results in: '"+localTestFile+"'");
			FileOutputStream out = new FileOutputStream( localTestFile );

			// 10K byte buffer
			byte[] imgbuf = new byte[10240];
			int result = in.read(imgbuf);
			while (result != -1) {
				out.write(imgbuf, 0, result);
				result = in.read(imgbuf);
			}
			out.close();
			in.close();
		}
		catch( SRBException e ) {
			//Get the standard error string
			String bar = e.getStandardMessage( );
			System.out.println( "Standardized SRB Server Error Message: "+ bar );

			//The original error message is still available through
			System.out.println( "\nGridTools Error Message: "+ e.getMessage() );


			e.printStackTrace();
		}
		catch ( Throwable e ) {
			e.printStackTrace();
		}


		System.exit(0);
	}
}
