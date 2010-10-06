package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;

import java.net.URI;
import java.util.Properties;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;


public class IRODSFileInputStreamParallelTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileInputStreamParalellTest";
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
     * Was not working locally on original install of IRODS, may be an error due to running on 
     * VirtualBox (ports).  This test is running when local to the server
     * @throws Exception
     */
    @Test
    public final void testParallelFileGet() throws Exception {
        // make up a test file that triggers parallel transfer
    	String testFileName = "testFileGet.csv";
    	String getFileName = testingProperties.getProperty(
                GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + '/' +"testFileGetReturn.csv";
    	long testFileLength = 70000 * 1024;

        String testFileFullPath =  FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + '/', testFileName, testFileLength);
        
        // put this test file into irods so I can 'get' it.
        // put scratch file into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

        iputCommand.setLocalFileName(testFileFullPath);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
       // Thread.sleep(3000);
       
        IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

        String irodsFileName = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + testFileName);        IRODSFile getFile = new IRODSFile(irodsFileSystem, irodsFileName);
        GeneralFile localFile = FileFactory.newFile(new URI("file:///" + getFileName));
        
        irodsFileSystem.commands.get(getFile, localFile);        
        irodsFileSystem.close();
        assertionHelper.assertLocalScratchFileLengthEquals(IRODS_TEST_SUBDIR_PATH + '/' +"testFileGetReturn.csv", testFileLength);
    }
}
