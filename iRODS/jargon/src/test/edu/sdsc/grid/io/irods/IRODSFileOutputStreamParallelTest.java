package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.net.URI;

import java.util.Properties;


public class IRODSFileOutputStreamParallelTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileOutputStreamParalellTest";
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
    public final void testParallelFilePut() throws Exception {
        // make up a test file that triggers parallel transfer
    	String testFileName = "testFilePut.csv";
        // make up a test file of 20kb
        String testFileFullPath =  FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + '/', testFileName, (70000 * 1024));
       

        IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

        // point to the local file I just built
        StringBuilder sourceFileName = new StringBuilder();
        sourceFileName.append("file:///");
        sourceFileName.append(testFileFullPath);


        GeneralFile sourceFile = FileFactory.newFile(new URI(
                    sourceFileName.toString()));

        // point to an irods file and put it
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                testFileName);
        IRODSFile destFile = (IRODSFile) FileFactory.newFile(irodsUri);
        irodsFileSystem.commands.put(sourceFile, destFile, false);
        
        assertionHelper.assertIrodsFileOrCollectionExists(IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        
        irodsFileSystem.close();
    }
}
