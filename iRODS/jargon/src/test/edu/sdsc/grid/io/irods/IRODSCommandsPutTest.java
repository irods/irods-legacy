package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
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


    @Test
    public void testPutNoOverwriteFileNotInIRODS() throws Exception {
    	// generate a local scratch file
        String testFileName = "testPut.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String localFileName = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);
        
        LocalFileSystem localFileSystem = new LocalFileSystem();
        LocalFile localFile = new LocalFile(localFileName);
        
        // now put the file
        String targetIrodsFile = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
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
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String localFileName = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);
        
        LocalFileSystem localFileSystem = new LocalFileSystem();
        LocalFile localFile = new LocalFile(localFileName);
        
        // now put the file
        String targetIrodsFile = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
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
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String localFileName = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);
        
        LocalFileSystem localFileSystem = new LocalFileSystem();
        LocalFile localFile = new LocalFile(localFileName);
        
        // now put the file
        String targetIrodsFile = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);
        irodsFile.createNewFile();
        
        irodsFileSystem.commands.put(localFile, irodsFile, true);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
    }
    
    @Test(expected=IRODSException.class)
    public void testPutNoOverwriteFileInIRODS() throws Exception {
    	// generate a local scratch file
        String testFileName = "testPutNoOverwriteFileInIRODS.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String localFileName = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);
        
        LocalFileSystem localFileSystem = new LocalFileSystem();
        LocalFile localFile = new LocalFile(localFileName);
        
        // now put the file
        String targetIrodsFile = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);
        irodsFile.createNewFile();
        
        irodsFileSystem.commands.put(localFile, irodsFile, false);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsFile);
    }
    
}
