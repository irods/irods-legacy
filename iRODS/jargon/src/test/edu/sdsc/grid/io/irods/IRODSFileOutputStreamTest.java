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

import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.net.URI;

import java.util.Properties;


public class IRODSFileOutputStreamTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileOutputStreamTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
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

    @Ignore
    public final void testWriteByteArrayIntInt() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testClose() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testOpen() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIRODSFileOutputStreamIRODSFileSystemString() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIRODSFileOutputStreamIRODSFile() {
        fail("Not yet implemented");
    }

    
    
    /**
     * This may be intermittent, based on the state of the local irods ENV.  When the URI is created for the
     * Dest file, it may be null, in which case the put may fail with a -78000 due to no resource being found.
     * @throws Exception
     * BUG: 31
     */
    @Test
    public final void testFilePutCreateDestFileByUri() throws Exception {
    	String testFileName = "testFilePut.csv";
        // make up a test file of 20kb
        String testFileFullPath =  FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + '/', testFileName, 17);
        IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

        // point to the local file I just built
         StringBuilder sourceFileName = new StringBuilder();
        sourceFileName.append("file:///");
        sourceFileName.append(testFileFullPath);

        GeneralFile sourceFile = FileFactory.newFile(new URI(
                    sourceFileName.toString()));

        // point to an irods file and put it
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSFile destFile = (IRODSFile) FileFactory.newFile(irodsUri);

        irodsFileSystem.commands.put(sourceFile, destFile, true);
        
        AssertionHelper assertionHelper = new AssertionHelper();
        assertionHelper.assertIrodsFileOrCollectionExists(destFile.getAbsolutePath());
        irodsFileSystem.close();
    }

    
    
    /**
     * TODO: Was not working locally on original install of IRODS, may be an error due to running on 
     * VirtualBox (ports).  Need to re-test
     * @throws Exception
     */
    @Ignore
    public final void testParallelFilePut() throws Exception {
        // make up a test file that triggers parallel transfer
        String testFileName = FileGenerator.generateFileOfFixedLength(testingProperties.getProperty(
                    GENERATED_FILE_DIRECTORY_KEY), (70000 * 1024));
        System.out.println("generating test file:" + testFileName);

        IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

        // point to the local file I just built
        StringBuilder sourceFileName = new StringBuilder();
        sourceFileName.append("file:///");
        sourceFileName.append(testingProperties.getProperty(
                GENERATED_FILE_DIRECTORY_KEY));
        sourceFileName.append(LocalFile.PATH_SEPARATOR_CHAR);
        sourceFileName.append(testFileName);

        System.out.println("generated file name:" + testFileName);

        GeneralFile sourceFile = FileFactory.newFile(new URI(
                    sourceFileName.toString()));

        // point to an irods file and put it
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                testFileName);
        IRODSFile destFile = (IRODSFile) FileFactory.newFile(irodsUri);
        irodsFileSystem.commands.put(sourceFile, destFile, false);
        
        // FIXME: add assert check of file in-place
        
        irodsFileSystem.close();
    }
}
