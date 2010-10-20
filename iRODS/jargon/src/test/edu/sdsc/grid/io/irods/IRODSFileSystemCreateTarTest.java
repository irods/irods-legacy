package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
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
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;


public class IRODSFileSystemCreateTarTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileSystemCreateTarTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;


    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
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

    /**
     * Create a tar, bug testcase
     * BUG: 34 -  [iROD-Chat:2969] Jargon: createTarFile
     * @throws Exception
     */
    @Test
    public void testCreateTar() throws Exception {

    	// test tuning variables
		String testFileNamePrefix = "tar5file";
		String testFileExtension = ".txt";
		String collectionSubdir = IRODS_TEST_SUBDIR_PATH + "/tar5dir";
		int numberOfTestFiles = 5;
		String newTarAbsPath = testingPropertiesHelper
		.buildIRODSCollectionAbsolutePathFromTestProperties(
				testingProperties, IRODS_TEST_SUBDIR_PATH);


		String irodsNewTarFileName = "tar5NewTar.tar";

		// create collection to tar
		String collectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, collectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(collectionAbsPath);
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
			iputCommand.setIrodsFileName(collectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and tar
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile newTarFile = new IRODSFile(irodsFileSystem, newTarAbsPath + '/' + irodsNewTarFileName);
		IRODSFile irodsCollectionToTar = new IRODSFile(irodsFileSystem, collectionAbsPath);
		Assert.assertTrue(irodsCollectionToTar.exists());
		Assert.assertTrue(irodsCollectionToTar.isDirectory());

		irodsFileSystem.createTarFile(newTarFile, irodsCollectionToTar, irodsFileSystem.getDefaultStorageResource());
		irodsFileSystem.close();
		assertionHelper.assertIrodsFileOrCollectionExists(newTarAbsPath + '/' + irodsNewTarFileName);


    }
}
