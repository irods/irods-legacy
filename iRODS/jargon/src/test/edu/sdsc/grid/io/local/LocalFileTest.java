/**
 *
 */
package edu.sdsc.grid.io.local;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralAccount;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.GeneralFileSystem;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IlsCommand;

import junit.framework.Assert;
import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.util.Properties;

/**
 * @author Mike Conway, DICE (www.irods.org)
 * @since 2.0.6
 */
public class LocalFileTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "LocalFileTest";
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

	/**
	 * @throws java.lang.Exception
	 */
	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
	}

	/**
	 * @throws java.lang.Exception
	 */
	@After
	public void tearDown() throws Exception {
	}

	/**
	 * Test method for
	 * {@link edu.sdsc.grid.io.local.LocalFile#getPathSeparator()}.
	 */
	@Ignore
	public final void testGetPathSeparator() {
		fail("Not yet implemented");
	}

	/**
	 * Test method for
	 * {@link edu.sdsc.grid.io.local.LocalFile#getPathSeparatorChar()}.
	 */
	@Ignore
	public final void testGetPathSeparatorChar() {
		fail("Not yet implemented");
	}

	/**
	 * Test method for
	 * {@link edu.sdsc.grid.io.local.LocalFile#copyTo(edu.sdsc.grid.io.GeneralFile)}
	 * .
	 */
	@Test
	public final void testCopyToGeneralFileUsingSmallSourceFile()
			throws Exception {
		GeneralAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		GeneralFileSystem fileSystem = FileFactory.newFileSystem(account);
		String targetCollection = testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		String targetFileName = "testCopyToGeneralFileUsingSmallSourceFile.txt";

		GeneralFile file = FileFactory.newFile(fileSystem, targetCollection);

		String fullPathToLocalFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY),
						targetFileName, (1 * 1024));
		GeneralFile localFile = new LocalFile(fullPathToLocalFile);

		if (localFile.canRead()) {
			GeneralFile remoteFile = FileFactory.newFile(file, localFile
					.getName());
			localFile.copyTo(remoteFile, true);
		}

		// now check if file exists in irods
		IlsCommand ilsCommand = new IlsCommand();
		targetCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		ilsCommand.setIlsBasePath(targetCollection);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		String res = invoker.invokeCommandAndGetResultAsString(ilsCommand);
		Assert.assertTrue("did not find file I just put", res
				.indexOf(targetFileName) > -1);
	}

	/**
	 * Bug 47 - LocalFile toString issues
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testFileGetString() throws Exception {
		// save two files in a directory

		// make sure local scratch is empty
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);

		String testFileName = "testgetstring1.txt";
		String testfileName2 = "testgetstring2.txt";

		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String testFileFullPath = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 2);

		String testFileFullPath2 = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testfileName2, 2);

		LocalFile localFile = new edu.sdsc.grid.io.local.LocalFile(
				testFileFullPath);
		String fileToStringVal = localFile.toString();
		Assert.assertEquals("did not properly format file as string", "file:"
				+ testFileFullPath, fileToStringVal);

	}

	// Bug 115 - copyFrom() for http files results in protocol exception
	@Test
	public final void testCopyToIRODSFromLocalInvokingCopyFromOnIRODSFile()
			throws Exception {
		String testFileName = "testCopyToIRODSFromLocalInvokingCopyFromOnIRODSFile";
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		GeneralFileSystem fileSystem = FileFactory.newFileSystem(testAccount);

		IRODSFile remoteFile = (IRODSFile) FileFactory.newFile(irodsFileSystem,
				targetIrodsCollection + "/" + testFileName);

		String fullPathToLocalFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY),
						testFileName, (1 * 1024));
		GeneralFile localFile = new LocalFile(fullPathToLocalFile);

		remoteFile.copyFrom(localFile);
		assertionHelper.assertIrodsFileOrCollectionExists(remoteFile
				.getAbsolutePath());
		irodsFileSystem.close();
	}

	// Bug 115 - copyFrom() for http files results in protocol exception
	@Test
	public final void testCopyToIRODSFromLocalInvokingCopyToOnLocalFile()
			throws Exception {
	
		
		String testFileName = "testCopyToIRODSFromLocalInvokingCopyToOnLocalFile";
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		GeneralFileSystem fileSystem = FileFactory.newFileSystem(testAccount);

		IRODSFile remoteFile = (IRODSFile) FileFactory.newFile(irodsFileSystem,
				targetIrodsCollection + "/" + testFileName);

		String fullPathToLocalFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY),
						testFileName, (1 * 1024));
		GeneralFile localFile = new LocalFile(fullPathToLocalFile);

		localFile.copyTo(remoteFile, true);
		assertionHelper.assertIrodsFileOrCollectionExists(remoteFile
				.getAbsolutePath());
		irodsFileSystem.close();
	}

}
