package edu.sdsc.grid.io;

import static org.junit.Assert.*;

import java.net.URI;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class FileFactoryTest {
	
	protected static Properties testingProperties = new Properties();
	protected static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	protected static ScratchFileUtils scratchFileUtils = null;
	public static String IRODS_TEST_SUBDIR_PATH = "FileFactoryTest";
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

	public static void tearDownAfterClass() throws Exception {
	}

	@Ignore
	public final void testRegisterFileSystemGeneralAccountGeneralFileSystemGeneralFile() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testRegisterFileSystemURIGeneralAccountGeneralFileSystemGeneralFileGeneralRandomAccessFile() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testRegisterFileSystemURIGeneralAccountGeneralFileSystemGeneralFileGeneralRandomAccessFileGeneralFileInputStreamGeneralFileOutputStream() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testRegisterFileSystemURIGeneralAccountGeneralFileSystemGeneralFileGeneralRandomAccessFileGeneralFileInputStreamGeneralFileOutputStreamMetaDataRecordList() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testRegisterFileSystemURIClassClassClassClassClassClassClass() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testIsFileSystemRegistered() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileSystemGeneralAccount() {
		fail("Not yet implemented");
	}

	@Test
	public final void testNewFileSystemURI() throws Exception {
		String irodsFilePath = testingPropertiesHelper
		.buildIRODSCollectionAbsolutePathFromTestProperties(
				testingProperties, IRODS_TEST_SUBDIR_PATH);
		URI testURI = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties, irodsFilePath);
		GeneralFileSystem generalFileSystem = FileFactory.newFileSystem(testURI);
		TestCase.assertNotNull("null file system returned", generalFileSystem);
		TestCase.assertTrue("expected an IRODSFileSystem", generalFileSystem instanceof IRODSFileSystem);
	}

	@Ignore
	public final void testNewFileSystemURIGSSCredential() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileURI() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileURIString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileURIGSSCredential() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileURIStringString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileGeneralAccountString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileGeneralAccountStringString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileGeneralFileSystemString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileGeneralFileSystemStringString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileGeneralFileString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileGeneralFileSystemStringMetaDataConditionArray() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewRandomAccessFileGeneralFileSystemStringString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewRandomAccessFileGeneralFileString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewRandomAccessFileURIString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileInputStreamGeneralFile() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileInputStreamURI() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileOutputStreamGeneralFile() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewFileOutputStreamURI() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewMetaDataRecordListGeneralFileSystemMetaDataFieldInt() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewMetaDataRecordListGeneralFileSystemMetaDataFieldFloat() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewMetaDataRecordListGeneralFileSystemMetaDataFieldString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testNewMetaDataRecordListGeneralFileSystemMetaDataFieldMetaDataTable() {
		fail("Not yet implemented");
	}

}
