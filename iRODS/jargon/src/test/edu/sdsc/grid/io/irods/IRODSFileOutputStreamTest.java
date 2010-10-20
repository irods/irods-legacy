package edu.sdsc.grid.io.irods;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class IRODSFileOutputStreamTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileOutputStreamTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
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
	public final void testWriteByteArrayIntInt() throws Exception {
		String testFileName = "testFileWriteByteArray.csv";
		String testIRODSFileName = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFileSystem, testIRODSFileName);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);

		irodsFileOutputStream.open(irodsFile);
		Assert
				.assertTrue("file I created does not exist", irodsFile.exists());
		// get a simple byte array
		String myBytes = "ajjjjjjjjjjjjjjjjjjjjjjjjfeiiiiiiiiiiiiiii54454545";
		byte[] myBytesArray = myBytes.getBytes();
		irodsFileOutputStream.write(myBytesArray);
		irodsFileOutputStream.close();
		long length = irodsFile.length();

		Assert.assertEquals("file length does not match bytes written",
				myBytesArray.length, length);

		irodsFileSystem.close();
	}

	@Test
	public final void testOpen() throws Exception {
		String testFileName = "testFileOpen.csv";
		String testIRODSFileName = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFileSystem, testIRODSFileName);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);

		irodsFileOutputStream.open(irodsFile);
		Assert
				.assertTrue("file I created does not exist", irodsFile.exists());
		irodsFileOutputStream.close();
		irodsFileSystem.close();
	}

	@Test
	public final void testIRODSFileOutputStreamIRODSFileSystemString()
			throws Exception {
		String testFileName = "testFilePut.csv";
		String testIRODSFileName = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);
		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		irodsFileSystem.close();

	}

	@Test
	public final void testIRODSFileOutputStreamIRODSFile() throws Exception {
		String testFileName = "testFilePut.csv";
		String testIRODSFileName = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);
		Assert.assertNotNull("did not create fileOutputStream",
				irodsFileOutputStream);

		irodsFileSystem.close();
	}

	@Test
	public final void testWriteToIRODSFileOutputStream() throws Exception {
		String testFileName = "testFilePut.csv";
		String testIRODSFileName = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String sourceFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 8);

		File fileToWrite = new File(sourceFileName);
		FileInputStream fin;
		byte[] buff = new byte[1024];
		// Get the size of the file
		long length = fileToWrite.length();

		// Open an input stream
		fin = new FileInputStream(fileToWrite);

		// Read in the bytes
		int offset = 0;
		int writeOffset = 0;
		int numRead = 0;
		while ((offset < buff.length)
				&& ((numRead = fin.read(buff, offset, buff.length - offset)) >= 0)) {

			offset += numRead;
			irodsFileOutputStream.write(buff, writeOffset, numRead);
			writeOffset += numRead;
		}
		// Close our input stream
		fin.close();

		irodsFileOutputStream.close();
		irodsFileSystem.close();
	}

	/**
	 * Test for Bug 60 - overwrite for file output stream Test is successful if
	 * no errors happen, bug resulted in an NPE
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testWriteToIRODSFileOutputStreamWhenFileExists()
			throws Exception {
		// generate a local scratch file
		String testFileName = "testWriteToIRODSFileOutputStreamWhenFileExists.txt";
		int fileLengthInKb = 4;
		long fileLengthInBytes = fileLengthInKb * 1024;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String sourceFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName,
						fileLengthInBytes);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now open an output stream and try to write to the same file

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		Assert.assertTrue("i cannot write an output stream", irodsFile
				.canWrite());

		File fileToWrite = new File(sourceFileName);
		FileInputStream fin;
		byte[] buff = new byte[1024];
		// Get the size of the file
		long length = fileToWrite.length();

		// Open an input stream
		fin = new FileInputStream(fileToWrite);

		// Read in the bytes
		int offset = 0;
		int writeOffset = 0;
		int numRead = 0;
		while ((offset < buff.length)
				&& ((numRead = fin.read(buff, offset, buff.length - offset)) >= 0)) {

			offset += numRead;
			irodsFileOutputStream.write(buff, writeOffset, numRead);
			writeOffset += numRead;
		}
		// Close our input stream
		fin.close();

		irodsFileOutputStream.close();
		irodsFileSystem.close();
	}

	/**
	 * Test for Bug 60 - overwrite for file output stream Test is successful if
	 * no errors happen, bug resulted in an NPE
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testWriteToIRODSFileOutputStreamOverwriteDifferentData()
			throws Exception {
		// generate a local scratch file
		String testFileName = "testWriteToIRODSFileOutputStreamOverwriteDifferentData.txt";

		long fileLength = 2048;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String sourceFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName,
						fileLength);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now open an output stream and try to overwrite the same file

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		Assert.assertTrue("i cannot write an output stream", irodsFile
				.canWrite());

		String myBytes = "ajjjjjjjjjjjjjjjjjjjf94949fjg94fj9jfasdofalkdfjfkdfjksdfjsiejfesifslas;efias;efiadfkadfdffjjjjjfeiiiiiiiiiiiiiii54454545";
		byte[] myBytesArray = myBytes.getBytes();
		int byteArraySize = myBytesArray.length;

		irodsFileOutputStream.write(myBytesArray, 0, myBytesArray.length);
		irodsFileOutputStream.close();

		// read back the file into scratch and inspect the first n bytes to see
		// if it matches my byte array that I overwrote
		IRODSFile getBackJoJo = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream irodsFileInputStream = new IRODSFileInputStream(
				getBackJoJo);
		byte[] readBackBytes = new byte[byteArraySize];
		irodsFileInputStream.read(readBackBytes);
		boolean areEqual = Arrays.equals(myBytesArray, readBackBytes);
		irodsFileSystem.close();
		Assert.assertTrue("did not overwrite and read back my bytes",
				areEqual);

	}

	@Test(expected = IOException.class)
	public final void testWriteToIRODSFileOutputStreamWhenFileExistsAndICannotWrite()
			throws Exception {
		// generate a local scratch file
		String testFileName = "testWriteToIRODSFileOutputStreamWhenFileExistsAndICannotWrite.txt";
		int fileLengthInKb = 4;
		long fileLengthInBytes = fileLengthInKb * 1024;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String sourceFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName,
						fileLengthInBytes);

		// use the jargon code to create and put the file, since my 'logged in'
		// user cannot thru iCommands...

		IRODSAccount user2Account = testingPropertiesHelper
				.buildIRODSAccountFromSecondaryTestProperties(testingProperties);
		IRODSFileSystem user2FileSystem = new IRODSFileSystem(user2Account);
		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSFile user2PutFile = new IRODSFile(user2FileSystem, targetIrodsFile);
		LocalFile localFile = new LocalFile(sourceFileName);
		user2PutFile.copyFrom(localFile, true);
		user2FileSystem.close();

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		// now open an output stream and try to write to the same file from a
		// different user (I should not be able to write)

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, targetIrodsFile);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		Assert
				.assertFalse(
						"i should not be able to write an output stream to a file I do not own",
						irodsFile.canWrite());

		File fileToWrite = new File(sourceFileName);
		FileInputStream fin;
		byte[] buff = new byte[1024];
		// Get the size of the file
		long length = fileToWrite.length();

		// Open an input stream
		fin = new FileInputStream(fileToWrite);

		// Read in the bytes
		int offset = 0;
		int writeOffset = 0;
		int numRead = 0;
		while ((offset < buff.length)
				&& ((numRead = fin.read(buff, offset, buff.length - offset)) >= 0)) {

			offset += numRead;
			irodsFileOutputStream.write(buff, writeOffset, numRead);
			writeOffset += numRead;
		}
		// Close our input stream
		fin.close();

		irodsFileOutputStream.close();
		irodsFileSystem.close();
	}

	/**
	 * Test for Bug 60 - overwrite for file output stream Test is successful if
	 * no errors happen, bug resulted in an NPE
	 * 
	 * This is the exact updated test case
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testWriteToIRODSFileOutputStreamOverwriteDifferentDataTestForNPE()
			throws Exception {
		String testFileName = "testWriteToIRODSFileOutputStreamOverwriteDifferentDataTestForNPE.txt";
		String expectedAttribName = "testattrib1";
		String expectedAttribValue = "testvalue1";

		// 1. Created an empty IRODSFile.
		// now open an output stream and try to overwrite the same file

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		irodsFileSystem.commands.fileCreate(irodsFile, false, true);

		// 2. Added AVUs to the empty IRODSFile just created.

		String[] metaData = { expectedAttribName, expectedAttribValue };
		irodsFile.modifyMetaData(metaData);

		/*
		 * 3. Instantiated an IRODSOutputStream using the IRODSFile just
		 * created. - since the IRODSFile had already been created,
		 * IRODSFileOutputStream would open the file instead of creating the
		 * file
		 */

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		Assert.assertTrue("i cannot write an output stream", irodsFile
				.canWrite());

		String myBytes = "ajjjjjjjjjjjjjjjjjjjf94949fjg94fj9jfasdofalkdfjfkdfjksdfjsiejfesifslas;efias;efiadfkadfdffjjjjjfeiiiiiiiiiiiiiii54454545";
		byte[] myBytesArray = myBytes.getBytes();
		int byteArraySize = myBytesArray.length;

		irodsFileOutputStream.write(myBytesArray, 0, myBytesArray.length);
		irodsFileOutputStream.close();

		irodsFileSystem.close();

	}

	@Test
	public void testBasicWriteFromCommands() throws Exception {
		String testFileName = "testBasicWriteFromCommands.doc";

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		irodsFile.createNewFile();
		Assert.assertTrue("i cannot write an output stream", irodsFile
				.canWrite());

		Assert
				.assertTrue("file I created does not exist", irodsFile.exists());

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);
		// get a simple byte array
		String myBytes = "ajjjjjjjjjjjjjjjjjjjjjjjjfeiiiiiiiiiiiiiii54454545";
		byte[] myBytesArray = myBytes.getBytes();
		irodsFileOutputStream.write(myBytesArray, 0, myBytesArray.length);

		irodsFile.close();		
		irodsFileSystem.close();
		assertionHelper.assertIrodsFileOrCollectionExists(irodsFile.getAbsolutePath());
	}

	/**
	 * Bug 72 - IRODSFile.canRead() returns false unexpectedly
	 */
	@Test
	public void testCanRead() throws Exception {

		String testFileName = "testCanRead.png";

		String sourceAbsPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String sourceFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(sourceAbsPath,
						testFileName, 8);

		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile destination = new IRODSFile(irodsFileSystem, targetIrodsFile);

		IRODSFileOutputStream out = new IRODSFileOutputStream(destination);

		File fileToWrite = new File(sourceFileName);
		FileInputStream fin;
		byte[] buff = new byte[1024];
		// Get the size of the file
		long length = fileToWrite.length();

		// Open an input stream
		fin = new FileInputStream(fileToWrite);

		// Read in the bytes
		int offset = 0;
		int writeOffset = 0;
		int numRead = 0;
		while ((offset < buff.length)
				&& ((numRead = fin.read(buff, offset, buff.length - offset)) >= 0)) {

			offset += numRead;
			out.write(buff, writeOffset, numRead);
			writeOffset += numRead;
		}
		// Close our input stream
		fin.close();

		boolean canRead = destination.canRead();
		Assert
				.assertTrue(
						"The newly copied file should be readable by the person who just put it.",
						canRead);

		IRODSFile destination2 = new IRODSFile(irodsFileSystem, targetIrodsFile);
		boolean canRead2 = destination2.canRead();
		Assert
				.assertTrue(
						"A new file object should be readable by the person who just put the underlying file.",
						canRead2);
		irodsFileSystem.close();

	}
}
