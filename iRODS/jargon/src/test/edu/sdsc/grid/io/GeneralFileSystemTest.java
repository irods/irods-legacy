package edu.sdsc.grid.io;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class GeneralFileSystemTest {

	protected static Properties testingProperties = new Properties();
	protected static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	protected static ScratchFileUtils scratchFileUtils = null;
	public static String IRODS_TEST_SUBDIR_PATH = "GeneralFileSystemTest";
	protected static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

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
	 * test relevant to BUG: 36 resource added with icommand does not show up in
	 * Jargon query
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testQueryMetaDataSelectArray() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		GeneralFileSystem irodsFileSystem = FileFactory.newFileSystem(account);

		MetaDataRecordList[] lists = irodsFileSystem
				.query(new String[] { ResourceMetaData.COLL_RESOURCE_NAME });

		boolean foundResc1 = false;
		boolean foundResc2 = false;

		for (MetaDataRecordList l : lists) {
			String resource = l.getStringValue(0);

			if (resource.equals(testingProperties
					.getProperty(IRODS_RESOURCE_KEY))) {
				foundResc1 = true;
			} else if (resource.equals(testingProperties
					.getProperty(IRODS_SECONDARY_RESOURCE_KEY))) {
				foundResc2 = true;
			}
		}

		Assert.assertTrue("did not find first resource", foundResc1);
		Assert.assertTrue("did not find second resource", foundResc2);
	}

	/**
	 * Test to check for Bug 45 - SYS_UNMATCHED_API_NUM (-12000) when attempting
	 * to get a file
	 * 
	 * @throws Exception
	 */
	@Test
	public void testGetViaGeneralRandomAccessFile() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testGetViaGenralRandomAccessFile.txt";
		int expectedFileLength = 1781;

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName,
						expectedFileLength);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		GeneralFile file = FileFactory.newFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);

		Assert.assertTrue("file I created does not exist according to irods",
				file.exists());
		Assert.assertTrue(
				"file I just created is not seen as a file on irods", file
						.isFile());

		GeneralRandomAccessFile raFile = FileFactory.newRandomAccessFile(file,
				"r");

		int nbytes = 0;
		int offset = 0;
		byte data[] = new byte[4096];

		while ((nbytes = raFile.read(data, offset, 4096)) > 0) {
			offset += nbytes;
		}

		Assert.assertEquals(
				"did not read all of the bytes of the file from irods",
				expectedFileLength, offset);

	}

	/**
	 * Test for Bug 68 - error in checksum -12000
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testIRODSFileChecksum() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testIRODSFileChecksum.txt";
		int expectedFileLength = 256;

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName,
						expectedFileLength);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		GeneralFile file = FileFactory.newFile(irodsFileSystem, iputCommand
				.getIrodsFileName()
				+ '/' + testFileName);

		Assert.assertTrue(file.exists());
		String fileChecksum = file.checksum();
		Assert.assertNotNull(fileChecksum);

	}

	/**
	 * Test for Bug 68 - error in checksum -12000
	 * Replication of test case provided
	 * @throws Exception
	 */
	@Test
	public final void testIRODSFileChecksumVersusLocalFile() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testIRODSFileChecksumVsLocal.txt";
		int expectedFileLength = 256;

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName,
						expectedFileLength);

		// move the local file per the code provided in bug report
		LocalFile lFile = new LocalFile(fullPathToTestFile);
		String irodsFilePath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, irodsFilePath);

		if (irodsFile.exists()) // same as overwriting
			irodsFile.delete();

		// System.err.println("Upload to:"+srbFile.getAbsolutePath());
		// This is where the error occurs if SRB is down.
		irodsFile.copyFrom(lFile);

		boolean checkSumPassed = irodsFile.checksum().equals(lFile.checksum());
		irodsFileSystem.close();

		Assert.assertTrue("checksums not equal", checkSumPassed);

	}
	
	/**
	 * Test for Bug 68 - error in checksum -12000
	 * Replication of test case provided
	 * @throws Exception
	 */
	@Test
	public final void testIRODSFileChecksumVersusLocalFileNoClose() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testIRODSFileChecksumVsLocalNoClose.txt";
		int expectedFileLength = 256;

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName,
						expectedFileLength);

		// move the local file per the code provided in bug report
		LocalFile lFile = new LocalFile(fullPathToTestFile);
		String irodsFilePath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH)
				+ '/' + testFileName;

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, irodsFilePath);

		if (irodsFile.exists()) // same as overwriting
			irodsFile.delete();

		// System.err.println("Upload to:"+srbFile.getAbsolutePath());
		// This is where the error occurs if SRB is down.
		irodsFile.copyFrom(lFile);

		boolean checkSumPassed = irodsFile.checksum().equals(lFile.checksum());

		Assert.assertTrue("checksums not equal", checkSumPassed);

	}

}
