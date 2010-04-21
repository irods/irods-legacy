package edu.sdsc.grid.io.irods;

import static org.junit.Assert.fail;

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;

public class DomainTest {


	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testDomain() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		Domain access = new Domain(irodsFileSystem, "access", "access_type", "");
		irodsFileSystem.close();
		TestCase.assertNotNull("no domain returned", access);
	}

	@Test
	public final void testListTypes() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		Domain access = new Domain(irodsFileSystem, "data", "data_type", "");
		String[] types = access.listTypes();
		irodsFileSystem.close();
		TestCase.assertTrue("no types found for data domain", types.length > 0);

	}

	@Ignore
	public final void testAddTypeString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testAddTypeStringStringStringString() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testDeleteType() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testModifyType() {
		fail("Not yet implemented");
	}

}
