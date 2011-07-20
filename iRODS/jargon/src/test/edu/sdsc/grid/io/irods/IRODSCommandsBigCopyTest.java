package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.net.URI;
import java.util.Properties;


public class IRODSCommandsBigCopyTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsCommandsBigCopyTest";
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
    public final void testCopyManyFilesFromCollToColl() throws Exception {
    	
        String testFilePrefix = "testCopyManyFromCollToColl";
        String testFileSuffix = ".txt";
        String sourceDir = "source";
        String destDir = "dest";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
       
        FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH + '/' +  sourceDir , testFilePrefix, testFileSuffix, 100, 20, 5000000);
        
        // put scratch files into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
        		IRODS_TEST_SUBDIR_PATH + '/' + sourceDir);
        LocalFile sourceFile = new LocalFile(absPath + sourceDir);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection);
        fileToPut.copyFrom(sourceFile, true);

        // i've put all the files, now do a copy
        URI irodsSourceUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + sourceDir);
        URI irodsDestUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + destDir);

        IRODSFile irodsSourceFile = new IRODSFile(irodsSourceUri);
        IRODSFile irodsDestFile = new IRODSFile(irodsDestUri);
        

        // just say overwrite
        irodsSourceFile.copyTo(irodsDestFile, true);

        GeneralFile localDest = FileFactory.newFile(testingPropertiesHelper.buildUriFromTestPropertiesForFileInLocalScratchDir(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + destDir));

        // copy the collection in irods back to the dest dir
        localDest.copyFrom(irodsDestFile, true);
       
        irodsFileSystem.close();

		assertionHelper.assertLocalDirectoriesHaveSameData(absPath + destDir, absPath + sourceDir);

    }

   
}