import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;

import java.net.URI;
import java.io.*;



public class ShadowFileTest
{
	/**
	 * Open a connection to the file system.
	 * The filesystem object represents the connection to the filesystem.
	 * Only one filesystem object is needed to access all the files on
	 * that system.
	 */
	SRBAccount account = null;
	SRBFileSystem fileSystem = null;
	GeneralFile file = null;
	SRBShadowFile shadow = null;



//----------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------
	/**
	 * Testing the shadow objects
	 */
	ShadowFileTest( )
		throws Throwable
	{
		this( new String[0] );
	}

	/**
	 * Testing the shadow objects
	 */
	public ShadowFileTest( GeneralFile file, String shadowPath )
		throws Throwable
	{
		if (file != null) {
			this.file = file;
			fileSystem = (SRBFileSystem) file.getFileSystem();
		}
    else {
      fileSystem = new SRBFileSystem();
      file = new SRBFile( fileSystem, fileSystem.getHomeDirectory() );
    }
    
    //TODO need a file factory really
    shadow = new SRBShadowFile( (SRBFile)file, shadowPath );
    run();
	}

	/**
	 * Testing the shadow objects
	 */
	ShadowFileTest( String args[] )
		throws Throwable
	{
		if (args == null) args = new String[0];

		if (args.length == 2) {
			//url to an srb file
			file = new SRBFile( new URI( args[0] ) );
			fileSystem = (SRBFileSystem) file.getFileSystem();

			//the shadow path from that srb file
			shadow = new SRBShadowFile( (SRBFile)file, args[1] );
		}
		else {
			throw new IllegalArgumentException(
				"Wrong number of arguments sent to Test.");
		}

		run();
	}


	private void run()
		throws Throwable
	{
    System.out.println("\n List the shadow directory.");
		/**
		 * List the shadow directory. Same as listing a java.io.File or
		 * SRBFile.
		 */
		String[] dirList = shadow.list();

		//print the directory list
		if ((dirList != null) && (dirList.length > 0)) {
		  for(int i=0;i<dirList.length;i++)
			System.out.println(dirList[i]);

		  //Create a new sub-shadow object
		  //(hopefully it is a file, see below)
		  shadow = new SRBShadowFile( shadow, dirList[0] );
		}


		System.out.println("\n Read from a shadow file.");
		/**
		 * Read from a shadow file. Same as RandomAccessFile. You can only
		 * read from a file, not a directory.
		 */
		System.out.println(shadow.read());
		System.out.println(shadow.read());
		System.out.println(shadow.read());
		System.out.println(shadow.read());
		System.out.println(shadow.read());
		System.out.println(shadow.read());

		System.exit(0);
	}




//----------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------
	/**
	 * Stand alone testing.
	 */
	public static void main(String args[])
	{
		try {
			ShadowFileTest shadowFileTest = new ShadowFileTest(args);
    } catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
			System.exit(1);
		}

		System.exit(0);
	}
}
