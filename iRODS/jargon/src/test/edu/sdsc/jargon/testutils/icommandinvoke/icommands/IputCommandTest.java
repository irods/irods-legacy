/**
 *
 */
package edu.sdsc.jargon.testutils.icommandinvoke.icommands;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandException;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;

/**
 * @author Mike Conway, DICE (www.irods.org)
 *
 */
public class IputCommandTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    public static final String IPUT_COMMAND_TEST_PATH = "IputCommandTest";
    private static ScratchFileUtils scratchFileUtils = null;
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
    

    
    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        testingProperties = testingPropertiesHelper.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IPUT_COMMAND_TEST_PATH);
    }

    /**
     * @throws java.lang.Exception
     */
    @AfterClass
    public static void tearDownAfterClass() throws Exception {
    }

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     * Test method for {@link org.irods.jargon.icommandinvoke.icommands.IputCommand#buildCommand()}.
     */
    @Test(expected = IcommandException.class)
    public void testNoLocalFile() throws Exception {
        IputCommand iputCommand = new IputCommand();
        iputCommand.buildCommand();
    }

    @Test
    public void testExecuteCommand() throws Exception {
    	// 	no exception = passed
        String testFileName = "testIputExecution.txt";
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);

        // generate testing file
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IPUT_COMMAND_TEST_PATH);
        String absPathToFile = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            20);

        IputCommand iputCommand = new IputCommand();

        iputCommand.setLocalFileName(absPathToFile);
        iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IPUT_COMMAND_TEST_PATH ));
        //iputCommand.setIrodsFileName("test-scratch/IputCommandTest");
        
        iputCommand.setForceOverride(true);

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
    }

    @Test
    public void testIputWithCollection() throws Exception {
        String testFileName = "testIputWithCollection.txt";
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        
        // generate testing file and get absolute path
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IPUT_COMMAND_TEST_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            20);

        StringBuilder localFileName = new StringBuilder();
        localFileName.append(absPath);
        localFileName.append(testFileName);
        
        String actualCollectionPath = testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(testingProperties, IPUT_COMMAND_TEST_PATH);

        // now put the file
        IputCommand iputCommand = new IputCommand();
        iputCommand.setIrodsFileName(actualCollectionPath);
        iputCommand.setForceOverride(true);
        iputCommand.setLocalFileName(localFileName.toString());

        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        // now check if file exists in irods
        IlsCommand ilsCommand = new IlsCommand();
        ilsCommand.setIlsBasePath(actualCollectionPath);

        String res = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        TestCase.assertTrue("did not find file I just put",
            res.indexOf(testFileName) > -1);
    }

    /**
     * Expect a -317000 USER_INPUT_PATH_ERR because the local file is not found
     * @throws Exception
     */
    @Test(expected = IcommandException.class)
    public void testExecuteNonExistantLocalFile() throws Exception {
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);

        IputCommand iputCommand = new IputCommand();
        iputCommand.setLocalFileName("c:/temp/bogusbogus.txt");

        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);

    }
    
        
}
