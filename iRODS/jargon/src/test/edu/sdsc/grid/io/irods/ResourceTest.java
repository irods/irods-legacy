package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;

public class ResourceTest extends IRODSTestCase {
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSResourceTest";

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		IRODSTestCase.setUpBeforeClass();
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
		IRODSTestCase.tearDownAfterClass();
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	/**
	 * BUG: 30 -  query a resource list for a user shows all resources user has files in
	 * Test case for work-around
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

		String actualResc1 =  testingProperties
		.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);
		String actualResc2 =  testingProperties
		.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY);

		for (int i = 0; i < resources.length; i++) {
			if (resources[i].trim().equals(actualResc1)) {
				resc1Found = true;
			} else if (resources[i].trim()
					.equals(actualResc2)) {
				resc2Found = true;
			}
		}

		irodsFileSystem.close();

		TestCase.assertTrue("did not return first resource", resc1Found);
		TestCase.assertTrue("did not return second resource", resc2Found);

	}

}
