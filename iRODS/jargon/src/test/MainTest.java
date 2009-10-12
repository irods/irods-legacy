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

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.grid.io.FileFactory;



import java.net.URI;


public class MainTest
{
//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
	/**
	 * Testing FileFactory
	 */
	public static void main(String args[])
	{
		/**
		 * The GeneralFile class is an abstract super class that can represent
		 * a filepath on any supported file system, local or remote.
		 * Use it as you would a regular java.io.File object.
		 */
		GeneralFile file = null;
		GeneralFile file2 = null;


		/**
		 * The GeneralRandomAccessFile class is used in much the same manner as
		 * the java.io.RandomAccessFile class.
		 */
		GeneralRandomAccessFile randomAccessFile = null;



		try {
      System.out.println("\n Create a GeneralFile object from a uri.");
      /*
      local file uri syntax:
      file:///path
      for example on windows:
      file:///c:/backups/testFile.txt


      srb file uri syntax:
      srb://userName.mdasDomain:password@host:port/path
      for example:
      srb://testuser.dice:myPASSWORD@srb.unc.edu:5544/home/testuser.dice/testFile.txt

      irods file uri syntax:
      irods://userName:password@host:port/path
      for example:
      irods://demouser:demo@irods.unc.edu:1247/tempZone/home/demouser/testFile.txt

      http and ftp URIs also work
       (though not all methods are available, due to protocol limits)
      */

      if (args.length == 2)
      {
        //note: storing/transferring plain text passwords can be a security risk
        //  an alternate 'separate password' method method is also available
        //  see the FileFactory class.
        //file = FileFactory.newFile( new URI( args[0] ), password );

        file = FileFactory.newFile( new URI( args[0] ) );
        file2 = FileFactory.newFile( new URI( args[1] ) );
      }
      else {
        throw new IllegalArgumentException(
          "\nUsage: java MainTest urlString urlString");
      }




System.out.println( "\n Make a new directory from the GeneralFile object." );
			file.mkdir();




System.out.println(
	"\n Create a new file in the new directory with the name \"child\"." );
			file = FileFactory.newFile( file,	"child" );
			file.createNewFile();




System.out.println( "\n Test the random access to the new file." );
			randomAccessFile = FileFactory.newRandomAccessFile( file, "rw" );
			randomAccessFile.write( new String(
				"This is a test file. It is ok to delete.\n") );

			randomAccessFile.seek( 0 );
			byte[] buffer = new byte[1000];
			int bytesRead = randomAccessFile.read( buffer );
			String fileContents = new String( buffer, 0, bytesRead );
			System.out.print( fileContents );

			randomAccessFile.close();




System.out.println( "\n Copy the file named \"child\" to the file given as "+
	"the second argument.");
			file.copyTo( file2 );





System.out.println("\n List the directory created earlier.");
			String[] dirList = file.getParentFile().list();
			if (dirList != null) {
				for(int i=0;i<dirList.length;i++)
					System.out.println(dirList[i]);
			}




System.out.println("\n Remove the file just created.");
			file.delete();




System.out.println("\n Remove the directory just created.");
			file.getParentFile().delete();




System.out.println("\n Exit.");
			file = null;
		}
		catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
      System.exit(1);
		}


		System.exit(0);
	}
}