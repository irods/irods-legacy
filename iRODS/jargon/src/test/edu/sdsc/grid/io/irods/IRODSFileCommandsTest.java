package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_GROUP_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

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
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
				testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        Assert.assertTrue("did not return a valid descriptor", fileDescriptor > 0);
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
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

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
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

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
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

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
        
        Assert.assertTrue("did not get same buffer back", Arrays.equals(inputBytes, buff));
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
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

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
        
        Assert.assertTrue("did not get same buffer back", Arrays.equals(inputBytes, actualReadByteStream.toByteArray()));
    }
    
    
    @Test
    public final void testFileSeek() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfileseek.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

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
    
    @Test(expected=Exception.class)
    public final void testFileSeeInvalidFd() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfileseek.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

        // now try to do the seek
        
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fileDescriptor = irodsFileSystem.commands.fileOpen(irodsFile, true, true);
        irodsFileSystem.commands.fileSeek(999, 1024, GeneralRandomAccessFile.SEEK_CURRENT);
       
    }
    
    @Test
    public final void testFileWriteWithStream() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfilewritestream.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileUri = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);
        FileInputStream fis = new FileInputStream(fileUri);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        int fd = irodsFileSystem.commands.fileCreate(irodsFile, true, true);
        irodsFileSystem.commands.fileWrite(fd, fis, 2);
        
        assertionHelper.assertIrodsFileOrCollectionExists(targetIrodsCollection + '/' + testFileName);
        irodsFileSystem.close();
    }
    
    @Test
    public final void testFileRename() throws Exception {
    	// generate a local scratch file
        String testFileName = "testfilerenameBefore.txt";
        String renameFileName = "testfilerenameAfter.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
			.buildIRODSCollectionAbsolutePathFromTestProperties(
					testingProperties, IRODS_TEST_SUBDIR_PATH);
        LocalFile sourceFile = new LocalFile(absPath + testFileName);

        IRODSFile fileToPut = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        fileToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        IRODSFile irodsFileAfter = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + renameFileName);
        irodsFileSystem.commands.renameFile(irodsFile, irodsFileAfter);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFileAfter.getAbsolutePath());
        
   
    }
    
    @Test
    public final void testCollectionRename() throws Exception {
    	// generate a local scratch file
        String beforeRename = "beforerename";
        String afterRename = "afterrename";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH  + "/" + beforeRename);
        
        String targetIrodsCollection = testingPropertiesHelper
    		.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
    				IRODS_TEST_SUBDIR_PATH + '/' + beforeRename);
        String afterTargetIrodsCollection = testingPropertiesHelper
        	.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
        			IRODS_TEST_SUBDIR_PATH + '/' + afterRename);
        
        // put scratch collection into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        LocalFile sourceFile = new LocalFile(absPath);

        IRODSFile dirToPut = new IRODSFile(irodsFileSystem1, targetIrodsCollection);
        dirToPut.copyFrom(sourceFile, true);
        irodsFileSystem1.close();

        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection);
        IRODSFile irodsFileAfter = new IRODSFile(irodsFileSystem, afterTargetIrodsCollection);
        irodsFileSystem.commands.renameDirectory(irodsFile, irodsFileAfter);
        irodsFileSystem.close();
        
        assertionHelper.assertIrodsFileOrCollectionExists(irodsFileAfter.getAbsolutePath());
        
   
    }
    
    // currently ignoring prior to 2.3.0 due to known iRODS bug
    @Test
    public final void testPhysicalMove() throws Exception {
    	// generate a local scratch file
        String testFileName = "testPmBeforex.txt";
        String otherFileName = "testPmAfterx.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOrig = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);
        // create a put a second file in the target resource, this is a bug documented in the IRODSFileTest class in testSetResourceShowQueryBug()
        String absPathOther = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOther = FileGenerator.generateFileOfFixedLengthGivenName(absPath, otherFileName,
            2);

        // put scratch file into irods in the right place
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        String targetIrodsCollection = testingPropertiesHelper
        	.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        
        LocalFile sourceFile1 = new LocalFile(fileNameOrig);
        IRODSFile fileToPut1 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        fileToPut1.copyFrom(sourceFile1, true);
        
        LocalFile sourceFile2 = new LocalFile(fileNameOther);
        IRODSFile fileToPut2 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + otherFileName);
        fileToPut2.setResource(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        fileToPut2.copyFrom(sourceFile2, true);
        
        irodsFileSystem1.close();
    	/*
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IputCommand iputCommand = new IputCommand();

        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);

      
        iputCommand.setLocalFileName(fileNameOrig);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        iputCommand = new IputCommand();
        iputCommand.setLocalFileName(fileNameOther);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setIrodsResource(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        iputCommand.setForceOverride(true);
        invoker.invokeCommandAndGetResultAsString(iputCommand);
        */      
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        if (irodsFileSystem.getCommands().getIrodsServerProperties().getRelVersion().compareTo("rods2.3") <= 0) {
        		return;
        }
        	
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        IRODSFile irodsFileAfter = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        irodsFileAfter.setResource(testingProperties.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));
        irodsFileSystem.commands.physicalMove(irodsFile, irodsFileAfter);
        
        List<String> resources = irodsFile.getAllResourcesForFile();
        irodsFileSystem.close();
        
        Assert.assertTrue("file is not in new resource", resources.indexOf(irodsFileAfter.resource) != -1);
        
        /*
        IlsCommand ilsCommand = new IlsCommand();
        ilsCommand.setLongFormat(true);
        ilsCommand.setIlsBasePath(targetIrodsCollection + '/' + testFileName);
        String ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        Assert.assertTrue("file is not in new resource", ilsResult.indexOf(irodsFileAfter.resource) != -1);
        */
    }
    
    @Test
    public final void testReplicate() throws Exception {
    	// generate a local scratch file
    	
        String testFileName = "testReplicate1.txt";
        String otherFileName = "testReplicate2.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOrig = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);
        // create a put a second file in the target resource, this is a bug documented in teh IRODSFileTest class in testSetResourceShowQueryBug()
        String absPathOther = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOther = FileGenerator.generateFileOfFixedLengthGivenName(absPathOther, otherFileName,
            2);
        
        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        
        // make sure all replicas are removed
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        IRODSFile fileToRemove = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        try{
        	irodsFileSystem1.commands.deleteFile(fileToRemove, true);
        }
        catch(IOException ex) {
        	// okay - do nothing
        }

        // put scratch file into irods in the right place
        LocalFile sourceFile1 = new LocalFile(fileNameOrig);
        IRODSFile fileToPut1 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        fileToPut1.copyFrom(sourceFile1, true);
        
        LocalFile sourceFile2 = new LocalFile(fileNameOther);
        IRODSFile fileToPut2 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + otherFileName);
        fileToPut2.setResource(testingProperties.getProperty(IRODS_RESOURCE_KEY));
        fileToPut2.copyFrom(sourceFile2, true);
        /*
        IputCommand iputCommand = new IputCommand();

        iputCommand.setLocalFileName(fileNameOrig);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);

        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        iputCommand = new IputCommand();

        iputCommand.setLocalFileName(fileNameOrig);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setIrodsResource(testingProperties.getProperty(IRODS_RESOURCE_KEY));
        iputCommand.setForceOverride(true);

        invoker.invokeCommandAndGetResultAsString(iputCommand);
        */
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        irodsFileSystem.commands.replicate(irodsFile, testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        
        List<String> resources = irodsFile.getAllResourcesForFile();
        irodsFileSystem.close();
        Assert.assertTrue("file is not in new resource", resources.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
        Assert.assertTrue("file is not in original resource", resources.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) != -1);
        /*
        IlsCommand ilsCommand = new IlsCommand();
        ilsCommand.setLongFormat(true);
        ilsCommand.setIlsBasePath(targetIrodsCollection + '/' + testFileName);
        String ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        Assert.assertTrue("file is not in new resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
        Assert.assertTrue("file is not in original resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) != -1);
        */
    }
    
    /*
     * May fail if the resource group is not set up
     * 
     */
    @Test
    public final void testReplicateToResourceGroup() throws Exception {
    	// generate a local scratch file
    	
        String testFileName = "testReplicateToResourceGroup.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOrig = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);
       
        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        
        // make sure all replicas are removed
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        IRODSFile fileToRemove = new IRODSFile(irodsFileSystem1,
    		targetIrodsCollection + "/" + testFileName);
        try{
        	irodsFileSystem1.commands.deleteFile(fileToRemove, true);
        }
        catch(IOException ex) {
        	// okay - do nothing
        }

        // put scratch file into irods in the right place
        LocalFile sourceFile1 = new LocalFile(fileNameOrig);
        IRODSFile fileToPut1 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        fileToPut1.copyFrom(sourceFile1, true);
        
        LocalFile sourceFile2 = new LocalFile(fileNameOrig);
        IRODSFile fileToPut2 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        fileToPut2.setResource(testingProperties.getProperty(IRODS_RESOURCE_KEY));
        fileToPut2.copyFrom(sourceFile2, true);
        /*
        IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
        IcommandInvoker invoker = new IcommandInvoker(invocationContext);
        
        IputCommand iputCommand = new IputCommand();

        iputCommand.setLocalFileName(fileNameOrig);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setForceOverride(true);

        invoker.invokeCommandAndGetResultAsString(iputCommand);
        
        iputCommand = new IputCommand();

        iputCommand.setLocalFileName(fileNameOrig);
        iputCommand.setIrodsFileName(targetIrodsCollection);
        iputCommand.setIrodsResource(testingProperties.getProperty(IRODS_RESOURCE_KEY));
        iputCommand.setForceOverride(true);

        invoker.invokeCommandAndGetResultAsString(iputCommand);
        */
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        irodsFileSystem.commands.replicateToResourceGroup(irodsFile, testingProperties.getProperty(IRODS_RESOURCE_GROUP_KEY));

        List<String> resources = irodsFile.getAllResourcesForFile();
        irodsFileSystem.close();
        
        Assert.assertTrue("file is not in new resource", resources.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
        Assert.assertTrue("file is not in original resource", resources.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) != -1);
        /*
        IlsCommand ilsCommand = new IlsCommand();
        ilsCommand.setLongFormat(true);
        ilsCommand.setIlsBasePath(targetIrodsCollection + '/' + testFileName);
        String ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        Assert.assertTrue("file is not in new resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
        Assert.assertTrue("file is not in original resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) != -1);
        */

       
    }
    
    @Test
    public final void testDeleteReplica() throws Exception {
    	// generate a local scratch file
        String testFileName = "testDelReplicate1.txt";
        String otherFileName = "testDelReplicate2.txt";
        String absPath = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOrig = FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
            2);
        // create a put a second file in the target resource, this is a bug documented in the IRODSFileTest class in testSetResourceShowQueryBug()
        String absPathOther = scratchFileUtils.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
        String fileNameOther = FileGenerator.generateFileOfFixedLengthGivenName(absPath, otherFileName,
            2);

        // put scratch file into irods in the right place
        String targetIrodsCollection = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties,
                IRODS_TEST_SUBDIR_PATH);
        IRODSAccount testAccount = testingPropertiesHelper
			.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem1 = new IRODSFileSystem(testAccount);
        
        LocalFile sourceFile1 = new LocalFile(fileNameOrig);
        IRODSFile fileToPut1 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + testFileName);
        fileToPut1.copyFrom(sourceFile1, true);
        
        LocalFile sourceFile2 = new LocalFile(fileNameOther);
        IRODSFile fileToPut2 = new IRODSFile(irodsFileSystem1,
        		targetIrodsCollection + "/" + otherFileName);
        fileToPut2.setResource(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        fileToPut2.copyFrom(sourceFile2, true);

        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties));
        IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/' + testFileName);
        irodsFileSystem.commands.replicate(irodsFile, testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
        
        List<String> resources = irodsFile.getAllResourcesForFile();
        Assert.assertTrue("file is not in new resource", resources.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
        Assert.assertTrue("file is not in original resource", resources.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) != -1);
        
        /*
        IlsCommand ilsCommand = new IlsCommand();
        ilsCommand.setLongFormat(true);
        ilsCommand.setIlsBasePath(targetIrodsCollection + '/' + testFileName);
        String ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        Assert.assertTrue("file is not in new resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
        Assert.assertTrue("file is not in original resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) != -1);
        */
        // now delete the replica from the first resource
        
        irodsFileSystem.commands.deleteReplica(irodsFile,testingProperties.getProperty(IRODS_RESOURCE_KEY));
        irodsFileSystem.close();
        
        // NOTE: not deleting!  Need to set min replicas to 1
        
        // replica should not show up in ils, it was deleted..
        /*ilsCommand = new IlsCommand();
        ilsCommand.setLongFormat(true);
        ilsCommand.setIlsBasePath(targetIrodsCollection + '/' + testFileName);
        ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
        TestCase.assertTrue("file is still in original resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_RESOURCE_KEY)) == -1);
        TestCase.assertTrue("file is not in new resource", ilsResult.indexOf(testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY)) != -1);
   */
    } 
    
}
