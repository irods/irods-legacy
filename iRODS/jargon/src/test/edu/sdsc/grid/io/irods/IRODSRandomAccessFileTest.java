package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.RandomAccessFile;
import java.util.Arrays;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;
// FIXME: enhance tests and add to suite
public class IRODSRandomAccessFileTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSRandomAccessFileTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
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
	public final void testRead() throws Exception {
		// generate a local scratch file
		String testFileName = "testfileseek.txt";
		int fileLengthInKb = 2;
		long fileLengthInBytes = fileLengthInKb * 1024;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String inputFileName = FileGenerator
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

		// read back the test file so I can compare

		// here I'm saving the source file as a byte array as my 'expected'
		// value for my test assertion
		BufferedInputStream fis = new BufferedInputStream(new FileInputStream(
				inputFileName));
		byte[] inputBytes = new byte[1024];
		fis.read(inputBytes);
		fis.close();

		// now try to do the seek

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSRandomAccessFile randomAccessFile = new IRODSRandomAccessFile(
				irodsFile, "rw");

		char readData = (char) randomAccessFile.read();
		char expectedReadData = (char) inputBytes[0];

		irodsFileSystem.close();
		Assert.assertEquals(
				"byte I read does not match the first byte I wrote",
				expectedReadData, readData);

	}

	@Ignore
	public final void testReadBytes() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testWriteBytesByteArrayIntInt() {
		fail("Not yet implemented");
	}

	@Test
	public final void testSeekLongInt() throws Exception {
		// generate a local scratch file
		String testFileName = "testfileseek.txt";
		int fileLengthInKb = 2;
		long fileLengthInBytes = fileLengthInKb * 1024;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String inputFileName = FileGenerator
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

		// here I'm saving the source file as a byte array as my 'expected'
		// value for my test assertion
		BufferedInputStream fis = new BufferedInputStream(new FileInputStream(
				inputFileName));
		byte[] inputBytes = new byte[1024];
		fis.read(inputBytes);
		fis.close();

		// now try to do the seek

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSRandomAccessFile randomAccessFile = new IRODSRandomAccessFile(
				irodsFile, "rw");
		randomAccessFile.seek(200L);
		byte[] bytesToRead = new byte[20];
		randomAccessFile.read(bytesToRead);
		byte[] expectedBytes = new byte[20];
		System.arraycopy(inputBytes, 200, expectedBytes, 0, 20);

		irodsFileSystem.close();

		Assert.assertTrue(
				"did not seek and read the same data that I originally wrote",
				Arrays.equals(expectedBytes, bytesToRead));

	}

	/**
	 * Bug 45 - SYS_UNMATCHED_API_NUM (-12000) when attempting to get a file
	 * 
	 * @throws Exception
	 */
	@Test
	public void testUnmatchedAPIWhenReadingRAFile() throws Exception {

		String testFileName = "testfileForApi.txt";
		int fileLength = 2;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String inputFileName = FileGenerator
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

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		GeneralFile file = FileFactory.newFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);
		Assert.assertTrue("file does not exist in IRODS, setup issue", file
				.exists());
		Assert.assertTrue("file is not a file in IRODS, setup issue", file
				.isFile());

		GeneralRandomAccessFile raFile = FileFactory.newRandomAccessFile(file,
				"r");

		int nbytes = 0;
		int offset = 0;
		byte data[] = new byte[4096];
		boolean dataRead = false;

		while ((nbytes = raFile.read(data, offset, 4096)) > 0) {
			offset += nbytes;
			dataRead = true;
		}

		Assert.assertTrue("did not read back any data", dataRead);
	}

	/**
	 * Bug 90 - error in checksum for paused transfer
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testTransferPauseAndThenChecksum() throws Exception {
		String testFileName = "testTransferPauseAndThenChecksum.txt";
		int fileLength = 40000;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String inputFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName,
						fileLength);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		// open the local file, transfer to iRODS, pause, then take a checksum
		// of the partial transfer
		RandomAccessFile localFile = new RandomAccessFile(inputFileName, "rw");

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSRandomAccessFile randomAccessFile = new IRODSRandomAccessFile(
				irodsFile, "rw");

		int transferSize = 1000;
		byte[] transferBuf = new byte[transferSize];

		int read = localFile.read(transferBuf);
		int totalRead = read;
		randomAccessFile.write(transferBuf, 0, read);

		String interimChecksum = irodsFile.checksumMD5();
		randomAccessFile.close();
		localFile.close();
		
		irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/'
				+ testFileName);
		randomAccessFile = new IRODSRandomAccessFile(irodsFile, "rw");
		randomAccessFile.seek(read);

		localFile = new RandomAccessFile(inputFileName, "rw");
		localFile.seek(read);
		
		while (true) {
			read = localFile.read(transferBuf);
			if (read == -1) {
				break;
			}
			totalRead += read;
			randomAccessFile.write(transferBuf, 0, read);
		}
		
		randomAccessFile.close();
		localFile.close();
		
		irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/'
				+ testFileName);
		
		String finalChecksum = irodsFile.checksumMD5();		
		Assert.assertFalse("got the same checksum for different files", finalChecksum.equals(interimChecksum));

	}
	
	/**
	 * Bug 90 - error in checksum for paused transfer - documents failing case for usage example, ignored in normal suite tests
	 * 
	 * @throws Exception
	 */
	@Ignore
	public final void testTransferPauseAndThenChecksumFailingCase() throws Exception {
		String testFileName = "testTransferPauseAndThenChecksum.txt";
		int fileLength = 40000;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String inputFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName,
						fileLength);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		// open the local file, transfer to iRODS, pause, then take a checksum
		// of the partial transfer
		RandomAccessFile localFile = new RandomAccessFile(inputFileName, "rw");

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSRandomAccessFile randomAccessFile = new IRODSRandomAccessFile(
				irodsFile, "rw");

		int transferSize = 1000;
		byte[] transferBuf = new byte[transferSize];

		int read = localFile.read(transferBuf);
		int totalRead = read;
		randomAccessFile.write(transferBuf, 0, read);

		String interimChecksum = irodsFile.checksumMD5();
		irodsFile.close();
		localFile.close();
		
		irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/'
				+ testFileName);
		randomAccessFile = new IRODSRandomAccessFile(irodsFile, "rw");
		randomAccessFile.seek(read);

		localFile = new RandomAccessFile(inputFileName, "rw");
		localFile.seek(read);
		
		while (true) {
			read = localFile.read(transferBuf);
			if (read == -1) {
				break;
			}
			totalRead += read;
			randomAccessFile.write(transferBuf, 0, read);
		}
		
		irodsFile.close();
		localFile.close();
		
		irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection + '/'
				+ testFileName);
		
		String finalChecksum = irodsFile.checksumMD5();		
		Assert.assertFalse("got the same checksum for different files", finalChecksum.equals(interimChecksum));

	}

}
