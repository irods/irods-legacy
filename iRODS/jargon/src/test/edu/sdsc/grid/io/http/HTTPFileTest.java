package edu.sdsc.grid.io.http;

import java.net.URI;
import java.util.Properties;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class HTTPFileTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "HTTPFileTest";
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

	
	
	
	// Bug 115 - copyFrom() for http files results in protocol exception
	@Test
	public final void testCopyToIRODSFromHTTPInvokingCopyFromOnIRODSFile() throws Exception {
		String testFileName = "testCopyToIRODSFromHTTPInvokingCopyFromOnHttpFile";
		String testUrl = "http://www.irods.org";
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IRODSFile remoteFile = (IRODSFile) FileFactory.newFile(irodsFileSystem,
				targetIrodsCollection + "/" + testFileName);
		
		HTTPFile httpFile = new HTTPFile(new URI(testUrl));
		
		
		remoteFile.copyFrom(httpFile);
		assertionHelper.assertIrodsFileOrCollectionExists(remoteFile.getAbsolutePath());
	}
	
	// Bug 115 - copyFrom() for http files results in protocol exception
	@Test
	public final void testCopyToIRODSFromHTTPInvokingCopyToOnHTTPFile() throws Exception {
		String testFileName = "testCopyToIRODSFromHTTPInvokingCopyToOnHTTPFile";
		String testUrl = "http://www.irods.org";
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IRODSFile remoteFile = (IRODSFile) FileFactory.newFile(irodsFileSystem,
				targetIrodsCollection + "/" + testFileName);
		
		HTTPFile httpFile = new HTTPFile(new URI(testUrl));
		
		httpFile.copyTo(remoteFile, true);
		assertionHelper.assertIrodsFileOrCollectionExists(remoteFile.getAbsolutePath());
	}
	
}
