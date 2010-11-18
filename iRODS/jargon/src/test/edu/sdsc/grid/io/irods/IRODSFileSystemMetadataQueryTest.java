package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.grid.io.GeneralMetaData;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
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

import junit.framework.Assert;
import junit.framework.TestCase;

public class IRODSFileSystemMetadataQueryTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileSystemMetadataQueryTest";
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
	public void testCreateAndModifyDate() throws Exception {
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

		// query(String[]) defined in GeneralFile
		// will call query(String[] conditions, String[] selects) in IRODSFile,
		// with null in conditions and these
		// values in selects. The values in the selects are abstract data names
		// that are translated
		// into IRODS-specific query symbols in IRODSMetaDataSet

		// trickles down to IRODSCommands.query()

		MetaDataRecordList[] lists = irodsFile.query(new String[] {
				GeneralMetaData.CREATION_DATE,
				GeneralMetaData.MODIFICATION_DATE });

		for (MetaDataRecordList l : lists) {
			String createDate = l.getStringValue(0);
			String modDate = l.getStringValue(1);

			// demonstration code for getting real dates from irods date, left
			// for reference
			TimeZone timeZone = TimeZone.getTimeZone("GMT");
			TimeZone easternTimeZone = TimeZone.getTimeZone("GMT-5:00");
			DateFormat dateFormat = DateFormat.getDateTimeInstance();
			dateFormat.setTimeZone(timeZone);
			Calendar calendar = Calendar.getInstance();
			// calendar.setTimeZone(timeZone);
			calendar.setTimeInMillis(0L);
			// System.out.println("epoch date: " +
			// dateFormat.format(calendar.getTime()));
			calendar.add(Calendar.SECOND, Integer.parseInt(createDate));
			Date computedDate = calendar.getTime();
			dateFormat.setTimeZone(easternTimeZone);
			dateFormat.format(computedDate);
			// System.out.println("create date: " +
			// dateFormat.format(computedDate.getTime()));

			Assert.assertEquals("create and mod date not equal", createDate,
					modDate);
		}

		irodsFileSystem.close();

	}

	@Test
	public void testQueryFileNameWithLike() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryFileName1.txt";

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

		String keyword = "testQueryFileName1%";
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
				MetaDataCondition.LIKE, keyword);
		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME, GeneralMetaData.SIZE };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100);
		Assert.assertTrue("did not return query result for file I just added",
				fileList != null);

		irodsFileSystem.close();

	}

	/**
	 * Bug 46 - Querying multiple metadata values returns no result
	 * 
	 * @throws Exception
	 */
	@Test
	public void testMultipleUserMetadataQuery() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testMultipleUserMetadata.txt";

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
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);

		metaAddCommand.setAttribName(meta2Attrib);
		metaAddCommand.setAttribValue(meta2Value);
		invoker.invokeCommandAndGetResultAsString(metaAddCommand);

		// now query for multiple value
		MetaDataCondition[] condition = new MetaDataCondition[2];
		condition[0] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.EQUAL, meta1Value);
		condition[1] = MetaDataSet.newCondition(meta2Attrib,
				MetaDataCondition.EQUAL, meta2Value);

		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100);

		irodsFileSystem.close();

		Assert.assertNotNull("no query results returned", fileList);
		Assert.assertEquals("did not find any query results", 1,
				fileList.length);
		Assert.assertTrue("did not find my file name in results", fileList[0]
				.toString().indexOf(testFileName) > -1);
	}

	@Test
	public void testSingleUserMetadataQuery() throws Exception {
		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testSingleUserMetadata.txt";

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
		MetaDataCondition[] condition = new MetaDataCondition[2];
		condition[0] = MetaDataSet.newCondition(meta1Attrib,
				MetaDataCondition.EQUAL, meta1Value);

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

	/**
	 * Bug 73 - query on file system for AVU's not returning results, where
	 * query on indiv directories does
	 * 
	 * @throws Exception
	 */
	@Test
	public void testMultipleAVUQueryOnCollectionMetaData() throws Exception {
		// add an irods collection and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String baseDir = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		String testSubdir1 = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH
								+ "/testSubdir1");

		String testSubdir1a = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH
								+ "/testSubdir1/testSubdir1a");

		String testSubdir1b = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH
								+ "/testSubdir1/testSubdir1b");

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		String testSubdir2 = testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH
								+ "/testSubdir2");

		String testSubdir2a = testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH
								+ "/testSubdir2/testSubdir2a");

		// make the dirs in irods

		ImkdirCommand imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(testSubdir1);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(testSubdir1a);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(testSubdir1b);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(testSubdir2);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(testSubdir2a);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		// seed files in each subdir

		String testFilePrefix = "testFile";
		String testFileSuffix = ".txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ "/testSubdir1", testFilePrefix, testFileSuffix, 10, 20, 500);

		// put the files by putting the collection
		IputCommand iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(baseDir);
		iputCommand.setLocalFileName(absPath + "/testSubdir1");
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		testFilePrefix = "testFile1a";
		testFileSuffix = ".txt";
		absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ "/testSubdir1/testSubdir1a", testFilePrefix, testFileSuffix,
				10, 20, 500);

		// put the files by putting the collection subdir1/subdir1a
		iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(testSubdir1);
		iputCommand.setLocalFileName(absPath + "/testSubdir1/testSubdir1a");
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		testFilePrefix = "testFile1b";
		testFileSuffix = ".txt";
		absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ "/testSubdir1/testSubdir1b", testFilePrefix, testFileSuffix,
				10, 20, 500);

		// put the files by putting the collection subdir1/subdir1a
		iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(testSubdir1);
		iputCommand.setLocalFileName(absPath + "/testSubdir1/testSubdir1b");
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		testFilePrefix = "testFile2";
		testFileSuffix = ".txt";
		absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ "/testSubdir2", testFilePrefix, testFileSuffix, 10, 20, 500);

		// put the files by putting the collection subdir1/subdir1a
		iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(baseDir);
		iputCommand.setLocalFileName(absPath + "/testSubdir2");
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		testFilePrefix = "testFile2a";
		testFileSuffix = ".txt";
		absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ "/testSubdir2/testSubdir2a", testFilePrefix, testFileSuffix,
				10, 20, 500);

		// put the files by putting the collection subdir1/subdir1a
		iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(testSubdir2);
		iputCommand.setLocalFileName(absPath + "/testSubdir2/testSubdir2a");
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// add avu's for the directories, apply to the 1a and 1b subdirs

		String avuAttr1 = "seed";
		String avuVal1 = "21026";

		String avuAttr2 = "T";
		String avuVal2 = "10000";

		addAVUsToDir(irodsFileSystem, testSubdir1a, avuAttr1, avuVal1);
		addAVUsToDir(irodsFileSystem, testSubdir1a, avuAttr2, avuVal2);

		addAVUsToDir(irodsFileSystem, testSubdir1b, avuAttr1, avuVal1);
		addAVUsToDir(irodsFileSystem, testSubdir1b, avuAttr2, avuVal2);

		// now query 1 val for all dirs, should get subdir1a and subdir1b
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition(avuAttr1,
				MetaDataCondition.EQUAL, avuVal1);

		String[] selectVals = { StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(selectVals);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100, Namespace.DIRECTORY);

		Assert.assertNotNull(fileList);
		Assert.assertEquals(2, fileList.length);
		irodsFileSystem.close();

	}

	/**
	 * Currently ignored pre 2.4.1 as this is a genquery issue... Bug 116 - Query with
	 * Query multiple user AVU on Collection returns no result
	 * 
	 * @throws Exception
	 */
	@Test
	public void testQueryCollectionMetadataTwoValuesEqualCondition()
			throws Exception {

		String testCollection = "testQueryCollectionMetadataTwoValuesEqualCondition";

		// add an irods collection and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSServerProperties irodsServerProperties = irodsFileSystem
				.getCommands().getIrodsServerProperties();

		// test only post 2.4.1
		if (irodsFileSystem.commands.getIrodsServerProperties().getRelVersion()
				.compareTo("rods2.4") <= 0) {
			return;
		}

		String baseDir = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		String testSubdir1 = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testCollection);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// make the dirs in irods

		ImkdirCommand imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(testSubdir1);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		// seed files in each subdir

		String testFilePrefix = "testFile";
		String testFileSuffix = ".txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ "/" + testCollection, testFilePrefix, testFileSuffix, 2, 20,
				25);

		// put the files by putting the collection
		IputCommand iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(baseDir);
		iputCommand.setLocalFileName(absPath + testCollection);
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// add avu's

		String avuAttr1 = "testmeta1";
		String avuVal1 = "180";

		String avuAttr2 = "testmeta2";
		String avuVal2 = "50";

		addAVUsToDir(irodsFileSystem, testSubdir1, avuAttr1, avuVal1);
		addAVUsToDir(irodsFileSystem, testSubdir1, avuAttr2, avuVal2);

		MetaDataCondition[] dataObjectConditions = new MetaDataCondition[2];
		dataObjectConditions[0] = MetaDataSet.newCondition(avuAttr1,
				MetaDataCondition.EQUAL, avuVal1);
		dataObjectConditions[1] = MetaDataSet.newCondition(avuAttr2,
				MetaDataCondition.EQUAL, avuVal2);
		String[] selectFieldNames = { IRODSMetaDataSet.META_COLL_ATTR_NAME,
				IRODSMetaDataSet.META_COLL_ATTR_VALUE,
				GeneralMetaData.DIRECTORY_NAME, };
		MetaDataSelect selects[] = MetaDataSet.newSelection(selectFieldNames);
		MetaDataRecordList[] recordList = irodsFileSystem.query(
				dataObjectConditions, selects, Namespace.DIRECTORY);
		irodsFileSystem.close();

		TestCase.assertNotNull("no results returned from metadata query",
				recordList);

	}

	static final void addAVUsToDir(IRODSFileSystem irodsFileSystem,
			String subdirAbsolutePath, String attr, String val)
			throws Exception {

		String[] metaData = new String[2];
		metaData[0] = attr;
		metaData[1] = val;

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, subdirAbsolutePath);

		// get a list of files underneath the top-level directory, and add some
		// avu's to each one

		irodsFile.modifyMetaData(metaData);

	}

}
