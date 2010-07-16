package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import org.junit.After;
import org.junit.AfterClass;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.net.URI;

import java.util.Properties;

import junit.framework.TestCase;

public class IRODSCommandsDeleteTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsCommandsDeleteTest";
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
	 * BUG: 29 Testing delete code
	 *
	 * @throws Exception
	 */
	@Test
	public void testDeleteOneFile() throws Exception {
		// generate a local scratch file
		String testFileName = "testDeleteOneFile.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

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

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);
		irodsFile.delete(true);
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(uriPath
				.toString());
	}
	
	@Test
	public void testDeleteOneFileNoForce() throws Exception {
		// generate a local scratch file
		String testFileName = "testDeleteOneFileNoForce.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				1);

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

		StringBuilder uriPath = new StringBuilder();
		uriPath.append(IRODS_TEST_SUBDIR_PATH);
		uriPath.append('/');
		uriPath.append(testFileName);

		URI irodsUri = testingPropertiesHelper
				.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
						uriPath.toString());
		IRODSFile irodsFile = new IRODSFile(irodsUri);
		irodsFile.delete(false);
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(uriPath
				.toString());
	}

	/**
	 * BUG: 29 - problem with Jargon physically deleting a file Testing delete
	 * code in collection to trigger status report
	 *
	 * @throws Exception
	 */
	@Test
	public void testDelete15ByDeletingCollection()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "del15file";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/del15dir";
		int numberOfTestFiles = 15;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and delete the collecton
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, deleteCollectionSubdir));

		boolean deleteResult = irodsFile.delete(true);
		irodsFileSystem.close();
		TestCase.assertTrue("delete was unsuccessful", deleteResult);
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(deleteCollectionAbsPath);

	}
	
	@Test
	public void testDelete15ByDeletingCollectionNoForce()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "del15file";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/del15noforcedir";
		int numberOfTestFiles = 15;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and delete the collecton
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, deleteCollectionSubdir));

		boolean deleteResult = irodsFile.delete(false);
		irodsFileSystem.close();
		TestCase.assertTrue("delete was unsuccessful", deleteResult);
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(deleteCollectionAbsPath);

	}


	/**
	 * BUG: 29 - problem with Jargon physically deleting a file Testing delete
	 * code in collection to trigger status report
	 *
	 * This tests an edge case where the number of files lines up with the status
	 * message boundary size
	 *
	 * @throws Exception
	 */
	@Test
	public void testDeleteAtStatusBoundaryByDeletingCollection()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "delBoundaryfile";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/delboundarydir";
		int numberOfTestFiles = 20;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and delete the collecton
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, deleteCollectionSubdir));

		boolean deleteResult = irodsFile.delete(true);
		irodsFileSystem.close();
		TestCase.assertTrue("delete was unsuccessful", deleteResult);
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(deleteCollectionAbsPath);

	}

	/**
	 * BUG: 29 - problem with Jargon physically deleting a file Testing delete
	 * code in collection to trigger status report
	 *
	 * @throws Exception
	 */
	@Test
	public void testDelete5FilesByDeletingCollection()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "del25file";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/delete5dir";
		int numberOfTestFiles = 5;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and delete the collecton
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, deleteCollectionSubdir));

		boolean deleteResult = irodsFile.delete(true);
		TestCase.assertTrue("delete was unsuccessful", deleteResult);
		irodsFileSystem.close();
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(deleteCollectionAbsPath);

	}
	
	/*
	 * Bug 108  - query fails with -816000 after delete
	 */
	@Test
	public void testDelete1200ByDeletingCollectionNoForceThenIssueGenQuery()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "del1200filethenquery";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/del1200noforcethenquerydir";
		int numberOfTestFiles = 1200;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, deleteCollectionSubdir));

		System.err.println("** Parent file before="+irodsFile.getParentFile());					
		boolean deleteResult = irodsFile.delete(false);
		System.err.println("** Parent file after="+irodsFile.getParentFile());					
		System.err.println("** Parent file exists="+irodsFile.getParentFile().exists());					

		// now do a query

		String[] fields = { IRODSMetaDataSet.FILE_NAME,	IRODSMetaDataSet.DIRECTORY_NAME };


		MetaDataSelect[] select = IRODSMetaDataSet.newSelection(fields);
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = IRODSMetaDataSet.newCondition(IRODSMetaDataSet.DIRECTORY_NAME,
				MetaDataCondition.EQUAL, irodsFile.getAbsolutePath());
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition, select, 100, Namespace.FILE, false);

		irodsFileSystem.close();
		TestCase.assertTrue("delete was unsuccessful", deleteResult);
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(deleteCollectionAbsPath);

	}
	
	@Test
	public void testDelete5FilesByDeletingCollectionNoForce()
			throws Exception {

		// test tuning variables
		String testFileNamePrefix = "del25file";
		String testFileExtension = ".txt";
		String deleteCollectionSubdir = IRODS_TEST_SUBDIR_PATH + "/delete5noforcedir";
		int numberOfTestFiles = 5;

		// create collection to zap
		String deleteCollectionAbsPath = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, deleteCollectionSubdir);
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImkdirCommand imkdrCommand = new ImkdirCommand();
		imkdrCommand.setCollectionName(deleteCollectionAbsPath);
		invoker.invokeCommandAndGetResultAsString(imkdrCommand);

		IputCommand iputCommand = new IputCommand();
		String genFileName = "";
		String fullPathToTestFile = "";

		// generate a number of files in the subdir
		for (int i = 0; i < numberOfTestFiles; i++) {
			genFileName = testFileNamePrefix + String.valueOf(i)
					+ testFileExtension;
			fullPathToTestFile = FileGenerator
					.generateFileOfFixedLengthGivenName(testingProperties
							.getProperty(GENERATED_FILE_DIRECTORY_KEY)
							+ "/", genFileName, 1);

			iputCommand.setLocalFileName(fullPathToTestFile);
			iputCommand.setIrodsFileName(deleteCollectionAbsPath);
			iputCommand.setForceOverride(true);
			invoker.invokeCommandAndGetResultAsString(iputCommand);
		}

		// now try and delete the collecton
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, deleteCollectionSubdir));

		boolean deleteResult = irodsFile.delete(false);
		TestCase.assertTrue("delete was unsuccessful", deleteResult);
		irodsFileSystem.close();
		assertionHelper.assertIrodsFileOrCollectionDoesNotExist(deleteCollectionAbsPath);

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
}
