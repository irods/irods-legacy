package edu.sdsc.grid.io.irods;

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


public class IRODSCommandsCopyToTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsCommandsCopyToTest";
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
     * This turned out to be a requirement that a default resource must be specified in core.irb like so:
     * acSetRescSchemeForCreate||msiSetDefaultResc(test1-resc,preferred)|nop
	 * if not defined, this method will fail with a -78000
     * BUG: 32
     * @throws Exception
     */
    @Test
    public final void testCopySourceByURIDestByURI() throws Exception {
    	
        String testFileName = "testCopySourceByURI.txt";
        String testCopyToFileName = "testCopySourceByURICopyTo.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
	
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testFileName);

        // can I use jargon to access the file on IRODS and verify that it indeed exists?
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsFile = new IRODSFile(irodsUri);

        uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testCopyToFileName);

        URI irodsCopyToUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsCopyToFile = new IRODSFile(irodsCopyToUri);

        irodsFile.copyTo(irodsCopyToFile, true);
        

        // see that the file I just copied to exists in irods
        String irodsFullPath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, uriPath.toString());
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFullPath);

    }

    @Test
    public final void testCopySourceByURIDestByAccount() throws Exception {
    	// generate a local scratch file
        String testFileName = "testCopySourceByURI.txt";
        String testCopyToFileName = "testCopySourceByURICopyTo.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
	
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testFileName);

        // can I use jargon to access the file on IRODS and verify that it indeed exists?
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsFile = new IRODSFile(irodsUri);

        uriPath = new StringBuilder();
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testCopyToFileName);

        IRODSFile irodsCopyToFile = new IRODSFile(irodsFileSystem, uriPath.toString());
        irodsFile.copyTo(irodsCopyToFile, true);

        // see that the file I just copied to exists in irods
        assertionHelper.assertIrodsFileOrCollectionExists(uriPath.toString());
        irodsFileSystem.close();

    }
    
    

    /**
     * Will fail if core.irb does not define a valid default resource in 
     * acSetRescSchemeForCreate||msiSetDefaultResc(test1-resc,preferred)|nop
     * BUG: 32
     * @throws Exception
     */
    @Test
    public final void testCopySourceFileFromAccountDestFromURI() throws Exception {
    	//fail -78000
    	// generate a local scratch file
        String testFileName = "testCopySourceFileFromAccount.pdf";
        String testCopyToFileName = "testCopySourceFileFromAccountCopyTo.pdf";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
	
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testFileName);

        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, uriPath.toString());

        uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testCopyToFileName);

        URI irodsCopyToUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsCopyToFile = new IRODSFile(irodsCopyToUri);

        irodsFile.copyTo(irodsCopyToFile, true);

        // see that the file I just copied to exists in irods
        String irodsFullPath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, uriPath.toString());
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFullPath);
        irodsFileSystem.close();

    }

    /**
     * @throws Exception
     */
    @Test
    public final void testCopySourceFileFromAccountDestFromAccount() throws Exception {
    	// generate a local scratch file
        String testFileName = "SourceFileFromAccountDestFromAccount.pdf";
        String testCopyToFileName = "SourceFileFromAccountDestFromAccountCopyTo.pdf";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
	
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem, targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testFileName);

        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, uriPath.toString());

        uriPath = new StringBuilder();
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testCopyToFileName);


        IRODSFile irodsCopyToFile = new IRODSFile(irodsFileSystem, uriPath.toString());

        irodsFile.copyTo(irodsCopyToFile, true);

        // see that the file I just copied to exists in irods
        assertionHelper.assertIrodsFileOrCollectionExists(uriPath.toString());
        irodsFileSystem.close();

    }

}
