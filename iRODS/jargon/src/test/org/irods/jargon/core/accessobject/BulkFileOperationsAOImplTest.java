package org.irods.jargon.core.accessobject;

import java.util.Properties;

import junit.framework.Assert;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.remoteexecute.RemoteExecuteServiceImpl;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class BulkFileOperationsAOImplTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "BulkFileOperationsAOImplTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;
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
		assertionHelper = new AssertionHelper();
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
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		Assert.assertNotNull("null bulkFileOperationsAO from factory",
				bulkFileOperationsAO);
	}

	@Test
	public void testCreateBundleNoOverwriteCollectionExists() throws Exception {
		String tarName = "testCreateBundleNoOverwriteCollectionExists.tar";
		String testSubdir = "testCreateBundleNoOverwriteCollectionExists";
		String bunSubdir = "testCreateBundleNoOverwriteCollectionExistsBunSubdir";
		String fileName = "fileName";
		int count = 200;

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			return;
		}

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();

		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");
		assertionHelper.assertIrodsFileOrCollectionExists(targetBunFileAbsPath);

	}

	@Test(expected = IllegalArgumentException.class)
	public void testCreateBundleNoOverwriteCollectionExistsNullBun()
			throws Exception {

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();

		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(null,
				"target", "");

	}

	@Test(expected = IllegalArgumentException.class)
	public void testCreateBundleNoOverwriteCollectionExistsBlankBun()
			throws Exception {

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods("",
				"target", "");

	}

	@Test(expected = IllegalArgumentException.class)
	public void testCreateBundleNoOverwriteCollectionExistsNullResc()
			throws Exception {

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods("xxxx",
				"target", null);

	}

	@Test(expected = JargonException.class)
	public void testCreateBundleNoOverwriteCollectionDoesNotExist()
			throws Exception {
		String tarName = "testCreateBundleNoOverwriteCollectionDoesNotExist.tar";
		String testSubdir = "testCreateBundleNoOverwriteCollectionDoesNotExist";
		String bunSubdir = "testCreateBundleNoOverwriteCollectionDoesNotExistBunSubdir";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");
	}

	@Test(expected = JargonException.class)
	public void testCreateBundleWhenTarFileAlreadyExists() throws Exception {

		String tarName = "testCreateBundleWhenTarFileAlreadyExists.tar";
		String testSubdir = "testCreateBundleWhenTarFileAlreadyExists";
		String bunSubdir = "testCreateBundleWhenTarFileAlreadyExistsBunSubdir";
		String fileName = "fileName";
		int count = 20;

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		// create the tar file with the same name as the one I will want to
		// create later
		irodsFile = new IRODSFile(irodsFileSystem, targetBunFileAbsPath);
		irodsFile.createNewFile();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();

		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");

	}

	@Test
	public void testCreateBundleWhenTarFileAlreadyExistsForceSpecified()
			throws Exception {

		String tarName = "testCreateBundleWhenTarFileAlreadyExistsForceSpecified.tar";
		String testSubdir = "testCreateBundleWhenTarFileAlreadyExistsForceSpecified";
		String bunSubdir = "testCreateBundleWhenTarFileAlreadyExistsForceSpecifiedBunSubdir";
		String fileName = "fileName";
		int count = 20;

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			return;
		}

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		// create the tar file with the same name as the one I will want to
		// create later
		irodsFile = new IRODSFile(irodsFileSystem, targetBunFileAbsPath);
		irodsFile.createNewFile();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO
				.createABundleFromIrodsFilesAndStoreInIrodsWithForceOption(
						targetBunFileAbsPath, targetIrodsCollection, "");
		assertionHelper.assertIrodsFileOrCollectionExists(targetBunFileAbsPath);
	}

	@Test
	public void testExtractBundleNoOverwriteNoBulk() throws Exception {
		String tarName = "testExtractBundleNoOverwriteNoBulk.tar";
		String testSubdir = "testExtractBundleNoOverwriteNoBulk";
		String bunSubdir = "testExtractBundleNoOverwriteNoBulkBunSubdir";
		String testExtractTargetSubdir = "testExtractBundleNoOverwriteNoBulkExtractTargetCollection";

		String fileName = "fileName.txt";
		int count = 5;

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			return;
		}
		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();

		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");

		// extract the bun file now to a different subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testExtractTargetSubdir);

		bulkFileOperationsAO.extractABundleIntoAnIrodsCollection(
				targetBunFileAbsPath, targetIrodsCollection, "");

		IRODSFile targetColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		IRODSFile sourceColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		// assertionHelper.assertTwoFilesAreEqualByRecursiveTreeComparison(
		// sourceColl, targetColl);

	}

	@Test(expected = JargonException.class)
	public void testExtractBundleNoOverwriteNoBulkWhenTargetCollectionAlreadyExists()
			throws Exception {
		// gets a SYS_COPY_ALREADY_IN_RESC -46000
		String tarName = "testExtractBundleNoOverwriteNoBulkWhenTargetCollectionAlreadyExists.tar";
		String testSubdir = "testExtractBundleNoOverwriteNoBulkWhenTargetCollectionAlreadyExists";
		String bunSubdir = "testExtractBundleNoOverwriteNoBulkWhenTargetCollectionAlreadyExistsBunSubdir";
		String testExtractTargetSubdir = "testExtractBundleNoOverwriteNoBulkWhenTargetCollectionAlreadyExistsTargetCollection";

		String fileName = "fileName.txt";
		int count = 5;

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();

		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");

		// extract the bun file now to a different subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testExtractTargetSubdir);

		bulkFileOperationsAO.extractABundleIntoAnIrodsCollection(
				targetBunFileAbsPath, targetIrodsCollection, "");
		// repeat the same operation, causing an overwrite situation, should get
		// an error
		bulkFileOperationsAO.extractABundleIntoAnIrodsCollection(
				targetBunFileAbsPath, targetIrodsCollection, "");
	}

	@Test
	public void testExtractBundleWithOverwriteNoBulkWhenTargetCollectionAlreadyExists()
			throws Exception {
		String tarName = "testExtractBundleWithOverwriteNoBulkWhenTargetCollectionAlreadyExists.tar";
		String testSubdir = "testExtractBundleWithOverwriteNoBulkWhenTargetCollectionAlreadyExists";
		String bunSubdir = "testExtractBundleWithOverwriteNoBulkWhenTargetCollectionAlreadyExistsBunSubdir";
		String testExtractTargetSubdir = "testExtractBundleWithOverwriteNoBulkWhenTargetCollectionAlreadyExistsTargetCollection";

		String fileName = "fileName.txt";
		int count = 5;

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			return;
		}

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");

		// extract the bun file now to a different subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testExtractTargetSubdir);

		bulkFileOperationsAO.extractABundleIntoAnIrodsCollection(
				targetBunFileAbsPath, targetIrodsCollection, "");
		// repeat the same operation, causing an overwrite situation, should get
		// an error
		bulkFileOperationsAO
				.extractABundleIntoAnIrodsCollectionWithForceOption(
						targetBunFileAbsPath, targetIrodsCollection, "");

		IRODSFile targetColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		IRODSFile sourceColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		// assertionHelper.assertTwoFilesAreEqualByRecursiveTreeComparison(
		// sourceColl, targetColl);
	}

	@Test
	public void testExtractBundleNoOverwriteWithBulk() throws Exception {
		String tarName = "testExtractBundleNoOverwriteWithBulk.tar";
		String testSubdir = "testExtractBundleNoOverwriteWithBulk";
		String bunSubdir = "testExtractBundleNoOverwriteWithBulkBunSubdir";
		String testExtractTargetSubdir = "testExtractBundleNoOverwriteWithBulkTargetCollection";

		String fileName = "fileName.txt";
		int count = 5;

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();
		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			return;
		}
		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, "");

		// extract the bun file now to a different subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testExtractTargetSubdir);

		bulkFileOperationsAO
				.extractABundleIntoAnIrodsCollectionWithBulkOperationOptimization(
						targetBunFileAbsPath, targetIrodsCollection, "");

		IRODSFile targetColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		IRODSFile sourceColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		// assertionHelper.assertTwoFilesAreEqualByRecursiveTreeComparison(
		// sourceColl, targetColl);

	}

	@Test
	public void testExtractBundleNoOverwriteWithBulkSpecifyResource()
			throws Exception {
		String tarName = "testExtractBundleNoOverwriteWithBulkSpecifyResource.tar";
		String testSubdir = "testExtractBundleNoOverwriteWithBulkSpecifyResource";
		String bunSubdir = "testExtractBundleNoOverwriteWithBulkSpecifyResourceBunSubdir";
		String testExtractTargetSubdir = "testExtractBundleNoOverwriteWithBulkSpecifyResourceTargetCollection";
		String testResource = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);

		String fileName = "fileName.txt";
		int count = 5;

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();

		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, testResource);

		// extract the bun file now to a different subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testExtractTargetSubdir);

		IRODSFile extractSubdir = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		extractSubdir.mkdirs();

		bulkFileOperationsAO
				.extractABundleIntoAnIrodsCollectionWithBulkOperationOptimization(
						targetBunFileAbsPath, targetIrodsCollection,
						testResource);

		IRODSFile targetColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		IRODSFile sourceColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		// assertionHelper.assertTwoFilesAreEqualByRecursiveTreeComparison(
		// sourceColl, targetColl);

	}

	/*
	 * the semantics of how 'data not found' is treated in this version of
	 * jargon is different than how it is treated in jargon-core at the
	 * connection level, that makes detection of this error difficult. For the
	 * time being, semantic changes in the interpretation of this condition are
	 * to be avoided.
	 */
	@Ignore
	public void testExtractBundleNoOverwriteWithBulkSpecifyWrongResource()
			throws Exception {
		String tarName = "testExtractBundleNoOverwriteWithBulkSpecifyWrongResource.tar";
		String testSubdir = "testExtractBundleNoOverwriteWithBulkSpecifyWrongResource";
		String bunSubdir = "testExtractBundleNoOverwriteWithBulkSpecifyWrongResourceBunSubdir";
		String testExtractTargetSubdir = "testExtractBundleNoOverwriteWithBulkSpecifyWrongResourceTargetCollection";
		String testResource = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);

		String fileName = "fileName.txt";
		int count = 5;

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFile irodsFile = null;

		String targetBunIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ bunSubdir);
		String targetBunFileAbsPath = targetBunIrodsCollection + "/" + tarName;
		irodsFile = new IRODSFile(irodsFileSystem, targetBunIrodsCollection);
		irodsFile.mkdir();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		irodsFile.mkdir();

		String myTarget = "";

		for (int i = 0; i < count; i++) {
			myTarget = targetIrodsCollection + "/c" + (10000 + i) + fileName;
			irodsFile = new IRODSFile(irodsFileSystem, myTarget);
			irodsFile.createNewFile();
		}

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		BulkFileOperationsAO bulkFileOperationsAO = accessObjectFactory
				.getBulkFileOperationsAO();
		bulkFileOperationsAO.createABundleFromIrodsFilesAndStoreInIrods(
				targetBunFileAbsPath, targetIrodsCollection, testResource);

		// extract the bun file now to a different subdir
		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testExtractTargetSubdir);

		IRODSFile extractSubdir = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		extractSubdir.mkdirs();

		bulkFileOperationsAO
				.extractABundleIntoAnIrodsCollectionWithBulkOperationOptimization(
						targetBunFileAbsPath,
						targetIrodsCollection,
						testingProperties
								.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));

		IRODSFile targetColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + "/"
								+ testSubdir);
		IRODSFile sourceColl = new IRODSFile(irodsFileSystem,
				targetIrodsCollection);

		// assertionHelper.assertTwoFilesAreEqualByRecursiveTreeComparison(
		// sourceColl, targetColl);

	}

}
