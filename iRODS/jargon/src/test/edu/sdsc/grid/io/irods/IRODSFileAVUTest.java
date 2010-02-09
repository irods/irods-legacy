package edu.sdsc.grid.io.irods;

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
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
	private static AssertionHelper assertionHelper = null;
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
		assertionHelper = new AssertionHelper();
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
		TestCase.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		TestCase.assertTrue("did not find expected attrib value", metaValues
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
		TestCase.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		TestCase.assertTrue("did not find expected attrib value", metaValues
				.indexOf(expectedAttribValue) > -1);
		TestCase.assertTrue("did not find expected units", metaValues
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
		
		TestCase.assertTrue("did not catch IllegalArgumentException", threwException);

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
		TestCase.assertTrue("did not find expected attrib name", metaValues
				.indexOf(expectedAttribName) > -1);
		TestCase.assertTrue("did not find expected attrib value", metaValues
				.indexOf(expectedAttribValue) > -1);
		TestCase.assertTrue("did not find expected units", metaValues
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

}
