package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IreplCommand;

import org.irods.jargon.core.exception.JargonException;
import org.junit.After;
import org.junit.AfterClass;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;
import java.util.List;
import java.util.Properties;

import junit.framework.TestCase;

/**
 * Tests for icommands that query metadata
 * 
 * @author Mike Conway, DICE (www.irods.org)
 * @since
 * 
 */
public class IRODSResourceQueryTest {

	protected static Properties testingProperties = new Properties();
	protected static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	protected static ScratchFileUtils scratchFileUtils = null;
	public static String IRODS_TEST_SUBDIR_PATH = "IRODSResourceQueryTest";
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
	public final void testQueryForResourceViaIRODSFileSystem() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

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

		irodsFileSystem.close();
		TestCase.assertTrue("did not find first resource", foundResc1);
		TestCase.assertTrue("did not find second resource", foundResc2);
	}

	/**
	 * test relevant to BUG: 24 Inconsistant return of resource
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testQueryForResourceViaIRODSFile() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryForResource.txt";

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

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

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH));

		String filename = irodsFile.getName();
		long size = irodsFile.length();

		MetaDataRecordList[] lists = irodsFile.query(new String[] {
				IRODSMetaDataSet.RESOURCE_NAME,
				IRODSMetaDataSet.RESOURCE_LOCATION });

		for (MetaDataRecordList l : lists) {
			String resource = l.getStringValue(0);
			String physicalResource = l.getStringValue(1);
		}

		irodsFileSystem.close();

		TestCase.assertTrue("did not get query result", lists.length > 0);
	}

	/**
	 * test relevant to BUG: 24 Inconsistant return of resource
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testGetAllResourcesForIRODSFile() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testGetAllResources.xsl";

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

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

		// replicate this file to the secondary resource

		IreplCommand ireplCommand = new IreplCommand();
		ireplCommand.setObjectToReplicate(iputCommand.getIrodsFileName() + '/'
				+ testFileName);
		ireplCommand.setDestResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));
		invoker.invokeCommandAndGetResultAsString(ireplCommand);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
										+ testFileName));

		List<String> resources = irodsFile.getAllResourcesForFile();
		irodsFileSystem.close();
		TestCase.assertEquals("did not get query result", 2, resources.size());
		boolean foundResc1 = false;
		boolean foundResc2 = false;

		for (String rescName : resources) {
			if (rescName.equals(testingProperties
					.getProperty(IRODS_RESOURCE_KEY))) {
				foundResc1 = true;
			} else if (rescName.equals(testingProperties
					.getProperty(IRODS_SECONDARY_RESOURCE_KEY))) {
				foundResc2 = true;
			} else {
				TestCase.fail("unknown resource found:" + rescName);
			}
		}

		TestCase.assertTrue("did not find first resource", foundResc1);
		TestCase.assertTrue("did not find second resource", foundResc2);

	}

	/**
	 * test relevant to BUG: 24 Inconsistant return of resource
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testGetAllResourcesOnCollection() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testGetAllResources.xsl";

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

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

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH));

		List<String> resources = irodsFile.getAllResourcesForFile();

		irodsFileSystem.close();

		TestCase.assertEquals(
				"should not have returned resources for the collection", 0,
				resources.size());
	}

	/**
	 *  Bug 82 -  error querying resources in resource group
	 * @throws Exception
	 */
	@Test
	public final void testAddAndQueryResourcesInResourceGroup() throws Exception {
		String testResourceGroup = "testResourceGroup";
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String rescName = testingProperties.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY);
		Resource resource = new Resource(irodsFileSystem);
		resource.removeResourceFromResourceGroup(rescName, testResourceGroup);
				
		// add a resource to the resource group
		resource.addResourceToResourceGroup(rescName, testResourceGroup);
	
		List<String> resources = resource.listResourcesInResourceGroup(testResourceGroup);
		irodsFileSystem.close();
		
		TestCase.assertTrue("did not get any resources from query", resources.size() > 0);
		
		boolean found = false;
		for (String resc : resources) {
			if (resc.equals(rescName)) {
				found = true;
				break;
			}
		}

		TestCase.assertTrue("did not find resc I added to the resource group", found);

	}

	/**
	 * Bug 87 - resource status in resource metadata, this test is ignored for
	 * versions prior to irods2.3, as the data was not available in the icat at
	 * the time
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testQueryResourceFields() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		if (irodsFileSystem.commands.getIrodsServerProperties().getRelVersion().compareTo(
				"rods2.3") <= 0) {
			return;
		}

		String[] fileds = { ResourceMetaData.RESOURCE_CLASS,
				ResourceMetaData.RESOURCE_COMMENTS,
				ResourceMetaData.RESOURCE_COMMENTS,
				ResourceMetaData.RESOURCE_CREATE_DATE,
				ResourceMetaData.RESOURCE_FREE_SPACE,
				ResourceMetaData.RESOURCE_FREE_SPACE_TIME,
				ResourceMetaData.RESOURCE_INFO,
				ResourceMetaData.RESOURCE_LOCATION,
				ResourceMetaData.RESOURCE_MODIFY_DATE,
				ResourceMetaData.RESOURCE_NAME,
				ResourceMetaData.RESOURCE_STATUS,
				ResourceMetaData.RESOURCE_TYPE,
				ResourceMetaData.RESOURCE_VAULT_PATH,
				ResourceMetaData.RESOURCE_ZONE };
		MetaDataSelect[] select = IRODSMetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.commands.query(null,
				select, 100, Namespace.RESOURCE, false);
		irodsFileSystem.close();
		TestCase.assertNotNull("query result were null", fileList);

	}

}
