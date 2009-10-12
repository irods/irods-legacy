/*
 *   Copyright (c) Feb 11, 2009  DICE Research,
 *   University of California, San Diego (UCSD), San Diego, CA, USA.
 *  
 *   Users and possessors of this source code are hereby granted a
 *   nonexclusive, royalty-free copyright and design patent license
 *   to use this code in individual software.  License is not granted
 *   for commercial resale, in whole or in part, without prior written
 *   permission from SDSC/UCSD.  This source is provided "AS IS"
 *   without express or implied warranty of any kind.
 * 
 * 
 *   IRODSMetaDataTest
 *   IRODSMetaDataTest.java  -  .IRODSMetaDataTest
 * 
 *   CLASS HIERARCHY
 *   java.lang.Object
 *       |
 *       +-.IRODSMetaDataTest
 * 
 * 
 *   PRINCIPAL AUTHOR
 *   Lucas Gilbert, SDSC/UCSD
 */
import edu.sdsc.grid.io.irods.*;
import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;

import java.net.URI;
import java.io.*;


/**
 * Example/Test for the iRODS AVU metadata.  Querying and modifying AVU
 * metadata has been simplified from the SRB user definable metadata queries.
 * iRODS AVU metadata is handle like the standard system metadata attributes
 * 
 * @author iktome
 */
public class IRODSMetaDataTest 
{
	/**
	 * Open a connection to the file system.
	 * The filesystem object represents the connection to the filesystem.
	 * Only one filesystem object is needed to access all the files on
	 * that system.
	 */
	GeneralAccount account = null;
	GeneralFileSystem fileSystem = null;
  
	/**
	 * The GeneralFile class is used in much the same manner as the
	 * java.io.File class. A GeneralFile object can represent
	 * a file or directory.
	 */
	IRODSFile file = null;


	/**
	 * The metadata records list for each file (directory, or other value
	 * the query selected for) is stored in this array.
	 */
	MetaDataRecordList[] rl = null;
  
  
//----------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------

	/**
	 * Testing the iRODS AVU metaData
	 */
	public IRODSMetaDataTest()
		throws Throwable
	{
		this( new String[0] );
	}

	/**
	 * Testing the metaData
	 */
	public IRODSMetaDataTest(IRODSFile file)
		throws Throwable
	{
		if (file != null) {
			this.file = file;
			fileSystem = file.getFileSystem();
			run();
		}
		else {
			String uri[] = { file.toString() };

			new MetaDataTest( uri );
		}
	}

	/**
	 * Testing the metaData
	 */
	public IRODSMetaDataTest(String args[])
		throws Throwable
	{
System.out.println("\n Connect to the iRODS fileSystem.");
		/**
		 * You can query any filesystem object though not all,
		 * such as LocalFileSystem, will return useful results.
		 */
    if ((args == null) || (args.length == 0)) {
			fileSystem = FileFactory.newFileSystem( new IRODSAccount( ) );

			String fileName = "myJARGONiRODSMetaDataTestFile";
			file = (IRODSFile) FileFactory.newFile( fileSystem, fileName );
      file.createNewFile();
		}
    else if (args.length == 1) {
			//url to file
			file = (IRODSFile) FileFactory.newFile(new URI( args[0] ));
      file.createNewFile();
			fileSystem = file.getFileSystem();
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
    String myTestAttributeName = "myTestAttributeName";
    String myTestAttributeValue = "myTestAttributeValue";
    String myTestAttributeUnit = "myTestAttributeUnit";
    
    
    
System.out.println("\n Change the AVU metadata of an iRODS file.");
    String[][] metaData = new String[][] {
      { myTestAttributeName, myTestAttributeValue, myTestAttributeUnit },
      { "Ford", "Model T" } //units are optional
    };

    //add to this file's AVU: 
    //myTestAttributeName, myTestAttributeValue, myTestAttributeUnit 
    file.modifyMetaData(metaData[0]);
    
    //Ford, Model T
    file.modifyMetaData(metaData[1]);

    
    

System.out.println("\n Query specific AVU metadata of an iRODS file.");
    //WHERE myTestAttributeName=myTestAttributeValue SELECT file_name
    MetaDataCondition conditions[] = { MetaDataSet.newCondition( 
      "myTestAttributeName", MetaDataCondition.EQUAL, "myTestAttributeValue" )};
    String[] selectFieldNames = { GeneralMetaData.FILE_NAME, null, null };
    MetaDataSelect selects[] =  MetaDataSet.newSelection( selectFieldNames ); 
    MetaDataRecordList[] rl = fileSystem.query( conditions, selects );
    MetaDataTest.printQueryResults( rl );
    System.out.println("\n");


    //WHERE myTestAttributeName=myTestAttributeValue 
    //SELECT file_name, myTestAttributeName
    selects[1] = MetaDataSet.newSelection( myTestAttributeName );
    rl = fileSystem.query( conditions, selects );
    MetaDataTest.printQueryResults( rl );
    System.out.println("\n");
    

    //WHERE size>=0 SELECT file_name, myTestAttributeName
    conditions[0] = MetaDataSet.newCondition( 
            GeneralMetaData.SIZE, MetaDataCondition.GREATER_OR_EQUAL, 0 );
    rl = fileSystem.query( conditions, selects );
    MetaDataTest.printQueryResults( rl );
    System.out.println("\n");

        
    
    
    //Note: cannot query two different AVU at once, e.g. this will not work:
    //WHERE Ford=Model T SELECT file_name, myTestAttributeValue
    
    
    
System.out.println("\n Query all AVU metadata of an iRODS file.");
    selects[0] = 
            MetaDataSet.newSelection( IRODSMetaDataSet.META_DATA_ATTR_NAME );
    selects[1] = 
            MetaDataSet.newSelection( IRODSMetaDataSet.META_DATA_ATTR_VALUE );
    selects[2] = 
            MetaDataSet.newSelection( IRODSMetaDataSet.META_DATA_ATTR_UNITS );    
    rl = file.query( selects );
    MetaDataTest.printQueryResults( rl );
    
    
System.out.println("\n Query all AVU metadata of an iRODS directory.");
    selects[0] = 
            MetaDataSet.newSelection( IRODSMetaDataSet.META_COLL_ATTR_NAME );
    selects[1] = 
            MetaDataSet.newSelection( IRODSMetaDataSet.META_COLL_ATTR_VALUE );
    selects[2] = 
            MetaDataSet.newSelection( IRODSMetaDataSet.META_COLL_ATTR_UNITS );    
    rl = file.getParentFile().query( selects );
    MetaDataTest.printQueryResults( rl );

    
    
System.out.println("\n Delete the AVU metadata of an iRODS file.");
    //must be an exact match, including units if they exist.
    file.deleteMetaData(metaData[0]);
    file.deleteMetaData(metaData[1]);
  }
  
  
	/**
	 * Stand alone testing.
	 */
	public static void main(String args[])
	{
		try {
      if (args == null)
        new IRODSMetaDataTest();
      else
        new IRODSMetaDataTest(args);
    	} catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
    		System.exit(1);
		}

		System.exit(0);
	}
}
