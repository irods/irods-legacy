package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

public class IRODSCommandsMiscTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSCommandsMiscTest";
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
	public void testMiscSverInfo() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		String miscSvrInfo = irodsFileSystem.commands.miscServerInfo();
		irodsFileSystem.close();
		Assert.assertTrue("no svr info returned", miscSvrInfo.length() > 0);
	}

	@Test
	public void testChmod() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		// generate a local scratch file
		String testFileName = "testChmodCommand.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		// put scratch file into irods in the right place on the first resource
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		irodsFileSystem.commands.chmod(irodsFile, "read", testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_USER_KEY),
				irodsFileSystem.getZone(), false);

		// just testing for now errors at the moment, better tests later on
		irodsFileSystem.close();
	}

	@Test
	public void testRenameFile() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		// generate a local scratch file
		String testFileName = "testRenameOriginal.txt";
		String testNewFileName = "testRenameNew.txt";

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		// put scratch file into irods in the right place on the first resource
		String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
		LocalFile sourceFile = new LocalFile(absPath + testFileName);

		IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
		fileToPut.copyFrom(sourceFile, true);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFile newIrodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testNewFileName);

		irodsFileSystem.commands.renameFile(irodsFile, newIrodsFile);

		irodsFileSystem.close();

		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsCollection
				+ '/' + testNewFileName);

	}

	@Test
	public void testRenameDirectory() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		// generate a local scratch file
		String testOrigDirectory = "origdirectory";
		String testNewDirectory = "newdirectory";

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		// put scratch file into irods in the right place on the first resource
		/*
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		ImkdirCommand mkdirCommand = new ImkdirCommand();
		mkdirCommand.setCollectionName(targetIrodsCollection + '/'
				+ testOrigDirectory);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(mkdirCommand);
		*/

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testOrigDirectory);
		
		// create the directory this way instead of using icommands		
		irodsFileSystem.commands.mkdir(irodsFile, false);

		IRODSFile newIrodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testNewDirectory);

		irodsFileSystem.commands.renameDirectory(irodsFile, newIrodsFile);

		irodsFileSystem.close();

		assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsCollection
				+ '/' + testNewDirectory);

	}

	@Ignore
	// possible error in IRODS, invoking this gets a stack trace in irods log
	public void testPhysicalMove() throws Exception {
		// only checking for errors right now, needs a better test
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount testAccount2 = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		testAccount2
				.setDefaultStorageResource(testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSFileSystem irodsFileSystem2 = new IRODSFileSystem(testAccount2);

		// generate a local scratch file
		String testFileName = "testPhysMoveOriginal.txt";

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String scratchFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		// put scratch file into irods in the right place on the first resource
		String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
		LocalFile sourceFile = new LocalFile(scratchFileName);

		IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
		fileToPut.copyFrom(sourceFile, true);
    	/*
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(scratchFileName);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		String putResult = invoker
				.invokeCommandAndGetResultAsString(iputCommand);
		*/

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		Assert.assertTrue("irods orig file does not exist", irodsFile
				.exists());
		Assert.assertFalse("irods orig file is not a file, but a directory",
				irodsFile.isDirectory());

		IRODSFile newIrodsFile = new IRODSFile(irodsFileSystem2,
				targetIrodsCollection + '/' + testFileName);

		irodsFileSystem.commands.physicalMove(irodsFile, newIrodsFile);

		irodsFileSystem.close();
		irodsFileSystem2.close();

	}

	@Test
	public void testChecksum() throws Exception {
		// generate a local scratch file
		String testFileName = "testChecksum.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		//LocalFileSystem localFileSystem = new LocalFileSystem();
		//LocalFile localFile = new LocalFile(localFileName);

		// put scratch file into irods in the right place on the first resource
		IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
		LocalFile sourceFile = new LocalFile(localFileName);

		IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
		fileToPut.copyFrom(sourceFile, true);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		String checksum = irodsFileSystem.commands.checksum(irodsFile);
		irodsFileSystem.close();

		Assert.assertTrue("checksum not generated", checksum.length() > -1);
	}

	

}
