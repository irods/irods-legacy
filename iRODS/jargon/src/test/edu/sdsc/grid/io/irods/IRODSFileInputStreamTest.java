package edu.sdsc.grid.io.irods;

import static org.junit.Assert.fail;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.net.URI;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.connection.ConnectionConstants;
import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.remoteexecute.RemoteExecuteServiceImpl;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
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

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
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

		ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();

		// now try to do the seek

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFileInputStream fis = new IRODSFileInputStream(irodsFile);
		long skipped = fis.skip(1024L);

		// may have skipped a different value?

		long numberBytesReadAfterSkip = 0L;

		// read the rest

		int readBytes;
		byte[] readBytesBuffer = new byte[512];
		while ((readBytes = (fis.read(readBytesBuffer, 0,
				readBytesBuffer.length))) > -1) {
			actualFileContents.write(readBytes);
			numberBytesReadAfterSkip += readBytes;
		}

		irodsFileSystem.close();
		long skippedPlusRead = skipped + numberBytesReadAfterSkip;

		Assert.assertEquals(
				"I did not skip and then read the remainder of the specified file",
				fileLengthInBytes, skippedPlusRead);
	}

	@Test
	public final void streamToBufferedStreamCopyTest() throws Exception {
		String testFileName = "streamToBufferedStreamCopyTest.txt";
		long fileLengthInBytes = 8091;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
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

		// now try to do the read

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(irodsFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		String testAbsPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String newPath = testAbsPath + "output" + testFileName;
		File outFile = new File(newPath);
		outFile.createNewFile();
		BufferedOutputStream bos = new BufferedOutputStream(
				new FileOutputStream(outFile));

		// read the rest
		int bytesRead = 0;

		int readBytes;
		while ((readBytes = bis.read()) > -1) {
			bos.write(readBytes);
			bytesRead++;
		}

		bos.flush();
		bos.close();
		bis.close();
		irodsFileSystem.close();
		Assert.assertEquals("whole file not read back", fileLengthInBytes,
				outFile.length());
	}

	@Test
	public final void streamToBufferedStreamCopyTestReadBuff() throws Exception {
		String testFileName = "streamToBufferedStreamCopyTestReadBuff.doc";
		long fileLengthInBytes = 8091;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
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

		// now try to do the read

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(irodsFile);
		BufferedInputStream bis = new BufferedInputStream(fis);
		String testAbsPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String newPath = testAbsPath + "output" + testFileName;
		File outFile = new File(newPath);
		outFile.createNewFile();
		BufferedOutputStream bos = new BufferedOutputStream(
				new FileOutputStream(outFile));

		byte[] buf = new byte[1024];
		int len = 0;
		while ((len = bis.read(buf)) >= 0) {
			bos.write(buf, 0, len);
		}

		bos.flush();
		bos.close();
		bis.close();
		irodsFileSystem.close();
		Assert.assertEquals("whole file not read back", fileLengthInBytes,
				outFile.length());
	}

	@Test
	public final void testRead() throws Exception {
		// generate a local scratch file
		String testFileName = "testread.txt";
		long fileLengthInBytes = 1024;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
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

		ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();

		// now try to do the read

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(irodsFile);

		// read the rest
		int bytesRead = 0;

		int readBytes;
		while ((readBytes = fis.read()) > -1) {
			actualFileContents.write(readBytes);
			bytesRead++;
		}

		irodsFileSystem.close();
		Assert.assertEquals("whole file not read back", fileLengthInBytes,
				bytesRead);
	}

	@Test
	public final void testReadLenOffset() throws Exception {
		// generate a local scratch file
		String testFileName = "testreadlenoffset.txt";
		int fileLengthInKb = 10;
		long fileLengthInBytes = fileLengthInKb * 1024;

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
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

		ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();

		// now try to do the read

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(irodsFile);

		// read the rest

		byte[] readBytesBuffer = new byte[512];
		while (((fis.read(readBytesBuffer, 0, readBytesBuffer.length))) > -1) {
			actualFileContents.write(readBytesBuffer);
		}

		irodsFileSystem.close();
		Assert.assertEquals(
				"I did not skip and then read the remainder of the specified file",
				fileLengthInBytes, actualFileContents.size());
	}

	@Test
	public final void testWriteThenRead() throws Exception {
		String testFileName = "testWriteThenRead.txt";

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		String testString = "jfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseij;ida8ehgasjfai'sjf;iadvajkdfgjasdl;jfasfjfaeiiiiiiiiiiiitsetseflyiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiispooniiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiisomewhereinthestringiiiiiiiiiconeiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiblarkiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiidangleiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskthisisthemiddleofthestringfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijjfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseijthisistheendofthestrring";
		byte[] myBytesArray = testString.getBytes();
		irodsFileOutputStream.write(myBytesArray, 0, myBytesArray.length);

		irodsFileOutputStream.close();

		irodsFile.close();

		irodsFileSystem = new IRODSFileSystem(account);

		// now read back
		IRODSFile readFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(readFile);
		long length = readFile.length();

		ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();

		// read the rest

		byte[] readBytesBuffer = new byte[512];
		while (((fis.read(readBytesBuffer, 0, readBytesBuffer.length))) > -1) {
			actualFileContents.write(readBytesBuffer);
		}

		irodsFileSystem.close();
		Assert.assertEquals(
				"file length from irods does not match string length",
				testString.length(), length);

		irodsFileOutputStream.close();
		irodsFileSystem.close();

	}

	@Test
	public final void testJustReadSomeBigChunk() throws Exception {
		String testFileName = "testJustReadSomeBigChunk.csv";

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		String testString = "jfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iadvajkdfgjasdl;jfasfjfaeiiiiiiiiiii";
		byte[] myBytesArray = testString.getBytes();
		irodsFileOutputStream.write(myBytesArray, 0, myBytesArray.length);

		irodsFileOutputStream.close();

		irodsFile.close();

		irodsFileSystem = new IRODSFileSystem(account);

		// now read back
		IRODSFile readFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(readFile);
		long length = readFile.length();

		ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();

		// read the rest

		byte[] readBytesBuffer = new byte[2048];
		while (((fis.read(readBytesBuffer, 0, readBytesBuffer.length))) > -1) {
			actualFileContents.write(readBytesBuffer);
		}

		irodsFileSystem.close();
		Assert.assertEquals(
				"file length from irods does not match string length",
				testString.length(), length);

		irodsFileOutputStream.close();
		irodsFileSystem.close();

	}

	@Test
	public final void testJustReadSomeBigChunkInTwoPiecesThatSumToASizeGreaterThenTheChunk()
			throws Exception {
		String testFileName = "testJustReadSomeBigChunkInTwoPiecesThatSumToASizeGreaterThenTheChunk.csv";

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFile);

		String testString = "jfaeiiiiiiiiiiiiiiiiiiiiii838ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iad38ejfiafjaskfjaisdfjaseij;idajjjjjjjjjjjjjjjjjjjjjjjjjj8ehgasjfai'sjf;iadvajkdfgjasdl;jfasfjfaeiiiiiiiiiiitheend******";
		byte[] myBytesArray = testString.getBytes();
		irodsFileOutputStream.write(myBytesArray, 0, myBytesArray.length);

		irodsFileOutputStream.close();

		irodsFile.close();

		irodsFileSystem = new IRODSFileSystem(account);

		// now read back
		IRODSFile readFile = new IRODSFile(irodsFileSystem,
				targetIrodsCollection + '/' + testFileName);
		IRODSFileInputStream fis = new IRODSFileInputStream(readFile);
		long length = readFile.length();

		ByteArrayOutputStream actualFileContents = new ByteArrayOutputStream();

		int readBytes;
		byte[] readBytesBuffer = new byte[512];
		while ((readBytes = (fis.read(readBytesBuffer, 0,
				readBytesBuffer.length))) > -1) {
			actualFileContents.write(readBytesBuffer, 0, readBytes);
		}

		irodsFileSystem.close();
		String actualString = actualFileContents.toString();

		Assert.assertEquals(
				"file length from irods does not match string length",
				testString.length(), length);

		Assert.assertEquals("string written does not match string read",
				testString, actualString);

	}

	@Test
	public void testGetInputStreamWithConnectionReroutingBySpecifiedResource()
			throws Exception {

		String useDistribResources = testingProperties
				.getProperty("test.option.distributed.resources");

		if (useDistribResources != null && useDistribResources.equals("true")) {
			// do the test
		} else {
			return;
		}

		if (ConnectionConstants.REROUTE_CONNECTIONS != true) {
			TestCase.fail("attempt to test connection re-routing, but reroute connections not set in ConnectionConstants");
		}

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IRODSServerProperties props = irodsFileSystem.getCommands()
				.getIrodsServerProperties();

		if (!props
				.isTheIrodsServerAtLeastAtTheGivenReleaseVersion(RemoteExecuteServiceImpl.STREAMING_API_CUTOFF)) {
			irodsFileSystem.close();
			return;
		}

		// generate a local scratch file
		String testFileName = "testGetInputStreamWithConnectionReroutingBySpecifiedResource.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

		// put scratch file into irods in the right place on the first resource
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
		iputCommand
				.setIrodsResource(testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY));
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);
		irodsFile
				.setResource(testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_TERTIARY_RESOURCE_KEY));
		IRODSFileInputStream irodsFileInputStream = new IRODSFileInputStream(
				irodsFile);
		IRODSAccount streamAccount = (IRODSAccount) irodsFileInputStream
				.getFileSystem().getAccount();
		TestCase.assertFalse("did not reroute connection", streamAccount
				.getHost().equals(testAccount.getHost()));

		// close the stream
		irodsFileInputStream.close();

		TestCase.assertTrue("did not close the rerouted file system",
				irodsFileInputStream.getFileSystem() == null);
		TestCase.assertTrue("original file system was closed",
				irodsFileSystem.isConnected() == true);

		irodsFileSystem.close();

	}

}
