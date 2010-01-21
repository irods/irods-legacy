package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaAddCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaListCommand;
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

public class IRODSFileSystemModifyMetadataTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileSystemModifyMetadataTest";
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

	@Test(expected = IllegalArgumentException.class)
	public void testModifyMetadataNullFile() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryModDate1.txt";

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
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH));

		irodsFileSystem.commands.modifyMetaData(null, null);
		irodsFileSystem.close();

	}

	@Test(expected = IllegalArgumentException.class)
	public void testModifyMetadataWrongNumberOfParms() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryModDate1.txt";

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
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH));

		String[] vals = { "", "", "", "" };
		irodsFileSystem.commands.modifyMetaData(irodsFile, vals);
		irodsFileSystem.close();

	}

	@Test
	public void deleteMetadataTest() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testDeleteMetadata.txt";

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
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + testFileName));

		// add metadata for this file

		String meta1Attrib = "attr1";
		String meta1Value = "a";

		String meta2Attrib = "attr2";
		String meta2Value = "b";

		ImetaAddCommand metaAddCommand = new ImetaAddCommand();
		metaAddCommand.setAttribName(meta1Attrib);
		metaAddCommand.setAttribValue(meta1Value);
		metaAddCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		String metaAddResult = invoker.invokeCommandAndGetResultAsString(metaAddCommand);
		
		ImetaListCommand metaList = new ImetaListCommand();
		metaList.setAttribName(meta1Attrib);
		metaList.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaList.setObjectPath(metaAddCommand.getObjectPath());
		String metaListResult = invoker.invokeCommandAndGetResultAsString(metaList);
		TestCase.assertTrue("did not add metadata attribute in test setup", metaListResult.indexOf("does not exist") == -1);
		
		// now delete the meta1Attrib metadata value
		String[] values = {meta1Attrib, meta1Value};
		irodsFileSystem.commands.deleteMetaData(irodsFile, values);
		
		// list and make sure value is not there
		metaList = new ImetaListCommand();
		metaList.setAttribName(meta1Attrib);
		metaList.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaList.setObjectPath(metaAddCommand.getObjectPath());
		String metaListResult2 = invoker.invokeCommandAndGetResultAsString(metaList);
		
		irodsFileSystem.close();
		TestCase.assertTrue("did not delete metadata attribute in test setup", metaListResult2.indexOf("does not exist") != -1);

		
		
	}

}
