package edu.sdsc.grid.io.irods;

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
     * BUG: 32
     * @throws Exception
     */
    @Ignore
    public final void testCopySourceByURIDestByURI() throws Exception {
    	//FIXME: fail -78000 when creating a file via uri...resource does not exist
    	//BUG: 32    	copyTo when dest file is from URI results in -78000 resource not found
    	// generate a local scratch file
        String testFileName = "testCopySourceByURI.txt";
        String testCopyToFileName = "testCopySourceByURICopyTo.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);

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

        uriPath = new StringBuilder();
        uriPath.append(IRODS_TEST_SUBDIR_PATH);
        uriPath.append('/');
        uriPath.append(testCopyToFileName);

        URI irodsCopyToUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
        IRODSFile irodsCopyToFile = new IRODSFile(irodsCopyToUri);

        irodsFile.copyTo(irodsCopyToFile, true);

        // see that the file I just copied to exists in irods
        assertionHelper.assertIrodsFileOrCollectionExists(uriPath.toString());

    }

    @Test
    public final void testCopySourceByURIDestByAccount() throws Exception {
    	// generate a local scratch file
        String testFileName = "testCopySourceByURI.txt";
        String testCopyToFileName = "testCopySourceByURICopyTo.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);

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

        uriPath = new StringBuilder();
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testCopyToFileName);

        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

        IRODSFile irodsCopyToFile = new IRODSFile(irodsFileSystem, uriPath.toString());
        irodsFile.copyTo(irodsCopyToFile, true);

        // see that the file I just copied to exists in irods
        assertionHelper.assertIrodsFileOrCollectionExists(uriPath.toString());
        irodsFileSystem.close();

    }


    /**
     * BUG: 32
     * @throws Exception
     */
    @Ignore
    public final void testCopySourceFileFromAccountDestFromURI() throws Exception {
    	//FIXME: fail -78000
    	// generate a local scratch file
        String testFileName = "testCopySourceFileFromAccount.pdf";
        String testCopyToFileName = "testCopySourceFileFromAccountCopyTo.pdf";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);

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
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testFileName);

        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
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
        assertionHelper.assertIrodsFileOrCollectionExists(uriPath.toString());
        irodsFileSystem.close();

    }

    /**
     * BUG: 32    	copyTo when dest file is from URI results in -78000 resource not found
     * @throws Exception
     */
    @Test
    public final void testCopySourceFileFromAccountDestFromAccount() throws Exception {
    	// generate a local scratch file
        String testFileName = "SourceFileFromAccountDestFromAccount.pdf";
        String testCopyToFileName = "SourceFileFromAccountDestFromAccountCopyTo.pdf";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            1);

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
        uriPath.append(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH));
        uriPath.append('/');
        uriPath.append(testFileName);

        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
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
