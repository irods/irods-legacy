package edu.sdsc.grid.io.irods;

import java.util.List;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.accessobject.IRODSAccessObjectFactory;
import org.irods.jargon.core.accessobject.IRODSAccessObjectFactoryImpl;
import org.irods.jargon.core.accessobject.IRODSGenQueryExecutor;
import org.irods.jargon.core.exception.DuplicateDataException;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.pub.domain.AvuData;
import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.JargonQueryException;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.ResourceMetaData;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaAddCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaListCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaRemoveCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaCommand.MetaObjectType;

public class ResourceTest {
	protected static Properties testingProperties = new Properties();
	protected static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	protected static ScratchFileUtils scratchFileUtils = null;
	public static String IRODS_TEST_SUBDIR_PATH = "IRODSResourceTest";
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

	/*
	 * Bug 124 - Resource.modifyFreeSpace()
	 */
	@Test
	public void testModifyResourceFreeSpace() throws Exception {

		String testval1 = "20";
		String testval2 = "30";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		resource.modifyFreespace(
				testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY),
				testval1);
		String rescQuery = "SELECT RESC_NAME, RESC_FREE_SPACE WHERE RESC_NAME = '"
				+ testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY)
				+ "'";
		IRODSQuery irodsQuery = IRODSQuery.instance(rescQuery, 50);

		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		IRODSGenQueryExecutor irodsGenQueryExecutor = irodsAccessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet;

		resultSet = irodsGenQueryExecutor.executeIRODSQuery(irodsQuery, 0);
		String freeSpace = resultSet.getFirstResult().getColumn(1);
		TestCase.assertEquals("first free space test did not set value",
				testval1, freeSpace);

		resource.modifyFreespace(
				testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY),
				testval2);
		rescQuery = "SELECT RESC_NAME, RESC_FREE_SPACE WHERE RESC_NAME = '"
				+ testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY)
				+ "'";
		irodsQuery = IRODSQuery.instance(rescQuery, 50);
		resultSet = irodsGenQueryExecutor.executeIRODSQuery(irodsQuery, 0);
		freeSpace = resultSet.getFirstResult().getColumn(1);
		TestCase.assertEquals("second free space test did not set value",
				testval2, freeSpace);

	}
	
	/*
	 *[#157] adding Resource.modifyStatus()
	 */
	@Test
	public void testModifyResourceStatus() throws Exception {

		String testval1 = "a";
		String testval2 = "b";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		resource.modifyStatus(
				testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY),
				testval1);
		String rescQuery = "SELECT RESC_NAME, RESC_STATUS WHERE RESC_NAME = '"
				+ testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY)
				+ "'";
		IRODSQuery irodsQuery = IRODSQuery.instance(rescQuery, 50);

		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		IRODSGenQueryExecutor irodsGenQueryExecutor = irodsAccessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet;

		resultSet = irodsGenQueryExecutor.executeIRODSQuery(irodsQuery, 0);
		String status = resultSet.getFirstResult().getColumn(1);
		TestCase.assertEquals("first status did not set value",
				testval1, status);

		resource.modifyStatus(
				testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY),
				testval2);
		rescQuery = "SELECT RESC_NAME, RESC_STATUS WHERE RESC_NAME = '"
				+ testingProperties
						.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY)
				+ "'";
		irodsQuery = IRODSQuery.instance(rescQuery, 50);
		resultSet = irodsGenQueryExecutor.executeIRODSQuery(irodsQuery, 0);
		status = resultSet.getFirstResult().getColumn(1);
		TestCase.assertEquals("second status test did not set value",
				testval2, status);

	}

	@Test
	public final void testListResourceGroups() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		List<String> resourceGroups = resource.listResourceGroups();
		Assert.assertNotNull(resourceGroups);
		// if no error, I'm successful

	}

	@Test
	public final void testAddResourceGroup() throws Exception {
		String testResourceGroup = "testrg";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);

		// make sure resource group does not exist
		resource.removeResourceFromResourceGroup(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				testResourceGroup);

		// now do the add
		resource.addResourceToResourceGroup(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				testResourceGroup);

		// list the resource groups
		List<String> resourceGroups = resource.listResourceGroups();
		Assert.assertTrue(resourceGroups.size() > 0);
		boolean testRgFound = false;
		for (String resourceGroup : resourceGroups) {
			if (resourceGroup.equals(testResourceGroup)) {
				testRgFound = true;
				break;
			}
		}

		irodsFileSystem.close();

		Assert.assertTrue("did not find the resource group I just added",
				testRgFound);
	}

	@Test(expected = DuplicateDataException.class)
	public final void testAddDuplicateResourceGroup() throws Exception {
		String testResourceGroup = "testrg";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);

		// now do the add twice to make sure a duplicate condition occurs
		resource.addResourceToResourceGroup(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				testResourceGroup);
		resource.addResourceToResourceGroup(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				testResourceGroup);
	}

	@Test
	public final void testRemoveResourceGroup() throws Exception {
		String testResourceGroup = "testrg";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		resource.removeResourceFromResourceGroup(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				testResourceGroup);

		// list the resource groups
		List<String> resourceGroups = resource.listResourceGroups();
		boolean testRgFound = false;
		for (String resourceGroup : resourceGroups) {
			System.out.println(resourceGroup);
			if (resourceGroup.equals(testResourceGroup)) {
				testRgFound = true;
				break;
			}
		}
		irodsFileSystem.close();

		Assert.assertFalse("found the resource group I just removed",
				testRgFound);
	}

	@Test
	public final void testRemoveNonExistantResourceGroup() throws Exception {
		String testResourceGroup = "testrgidontexist";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		resource.removeResourceFromResourceGroup(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				testResourceGroup);
		irodsFileSystem.close();
	}

	/**
	 * BUG: 30 - query a resource list for a user shows all resources user has
	 * files in Test case for work-around
	 * 
	 * @throws Exception
	 */
	@Test
	public final void testListSubjects() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		String[] resources = resource.listSubjects();
		Assert.assertTrue(resources.length > 0);
		boolean resc1Found = false;
		boolean resc2Found = false;

		String actualResc1 = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);
		String actualResc2 = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY);

		for (int i = 0; i < resources.length; i++) {
			if (resources[i].trim().equals(actualResc1)) {
				resc1Found = true;
			} else if (resources[i].trim().equals(actualResc2)) {
				resc2Found = true;
			}
		}

		irodsFileSystem.close();

		Assert.assertTrue("did not return first resource", resc1Found);
		Assert.assertTrue("did not return second resource", resc2Found);

	}

	@Test
	public final void testAddResourceMetadata() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		String expectedAVUAttrib = "testAddResourceMetadata";
		String expectedAVUValue = "value1";

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// clean up avu

		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAVUAttrib);
		imetaRemoveCommand.setAttribValue(expectedAVUValue);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaRemoveCommand.setObjectPath(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

		String[] newAvu = { expectedAVUAttrib, expectedAVUValue };
		resource.addMetadataToResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				newAvu);

		irodsFileSystem.close();

		// verify the metadata was added
		// now get back the avu data and make sure it's there
		ImetaListCommand imetaList = new ImetaListCommand();
		imetaList.setAttribName(expectedAVUAttrib);
		imetaList.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaList.setObjectPath(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		String metaValues = invoker
				.invokeCommandAndGetResultAsString(imetaList);
		Assert.assertTrue("did not find expected attrib name",
				metaValues.indexOf(expectedAVUAttrib) > -1);
		Assert.assertTrue("did not find expected attrib value",
				metaValues.indexOf(expectedAVUValue) > -1);

	}

	@Test
	public final void testDeleteResourceMetadata() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		String expectedAVUAttrib = "testDeleteResourceMetadata";
		String expectedAVUValue = "testDeleteResourceMetadata-value1";

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// clean up avu

		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAVUAttrib);
		imetaRemoveCommand.setAttribValue(expectedAVUValue);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaRemoveCommand.setObjectPath(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

		String[] newAvu = { expectedAVUAttrib, expectedAVUValue };
		resource.addMetadataToResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				newAvu);

		// added, now delete

		resource.deleteMetadataFromResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				newAvu);
		irodsFileSystem.close();

		// verify the metadata was added
		// now get back the avu data and make sure it's there
		ImetaListCommand imetaList = new ImetaListCommand();
		imetaList.setAttribName(expectedAVUAttrib);
		imetaList.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaList.setObjectPath(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY));
		String metaValues = invoker
				.invokeCommandAndGetResultAsString(imetaList);
		Assert.assertTrue("should have deleted attrib name",
				metaValues.indexOf(expectedAVUAttrib) == -1);
		Assert.assertTrue("should have deleted attrib value",
				metaValues.indexOf(expectedAVUValue) == -1);

	}

	@Test
	public final void testListResourceMetadata() throws Exception {
		String testResource = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);

		// initialize the AVU data
		String expectedAttribName = "testattrib1";
		String expectedAttribValue = "testvalue1";
		String expectedAttribUnits = "test1units";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAttribName);
		imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setAttribUnits(expectedAttribUnits);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaRemoveCommand.setObjectPath(testResource);
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

		ImetaAddCommand imetaAddCommand = new ImetaAddCommand();
		imetaAddCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaAddCommand.setAttribName(expectedAttribName);
		imetaAddCommand.setAttribValue(expectedAttribValue);
		imetaAddCommand.setAttribUnits(expectedAttribUnits);
		imetaAddCommand.setObjectPath(testResource);
		invoker.invokeCommandAndGetResultAsString(imetaAddCommand);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);

		List<AvuData> avuData = resource.listResourceMetadata(testResource);
		irodsFileSystem.close();
		Assert.assertFalse("no query result returned", avuData.isEmpty());
		AvuData avuDataItem = null;

		for (AvuData foundItem : avuData) {
			if (foundItem.getAttribute().equals(expectedAttribName)) {
				avuDataItem = foundItem;
				break;
			}
		}

		Assert.assertNotNull("did not find the testing attrib in the resource",
				avuDataItem);
		Assert.assertEquals("did not get expected attrib", expectedAttribName,
				avuDataItem.getAttribute());
		Assert.assertEquals("did not get expected value", expectedAttribValue,
				avuDataItem.getValue());

	}

	/*
	 * Bug 112 - query to get a resource AVU on a resource on an iRODS 2.3 using
	 * jargon 2.3
	 */
	@Test
	public final void testListResourceMetadataForResourceName()
			throws Exception {
		String testResource = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);

		// initialize the AVU data
		String expectedAttribName = "testtestListResourceMetadataForResourceNameattrib1";
		String expectedAttribValue = "testtestListResourceMetadataForResourceNamevalue1";
		String expectedAttribUnits = "test1testListResourceMetadataForResourceNameunits";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAttribName);
		imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setAttribUnits(expectedAttribUnits);

		imetaRemoveCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaRemoveCommand.setObjectPath(testResource);
		invoker.invokeCommandAndGetResultAsString(imetaRemoveCommand);

		ImetaAddCommand imetaAddCommand = new ImetaAddCommand();
		imetaAddCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaAddCommand.setAttribName(expectedAttribName);
		imetaAddCommand.setAttribValue(expectedAttribValue);
		imetaAddCommand.setAttribUnits(expectedAttribUnits);
		imetaAddCommand.setObjectPath(testResource);
		invoker.invokeCommandAndGetResultAsString(imetaAddCommand);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		final MetaDataSelect[] selects = {
				MetaDataSet
						.newSelection(IRODSMetaDataSet.META_RESOURCE_ATTR_VALUE),
				MetaDataSet
						.newSelection(IRODSMetaDataSet.META_RESOURCE_ATTR_UNITS) };

		final MetaDataCondition[] conditions = {
				MetaDataSet.newCondition(
						IRODSMetaDataSet.META_RESOURCE_ATTR_NAME,
						MetaDataCondition.EQUAL, expectedAttribName),
				MetaDataSet
						.newCondition(
								ResourceMetaData.COLL_RESOURCE_NAME,
								MetaDataCondition.EQUAL,
								testingProperties
										.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY)) };
		final MetaDataRecordList[] results = irodsFileSystem.query(conditions,
				selects, Namespace.RESOURCE);

		irodsFileSystem.close();

		Assert.assertNotNull("null query results were not expected", results);
		Assert.assertTrue("did not get query results", results.length > 0);

	}

	/*
	 * Bug 112 - query to get a resource AVU on a resource on an iRODS 2.3 using
	 * jargon 2.3
	 */
	@Test
	public final void testListResourceMetadataValAndUnitsNoConditionsForResourceName()
			throws Exception {
		String testResource = testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		// initialize the AVU data
		String expectedAttribName = "testListResourceMetadataValAndUnitsNoConditionsForResourceNameattrib1";
		String expectedAttribValue = "testListResourceMetadataValAndUnitsNoConditionsForResourceNamevalue1";
		String expectedAttribUnits = "testListResourceMetadataValAndUnitsNoConditionsForResourceNameunits";
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		ImetaRemoveCommand imetaRemoveCommand = new ImetaRemoveCommand();
		imetaRemoveCommand.setAttribName(expectedAttribName);
		imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setAttribUnits(expectedAttribUnits);
		// imetaRemoveCommand.setAttribValue(expectedAttribValue);
		imetaRemoveCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		imetaRemoveCommand.setObjectPath(testResource);
		String removeResult = invoker
				.invokeCommandAndGetResultAsString(imetaRemoveCommand);

		// ImetaAddCommand imetaAddCommand = new ImetaAddCommand();
		// imetaAddCommand.setMetaObjectType(MetaObjectType.RESOURCE_META);
		// imetaAddCommand.setAttribName(expectedAttribName);
		// imetaAddCommand.setAttribValue(expectedAttribValue);
		// imetaAddCommand.setAttribUnits(expectedAttribUnits);
		// imetaAddCommand.setObjectPath(testResource);
		// String addResult = invoker
		// .invokeCommandAndGetResultAsString(imetaAddCommand);

		Resource resource = new Resource(irodsFileSystem);
		resource.addMetadataToResource(testingProperties
				.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY),
				new String[] { expectedAttribName, expectedAttribValue,
						expectedAttribUnits });

		final MetaDataSelect[] selects = {
				MetaDataSet
						.newSelection(IRODSMetaDataSet.META_RESOURCE_ATTR_VALUE),
				MetaDataSet
						.newSelection(IRODSMetaDataSet.META_RESOURCE_ATTR_UNITS) };

		final MetaDataCondition[] conditions = {};
		final MetaDataRecordList[] results = irodsFileSystem.query(conditions,
				selects, Namespace.RESOURCE);

		irodsFileSystem.close();
		irodsFileSystem.close();

		Assert.assertNotNull("null query results were not expected", results);
		Assert.assertTrue("did not get query results", results.length > 0);

	}

}
