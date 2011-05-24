package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.grid.io.StandardMetaData;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaAddCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaCommand.MetaObjectType;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.remoteexecute.RemoteExecuteServiceImpl;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.util.Properties;
import junit.framework.Assert;

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
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);

		// now query
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.EQUAL, meta1Value);

		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(
				condition, select, 100, Namespace.FILE, false);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);

	}

	@Test
	public void queryWithNoAvuCondition() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryNoAvuCondition.txt";

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

		// now query
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition(
				StandardMetaData.FILE_NAME, MetaDataCondition.EQUAL,
				testFileName);

		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(
				condition, select, 100, Namespace.FILE, false);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);
	}

	@Test
	public void queryMetadataForFilesInTwoDirectories() throws Exception {
		// put in the thousand files
		String testFilePrefix1 = "multiDir1Test";
		String testFilePrefix2 = "multiDir2Test";

		String testFileSuffix = ".txt";

		String dir1 = "queryTwoDirsDir1";
		String dir2 = "queryTwoDirsDir2";

		String avuAttrib = "queryTwoDirsAttr1";
		String avuValue = "10";

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH
						+ '/' + dir1);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ '/' + dir1, testFilePrefix1, testFileSuffix, 10, 20, 40);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// make the put subdir
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		ImkdirCommand iMkdirCommand = new ImkdirCommand();
		iMkdirCommand.setCollectionName(targetIrodsCollection);
		invoker.invokeCommandAndGetResultAsString(iMkdirCommand);

		String localPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH)
				+ dir1;

		// put the files by putting the collection
		IputCommand iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setLocalFileName(localPath);
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now add avu's to each
		irodsTestSetupUtilities.addAVUsToEachFile(targetIrodsCollection + '/'
				+ dir1, irodsFileSystem, avuAttrib, avuValue);

		// make a second collection

		absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH
						+ '/' + dir2);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ '/' + dir2, testFilePrefix2, testFileSuffix, 10, 20, 40);

		localPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH)
				+ dir2;

		// make the put subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		iMkdirCommand.setCollectionName(targetIrodsCollection);
		invoker.invokeCommandAndGetResultAsString(iMkdirCommand);

		// put the files by putting the collection
		iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setLocalFileName(localPath);
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now add avu's to each
		irodsTestSetupUtilities.addAVUsToEachFile(targetIrodsCollection + '/'
				+ dir2, irodsFileSystem, avuAttrib, avuValue);

		// make a query to find the files based on an avu query for files
		// now query
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition(avuAttrib,
				MetaDataCondition.EQUAL, avuValue);

		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(
				condition, select, 100, Namespace.FILE, false);

		Assert.assertNotNull(
				"no records returned from avu query, I expected results",
				fileList);
		Assert
				.assertEquals(
						"did not find the 10 files in the 2 directories based on the common AVU",
						20, fileList.length);
	}

	@Test
	public void queryMetadataForFileWithTwoAVUs() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "queryMetadataForFileWithTwoAVUs.txt";

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

		String meta1Attrib = "twoavutest1";
		String meta1Value = "c";

		ImetaAddCommand metaAddCommand = new ImetaAddCommand();
		metaAddCommand.setAttribName(meta1Attrib);
		metaAddCommand.setAttribValue(meta1Value);
		metaAddCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);

		// add metadata for this file

		String meta2Attrib = "twoavutest2";
		String meta2Value = "d";

		metaAddCommand = new ImetaAddCommand();
		metaAddCommand.setAttribName(meta2Attrib);
		metaAddCommand.setAttribValue(meta2Value);
		metaAddCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);

		// now query
		MetaDataCondition[] condition = new MetaDataCondition[2];
		condition[0] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.EQUAL, meta1Value);
		condition[1] = MetaDataSet.newCondition(meta2Attrib,
				MetaDataCondition.EQUAL, meta2Value);

		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(
				condition, select, 100, Namespace.FILE, false);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);

	}
	
	@Test 
	public void queryForFileNameUsingIn() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		IRODSServerProperties props = irodsCommands.getIrodsServerProperties();
		//FIXME: increase to 2.5x before release
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion("rods2.4.1")) {
			irodsFileSystem.close();
			return;
		}
		
		String testFileName = "queryForFileNameUsingIn.txt";

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
		
		MetaDataCondition[] condition = new MetaDataCondition[1];
		String fileNames[] = new String[2];
		fileNames[0] = "someFileName.txt";
		fileNames[1] = testFileName;
		condition[0] = MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
				MetaDataCondition.IN, fileNames);
		
		MetaDataSelect select[] = MetaDataSet.newSelection(new String[] {
				StandardMetaData.DIRECTORY_NAME,
				StandardMetaData.FILE_NAME });
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(
				condition, select, 100, Namespace.FILE, false);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);

	}
	
	@Test
	public void queryAVUWithBetween() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		IRODSServerProperties props = irodsCommands.getIrodsServerProperties();
		//FIXME: increase to 2.5x before release
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion("rods2.4.1")) {
			irodsFileSystem.close();
			return;
		}
		
		String testFileName = "queryAVUWithBetween.txt";

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

		String meta1Attrib = "queryAVUWithBetween";
		String meta1Value = "3";

		ImetaAddCommand metaAddCommand = new ImetaAddCommand();
		metaAddCommand.setAttribName(meta1Attrib);
		metaAddCommand.setAttribValue(meta1Value);
		metaAddCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);
		
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.BETWEEN, "1", "5");
	
		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);
	}

	/*
	 * Bug 110 - error when asking IRODSFile.exists with & in file name #2 case
	 */
	@Test
	public void queryMetadataForFileWithLeadingAmpInName() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "&queryMetadataForFileWithLeadingAmpInName";
		  String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
	                IRODS_TEST_SUBDIR_PATH);


		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		MetaDataCondition conditions[] = {
				MetaDataSet
						.newCondition(StandardMetaData.DIRECTORY_NAME,
								MetaDataCondition.EQUAL, targetIrodsCollection),
				MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
						MetaDataCondition.EQUAL, testFileName),
				};
		MetaDataSelect selects[] = MetaDataSet.newSelection(new String[] {
				StandardMetaData.DIRECTORY_NAME,
				StandardMetaData.FILE_NAME });
		MetaDataRecordList[] fileDetails = irodsFileSystem
				.query(conditions, selects);

		irodsFileSystem.close();
		Assert.assertNotNull("no query results returned", fileDetails);

	}
	
	/*
	 * Bug 110 - error when asking IRODSFile.exists with & in file name #2 case, currently triggered a bug in iRODS, so ignored
	 * for now
	 */
	@Ignore
	public void queryMetadataForFileWithTwoLeadingAmpInName() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "&&queryMetadataForFileWithTwoLeadingAmpInName";
		  String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
	                IRODS_TEST_SUBDIR_PATH);


		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		MetaDataCondition conditions[] = {
				MetaDataSet
						.newCondition(StandardMetaData.DIRECTORY_NAME,
								MetaDataCondition.EQUAL, targetIrodsCollection),
				MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
						MetaDataCondition.EQUAL, testFileName),
				};
		MetaDataSelect selects[] = MetaDataSet.newSelection(new String[] {
				StandardMetaData.DIRECTORY_NAME,
				StandardMetaData.FILE_NAME });
		MetaDataRecordList[] fileDetails = irodsFileSystem
				.query(conditions, selects);

		irodsFileSystem.close();
		Assert.assertNotNull("no query results returned", fileDetails);

	}

	/*
	 * Bug 110 - error when asking IRODSFile.exists with & in file name #2 case
	 */
	@Test
	public void queryResourceNameAndStatus() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "&queryMetadataForFileWithLeadingAmpInName";
		  String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
	                IRODS_TEST_SUBDIR_PATH);

		MetaDataCondition conditions[] = {
				
				};
		MetaDataSelect selects[] = MetaDataSet.newSelection(new String[] {
				ResourceMetaData.RESOURCE_STATUS,
                ResourceMetaData.RESOURCE_NAME });
		MetaDataRecordList[] fileDetails = irodsFileSystem
				.query(conditions, selects);

		irodsFileSystem.close();
		Assert.assertNotNull("no query results returned", fileDetails);

	}
	
	/*
	 * Bug 114 - performance of specific queries in Jargon 2.4
	 */
	@Test
	public void queryFileAndDirectoryAndAVU() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "queryFileAndDirectoryAndAVU.txt";

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

		String meta1Attrib = "queryFileAndDirectoryAndAVU";
		String meta1Value = "5";

		ImetaAddCommand metaAddCommand = new ImetaAddCommand();
		metaAddCommand.setAttribName(meta1Attrib);
		metaAddCommand.setAttribValue(meta1Value);
		metaAddCommand.setMetaObjectType(MetaObjectType.DATA_OBJECT_META);
		metaAddCommand.setObjectPath(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);

		// now query
		MetaDataCondition[] condition = new MetaDataCondition[2];
		condition[0] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.GREATER_OR_EQUAL, meta1Value);
		condition[1] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.LESS_OR_EQUAL, meta1Value);
		
		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(
				condition, select, 100, Namespace.FILE, false);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find my file and metadata", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);

	}

}
