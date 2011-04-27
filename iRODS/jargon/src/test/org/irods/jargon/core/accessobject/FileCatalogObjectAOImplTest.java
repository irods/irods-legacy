package org.irods.jargon.core.accessobject;

import java.net.URI;
import java.util.Properties;

import junit.framework.Assert;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.remoteexecute.RemoteExecuteServiceImpl;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class FileCatalogObjectAOImplTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "FileCatalogObjectAOImplTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static IRODSFileSystem irodsFileSystem;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils
				.clearAndReinitializeScratchDirectory(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		irodsFileSystem = new IRODSFileSystem(irodsAccount);
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
		irodsFileSystem.close();
	}

	@Test
	public void testGetAOFromFactory() throws Exception {
		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		FileCatalogObjectAO dataObjectAO = accessObjectFactory.getFileCatalogObjectAO();
		Assert.assertNotNull("null dataObjectAO from factory", dataObjectAO);
	}

	@Test
	public void testGetHostForGetProvidingResourceNameWhenShouldBeSameHost()
			throws Exception {

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
		String testFileName = "testGetHostForGetProvidingResourceName.txt";
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
		new IRODSFile(irodsUri);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		FileCatalogObjectAO dataObjectAO = accessObjectFactory.getFileCatalogObjectAO();

		String hostInfo = dataObjectAO
				.getHostForGetOperation(
						targetIrodsCollection + "/" + testFileName,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));
		irodsFileSystem.close();
		Assert.assertNotNull("null info from lookup of host for get operation",
				hostInfo);
		Assert.assertEquals("should have resolved this to the same host",
				FileCatalogObjectAOImpl.USE_THIS_ADDRESS, hostInfo);

	}

	@Test
	public void testGetHostForGetProvidingResourceNameWhenShouldDifferentResource()
			throws Exception {

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
		String testFileName = "testGetHostForGetProvidingResourceNameWhenShouldDifferentResource.txt";
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

		// can I use jargon to access the file on IRODS and verify that it
		// indeed exists?
		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		new IRODSFile(irodsUri);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		FileCatalogObjectAO dataObjectAO = accessObjectFactory.getFileCatalogObjectAO();

		String hostInfo = dataObjectAO
				.getHostForGetOperation(
						targetIrodsCollection + "/" + testFileName,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY));
		irodsFileSystem.close();
		Assert.assertNotNull("null info from lookup of host for get operation",
				hostInfo);
		Assert.assertFalse("should have resolved this to the same host",
				FileCatalogObjectAOImpl.USE_THIS_ADDRESS.equals(hostInfo));

	}
	
	@Test
	public void testGetHostForGetCollectionProvidingResourceNameWhenShouldDifferentResource()
			throws Exception {

		String useDistribResources = testingProperties
				.getProperty("test.option.distributed.resources");

		if (useDistribResources != null && useDistribResources.equals("true")) {
			// do the test
		} else {
			return;
		}
		
		String testSubdir = "testGetHostSubdir";
		String testFilePrefix = "testGetFile";
		String testFileSuffix = ".doc";
		
		String localAbsPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH + "/" + testSubdir);
		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH + "/" + testSubdir, testFilePrefix, testFileSuffix, 20, 10, 20);

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
		
		// put scratch file into irods in the right place on the first resource
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

	
		iputCommand.setLocalFileName(localAbsPath.substring(0, localAbsPath.length() - 1));
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand
				.setIrodsResource(testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY));
		iputCommand.setRecursive(true);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		FileCatalogObjectAO fileCatalogObjectAO = accessObjectFactory.getFileCatalogObjectAO();

		String hostInfo = fileCatalogObjectAO
				.getHostForGetOperation(
						targetIrodsCollection + "/" + testSubdir,
						"");
		irodsFileSystem.close();
		Assert.assertNotNull("null info from lookup of host for get operation",
				hostInfo);
		Assert.assertFalse("should have resolved this to the same host",
				FileCatalogObjectAOImpl.USE_THIS_ADDRESS.equals(hostInfo));

	}
	
	@Test
	public void testGetHostForPutProvidingResourceNameWhenShouldBeSameHost()
			throws Exception {

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
		String testFileName = "testGetHostForGetProvidingResourceName.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		new IRODSFile(irodsUri);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		FileCatalogObjectAO fileCatalogObjectAO = accessObjectFactory.getFileCatalogObjectAO();

		String hostInfo = fileCatalogObjectAO
				.getHostForPutOperation(
						targetIrodsCollection + "/" + testFileName,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));
		irodsFileSystem.close();
		Assert.assertNotNull("null info from lookup of host for get operation",
				hostInfo);
		Assert.assertEquals("should have resolved this to the same host",
				FileCatalogObjectAOImpl.USE_THIS_ADDRESS, hostInfo);

	}

}
