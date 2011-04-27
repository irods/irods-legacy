package edu.sdsc.grid.io.irods;

import java.util.List;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSCommandsPutTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsCommandsPutTest";
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
	public void testPutNoOverwriteFileNotInIRODS() throws Exception {
		// generate a local scratch file
		String testFileName = "testPut.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		new LocalFileSystem();
		LocalFile localFile = new LocalFile(localFileName);

		// now put the file
		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);

		irodsFileSystem.commands.put(localFile, irodsFile, false);
		irodsFileSystem.close();

		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
	}

	@Test
	public void testPutOverwriteFileNotInIRODS() throws Exception {
		// generate a local scratch file
		String testFileName = "testPutOverwriteFileNotInIRODS.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		new LocalFileSystem();
		LocalFile localFile = new LocalFile(localFileName);

		// now put the file
		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);

		irodsFileSystem.commands.put(localFile, irodsFile, true);
		irodsFileSystem.close();

		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
	}

	@Test
	public void testPutOverwriteFileInIRODS() throws Exception {
		// generate a local scratch file
		String testFileName = "testPutOverwriteFileInIRODS.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		new LocalFileSystem();
		LocalFile localFile = new LocalFile(localFileName);

		// now put the file
		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);
		irodsFile.createNewFile();

		irodsFileSystem.commands.put(localFile, irodsFile, true);
		irodsFileSystem.close();

		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
	}

	@Test(expected = IRODSException.class)
	public void testPutNoOverwriteFileInIRODS() throws Exception {
		// generate a local scratch file
		String testFileName = "testPutNoOverwriteFileInIRODS.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		new LocalFileSystem();
		LocalFile localFile = new LocalFile(localFileName);

		// now put the file
		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);
		irodsFile.createNewFile();

		irodsFileSystem.commands.put(localFile, irodsFile, false);
		irodsFileSystem.close();

		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
	}

	@Test
	public void testCopyToDataObjectWithReroute() throws Exception {

		String useDistribResources = testingProperties
				.getProperty("test.option.distributed.resources");

		if (useDistribResources != null && useDistribResources.equals("true")) {
			// do the test
		} else {
			return;
		}

		// generate a local scratch file
		String testFileName = "testCopyToDataObjectWithReroute.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		new LocalFileSystem();
		LocalFile localFile = new LocalFile(localFileName);

		// now put the file
		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);

		irodsFile
				.setResource(testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY));
		irodsFile.copyFrom(localFile, true, true);
		List<String> rescNames = irodsFile.getAllResourcesForFile();
		irodsFileSystem.close();
		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
		Assert.assertTrue("resc name for file not found", rescNames.size() == 1);
		Assert.assertEquals(
				"resc name should be tertiary resource for this file",
				testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY),
				rescNames.get(0));
	}

}
