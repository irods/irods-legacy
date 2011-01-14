package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_HOST_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PASSWORD_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PORT_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_ZONE_KEY;

import java.net.URI;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IlsCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class IRODSFileTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

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
	public final void testGetIRODSFileUsingUriAndSeeIfExistsWorks()
			throws Exception {
		// generate a local scratch file
		String testFileName = "testfileuri.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				2);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);

		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		irodsFile.close();

	}

	/**
	 * BUG: 24
	 **/
	@Test
	public final void testGetResourceOnExistingFileAndVerify() throws Exception {
		// generate a local scratch file
		String testFileName = "testGetResourceOnExistingFileAndVerify.xsl";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				2);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);
		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		String actualResource = irodsFile.getResource();
		irodsFile.close();
		TestCase.assertEquals("I should have gotten the default resource",
				testingProperties.getProperty(IRODS_RESOURCE_KEY),
				actualResource);
	}

	/**
	 * BUG: 24 - Is this appropriate behavior to query for physical resource
	 * when collection? Will give null if the IRODSFile in question is a
	 * collection, and not a file. Get the default resource for this collection
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testGetResourceOnExistingCollectionAndVerify()
			throws Exception {

		// can I use jargon to access the collection on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						IRODS_TEST_SUBDIR_PATH);
		IRODSFile irodsFile = new IRODSFile(irodsUri);
		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		TestCase.assertTrue("testing file is not a collection", irodsFile
				.isDirectory());
		// should query for resource in IRODSFile
		String actualResource = irodsFile.getResource();
		TestCase.assertEquals("I should have gotten the default resource", "",
				actualResource);
		// note on this test that it should, for consistency, have null in
		// resource
		irodsFile.close();

	}

	@Test(expected = IRODSException.class)
	public final void testIRODSFileURIInvalidUser() throws Exception {
		StringBuilder irodsUri = new StringBuilder();

		irodsUri.append("irods://");
		irodsUri.append("iminvalid");
		irodsUri.append(".");
		irodsUri.append(testingProperties.getProperty(IRODS_ZONE_KEY));
		irodsUri.append(":");
		irodsUri.append(testingProperties.getProperty(IRODS_PASSWORD_KEY));
		irodsUri.append("@");
		irodsUri.append(testingProperties.getProperty(IRODS_HOST_KEY));
		irodsUri.append(":");
		irodsUri.append(String.valueOf(testingProperties
				.getProperty(IRODS_PORT_KEY)));
		irodsUri.append("/");
		irodsUri.append("fwd.txt");

		@SuppressWarnings("unused")
		IRODSFile irodsFile = new IRODSFile(new URI(irodsUri.toString()));
	}

	/**
	 * Put a file to a specific resource, should get that resource for the file
	 * in IRODSFile BUG: 24
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testPutFileInResc2AndCheckIfRightDefault()
			throws Exception {
		// generate a local scratch file
		String testFileName = "testPutFileInResc2AndCheckIfRightDefault.xsl";
		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testFileName, 4);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(testFileFullPath);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);
		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		String actualResource = irodsFile.getResource();
		TestCase
				.assertEquals(
						"I should have gotten the specific resource I used for the iput",
						testingProperties
								.getProperty(IRODS_SECONDARY_RESOURCE_KEY),
						actualResource);
		irodsFile.close();
	}

	/**
	 * This is based on the state of the local irods ENV. When the URI is
	 * created for the Dest file, it may be null, in which case the put may fail
	 * with a -78000 due to no resource being found.
	 * 
	 * The core.irb contains a rule that must be modified to support the setting
	 * of default resources, e.g.
	 * acSetRescSchemeForCreate||msiSetDefaultResc(test1-resc,preferred)|nop
	 * 
	 * Note here that test1-resc is a resource set up on the test irods
	 * instance, and configured in testing.properties
	 * 
	 * @throws Exception
	 *             BUG: 31
	 */
	@Test
	public final void testFilePutCreateDestFileByUri() throws Exception {
		String testFileName = "testFilePut.csv";

		// make up a test file of 20kb
		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testFileName, 17);
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		// point to the local file I just built
		StringBuilder sourceFileName = new StringBuilder();
		sourceFileName.append("file:///");
		sourceFileName.append(testFileFullPath);

		GeneralFile sourceFile = FileFactory.newFile(new URI(sourceFileName
				.toString()));

		// point to an irods file and put it
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
		IRODSFile destFile = (IRODSFile) FileFactory.newFile(irodsUri);

		irodsFileSystem.commands.put(sourceFile, destFile, true);

		AssertionHelper assertionHelper = new AssertionHelper();
		assertionHelper.assertIrodsFileOrCollectionExists(destFile
				.getAbsolutePath());
		irodsFileSystem.close();
	}

	@Test
	public final void testFileGetByUri() throws Exception {
		// generate a file and put it in irods
		String testFileName = "testFileGet.doc";
		String returnTestFileName = "testFileGetCopy.doc";
		int expectedLength = 15;
		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testFileName,
						expectedLength);
		IputCommand iputCommand = new IputCommand();

		iputCommand.setLocalFileName(testFileFullPath);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now try and create the file by uri and get it
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
		IRODSFile getFile = (IRODSFile) FileFactory.newFile(irodsUri);

		TestCase.assertTrue("file I put does not exist", getFile.exists());

		String fullPathToReboundFile = "file:///"
				+ testingProperties.getProperty(GENERATED_FILE_DIRECTORY_KEY)
				+ IRODS_TEST_SUBDIR_PATH + '/' + returnTestFileName;

		GeneralFile reboundFile = FileFactory.newFile(new URI(
				fullPathToReboundFile));
		getFile.copyTo(reboundFile, true);
		assertionHelper.assertLocalScratchFileLengthEquals(
				IRODS_TEST_SUBDIR_PATH + '/' + returnTestFileName,
				expectedLength);
	}

	@Test
	public final void testFileExistsIsFalseWhenNew() throws Exception {
		String testFileName = "IDontExist.doc";
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
		IRODSFile testFile = (IRODSFile) FileFactory.newFile(irodsUri);
		TestCase.assertFalse("I shouldnt exist, I am new", testFile.exists());
		testFile.close();

	}

	@Test
	public final void testDirExistsIsFalseWhenNew() throws Exception {
		String testFileName = "IDontExistPath";
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
		IRODSFile testFile = (IRODSFile) FileFactory.newFile(irodsUri);
		TestCase.assertFalse("I shouldnt exist, I am new", testFile.exists());
		testFile.close();
	}

	@Test
	public final void testCopyToFromIRODSToLocalNoResourceSpecified()
			throws Exception {

		// generate a local scratch file
		String testFileName = "testGet.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		// put scratch file into irods in the right place on the first resource
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// create a GeneralFile (local) for the get results

		String getTargetFilePath = absPath + "GetResult" + testFileName;
		GeneralFile localFile = new LocalFile(getTargetFilePath);

		irodsFile.copyTo(localFile, true);
		irodsFileSystem.close();

		assertionHelper.assertLocalFileExistsInScratch(IRODS_TEST_SUBDIR_PATH
				+ "/" + "GetResult" + testFileName);

	}

	@Test
	public final void testCopyToFromIRODSToLocalResourceSpecified()
			throws Exception {

		// generate a local scratch file
		String testFileName = "testCopyIRODSToLocalWithResc.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		// put scratch file into irods in the right place on the first resource
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// create a GeneralFile (local) for the get results

		String getTargetFilePath = absPath + "GetResult" + testFileName;
		GeneralFile localFile = new LocalFile(getTargetFilePath);

		irodsFile.copyTo(localFile, true, testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		irodsFileSystem.close();
		irodsFile.close();

		assertionHelper.assertLocalFileExistsInScratch(IRODS_TEST_SUBDIR_PATH
				+ "/" + "GetResult" + testFileName);

	}

	@Test
	public final void testCopyToFromIRODSToLocalDifferentResourceSpecified()
			throws Exception {

		// generate a local scratch file
		String testFileName = "testCopyIRODSToLocalWithResc.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		// put scratch file into irods in the right place on the first resource
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// create a GeneralFile (local) for the get results

		String getTargetFilePath = absPath + "GetResult" + testFileName;
		GeneralFile localFile = new LocalFile(getTargetFilePath);

		irodsFile
				.copyTo(
						localFile,
						true,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));
		irodsFileSystem.close();
		irodsFile.close();

		assertionHelper
				.assertLocalFileNotExistsInScratch(IRODS_TEST_SUBDIR_PATH + "/"
						+ "GetResult" + testFileName);

	}

	/**
	 * BUG: this test documents a current bug, where a query on resource using
	 * metadata query will not return a valid resource if there are no files in
	 * that resource
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testSetResourceShowQueryBug() throws Exception {
		// generate a local scratch file
		String testFileName = "testSetResource.xsl";

		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testFileName, 4);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(testFileFullPath);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(IRODS_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		String actualResource = irodsFile.getResource();
		TestCase.assertEquals("did not set up file in correct resource",
				testingProperties.getProperty(IRODS_RESOURCE_KEY),
				actualResource);

		irodsFile.setResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
		TestCase.assertEquals("did not set up file in correct resource",
				testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY),
				irodsFile.getResource());

		irodsFileSystem.close();

	}

	@Test
	public final void testSetResource() throws Exception {
		// generate a local scratch file
		String testFileName = "testSetResource.xsl";
		String testOtherFileName = "fileInOtherResource.xsl";

		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testFileName, 4);

		String testOtherFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testOtherFileName, 4);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(testFileFullPath);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(IRODS_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		iputCommand = new IputCommand();

		iputCommand.setLocalFileName(testOtherFileFullPath);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		invoker.invokeCommandAndGetResultAsString(iputCommand);

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		String actualResource = irodsFile.getResource();
		TestCase.assertEquals("did not set up file in correct resource",
				testingProperties.getProperty(IRODS_RESOURCE_KEY),
				actualResource);

		irodsFile.setResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
		TestCase.assertEquals("did not set up file in correct resource",
				testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY),
				irodsFile.getResource());

		irodsFileSystem.close();

	}

	@Test(expected=IllegalArgumentException.class)
	public final void testSetInvalidResource() throws Exception {
		// generate a local scratch file
		String testFileName = "testSetResource.xsl";
		String testOtherFileName = "fileInOtherResource.xsl";

		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testFileName, 4);

		String testOtherFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + '/', testOtherFileName, 4);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(testFileFullPath);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(IRODS_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		iputCommand = new IputCommand();

		iputCommand.setLocalFileName(testOtherFileFullPath);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setIrodsResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		invoker.invokeCommandAndGetResultAsString(iputCommand);

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
		String actualResource = irodsFile.getResource();
		TestCase.assertEquals("did not set up file in correct resource",
				testingProperties.getProperty(IRODS_RESOURCE_KEY),
				actualResource);

		irodsFile.setResource("idontexist");
		
	}

	
	
	@Test
	public final void testIsFileReadable() throws Exception {

		// create a file and place on two resources
		String testFileName = "testIsFileReadable.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		TestCase.assertTrue("I own the file and should be able to read",
				irodsFile.canRead());
		irodsFileSystem.close();

	}

	@Test
	public final void testIsFileNotReadable() throws Exception {

		// create a file and place on two resources
		String testFileName = "testIsFileReadable.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromSecondaryTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		TestCase.assertFalse("I own the file and should be able to read",
				irodsFile.canRead());
		irodsFileSystem.close();

	}

	@Test
	public final void testIsFileExists() throws Exception {

		// create a file and place on two resources
		String testFileName = "testIsFileExists.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertTrue(exists);

	}
	
	/*
	 * Bug 128 - NPE on query, reported as miscServerInfo() problem
	 */
	@Test
	public final void testIsFileExistsABunchOfTimesAlsoAskingForMiscServerInfo() throws Exception {

		int numberOfIterations = 10000;
		
		// create a file and place on two resources
		String testFileName = "testIsFileExistsABunchOfTimesAlsoAskingForMiscServerInfo.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		for (int i = 0; i < numberOfIterations; i++) {
			boolean exists = irodsFile.exists();
			irodsFileSystem.commands.miscServerInfo();
		}
		
		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertTrue(exists);

	}
	
	/*
	 * Bug 110 - error when asking IRODSFile.exists with & in file name
	 * Currently an outstanding issue in iRODS thus ignore so tests pass
	 */
	@Ignore
	public final void testIsFileExistsTwoAmpInName() throws Exception {

		// create a file and place on two resources
		String testFileName = "&&testIsFileExists.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertTrue(exists);

	}
	
	/*
	 * Bug 110 - error when asking IRODSFile.exists with & in file name
	 */
	@Test
	public final void testIsFileExistsOneAmpInName() throws Exception {

		// create a file and place on two resources
		String testFileName = "&testIsFileExists.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertTrue(exists);

	}
	
	
	/**
	 *  Bug 91 -  problems with Jargon not detecting dead IRODS Agent  
	 *  
	 *  This test actually requires some manual intervention to kill the agent at a breakpoint before
	 *  calling exists().
	 *  
	 *  //TODO: can I kill the agent somehow?
	 *  
	 * @throws Exception
	 */
	@Test
	public final void testIsDirectoryExistsFalseWhenIRODSAgentDiesAndDoesNotDetectAgentIsDead() throws Exception {

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertTrue(exists);

	}

	@Test
	public final void testIsDirectoryExists() throws Exception {

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertTrue(exists);

	}

	@Test
	public final void testIsFileNotExistsButDirectoryDoesShouldNotExist()
			throws Exception {

		// create a file and place on two resources
		String testFileName = "testIsFileNotExists.txt";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		boolean exists = irodsFile.exists();
		irodsFileSystem.close();
		TestCase.assertFalse(exists);

	}

	@Test
	public final void testGetAbsolutePathFile() throws Exception {

		// create a file and place on two resources
		String testFileName = "testGetAbsolutePathFile.txt";

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		// now get an irods file and see if it is readable, it should be

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		String actualAbsolutePath = irodsFile.getAbsolutePath();
		irodsFileSystem.close();

		TestCase.assertEquals("paths do not match", targetIrodsCollection + '/'
				+ testFileName, actualAbsolutePath);

	}

	/**
	 * Test method for
	 * {@link org.irods.jargon.core.pub.io.IRODSFile#lastModified()}.
	 */
	@Test
	public final void testLastModified() throws Exception {
		String testFileName = "testLastModified.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		long modDate = irodsFile.lastModified();
		irodsFileSystem.close();
		TestCase.assertTrue("mod date should be gt 0", modDate > 0);

	}

	@Test
	public final void testGetLengthFileNoResourceSet() throws Exception {

		// create a file and place on two resources
		String testFileName = "testGetLengthFile.txt";
		long expectedLength = 8;
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		long length = irodsFile.length();

		irodsFileSystem.close();
		TestCase.assertEquals("size does not match", expectedLength, length);
	}
	
	@Test
	public final void testGetLengthFileWithResourceSet() throws Exception {

		// create a file and place on two resources
		String testFileName = "testGetLengthFileWithResourceSet.txt";
		long expectedLength = 11;
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				expectedLength);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		
		irodsFile.setResource(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));

		long length = irodsFile.length();

		irodsFileSystem.close();
		TestCase.assertEquals("size does not match", expectedLength, length);
	}
	
	@Test
	public final void testGetLengthFileDoesNotExist() throws Exception {

		// create a file and place on two resources
		String testFileName = "thisFileReallyDoesntExist.txt";
		long expectedLength = 0;
		

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		// now get an irods file and see if it is readable, it should be
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		
		irodsFile.setResource(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));

		long length = irodsFile.length();

		irodsFileSystem.close();
		TestCase.assertEquals("size does not match", expectedLength, length);
	}

	@Test
	public final void testGetListInDir() throws Exception {
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, "");

		// now get an irods file and see if it is readable, it should be
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		String[] dirs = irodsFile.list();
		irodsFile.close();
		TestCase.assertNotNull(dirs);
		TestCase.assertTrue("no results", dirs.length > 0);
	}

	@Test
	public final void testGetListFromPreparedSubdir() throws Exception {
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		String topLevelTestDir = "getListFromPreparedSubdirTopLevel";
		String subdir1 = "subdir1";
		String subdir2 = "subdir2";

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		// make a 'top level' dir for this

		ImkdirCommand imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(targetIrodsCollection + '/'
				+ topLevelTestDir);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		// + 2 subdirs

		imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(targetIrodsCollection + '/'
				+ topLevelTestDir + '/' + subdir1);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		imkdirCommand = new ImkdirCommand();
		imkdirCommand.setCollectionName(targetIrodsCollection + '/'
				+ topLevelTestDir + '/' + subdir2);
		invoker.invokeCommandAndGetResultAsString(imkdirCommand);

		// +1 file
		String testFileName = "testList.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

		IputCommand iputCommand = new IputCommand();

		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ topLevelTestDir);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now get an irods file and see if it is readable, it should be
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		String[] dirs = irodsFile.list();
		irodsFile.close();
		TestCase.assertNotNull(dirs);
		TestCase.assertTrue("no results", dirs.length == 3);
	}

	/**
	 * Test method for
	 * {@link org.irods.jargon.core.pub.io.IRODSFile#equals(java.lang.Object)}.
	 */
	@Test
	public final void testEqualsObject() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		String testFileName = "testEquals.txt";

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		IRODSFile irodsFile2 = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		irodsFileSystem.close();

		TestCase.assertEquals(irodsFile, irodsFile2);

	}

	/**
	 * Test method for
	 * {@link org.irods.jargon.core.pub.io.IRODSFile#equals(java.lang.Object)}.
	 */
	@Test
	public final void testNotEqualsObject() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		String testFileName = "testEquals.txt";

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		IRODSFile irodsFile2 = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + "xxx");

		irodsFileSystem.close();

		TestCase.assertFalse("files should not be equal", irodsFile
				.equals(irodsFile2));

	}

	/**
	 * Test method for
	 * {@link org.irods.jargon.core.pub.io.IRODSFile#getCanonicalPath()}.
	 */
	@Test
	public final void testGetCanonicalPath() throws Exception {

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		String testFileName = "testEquals.txt";

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		String actualPath = irodsFile.getCanonicalPath();
		irodsFileSystem.close();

		TestCase.assertEquals("paths do not match", targetIrodsCollection,
				actualPath);

		irodsFileSystem.close();
	}

	@Test
	public final void testGetCanonicalFile() throws Exception {

		String testFileName = "testGetCanonicalPath.txt";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		IRODSFile irodsCanonicalFile = (IRODSFile) irodsFile.getCanonicalFile();
		irodsFileSystem.close();
		TestCase.assertEquals("files", irodsCanonicalFile, irodsFile);
	}

	/**
	 * Test method for {@link org.irods.jargon.core.pub.io.IRODSFile#getName()}.
	 */
	@Test
	public final void testGetName() throws Exception {
		String testFileName = "testGetName.txt";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		String actualName = irodsFile.getName();
		irodsFileSystem.close();
		TestCase.assertEquals("names do not match", testFileName, actualName);
	}

	@Test
	public final void testGetParentFile() throws Exception {

		String testFileName = "testGetCanonicalPath.txt";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		IRODSFile irodsParentFile = (IRODSFile) irodsFile.getParentFile();
		irodsFileSystem.close();
		TestCase.assertEquals("files", irodsFile.getParent(), irodsParentFile
				.getAbsolutePath());
	}

	@Test
	public final void testMkdir() throws Exception {
		String testDir = "testMkdir";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testDir;

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		irodsFile.mkdir();
		AssertionHelper assertionHelper = new AssertionHelper();
		assertionHelper.assertIrodsFileOrCollectionExists(irodsFile
				.getAbsolutePath());
		irodsFileSystem.close();
	}

	@Test
	public final void testMkdirWhenNonExistentIntermediateDirectory() throws Exception {
		String testDir = "testMkdirWhenNonExistentItermediateDirectory/andTryToMakeThisDir";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testDir;

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		boolean result =irodsFile.mkdir();
		irodsFileSystem.close();
		TestCase.assertFalse("should not have created file, and returned false from the mkdir", result);
	}
	
	@Test
	public final void testMkdirsWithIntermediateDirectories() throws Exception {
		String testDir = "testMkdirsWithIntermediateDirectoies/andTryToMakeThisDir";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testDir;

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		boolean result =irodsFile.mkdirs();
		irodsFileSystem.close();
		TestCase.assertTrue("should have created intermediate directories", result);
		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsCollection);
	}
	
	@Test
	public final void testMkdirsWithNoIntermediateDirectories() throws Exception {
		String testDir = "testMkdirsWithNoIntermediateDirectories";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testDir;

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);
		boolean result =irodsFile.mkdirs();
		irodsFileSystem.close();
		TestCase.assertTrue("should have created directory", result);
		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsCollection);
	}
	
	
	/**
	 * Bug 78 - issues with getHomeDirectory after IRODS2.3 upgrade
	 */
	@Test
	public final void testGetHomeDir() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFile irodsFile = (IRODSFile) FileFactory.newFile(irodsFileSystem,
				irodsFileSystem.getHomeDirectory());
		boolean canWrite = irodsFile.canWrite();
		irodsFileSystem.close();
		TestCase.assertTrue("file should be writable", canWrite);

	}

	/**
	 * Bug 78 - issues with getHomeDirectory after IRODS2.3 upgrade
	 */
	@Test
	public final void testGetHomeDirMultipleTimes() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		int numberOfTimes = 30;

		for (int i = 0; i < numberOfTimes; i++) {

			IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

			IRODSFile irodsFile = (IRODSFile) FileFactory.newFile(
					irodsFileSystem, irodsFileSystem.getHomeDirectory());
			boolean canWrite = irodsFile.canWrite();
			irodsFileSystem.close();
			TestCase.assertTrue("file should be writable", canWrite);
		}

	}
	
	/**
	 *  Bug 86 -  renameTo does not rename file
	 * @throws Exception
	 */
	@Test
    public final void testFileRename() throws Exception {
    	// generate a local scratch file
        String testFileName = "testFilerenameBefore.txt";
        String renameFileName = "testFilerenameAfter.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

        StringBuilder fileNameAndPath = new StringBuilder();
        fileNameAndPath.append(absPath);

        fileNameAndPath.append(testFileName);
        iputCommand.setLocalFileName(fileNameAndPath.toString());
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        IRODSFile irodsFileAfter = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + renameFileName);
        irodsFile.renameTo(irodsFileAfter);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFileAfter.getAbsolutePath());
        assertionHelper.assertIrodsFileOrCollectionDoesNotExist(irodsFile.getAbsolutePath());

    }
	
	/**
	 *  Bug 86 -  renameTo does not rename file
	 * @throws Exception
	 */
	@Test
    public final void testCollectionRename() throws Exception {
    	// generate a local scratch file
        String beforeRename = "testCollectionRenameBefore";
        String afterRename = "testCollectionRenameAfter";
        
        // put scratch collection into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + beforeRename);
        String afterTargetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                        IRODS_TEST_SUBDIR_PATH + '/' + afterRename);


        ImkdirCommand imkdirCommand = new ImkdirCommand();
        imkdirCommand.setCollectionName(targetIrodsCollection);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(imkdirCommand);
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
        IRODSFile irodsFileAfter = new IRODSFile(irodsFileSystem, afterTargetIrodsCollection);
        irodsFile.renameTo(irodsFileAfter);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFileAfter.getAbsolutePath());
        assertionHelper.assertIrodsFileOrCollectionDoesNotExist(irodsFile.getAbsolutePath());

    }
	
	/**
	 *  Bug 86 -  renameTo does not rename file
	 * @throws Exception
	 */
	@Test
    public final void testFileRenameIsPhysicalMove() throws Exception {
    	// generate a local scratch file
        String testFileName = "testFileRenameIsPhysicalMove.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

        StringBuilder fileNameAndPath = new StringBuilder();
        fileNameAndPath.append(absPath);

        fileNameAndPath.append(testFileName);
        iputCommand.setLocalFileName(fileNameAndPath.toString());
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        IRODSFile irodsFileAfter = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        irodsFileAfter.setResource(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        irodsFile.renameTo(irodsFileAfter);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFileAfter.getAbsolutePath());
        IlsCommand ilsCommand = new IlsCommand();
        ilsCommand.setLongFormat(true);
        ilsCommand.setIlsBasePath(targetIrodsCollection + '/' + testFileName);
        String ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        TestCase.assertTrue("file is not in new resource", ilsResult.indexOf(irodsFileAfter.resource) != -1);

    }

}
