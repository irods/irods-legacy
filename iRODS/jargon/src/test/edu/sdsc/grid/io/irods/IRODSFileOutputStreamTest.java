package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static org.junit.Assert.*;

import java.net.URI;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

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
		String testIRODSFileName = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			IRODS_TEST_SUBDIR_PATH + '/' + testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFileSystem, testIRODSFileName);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);
		
		irodsFileOutputStream.open(irodsFile);
		TestCase.assertTrue("file I created does not exist", irodsFile.exists());
		// get a simple byte array
		String myBytes = "ajjjjjjjjjjjjjjjjjjjjjjjjfeiiiiiiiiiiiiiii54454545";
		byte[] myBytesArray = myBytes.getBytes();
		irodsFileOutputStream.write(myBytesArray);
		irodsFileOutputStream.close();		
		long length = irodsFile.length();
		
		TestCase.assertEquals("file length does not match bytes written", myBytesArray.length, length);
		
		irodsFileSystem.close();	}

	@Test
	public final void testOpen() throws Exception {
		String testFileName = "testFileOpen.csv";
		String testIRODSFileName = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			IRODS_TEST_SUBDIR_PATH + '/' + testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(
				irodsFileSystem, testIRODSFileName);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);
		
		irodsFileOutputStream.open(irodsFile);
		TestCase.assertTrue("file I created does not exist", irodsFile.exists());
		irodsFileOutputStream.close();		
		irodsFileSystem.close();
	}

	@Test
	public final void testIRODSFileOutputStreamIRODSFileSystemString()
			throws Exception {
		String testFileName = "testFilePut.csv";
		String testIRODSFileName = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			IRODS_TEST_SUBDIR_PATH + '/' + testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);
		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(irodsFile);

		irodsFileSystem.close();

	}

	@Test
	public final void testIRODSFileOutputStreamIRODSFile() throws Exception {
		String testFileName = "testFilePut.csv";
		String testIRODSFileName = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			IRODS_TEST_SUBDIR_PATH + '/' + testFileName);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testIRODSFileName);

		IRODSFileOutputStream irodsFileOutputStream = new IRODSFileOutputStream(irodsFile);
		TestCase.assertNotNull("did not create fileOutputStream",
				irodsFileOutputStream);

		irodsFileSystem.close();	
	}

}
