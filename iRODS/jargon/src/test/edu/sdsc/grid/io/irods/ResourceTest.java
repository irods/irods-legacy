package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.util.List;
import java.util.Properties;

import junit.framework.TestCase;

import org.irods.jargon.core.exception.DuplicateDataException;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

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
	
	@Test
	public final void testListResourceGroups() throws Exception {
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		List<String> resourceGroups = resource.listResourceGroups();
		TestCase.assertNotNull(resourceGroups);
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
		resource.removeResourceFromResourceGroup(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), testResourceGroup);
		
		// now do the add
		resource.addResourceToResourceGroup(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), testResourceGroup);
		
		// list the resource groups
		List<String> resourceGroups = resource.listResourceGroups();
		TestCase.assertTrue(resourceGroups.size() > 0);
		boolean testRgFound = false;
		for (String resourceGroup : resourceGroups) {
			if (resourceGroup.equals(testResourceGroup)) {
				testRgFound = true;
				break;
			}
		}
		
		irodsFileSystem.close();
		
		TestCase.assertTrue("did not find the resource group I just added", testRgFound);
	}
	
	@Test(expected=DuplicateDataException.class)
	public final void testAddDuplicateResourceGroup() throws Exception {
		String testResourceGroup = "testrg";
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		
		
		// now do the add twice to make sure a duplicate condition occurs
		resource.addResourceToResourceGroup(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), testResourceGroup);
		resource.addResourceToResourceGroup(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), testResourceGroup);
	}
	
	@Test
	public final void testRemoveResourceGroup() throws Exception {
		String testResourceGroup = "testrg";
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		resource.removeResourceFromResourceGroup(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), testResourceGroup);
		
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
		
		TestCase.assertFalse("found the resource group I just removed", testRgFound);
	}
	
	@Test
	public final void testRemoveNonExistantResourceGroup() throws Exception {
		String testResourceGroup = "testrgidontexist";
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAdminAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		Resource resource = new Resource(irodsFileSystem);
		resource.removeResourceFromResourceGroup(testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY), testResourceGroup);
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
		TestCase.assertTrue(resources.length > 0);
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

		TestCase.assertTrue("did not return first resource", resc1Found);
		TestCase.assertTrue("did not return second resource", resc2Found);

	}

}
