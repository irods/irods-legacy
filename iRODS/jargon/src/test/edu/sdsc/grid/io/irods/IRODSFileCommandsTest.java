package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralRandomAccessFile;

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

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.URI;

import java.util.Arrays;
import java.util.Properties;

public class IRODSFileCommandsTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileCommandsTest";
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
    public final void testFileOpen()
        throws Exception {
        // generate a local scratch file
        String testFileName = "testfileopen.txt";
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
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        TestCase.assertTrue("did not return a valid descriptor", fileDescriptor > 0);
        irodsFileSystem.commands.fileClose(fileDescriptor);
        irodsFileSystem.close();
        
    }
    
    @Test
    public final void testFileRead() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfileread.txt";
        String readbackFileName = "testfilereadback.txt";
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
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        
        String readbackFileAbsPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH) + readbackFileName;
        FileOutputStream fos = new FileOutputStream(readbackFileAbsPath);
        
        irodsFileSystem.commands.fileRead(fileDescriptor, fos, 1000);
        irodsFileSystem.commands.fileClose(fileDescriptor);
        irodsFileSystem.close();
        
        assertionHelper.assertLocalFileExistsInScratch(IRODS_TEST_SUBDIR_PATH + '/' + readbackFileName);
   
    }
    
    /**
     *  BUG: 40 -  IRODSCommands.fileRead() with length of 0 causes null message from irods 
     * @throws Exception
     */
    @Test
    public final void testFileReadWithLenghtOfZeroCausesNullPointerException() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfileread.txt";
        String readbackFileName = "testfilereadback.txt";
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
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        
        String readbackFileAbsPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH) + readbackFileName;
        FileOutputStream fos = new FileOutputStream(readbackFileAbsPath);
        
        irodsFileSystem.commands.fileRead(fileDescriptor, fos, 0);
        irodsFileSystem.commands.fileClose(fileDescriptor);
        irodsFileSystem.close();
        
        assertionHelper.assertLocalFileExistsInScratch(IRODS_TEST_SUBDIR_PATH + '/' + readbackFileName);
   
    }
    
    @Test
    public final void testFileReadIntoBuffer() throws Exception {
    	// generate a local scratch file
    	int fileSizeKb = 2;
        String testFileName = "testfilereadbuffer.txt";
        String readbackFileName = "testfilereadbufferback.txt";

        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String inputFileName = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            fileSizeKb);

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
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        byte[] buff = new byte[fileSizeKb * 1024];
        String readbackFileAbsPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH) + readbackFileName;
        
        BufferedInputStream fis = new BufferedInputStream(new FileInputStream(inputFileName));
        byte[] inputBytes = new byte[fileSizeKb * 1024];
        fis.read(inputBytes);
        fis.close();
        
        irodsFileSystem.commands.fileRead(fileDescriptor, buff, 0, fileSizeKb * 1024);
        irodsFileSystem.commands.fileClose(fileDescriptor);
        irodsFileSystem.close();
        
        TestCase.assertTrue("did not get same buffer back", Arrays.equals(inputBytes, buff));
    }
    
    @Test
    public final void testFileReadIntoBufferUsingOffsetToReadTwice() throws Exception {
    	// generate a local scratch file
    	int fileSizeKb = 2;
        String testFileName = "testfilereadbuffertwice.txt";
        String readbackFileName = "testfilereadbuffertwice_readback.txt";

        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String inputFileName = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            fileSizeKb);

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
        
        // I've put the file on IRODS, retrieve it back, using the 'offset' feature of the read to bring it back in two chunks
        // accumulate these in a byte arroy output stream and then compare what I got back to the original file
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        byte[] buff = new byte[1024];
        byte[] buff2 = new byte[1024];

        String readbackFileAbsPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH) + readbackFileName;
        
        
        // here I'm saving the source file as a byte array as my 'expected' value for my test assertion
        BufferedInputStream fis = new BufferedInputStream(new FileInputStream(inputFileName));
        byte[] inputBytes = new byte[fileSizeKb * 1024];
        fis.read(inputBytes);
        fis.close();
        
        
        ByteArrayOutputStream actualReadByteStream = new ByteArrayOutputStream(); 
        // here I'm doing the actual operation using IRODSbyte[] accumBuff = new byte[fileSizeKb * 1024];
        irodsFileSystem.commands.fileRead(fileDescriptor, buff, 0, 1024);
        actualReadByteStream.write(buff);
        irodsFileSystem.commands.fileRead(fileDescriptor, buff2, 1024, 1024);
        actualReadByteStream.write(buff2);
        
        irodsFileSystem.commands.fileClose(fileDescriptor);
        irodsFileSystem.close();
        
        TestCase.assertTrue("did not get same buffer back", Arrays.equals(inputBytes, actualReadByteStream.toByteArray()));
    }
    
    
    @Test
    public final void testFileSeek() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfileseek.txt";
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
        
        // now try to do the seek
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
          
        irodsFileSystem.commands.fileSeek(fileDescriptor, 1024, GeneralRandomAccessFile.SEEK_CURRENT);
        irodsFileSystem.commands.fileSeek(fileDescriptor, 1024, GeneralRandomAccessFile.SEEK_END);
        irodsFileSystem.commands.fileSeek(fileDescriptor, 1024, GeneralRandomAccessFile.SEEK_START);
        
        irodsFileSystem.commands.fileClose(fileDescriptor);
        irodsFileSystem.close();
        
    }
    
}
