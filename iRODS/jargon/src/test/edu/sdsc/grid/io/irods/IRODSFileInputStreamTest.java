package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.io.ByteArrayOutputStream;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class IRODSFileInputStreamTest {

	  private static Properties testingProperties = new Properties();
	    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	    private static ScratchFileUtils scratchFileUtils = null;
	    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileInputStreamTest";
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

	@Ignore
	public final void testRead() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testReadByteArrayIntInt() {
		fail("Not yet implemented");
	}

	@Test
	public final void testSkip() throws Exception {
		// generate a local scratch file
        String testFileName = "testfileskip.txt";
        int fileLengthInKb = 2;
        long fileLengthInBytes = fileLengthInKb * 1024;
        
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            fileLengthInBytes);

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
        
        ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();
        
        // now try to do the seek
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        
        IRODSFileInputStream fis = new IRODSFileInputStream(irodsFile);
        long skipped = fis.skip(1024L);
        
        // may have skipped a different value?
        
        long leftToRead = (fileLengthInBytes - skipped);
        long numberBytesReadAfterSkip = 0L;
        
        // read the rest
        
        int readBytes;
        byte[] readBytesBuffer = new byte[512];
		while((readBytes = (fis.read(readBytesBuffer, 0, readBytesBuffer.length))) > -1) {
        	actualFileContents.write(readBytes);
        	numberBytesReadAfterSkip += readBytes;
        }
		
        irodsFileSystem.close();       
		TestCase.assertEquals("I did not skip and then read the remainder of the specified file", fileLengthInBytes, skipped + numberBytesReadAfterSkip);
	}

}
