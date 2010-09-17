package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaAddCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaListCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaRemoveCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaCommand.MetaObjectType;

/**
 * Tests for manipulating AVU's via the IRODSFile
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class IRODSFileAVUTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileAVUTest";

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
		new AssertionHelper();
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
	public void testAddOneAVUToDataObjectNoUnits() throws Exception {
		String testFileName = "testAddAvu.txt";
		String expectedAttribName = "testattrib1";
		String expectedAttribValue = "testvalue1";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);

		// generate testing file
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String absPathToFile = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 20);

		IputCommand iputCommand = new IputCommand();

		iputCommand.setLocalFileName(absPathToFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));

		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// open the file and add an AVU
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);
		String[] metaData = { expectedAttribName, expectedAttribValue };
		irodsFile.modifyMetaData(metaData);
		irodsFileSystem.close();

		// verify the metadata was added
		// now get back the avu data and make sure it's there
		ImetaListCommand imetaList = new ImetaListCommand();
		imetaList.setAttribName(expectedAttribName);
		imetaList.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaList.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		String metaValues = invoker
				.invokeCommandAndGetResultAsString(imetaList);
		Assert.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		Assert.assertTrue("did not find expected attrib value", metaValues
				.indexOf(expectedAttribValue) > -1);

		// clean up avu

		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAttribName);
		imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaRemoveCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

	}

	@Test
	public void testAddOneAVUToDataObjectWithUnits() throws Exception {
		String testFileName = "testAddAvuUnits.txt";
		String expectedAttribName = "testattrib1";
		String expectedAttribValue = "testvalue1";
		String expectedUnits = "units1";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);

		// generate testing file
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String absPathToFile = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 20);

		IputCommand iputCommand = new IputCommand();

		iputCommand.setLocalFileName(absPathToFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));

		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// open the file and add an AVU
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);
		String[] metaData = { expectedAttribName, expectedAttribValue,
				expectedUnits };
		irodsFile.modifyMetaData(metaData);
		irodsFileSystem.close();

		// verify the metadata was added
		// now get back the avu data and make sure it's there
		ImetaListCommand imetaList = new ImetaListCommand();
		imetaList.setAttribName(expectedAttribName);
		imetaList.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaList.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		String metaValues = invoker
				.invokeCommandAndGetResultAsString(imetaList);
		Assert.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		Assert.assertTrue("did not find expected attrib value", metaValues
				.indexOf(expectedAttribValue) > -1);
		Assert.assertTrue("did not find expected units", metaValues
				.indexOf(expectedUnits) > -1);

		// clean up avu
		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAttribName);
		imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaRemoveCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

	}

	@Test
	public void testAddOneAVUToDataObjectWithTwoAVUTriples() throws Exception {
		String testFileName = "testAddAvuTwoTriples.txt";
		String expectedAttribName = "testattrib1";
		String expectedAttribValue = "testvalue1";
		String expectedUnits = "units1";

		String expectedAttribName2 = "testattrib2";
		String expectedAttribValue2 = "testvalue2";
		String expectedUnits2 = "units2";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);

		// generate testing file
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String absPathToFile = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 20);

		IputCommand iputCommand = new IputCommand();

		iputCommand.setLocalFileName(absPathToFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));

		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		boolean threwException = false;
		IRODSFileSystem irodsFileSystem = null;

		try {
			// open the file and add two AVU's
			irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper
					.buildIRODSAccountFromTestProperties(testingProperties));
			IRODSFile irodsFile = new IRODSFile(irodsFileSystem, iputCommand
					.getIrodsFileName()
					+ '/' + testFileName);
			String[] metaData = { expectedAttribName, expectedAttribValue,
					expectedUnits, expectedAttribName2, expectedAttribValue2,
					expectedUnits2 };
			irodsFile.modifyMetaData(metaData);
		} catch (IllegalArgumentException iae) {
			threwException = true;
		} finally {
			irodsFileSystem.close();
		}

		Assert.assertTrue("did not catch IllegalArgumentException",
				threwException);

	}

	/**
	 * Test for Bug 37 - Add AVU allows more than 3 fields
	 * 
	 * @throws Exception
	 */
	@Test(expected = IllegalArgumentException.class)
	public void testAddOneAVUToDataObjectWithFourParams() throws Exception {
		String testFileName = "testAddAvuUnitsFour.txt";
		String expectedAttribName = "testattrib1";
		String expectedAttribValue = "testvalue1";
		String expectedUnits = "units1";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);

		// generate testing file
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String absPathToFile = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 20);

		IputCommand iputCommand = new IputCommand();

		iputCommand.setLocalFileName(absPathToFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));

		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// open the file and add an AVU
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);
		String[] metaData = { expectedAttribName, expectedAttribValue,
				expectedUnits, "bogus" };
		irodsFile.modifyMetaData(metaData);
		irodsFileSystem.close();

		// verify the metadata was added
		// now get back the avu data and make sure it's there
		ImetaListCommand imetaList = new ImetaListCommand();
		imetaList.setAttribName(expectedAttribName);
		imetaList.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaList.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		String metaValues = invoker
				.invokeCommandAndGetResultAsString(imetaList);
		Assert.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		Assert.assertTrue("did not find expected attrib value", metaValues
				.indexOf(expectedAttribValue) > -1);
		Assert.assertTrue("did not find expected units", metaValues
				.indexOf(expectedUnits) > -1);

		// clean up avu

		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAttribName);
		imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaRemoveCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

	}

	/**
	 *  Bug 85 -  issue with modifyMetadata in Jargon 2.2
	 * @throws Exception
	 *FIXME: currently ignored, need an 'overwrite'? mode
	 */
	@Ignore 
	public void testModifyMetadataThenRepeatModify() throws Exception {
		String testFileName = "testModifyMetadataThenRepeatModify.txt";
		String expectedAttribName = "Upload*****Fix";
		String expectedAttribValue = "Jack*****Luis";

		String expectedAttribValue2 = "Tom*****John";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);

		// generate testing file
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String absPathToFile = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 2);

		IputCommand iputCommand = new IputCommand();

		iputCommand.setLocalFileName(absPathToFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));

		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		boolean threwException = false;
		IRODSFileSystem irodsFileSystem = null;

		// open the file and add two AVU's
		irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);
		
		// first modify
		
		String[] metaData = { expectedAttribName, expectedAttribValue, null };
		irodsFile.modifyMetaData(metaData);
		
		// second modify
		
		String[] metaData2 = { expectedAttribName, expectedAttribValue2, null };
		irodsFile.modifyMetaData(metaData2);
		
		
		irodsFileSystem.close();
		
		// verify the metadata was added
		// now get back the avu data and make sure it's there
		ImetaListCommand imetaList = new ImetaListCommand();
		imetaList.setAttribName(expectedAttribName);
		imetaList.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		imetaList.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		String metaValues = invoker
				.invokeCommandAndGetResultAsString(imetaList);
		Assert.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		Assert.assertTrue("did not find expected attrib value as the second, updated value", metaValues
				.indexOf(expectedAttribValue2) > -1);
		
		Assert.assertTrue("found a duplicate AVU with the value from the first 'set'", metaValues
				.indexOf(expectedAttribValue) == -1);

	}
	
	
	/*
	 * Bug 114 - performance of specific queries in Jargon 2.4
	 */
	@Test
	public void testQueryFileForMetadataTwoAttrib() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryFileForMetadataTwoAttrib.txt";

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
		String meta1Value = "5";
		String meta2Attrib = "attrsingle2";
		String meta2Value = "6";

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,iputCommand.getIrodsFileName() + "/" + testFileName);
		
		// first modify
		
		String[] metaData = { meta1Attrib, meta1Value, null };
		irodsFile.modifyMetaData(metaData);
		
		// second modify
		
		String[] metaData2 = { meta2Attrib, meta2Value, null };
		irodsFile.modifyMetaData(metaData2);
		// now query
		MetaDataCondition[] condition = new MetaDataCondition[2];
		condition[0] = IRODSMetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.GREATER_OR_EQUAL, meta1Value);
		condition[1] = IRODSMetaDataSet.newCondition(meta2Attrib,
				MetaDataCondition.LESS_OR_EQUAL, meta2Value);

		String[] fileds = { IRODSMetaDataSet.FILE_NAME,
				IRODSMetaDataSet.DIRECTORY_NAME };
		MetaDataSelect[] select = IRODSMetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100);

		irodsFileSystem.close();

		TestCase.assertNotNull("no query results returned", fileList);
		TestCase.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		TestCase.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);

	}
	
	
	/*
	 * Bug 114 - performance of specific queries in Jargon 2.4
	 */
	@Test
	public void testQueryFileForMetadataOneAttribTwoValuesInCondition() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryFileForMetadataOneAttribTwoValues.txt";

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

		String metaAttrib = "testQueryFileForMetadataOneAttribTwoValuesInConditionAttrsingle1";
		String metaValue = "5";
		

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,iputCommand.getIrodsFileName() + "/" + testFileName);
		
		// first modify
		
		String[] metaData = { metaAttrib, metaValue, null };
		irodsFile.modifyMetaData(metaData);
		
	
		// now query
		MetaDataCondition[] condition = new MetaDataCondition[2];
		condition[0] = IRODSMetaDataSet.newCondition(metaAttrib,
				MetaDataCondition.GREATER_OR_EQUAL,"4");
		condition[1] = IRODSMetaDataSet.newCondition(metaAttrib,
				MetaDataCondition.LESS_OR_EQUAL, "6");

		String[] fileds = { IRODSMetaDataSet.FILE_NAME,
				IRODSMetaDataSet.DIRECTORY_NAME };
		MetaDataSelect[] select = IRODSMetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100);

		irodsFileSystem.close();

		TestCase.assertNotNull("no query results returned", fileList);
		TestCase.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		TestCase.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);

	}

}
