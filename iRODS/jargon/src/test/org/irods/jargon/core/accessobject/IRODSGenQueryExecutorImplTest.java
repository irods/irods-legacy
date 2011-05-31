/**
 * 
 */
package org.irods.jargon.core.accessobject;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.RodsGenQueryEnum;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

/**
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class IRODSGenQueryExecutorImplTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsGenQueryExecutorImplTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;
	public static final String collDir = "coll";

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils
				.clearAndReinitializeScratchDirectory(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
		assertionHelper = new AssertionHelper();

		// put in the thousand files
		String testFilePrefix = "IRODSGenQueryExcecutorImplTest";
		String testFileSuffix = ".txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ '/' + collDir, testFilePrefix, testFileSuffix, 100, 5, 10);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// make the put subdir
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		ImkdirCommand iMkdirCommand = new ImkdirCommand();
		iMkdirCommand.setCollectionName(targetIrodsCollection);
		invoker.invokeCommandAndGetResultAsString(iMkdirCommand);

		// put the files by putting the collection
		IputCommand iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setLocalFileName(absPath + collDir);
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now add avu's to each
		addAVUsToEachFile();

	}

	/**
	 * @throws java.lang.Exception
	 */
	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testIRODSGenQueryExecutorImpl() throws Exception {

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		IRODSGenQueryExecutor irodsGenQueryExecutor = accessObjectFactory
				.getIRODSGenQueryExcecutor();

		Assert.assertNotNull(irodsGenQueryExecutor);
		irodsFileSystem.close();

	}

	@Test
	public final void testExecuteIRODSQuery() throws Exception {

		String queryString = "select "
				+ RodsGenQueryEnum.COL_R_RESC_NAME.getName()
				+ " ,"
				+ RodsGenQueryEnum.COL_R_ZONE_NAME.getName()
				+ " where "
				+ RodsGenQueryEnum.COL_R_ZONE_NAME.getName()
				+ " = "
				+ "'"
				+ testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_ZONE_KEY)
				+ "'";

		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 100);

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		IRODSGenQueryExecutor irodsGenQueryExecutor = accessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet = irodsGenQueryExecutor
				.executeIRODSQuery(irodsQuery, 0);

		irodsFileSystem.close();

		Assert.assertNotNull(resultSet);
		Assert.assertFalse("did not expect continuation", resultSet.isHasMoreRecords());

	}

	@Test
	public final void testExecuteIRODSQueryForResource() throws Exception {

		String queryString = "select "
				+ RodsGenQueryEnum.COL_R_RESC_NAME.getName()
				+ " where "
				+ RodsGenQueryEnum.COL_R_RESC_NAME.getName()
				+ " = "
				+ "'"
				+ testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY)
				+ "'";

		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 100);

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		IRODSGenQueryExecutor irodsGenQueryExecutor = accessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet = irodsGenQueryExecutor
				.executeIRODSQuery(irodsQuery, 0);

		irodsFileSystem.close();

		Assert.assertNotNull("null result set",resultSet);
		Assert.assertFalse("empty result set", resultSet.getResults().size() == 0);
		String returnedResourceName = resultSet.getFirstResult().getColumn(0);
		Assert.assertEquals("did not get expected result", testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), returnedResourceName);
		
	}

	
	/**
	 * Test method for
	 * {@link org.irods.jargon.core.accessobject.IRODSGenQueryExecutorImpl#getMoreResults(org.irods.jargon.core.query.IRODSQueryResultSet)}
	 * .
	 */
	@Test
	public final void testGetMoreResults() throws Exception {
		String queryString = "select "
				+ RodsGenQueryEnum.COL_COLL_NAME.getName() + " ,"
				+ RodsGenQueryEnum.COL_DATA_NAME.getName();

		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 50);

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		IRODSGenQueryExecutor irodsGenQueryExecutor = accessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet = irodsGenQueryExecutor
				.executeIRODSQuery(irodsQuery, 0);
		
		Assert.assertTrue("did not get expected continuation", resultSet.isHasMoreRecords());

		// now requery and get a new result set
		
		resultSet = irodsGenQueryExecutor.getMoreResults(resultSet);
		
		irodsFileSystem.close();
		Assert.assertNotNull("result set was null", resultSet);		
		Assert.assertTrue("did not get expected continuation", resultSet.isHasMoreRecords());
		Assert.assertTrue("no results, some expected", resultSet.getResults().size() > 0);
	}

	public static final void addAVUsToEachFile() throws Exception {

		String avu1Attrib = "avu1";
		String avu1Value = "avu1value";
		String avu2Attrib = "avu2";
		int avuIncr = 0;

		String[] metaData = new String[2];
		metaData[0] = avu1Attrib;
		metaData[1] = avu1Value;

		String[] metaData2 = new String[2];

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
										+ collDir));

		// get a list of files underneath the top-level directory, and add some
		// avu's to each one

		String[] fileList = irodsFile.list();
		IRODSFile subFile = null;

		for (int i = 0; i < fileList.length; i++) {
			subFile = new IRODSFile(irodsFileSystem, irodsFile
					.getAbsolutePath()
					+ '/' + fileList[i]);
			subFile.modifyMetaData(metaData);

			metaData2[0] = avu2Attrib;
			metaData2[1] = String.valueOf(avuIncr++);
			subFile.modifyMetaData(metaData2);
		}

		irodsFileSystem.close();
	}

}
