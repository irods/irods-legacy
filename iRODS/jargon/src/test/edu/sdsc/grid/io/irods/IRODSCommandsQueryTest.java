package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaAddCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaCommand.MetaObjectType;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

import junit.framework.TestCase;

public class IRODSCommandsQueryTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSCommandsQueryTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	
	
	@Test
	public void testQueryNoDistinct() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryNoDistinct.txt";

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);
		
		// add metadata for this file
		
		String meta1Attrib = "attrsingle1";
		String meta1Value = "c";
		
		
		ImetaAddCommand metaAddCommand = new ImetaAddCommand();
		metaAddCommand.setAttribName(meta1Attrib);
		metaAddCommand.setAttribValue(meta1Value);
		metaAddCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/' + testFileName);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);
	
		// now query
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = IRODSMetaDataSet.newCondition(meta1Attrib, MetaDataCondition.EQUAL, meta1Value);

		String[] fileds = { IRODSMetaDataSet.FILE_NAME,
				IRODSMetaDataSet.DIRECTORY_NAME };
		MetaDataSelect[] select = IRODSMetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(condition, select, 100, Namespace.FILE, false);
		
		irodsFileSystem.close();

		TestCase.assertNotNull("no query results returned", fileList);
		TestCase.assertEquals("did not find my file and metadata", 1, fileList.length);
		TestCase.assertTrue("did not find my file name in results", fileList[0].toString().indexOf(testFileName) > -1);

	}

}
