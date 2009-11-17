package edu.sdsc.jargon.testutils.icommandinvoke.icommands;

import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;

import junit.framework.Assert;

import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.Properties;


/**
 * Test chksum via icommand interface
 *
 * @author Mike Conway, DICE (www.renci.org)
 * @since
 */
public class IchksumCommandTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IchksumCommandTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        testingProperties = testingPropertiesHelper.getTestProperties();
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

    @Test
    public final void testBuildCommand() {
        fail("Not yet implemented");
    }

    @Test
    public void testPutThenChecksumAFile() throws Exception {
    	fail("Not yet implemented");
        String testFileName = "testQueryForResource.txt";

        // FIXME: finish implementing and test
        
        // generate a file and put into irods
        String fullPathToTestFile = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                    GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH +
                "/", testFileName, 7);

        // get the actual checksum
        long actualChecksum = scratchFileUtils.computeFileCheckSum(testingProperties.getProperty(
                    GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH +
                "/" + testFileName);

        IputCommand iputCommand = new IputCommand();
        iputCommand.setLocalFileName(fullPathToTestFile);
        iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(
                testingProperties, IRODS_TEST_SUBDIR_PATH));
        iputCommand.setForceOverride(true);

        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        // I have put the file, now assert that the file I put has the same checksum as the file I generated
        IchksumCommand chksumCommand = new IchksumCommand();
        chksumCommand.setIrodsFileName(IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
        String chksumResult = invoker.invokeCommandAndGetResultAsString(chksumCommand);
        // FIXME: how to parse result?  add that code to the Ichksum command as a static helper method that returns a long checksum
        
        
        
        
    }
}
