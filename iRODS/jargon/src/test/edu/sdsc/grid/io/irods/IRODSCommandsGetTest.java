package edu.sdsc.grid.io.irods;

import java.net.URI;
import java.util.Properties;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.remoteexecute.RemoteExecuteServiceImpl;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class IRODSCommandsGetTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsCommandsGetTest";
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
	public final void testGetSpecifyingResource() throws Exception {
		// generate a local scratch file
		String testFileName = "testGetSpecifyingResource.txt";
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

		irodsFileSystem.commands.get(irodsFile, localFile, testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));

		irodsFileSystem.close();

		assertionHelper.assertLocalFileExistsInScratch(IRODS_TEST_SUBDIR_PATH
				+ "/" + "GetResult" + testFileName);

	}

	@Test
	public final void testGet() throws Exception {
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

		irodsFileSystem.commands.get(irodsFile, localFile);

		irodsFileSystem.close();

		assertionHelper.assertLocalFileExistsInScratch(IRODS_TEST_SUBDIR_PATH
				+ "/" + "GetResult" + testFileName);

	}

	@Test
	public final void testGetSpecifyingDifferentResource() throws Exception {
		// generate a local scratch file
		String testFileName = "testGetSpecifyingDifferentResource.txt";
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

		irodsFileSystem.commands
				.get(irodsFile,
						localFile,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));

		irodsFileSystem.close();

		assertionHelper
				.assertLocalFileNotExistsInScratch(IRODS_TEST_SUBDIR_PATH + "/"
						+ "GetResult" + testFileName);

	}

	@Test
	public void testGetDataObjectWithConnectionRerouting() throws Exception {

		String useDistribResources = testingProperties
				.getProperty("test.option.distributed.resources");

		if (useDistribResources != null && useDistribResources.equals("true")) {
			// do the test
		} else {
			return;
		}

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();

		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			irodsFileSystem.close();
			return;
		}

		// generate a local scratch file
		String testFileName = "testGetHDataObjectWithConnectionRerouting.txt";
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
		iputCommand
				.setIrodsResource(testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);

		String getTargetFilePath = absPath + "GetResult" + testFileName;
		GeneralFile localFile = new LocalFile(getTargetFilePath);
		irodsFile
				.copyTo(localFile,
						true,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY),
						true);

		irodsFileSystem.close();

	}

}
