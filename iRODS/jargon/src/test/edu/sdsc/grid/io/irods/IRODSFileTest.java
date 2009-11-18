package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;

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

public class IRODSFileTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileTest";
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
    public final void testGetIRODSFileUsingUriAndSeeIfExistsWorks()
        throws Exception {
        // generate a local scratch file
        String testFileName = "testfileuri.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

        StringBuilder fileNameAndPath = new StringBuilder();
        fileNameAndPath.append(absPath);

        fileNameAndPath.append(testFileName);

        iputCommand.setLocalFileName(fileNameAndPath.toString());
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testFileName);

        // can I use jargon to access the file on IRODS and verify that it indeed exists?
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsFile = new IRODSFile(irodsUri);
        TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
    }
    
    /**
     * BUG: 24
     **/
    @Test
    public final void testGetResourceOnExistingFileAndVerify()
        throws Exception {
        // generate a local scratch file
        String testFileName = "testGetResourceOnExistingFileAndVerify.xsl";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

        StringBuilder fileNameAndPath = new StringBuilder();
        fileNameAndPath.append(absPath);

        fileNameAndPath.append(testFileName);

        iputCommand.setLocalFileName(fileNameAndPath.toString());
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testFileName);

        // can I use jargon to access the file on IRODS and verify that it indeed exists?
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsFile = new IRODSFile(irodsUri);
        TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
        String actualResource = irodsFile.getResource();
        TestCase.assertEquals("I should have gotten the default resource", testingProperties.getProperty(IRODS_RESOURCE_KEY), actualResource);
    }
    
    /**
     * BUG: 24 - Is this appropriate behavior to query for physical resource when collection?  Will give null if 
     * the IRODSFile in question is a collection, and not a file.
     * Get the default resource for this collection
     * @throws Exception
     */
    @Test
    public final void testGetResourceOnExistingCollectionAndVerify()
        throws Exception {
       
        // can I use jargon to access the collection on IRODS and verify that it indeed exists?
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
        		IRODS_TEST_SUBDIR_PATH);
        IRODSFile irodsFile = new IRODSFile(irodsUri);
        TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
        TestCase.assertTrue("testing file is not a collection", irodsFile.isDirectory());
        // should query for resource in IRODSFile
        String actualResource = irodsFile.getResource();
        TestCase.assertEquals("I should have gotten the default resource", null, actualResource);
        // FLAG: note on this test that it should, for consistency, have null in resource
    }
    
    @Test(expected = IRODSException.class)
    public final void testIRODSFileURIInvalidUser() throws Exception {
        StringBuilder irodsUri = new StringBuilder();

        irodsUri.append("irods://");
        irodsUri.append("iminvalid");
        irodsUri.append(".");
        irodsUri.append(testingProperties.getProperty(IRODS_ZONE_KEY));
        irodsUri.append(":");
        irodsUri.append(testingProperties.getProperty(IRODS_PASSWORD_KEY));
        irodsUri.append("@");
        irodsUri.append(testingProperties.getProperty(IRODS_HOST_KEY));
        irodsUri.append(":");
        irodsUri.append(String.valueOf(testingProperties.getProperty(
                    IRODS_PORT_KEY)));
        irodsUri.append("/");
        irodsUri.append("fwd.txt");

        @SuppressWarnings("unused")
        IRODSFile irodsFile = new IRODSFile(new URI(irodsUri.toString()));
    }
    
    
    /**
     * Put a file to a specific resource, should get that resource for the file in IRODSFile
     * BUG: 24
     * @throws Exception
     */
    @Test
    public final void testPutFileInResc2AndCheckIfRightDefault() throws Exception {
    	 // generate a local scratch file
        String testFileName = "testPutFileInResc2AndCheckIfRightDefault.xsl";
        String testFileFullPath = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                    GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH +
                '/', testFileName, 4);
    
        // put scratch file into irods in the right place
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

        iputCommand.setLocalFileName(testFileFullPath);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setIrodsResource(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        iputCommand.setForceOverride(true);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);

        StringBuilder uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testFileName);

        // can I use jargon to access the file on IRODS and verify that it indeed exists?
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsFile = new IRODSFile(irodsUri);
        TestCase.assertTrue("testing file does not exist!", irodsFile.exists());
        String actualResource = irodsFile.getResource();
        TestCase.assertEquals("I should have gotten the specific resource I used for the iput", testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY), actualResource);
  
    }

    /**
     * This may be intermittent, based on the state of the local irods ENV.  When the URI is created for the
     * Dest file, it may be null, in which case the put may fail with a -78000 due to no resource being found.
     * @throws Exception
     * BUG: 31
     */
    @Ignore
    public final void testFilePutCreateDestFileByUri()
        throws Exception {
        String testFileName = "testFilePut.csv";

        // make up a test file of 20kb
        String testFileFullPath = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                    GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH +
                '/', testFileName, 17);
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
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSFile destFile = (IRODSFile) FileFactory.newFile(irodsUri);

        irodsFileSystem.commands.put(sourceFile, destFile, true);

        AssertionHelper assertionHelper = new AssertionHelper();
        assertionHelper.assertIrodsFileOrCollectionExists(destFile.getAbsolutePath());
        irodsFileSystem.close();
    }

    @Test
    public final void testFileGetByUri() throws Exception {
        // generate a file and put it in irods
        String testFileName = "testFileGet.doc";
        String returnTestFileName = "testFileGetCopy.doc";
        int expectedLength = 15;
        String testFileFullPath = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                    GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH +
                '/', testFileName, expectedLength);
        IputCommand iputCommand = new IputCommand();

        iputCommand.setLocalFileName(testFileFullPath);
        iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(
                testingProperties, IRODS_TEST_SUBDIR_PATH));
        iputCommand.setForceOverride(true);

        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);

        // now try and create the file by uri and get it
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSFile getFile = (IRODSFile) FileFactory.newFile(irodsUri);

        TestCase.assertTrue("file I put does not exist", getFile.exists());

        String fullPathToReboundFile = "file:///" +
            testingProperties.getProperty(GENERATED_FILE_DIRECTORY_KEY) +
            IRODS_TEST_SUBDIR_PATH + '/' + returnTestFileName;

        GeneralFile reboundFile = FileFactory.newFile(new URI(
                    fullPathToReboundFile));
        getFile.copyTo(reboundFile, true);
        assertionHelper.assertLocalScratchFileLengthEquals(IRODS_TEST_SUBDIR_PATH +
            '/' + returnTestFileName, expectedLength);
    }
    
    @Test
    public final void testFileExistsIsFalseWhenNew() throws Exception {
        String testFileName = "IDontExist.doc";
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSFile testFile = (IRODSFile) FileFactory.newFile(irodsUri);
        TestCase.assertFalse("I shouldnt exist, I am new", testFile.exists());
    }
    
    @Test
    public final void testDirExistsIsFalseWhenNew() throws Exception {
        String testFileName = "IDontExistPath";
        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        IRODSFile testFile = (IRODSFile) FileFactory.newFile(irodsUri);
        TestCase.assertFalse("I shouldnt exist, I am new", testFile.exists());
    }
}
