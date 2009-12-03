package edu.sdsc.grid.io.irods;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import org.junit.After;
import org.junit.AfterClass;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.net.URI;

import java.util.Properties;

public class IRODSCommandsDeleteBigTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsCommandsDeleteBigTest";
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


	/**
	 * BUG: 29 - problem with Jargon physically deleting a file Testing delete
	 * code in collection to trigger status report
	 *
	 * @throws Exception
	 */
	@Test
	public void testDeleteMultipleFilesInCollectionWithVerboseStatusTriggerDialog()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "multiplefile";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/deleteme";
		int numberOfTestFiles = 300;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and delete the collecton
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, deleteCollectionSubdir));

		irodsFile.delete(true);
		irodsFileSystem.close();

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
}
