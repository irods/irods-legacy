package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_HOST_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PASSWORD_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PORT_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_ZONE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;

import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.net.URI;

import java.util.Properties;

public class IRODSFileDeleteTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileDeleteTest";
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

    /**
     *  Bug 48 -  NPE on Delete
     * @throws Exception
     */
    @Test
    public void testExistsOnDeletedDirectory() throws Exception {
    	
    	String testNewDir = "testExistsOnDeletedDir";
    	String testNewDirIrodsPath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			testNewDir);
    	String testNewFileName = "testExistsOnDeletedFile.csv";
    	
    	
    	IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
    	IRODSFile newFolder = new IRODSFile(irodsFileSystem, testNewDirIrodsPath);
    	IRODSFile newFile = new IRODSFile(irodsFileSystem, newFolder.getAbsolutePath(), testNewFileName);

    	newFolder.mkdir();
    	TestCase.assertTrue("directory I just created should exist", newFolder.exists());
    	newFile.createNewFile();
    	TestCase.assertTrue("file I just created should exist", newFile.exists());

    	TestCase.assertTrue("could not delete file I just created", newFile.delete());
    	TestCase.assertFalse("file I just deleted should not exist", newFile.exists());

    	TestCase.assertTrue("unable to delete folder I just created", newFolder.delete());
    	TestCase.assertFalse("folder I just deleted should not exist", newFolder.exists());
    	irodsFileSystem.close();

    }
    
}
