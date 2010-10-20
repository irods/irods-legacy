package edu.sdsc.grid.io.irods;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;

public class IRODSAdminTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
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
	public final void testListUserGroups() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSAdmin irodsAdmin = new IRODSAdmin(irodsFileSystem);
		String[] userGroups = irodsAdmin.listUserGroups();
		irodsFileSystem.close();
		Assert.assertTrue("returned no user groups", userGroups.length > 0);

	}

	@Test
	public final void testListUsers() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSAdmin irodsAdmin = new IRODSAdmin(irodsFileSystem);
		String[] users = irodsAdmin.listUsers();
		irodsFileSystem.close();
		Assert.assertTrue("returned no users", users.length > 0);
	}

	@Test
	public final void testListResources() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSAdmin irodsAdmin = new IRODSAdmin(irodsFileSystem);
		String[] resources = irodsAdmin.listResources();
		irodsFileSystem.close();
		Assert.assertTrue("returned no resources", resources.length > 0);
	}

	@Test
	public final void testListZones() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSAdmin irodsAdmin = new IRODSAdmin(irodsFileSystem);
		String[] zones = irodsAdmin.listZones();
		irodsFileSystem.close();
		Assert.assertTrue("returned no zones", zones.length > 0);
	}

	@Test
	public final void testCreateGroup() throws Exception {
		
		String testGroup = "testGroup";
		
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		IRODSAdmin irodsAdmin = new IRODSAdmin(irodsFileSystem);
		
		// delete the test group initially, discard error if not found
		try {
			irodsAdmin.deleteGroup(testGroup);
		} catch (Exception e) {
			// ignore
		}
		
		String[] initialGroups = irodsAdmin.listUserGroups();
		int initialCount = initialGroups.length;
		
		irodsAdmin.createGroup(testGroup);
		
		String[] postCreateGroups = irodsAdmin.listUserGroups();
		int postCreateCount = postCreateGroups.length;
		
		irodsAdmin.deleteGroup(testGroup);
		
		irodsFileSystem.close();
		Assert.assertEquals("did not add the group", initialCount + 1, postCreateCount);
	}

}
