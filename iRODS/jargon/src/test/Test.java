import edu.sdsc.grid.io.irods.*;
import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.ftp.*;
import edu.sdsc.grid.io.http.*;
import edu.sdsc.grid.io.*;


import java.io.FileNotFoundException;
import java.io.InputStream;

import java.net.URI;
import java.util.Date;


public class Test
{
  /*
   * Use these names so the test doesn't overwrite important files.
   * Also add the date to the end,
   * so previous tests don't interefere with each other'
   */
  public static String TEST_DIR = "myJargonTestDir";
  public static String TEST_FILE = "myJargonTestFile";
  public static String LOCAL_TEST_FILE = "myLocalJargonFile";
  public static String INITIAL_FILE = "myInitialJargonFile";




//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
	/**
	 * Testing Remote file system functions
	 */
	public static void main(String args[])
  {
    /**
     * Open a connection to the remote filesystem.
     * Equivalent to Sinit or iinit in the SRB Scommands and iRODS iCommands
     * command-line tools, respectively.
     * The filesystem object represents the connection to the filesystem.
     * Only one filesystem object is needed to access all the files on
     * that system.
     *<P>
     * Using the seven variable constructor is recommended,
     * as all necessary data is passed to the constructor.
     * Other constructors may use some system or configuration
     * dependent default information and are provided for
     * convenience.
     */
    GeneralAccount[] accounts = null;
    GeneralFileSystem fileSystem = null;


    /**
     * The LocalFile class is a wrapper class for the
     * java.io.File class. Use it as you would a regular File object.
     * The localFile.getFile() method will return a java.io.File object
     * for those situations where a true java.io.File is required.
     */
    LocalFile localFile = null;


    /**
     * The GeneralFile class is used in much the same manner as the
     * java.io.File class. A GeneralFile object can represent
     * a file or directory on the remote server.
     */
    GeneralFile file = null;


    /**
     * The SRBContainer class is an extension of the SRBFile class.
     * Used for manipulating srb containers.
     */
    SRBContainer srbContainer = null;


    /**
     * The GeneralRandomAccessFile class is used in much the same manner as the
     * java.io.RandomAccessFile class.
     */
    GeneralRandomAccessFile randomAccessFile = null;

    /**
     * Used in the printout statements to show the results of methods.
     */
    String result;

    /**
     * If error occurs exit with this variable;
     */
    int err = 0;

    /**
     * Flag to trigger the copyTest.
     * Creates, uploads, downloads and checksums 5000+ files.
     */
    boolean doCopyTest = false;

    /**
     * Flag to trigger all the rests of the tests available.
     */
    boolean doFullTest = false;

    try {
			if (args.length >= 1) {
				if (args[0].equals( "-copy" )) {
						doCopyTest = true;
				}
				if (args[0].equals( "-fullTest" )) {
						doCopyTest = true;
						doFullTest = true;
				}
			}


      
      for (int i=args.length-1;i>=0;i--) {
        if (args[i].equals( "srb" ) || args[i].equals( "SRB" )) {
  System.out.println("\n Testing only the default SRB filesystem.");
          accounts = new GeneralAccount[1];
          accounts[0] = new SRBAccount( );
          break;
        }
        if (args[i].equals( "irods" ) || args[i].equals( "iRODS" )) {
  System.out.println("\n Testing only the default iRODS filesystem.");
          accounts = new GeneralAccount[1];
          accounts[0] = new IRODSAccount( );
          break;
        }
      }


      if (accounts == null) {
  System.out.println("\n Testing all remote filesystems.");
        accounts = new GeneralAccount[3];


        //irods
        try {
          accounts[0] = new IRODSAccount( );
        } catch (FileNotFoundException e) {
          System.err.print("No ~/.irods/.irodsEnv account file, cannot test iRODS");
        }


        //srb
        try {
          accounts[1] = new SRBAccount( );
        } catch (FileNotFoundException e) {
          System.err.print("No ~/.srb/.MdasEnv account file, cannot test SRB");
        }


        //http (read-only)
        accounts[2] = FileFactory.newFileSystem(
          new URI("http://www.yahoo.com") ).getAccount();


        //Not yet much to test for ftp and GridFTP. Just try this:
        copy( FileFactory.newFile( new URI("ftp://ftp.gnu.org/welcome.msg") ),
          new LocalFile( fakeName(LOCAL_TEST_FILE) ));
      }


      for (int n=0;n<accounts.length;n++) {
       if (accounts[n] != null) {

  System.out.println("\n Connect to the remote server.");
        //Set account variables before initializing the file system.
        //Once the fileSystem is intialized,
        //the connection variables cannot be changed.

        fileSystem = FileFactory.newFileSystem( accounts[n] );

        //Also available, using the default account constructor:
        //fileSystem = new SRBFileSystem( );




        //Only SRB has containers
        if (fileSystem instanceof SRBFileSystem)
        {
          /**
           * Create a new container on the SRB.
           * Equivalent to Smkcont in the SRB Scommands command-line tools.
           *<P>
           * This method will create a new container from the name defined
           * by the srbContainer object.
           */
  System.out.println(
    "\n Make a new container in your SRB called mySRBJargonTestContainer.");

          srbContainer = new SRBContainer( (SRBFileSystem)fileSystem,
            fakeName("mySRBJargonTestContainer")  );
          srbContainer.createNewFile();
        }





  System.out.println(
    "\n Make a new collection in your remote server home called "+TEST_DIR);
        /**
         * Create a new directory on the remote server, also known as a collection.
         * Equivalent to Smkdir in the SRB Scommands command-line tools.
         *<P>
         * This method will create a new collection from the
         * abstract pathname defined by the RemoteFile object.
         */
        //using constructor SRBFile( SRBFileSystem fileSystem, String filePath )
        file = FileFactory.newFile( fileSystem, TEST_DIR );
        result = ""+file.mkdir();
        System.out.println( result );






  System.out.println(
    "\n Put an example local file in the new collection as "+TEST_FILE);
        /**
         * Copy a local file to the remote server.
         * Equivalent to Sput in the SRB Scommands command-line tools.
         *<P>
         * Open a remote server file by passing the GeneralFileSystem object and
         * remote filepath to the constructor. Next open a local file
         * using the LocalFile class. Then pass the local file to
         * the RemoteFile method copyFrom.
         */
        //creating an example local file.
        localFile = new LocalFile( fakeName(INITIAL_FILE) );
        if (!localFile.exists()) {
          LocalRandomAccessFile out = new LocalRandomAccessFile( localFile, "rw" );
          out.write(new String("This file is used to test the Jargon API. "+
            "It is ok to delete.").getBytes());
          out.close();
        }
        //using constructor SRBFile( SRBFile parent, String fileName )
        file = FileFactory.newFile( file, fakeName(TEST_FILE) );
        file.copyFrom( localFile, true );
        localFile.delete();



        /**
         * Print the remote URI for the new remote file.
         */
        System.out.println(file.toString());





  System.out.println(
    "\n Random access read and write to the file\n"+ file);
        /**
         * Open a random access connection to a file.
         * The second variable passed to the constructor is the mode string:
         * r = read-only, rw = read/write
         */
        try {
          if (file.canWrite())
            randomAccessFile = FileFactory.newRandomAccessFile( file, "rw" );
          else
            randomAccessFile = FileFactory.newRandomAccessFile( file, "r" );

          //Read from the file,
          byte[] buffer = new byte[1000];
          int bytesRead = randomAccessFile.read( buffer );

          //seek to somewhere in the file,
          String fileContents = new String( buffer, 0, bytesRead );
          int insert = fileContents.indexOf( "It is ok to delete." );
          if (insert >= 0)
            randomAccessFile.seek( insert );
          if (file.canWrite()) {
            //write new data.
            randomAccessFile.write( new String(
              "The copies created on the remote system should get deleted "+
              "automatically.\n"+
              "The local version was not deleted, "+
              "so you know if it worked.").getBytes() );
          }

          //Be sure to close the file when you are finished.
          randomAccessFile.close(); //or randomAccessFile = null;
        } catch (UnsupportedOperationException e) {
          //Not all filesystem support random access or streams,
          //such as ftp and gridftp
          if (!System.getProperty("jargon.debug").equals(0))
            e.printStackTrace();
        }




  System.out.println("\n List the contents of the new collection:");
        /**
         * List the contents of a collection.
         * Equivalent to Sls in the SRB Scommands command-line tools.
         *<P>
         * The list method will return a string array containing
         * First, the collection name.
         * Then, all the datasets in that collection.
         * Lastly, all the sub-collections to that collection.
         */
        String[] dirList = file.list();

        //print the directory list
        if (dirList != null) {
          for(int i=0;i<dirList.length;i++)
            System.out.println(dirList[i]);
        }




        /**
         * List the contents of a container.
         * Equivalent to Slscont in the SRB Scommands command-line tools.
         *<P>
         * The list method will return a string array containing
         * First, the container name.
         * Then, all the datasets in that container. (in no particular order)
         */
        if (fileSystem instanceof SRBFileSystem) {
  System.out.println("\n List the contents of the new container:");
          String[] contList = srbContainer.list();

          //print the files in the container
          if (contList != null) {
            for(int i=0;i<contList.length;i++)
              System.out.println(contList[i]);
          }
        }




  System.out.println("\n Replicate "+file);
        /**
         * Replicate a file on the remote server. Creates another copy of the file
         * on a different resource.
         * Equivalent to Sreplicate in the SRB Scommands command-line tools.
         */
        if (fileSystem instanceof SRBFileSystem) {
          //Uses the resource the file is already on in this example,
          //because it is the only known resource.
          //Which doesn't work on IRODS
          ((RemoteFile)file).replicate( ((RemoteFile)file).getResource() );
        }




        /**
         * Changing the permissions on a file.
         * null = none, "r" = read, "w" = write, "rw" = all
         * Equivalent to Schmod in the SRB Scommands command-line tools.
         */
        if (fileSystem instanceof SRBFileSystem) {
  System.out.println("\n Add read/write permissions to the file for \"testuser\".");
          String permission = "rw";
          String newUserName = "testuser";
          String newMdasDomain = "sdsc";
          ((SRBFile)file).changePermissions( permission, newUserName, newMdasDomain );
          MetaDataTest.printQueryResults(((SRBFile)file).getPermissions(true));
        }





  System.out.println("\n Copy "+file+" to another"+TEST_FILE);
        /**
         * Copy a file from one place to another on the remote file system.
         * Equivalent to Scp in the SRB Scommands command-line tools.
         */
        if (file.canWrite()) { //just to check the permissions on this server
          file.copyTo( FileFactory.newFile(
            fileSystem, TEST_DIR, fakeName(TEST_FILE)), true );
        }





  System.out.println("\n Rename "+file+" to new"+TEST_FILE);
        /**
         * Rename the file.
         * Equivalent to Smv in the SRB Scommands command-line tools.
         */
        if (file.canWrite()) { //just to check the permissions on this server
          file.renameTo(FileFactory.newFile( fileSystem,
            TEST_DIR, fakeName(TEST_FILE) ));
        }



  System.out.println(
    "\n Get "+TEST_DIR+" from the remote server and\n" +
    " put it in the current local directory as "+LOCAL_TEST_FILE);
        /**
         * Copy a remote server file to a local drive.
         * Equivalent to Sget in the SRB Scommands command-line tools.
         *<P>
         * Open a remote server file by passing the GeneralFileSystem object and
         * remote server filepath to the constructor. Next open a local file
         * using the LocalFile class. Then pass the local file to
         * the GeneralFile method copyToLocal.
         */
        //using constructor SRBFile( SRBFileSystem fileSystem, String filePath )
        file = FileFactory.newFile( fileSystem, TEST_DIR );
        localFile = new LocalFile( fakeName(LOCAL_TEST_FILE) );
        file.copyTo( localFile );







        /**
         * Execute a proxy command on the SRB. The protocol of the value
         * returned on the InputStream depends on the command run.
         */
        if (fileSystem instanceof SRBFileSystem) {
  System.out.println("\n Execute the 'hello' proxy command on the SRB.");
          InputStream in = ((SRBFileSystem)fileSystem).executeProxyCommand(
            "hello", null );
          int b = in.read();
          while (b != -1) {
            System.out.print((char)b);
            b = in.read();
          }
        }



  System.out.println("\n Link a HTML page to act as if it were a SRBFile.");
        /**
         * The link must be registered to a resource of type 'http file system'.
         *
         */
        /*
        //note: This isn't run because "myHTTP-server" doesn't exist by default.

        URL url = new URL("http://myURL.com");

        //Create a new abstract pathname.
        SRBFile file = FileFactory.newFile( fileSystem, "myURL2" );

        //a url must be registered with an http resource.
        file.setResource( "myHTTP-server" );

        //associate the URL with the SRB path.
        fileSystem.registerURL( file, url );

        randomAccessFile = new SRBRandomAccessFile( file, "r" );
        bytesRead = randomAccessFile.read( buffer );
        System.out.println( new String( buffer ) );
        randomAccessFile.close();
        */


  System.out.println("\n Run the MetaDataTest");
        /**
         * Test the metadata.
         */
        try {
          new MetaDataTest(
            FileFactory.newFile( fileSystem,
              TEST_DIR, fakeName(TEST_FILE) ));
        } catch (UnsupportedOperationException e) {
          //Not all filesystem support querying,
          //such as local, http, ftp and gridftp
          String debug = System.getProperty("jargon.debug");
          if (debug != null && !debug.equals("0"))
            e.printStackTrace();
        }



        /**
         * Test the file tranfers.
         */
        if (doCopyTest) {
  System.out.println("\n Run the CopyTest ");
          if (doFullTest) {
            localFile = (LocalFile) localFile.getParentFile();
            System.out.println(localFile);            
          }
          else {
            localFile = new LocalFile( localFile, "5017_files" );
          }
          if (doFullTest) {
            int connections = 5;
            int threadsPerConnection = 3;
            CopyTest.superCopy(localFile, 
              FileFactory.newFile( fileSystem, TEST_DIR, "Copy"+TEST_DIR ), 
              connections, threadsPerConnection);       
          }
          else {
            CopyTest copyTest = new CopyTest( localFile,
              FileFactory.newFile( fileSystem, TEST_DIR, "Copy"+TEST_DIR ), null);
            copyTest.copy();
            try {           
              if (fileSystem instanceof SRBFileSystem)
                err = copyTest.compare() ? 0 : 1;
              else {     
                err = MoreTests.fileCompare( 
                        copyTest.getSource(), 
                        copyTest.getReturnDestination() ) ? 0 : 1;
              }
            } catch ( Throwable e ) {
              //All kinds of unimportant errors can happen
              e.printStackTrace();
              err = 1;
            }
          }
        }



        /**
         * Test the file tranfers.
         */
        if (doFullTest) {
  System.out.println("\n Run the full test");
          //Currently only setup for SRB
          MoreTests moreTests = new MoreTests( file );
          moreTests.run();
        }



  System.out.println("\n Remove the files created by this program from the remote server.");
        /**
         * Delete a remote server file.
         * Equivalent to Srm in the SRB Scommands command-line tools.
         *<P>
         * Delete a remote server file by passing the GeneralFileSystem object and
         * remote server filepath to the constructor.
         */

        GeneralFile[] files = FileFactory.newFile(
          fileSystem, TEST_DIR ).listFiles();
        //print the directory list
        if (files != null) {
          for(int i=0;i<files.length;i++) {
            if (files.toString().startsWith("my")) {
              files[i].delete();
            }
            else {
              //not a file created by this test program?
            }
          }
        }




  System.out.println("\n Remove "+TEST_DIR+" from the remote server.");
        /**
         * Delete a remote server collection.
         * Equivalent to Srmdir in the SRB Scommands command-line tools.
         */
        //using constructor SRBFile( SRBFileSystem fileSystem, String filePath )
        //but this time giving the constructor an absolute path, if you do not
        //give the constructor an absolute path, the SRBFile class will default
        //to your home directory, e.g. /home/testuser.sdsc/
        String absolutePath = fileSystem.getHomeDirectory() +
          file.getPathSeparator() + TEST_DIR;
        file = FileFactory.newFile( fileSystem, absolutePath );
        file.delete();




        /**
         * Delete a SRB container.
         * Equivalent to Srmcont in the SRB Scommands command-line tools.
         */
        //Certain privileges may be required for this method
        if (srbContainer != null) {
  System.out.println("\n Remove mySRBJargonTestContainer from the SRB.");
          srbContainer.delete();
          srbContainer = null;
        }




  System.out.println("\n Exit.");
        /**
         * Close the connection to the remote server.
         * Equivalent to Sexit in the SRB Scommands command-line tools.
         */
        fileSystem = null;
       }
      } //end for loop of this filesystem type
		}
		catch( SRBException e ) {
System.out.println("\n Handling an error.");
			/**
			 * Handling an error from the remote server.
			 *<P>
			 * If there is an error returned from the SRB server,
			 * a SRBException will be thrown.
			 * SRBException and IRODSException subclass IOException.
       * Additionally a SRBException corresponds to a standardized error type.
			 * The standardized error can be expressed as a string or int.
			 */
			//Get the specific error type as an int
			int foo = e.getType( );
			//then do whatever is need with foo
			System.out.println( "Standardized SRB Server Error Type: "+ foo );

			//Get the standard error string
			String bar = e.getStandardMessage( );
			System.out.println( "Standardized SRB Server Error Message: "+ bar );

			//The original error message is still available through
			System.out.println( "\nJargon Error Message: "+ e.getMessage() );


			e.printStackTrace();

			Throwable chain = e.getCause();
			while (chain != null) {
				chain.printStackTrace();
				chain = chain.getCause();
			}
			err = foo;
		}
		catch( IRODSException e ) {      
			//Get the specific error type as an int
			int foo = e.getType( );
			//then do whatever is need with foo
			System.out.println( "Standardized iRODS Server Error Type: "+ foo );

			//The original error message is still available through
			System.out.println( "\nJargon Error Message: "+ e.getMessage() );

			e.printStackTrace();

			Throwable chain = e.getCause();
			while (chain != null) {
				chain.printStackTrace();
				chain = chain.getCause();
			}
			err = foo;
    }
		catch ( Throwable e ) {
			e.printStackTrace();

			Throwable chain = e.getCause();
			while (chain != null) {
				chain.printStackTrace();
				chain = chain.getCause();
			}
			err = 1;
		}


		/**
		 * Testing if the connection is still valid after an exception is thrown.
		 * If a server error occured, and was handled improperly by the API, the
		 * program will block indefinately at this method.
		 */
		try{
			if ((file == null) && (fileSystem != null)) {
				file = FileFactory.newFile( fileSystem, "asdf");

				System.out.println("\nSocket Exception test, file.length="+file.length());
			}
		}
		catch ( Throwable e ) {
			e.printStackTrace();

			Throwable chain = e.getCause();
			while (chain != null) {
				chain.printStackTrace();
				chain = chain.getCause();
			}
			err = 1;
		}


		System.exit(err);
	}

  static void copy( GeneralFile source, GeneralFile destination )
    throws java.io.IOException
  {
    if (source.isDirectory()) {
      System.out.println("Copying directory: "+source+" to: "+destination);
      //recursive copy
      GeneralFile[] fileList = source.listFiles();

      destination.mkdir();
      if (fileList != null) {
        for (int i=0;i<fileList.length;i++) {
          fileList[i].copyTo(
            FileFactory.newFile(
              destination.getFileSystem(), destination.getAbsolutePath(),
              fileList[i].getName()), true );
        }
      }
    }
    else {
      System.out.println("Copying file: "+source+" to: "+destination);
      source.copyTo(destination);
    }
  }

  static String fakeName( String base )
  {
   return base+new Date().getTime();
  }
}

