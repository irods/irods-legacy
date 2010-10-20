import edu.sdsc.grid.io.irods.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;

import java.net.URI;



public class MetaDataTest
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
	GeneralFile file = null;


	/**
	 * The metadata records list for each file (directory, or other value
	 * the query selected for) is stored in this array.
	 */
	MetaDataRecordList[] rl = null;


//----------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------
	/**
	 * Testing the metaData
	 */
	public MetaDataTest()
		throws Throwable
	{
		this( new String[0] );
	}

	/**
	 * Testing the metaData
	 */
	public MetaDataTest(GeneralFile file)
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
	public MetaDataTest(String args[])
		throws Throwable
	{

System.out.println("\n Connect to the fileSystem.");
		/**
		 * You can query any filesystem object though not all,
		 * such as LocalFileSystem, will return useful results.
		 */
    if ((args == null) || (args.length == 0)) {
			fileSystem = FileFactory.newFileSystem( new SRBAccount( ) );

			String fileName = "myJARGONMetaDataTestFile";
			file = FileFactory.newFile( fileSystem, fileName );
		}
    else if (args.length == 1) {
			//url to file
			file = FileFactory.newFile(new URI( args[0] ));
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
		//Make sure file exists
		file.createNewFile();

    
    //-----------------------------------------------------------------
    // More info about iRODS AVU querying.    
    //-----------------------------------------------------------------
    if (file instanceof IRODSFile) new IRODSMetaDataTest((IRODSFile)file);
    
    
    
    
		//A list of values used with MetaDataCondition.IN
		String[] values = { "sysadmin", "staff", "public" };

System.out.println("\n Query the file system for all files matching certain "+
"query conditions");
		/**
		 * The conditions of a metadata query are created using the static
		 * methods of the MetaDataSet class.
		 * The .newCondition method takes three variables:
		 * a static metadata name, an operator, and the value to compare.
		 */
		MetaDataCondition conditions[] = {
/*      
			MetaDataSet.newCondition(
				GeneralMetaData.OFFSET, MetaDataCondition.BETWEEN, 0, 5000 ),

			MetaDataSet.newCondition(
				UserMetaData.USER_TYPE_NAME, MetaDataCondition.IN, values  ),
*/        
			MetaDataSet.newCondition(
				GeneralMetaData.SIZE, MetaDataCondition.LESS_THAN, 5000 ),
      
			MetaDataSet.newCondition( StandardMetaData.DIRECTORY_NAME,
				MetaDataCondition.EQUAL, file.getParent() )
		};


		/**
		 * For items which matched the query, met the conditions above,
		 * the following values will be returned.
		 */
		String[] selectFieldNames = {
			StandardMetaData.FILE_NAME,
			FileMetaData.FILE_COMMENTS,
			GeneralMetaData.SIZE,
			UserMetaData.USER_NAME,
		};
		MetaDataSelect selects[] =
			MetaDataSet.newSelection( selectFieldNames );


System.out.println("\n Query a single file/directory.");
		/**
		 * The simplest method when querying a specific file would be to use
		 * GeneralFile.query( String[] selectFieldNames ). For a particular file
		 * this will select and return the metadata named by selectFieldNames.
		 */
		rl = file.query( selectFieldNames );

		printQueryResults( rl );

    
    

System.out.println("\n Query a file system.");
		/**
		 * The number of metadata values you want the query to return at one
		 * time. If more values would result from the query the can be appended
		 * to the MetaDataRecordList object using the .getMoreResults( int )
		 */
		//Number of values returned from server in first iteration.
    //Note: sometimes problems have occured,
    //when asking for less than 20 results from the SRB.
		int numberOfResults = 2;
		rl = fileSystem.query( conditions, selects, numberOfResults );

		printQueryResults( rl );





System.out.println("\n Get further results from the query.");
		/**
		 * By default a query will only return the first 300 values. The query
		 * above was set to return only the first two. The methods
		 * isQueryComplete(), getMoreResults(), getMoreResults(int
		 * numberOfResults) and the static method
		 * MetaDataRecordList.getAllResults(MetaDataRecordList[]),
		 * are used to retrieve further results from the query.
		 */
		if (rl != null) {
			/**
			 * Note; rl[0] was used in the past, since all recordlists in the array
			 * were the same, but the last item in the array is best due to the
			 * way the new getAllResults() returns.
			 */
			if (!rl[rl.length-1].isQueryComplete()) {
				rl = rl[rl.length-1].getMoreResults();
			}
			else {
				rl = null;
			}
		}
		//else Nothing in the database matched the above query

		printQueryResults( rl );


    
    
    //-----------------------------------------------------------------
    //The rest is only for SRB querying
    //-----------------------------------------------------------------
    if (!(file instanceof SRBFile)) return;
    
    
    
    
System.out.println("\n Change the system metadata of a SRB file.");
		/**
		 * The metadata can be changed by creating a new, or modifying an
		 * existing, MetaDataRecordList and then sending it back to the
		 * file system.
		 */
		//This will set the file's metadata to the same as is defined
		//by the MetaDataRecordList.
		//You can add new fields to the record list, change existing values,
		//or create a new MetaDataRecordList.
		rl = new MetaDataRecordList[1];
		//this is just an example, changing the size without actually
		//changing the file's real size is probably a bad idea,
		//but I couldn't think of anything else.
		rl[0] = FileFactory.newMetaDataRecordList( fileSystem, MetaDataSet.getField(
			GeneralMetaData.SIZE ), 123 );
		rl[0].addRecord( MetaDataSet.getField(
			FileMetaData.FILE_COMMENTS ), "new comments go here." );

		//send the new metadata to the file system.
    //iRODS does not allow modification of System metadata
    file.modifyMetaData( rl[0] );






System.out.println("\n Set the SRB definable metadata.");
		/**
		 * Setting the definable metadata is the same as system metadata.
		 * The operators of the MetaDataTable will all be treated as if
		 * they were MetaDataCondition.EQUAL.
		 */
		String[][] definableMetaDataValues = new String[2][2];

		definableMetaDataValues[0][0] = "a";
		definableMetaDataValues[0][1] = "1";
    
		definableMetaDataValues[1][0] = "b";
		definableMetaDataValues[1][1] = "2";

		int[] operators = new int[definableMetaDataValues.length];
		operators[0] = MetaDataCondition.EQUAL;
		operators[1] = MetaDataCondition.LESS_OR_EQUAL;

		//	The operators don't matter when setting metadata, however
		//	as a query this table would find those items with user
		//	defined metadata matching: (see below)
		//	a = 1 
		//	b <= 2
    MetaDataTable metaDataTable = null;
    metaDataTable = 
      new MetaDataTable( operators, definableMetaDataValues );

    rl = new MetaDataRecordList[1];
    rl[0] = new SRBMetaDataRecordList( MetaDataSet.getField(
      SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES ), metaDataTable );
    rl[0].addRecord( MetaDataSet.getField(
      FileMetaData.FILE_COMMENTS ),"Hi." );

    file.modifyMetaData( rl[0] );

    rl = (file.getParentFile()).query( selects );
    printQueryResults( rl );





System.out.println("\n Using the SRB user definable metadata.");
		/**
		 * The SRB supports user definable metadata. Access to this
		 * data is provided through the MetaDataTable class.
		 */
    conditions = new MetaDataCondition[1];
    conditions[0] = MetaDataSet.newCondition(
      SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES, metaDataTable );

    selects = new MetaDataSelect[1];
    selects[0] = MetaDataSet.newSelection(
      SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES );

    rl = fileSystem.query( conditions, selects );

    printQueryResults( rl );
	}




//----------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------
	public static void printQueryResults( MetaDataRecordList[] rl )
	{
		//print query results
		if (rl != null) { //Nothing in the database matched the query
				System.out.print("\n");
				for (int i=0;i<rl.length;i++) {
					System.out.print("\n"+rl[i]);
				}
		}
		System.out.println("\n");
	}
  
	public static void printMetaDataGroups( )
	{
		//
		// Prints all the groups and their fields that have been registered
		// in the MetaDataSet class.
		//
		MetaDataGroup[] groups = MetaDataSet.getMetaDataGroups();
		MetaDataField[] fields = null;
		System.out.println("Number of MetaDataGroups: "+groups.length);
		for (int j=0;j<groups.length;j++) {
			System.out.println("\nMetaDataGroup: "+groups[j]+
				",  MetaDataFields in this group: "+groups[j].getFields().length);
			fields = groups[j].getFields();
			for (int i=0;i<fields.length;i++) {
				System.out.println(fields[i].getName()+", "+fields[i]);
			}
		}
		System.out.println("\n");
	}


	/**
	 * Stand alone testing.
	 */
	public static void main(String args[])
	{
		try {
			MetaDataTest mdt = new MetaDataTest(args);
    } catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
    	System.exit(1);
		}

		System.exit(0);
	}
}
