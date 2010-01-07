package edu.sdsc.grid.io;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;
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
				.query(new String[] { IRODSMetaDataSet.COLL_RESOURCE_NAME });

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

		TestCase.assertTrue("did not find first resource", foundResc1);
		TestCase.assertTrue("did not find second resource", foundResc2);
	}

	/**
	 * Test to check for  Bug 45 - SYS_UNMATCHED_API_NUM (-12000) when attempting to get a file
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
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, expectedFileLength);

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

		TestCase.assertTrue("file I created does not exist according to irods",
				file.exists());
		TestCase.assertTrue(
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
		
		TestCase.assertEquals("did not read all of the bytes of the file from irods", expectedFileLength, offset);

	}

}
