package edu.sdsc.grid.io.irods;

import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_HOST_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PASSWORD_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PORT_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_ZONE_KEY;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.fail;

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

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
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
    public final void testFirstQueryResult() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testQueryMetaDataConditionArrayMetaDataSelectArray() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCopyToGeneralFileBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCopyFromGeneralFileBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testChecksumMD5() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testGetPermissions() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCanRead() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCanWrite() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCreateNewFile() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCreateTempFileStringStringGeneralFile() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testDelete() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testDeleteOnExit() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testEqualsObject() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testExists() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testGetCanonicalPath() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testGetPath() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIsAbsolute() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIsDirectory() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIsFile() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIsHidden() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testLastModified() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testList() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testListMetaDataConditionArray() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testMkdir() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testRenameTo() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testToURI() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testGetResource() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testReplicate() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testToURIBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIRODSFileIRODSFileSystemString() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIRODSFileIRODSFileSystemStringString() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIRODSFileIRODSFileString() {
        fail("Not yet implemented");
    }

    @Test
    public final void testGetIRODSFileUsingUriAndSeeIfExistsWorks()
        throws Exception {
        // generate a local scratch file
        String testFileName = "testfileuri.txt";

        StringBuilder pathPlusName = new StringBuilder();
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

        IRODSFile irodsFile = new IRODSFile(new URI(irodsUri.toString()));
    }

    @Ignore
    public final void testGetAvailableResource() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testMakePathCanonical() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testModifyMetaDataStringArray() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testDeleteMetaDataStringArray() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testDeleteMetaData() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testSetResource() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testGetDataType() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testChangePermissions() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCanReadBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCanWriteBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testCreateNewFileBooleanBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testDeleteBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testDeleteReplica() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIsDirectoryBoolean() {
        fail("Not yet implemented");
    }

    @Ignore
    public final void testIsFileBoolean() {
        fail("Not yet implemented");
    }
}
