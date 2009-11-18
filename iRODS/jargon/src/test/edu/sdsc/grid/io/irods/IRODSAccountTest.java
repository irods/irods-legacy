package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSAccountTest {
	 private static Properties testingProperties = new Properties();
	    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	    private static ScratchFileUtils scratchFileUtils = null;
	    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSAccountTest";
	    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

	    @BeforeClass
	    public static void setUpBeforeClass() throws Exception {
	        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
	        testingProperties = testingPropertiesLoader.getTestProperties();
	        scratchFileUtils = new ScratchFileUtils(testingProperties);
	        scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
	        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
	        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
	        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
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
	public final void testIRODSAccount() throws Exception {
		// can I create an IRODSAccount on this platform based on data in my .irodsEnv?  OK if no exception
		IRODSAccount account = new IRODSAccount();
		
	}

	@Test
	public final void testIRODSAccountGeneralFile() throws Exception {
		LocalFile info = new LocalFile(System.getProperty("user.home")+"/.irods/");
	    if (!info.exists()) {
	      //Windows Scommands doesn't setup as "."
	      info = new LocalFile(System.getProperty("user.home")+"/irods/");
	    }
	    IRODSAccount irodsAccount = new IRODSAccount(info);
	}

	@Test
	public final void testIRODSAccountStringIntStringStringStringStringString() {
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), expectedIRODSAccount.getDefaultStorageResource());
		TestCase.assertEquals(expectedIRODSAccount.getDefaultStorageResource(), actualIRODSAccount.getDefaultStorageResource());
		TestCase.assertEquals(expectedIRODSAccount.getZone(), actualIRODSAccount.getZone());
		TestCase.assertEquals(expectedIRODSAccount.getHomeDirectory(), actualIRODSAccount.getHomeDirectory());
		TestCase.assertEquals(expectedIRODSAccount.getHost(), actualIRODSAccount.getHost());
		TestCase.assertEquals(expectedIRODSAccount.getPassword(), actualIRODSAccount.getPassword());
		TestCase.assertEquals(expectedIRODSAccount.getPort(), actualIRODSAccount.getPort());
		TestCase.assertEquals(expectedIRODSAccount.getUserName(), actualIRODSAccount.getUserName());		
	}
	
	@Test(expected=NullPointerException.class)
	public final void testIRODSAccountNoDefaultStorageResource() {
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), null);
		TestCase.assertEquals(expectedIRODSAccount.getDefaultStorageResource(), actualIRODSAccount.getDefaultStorageResource());
		TestCase.assertEquals(expectedIRODSAccount.getZone(), actualIRODSAccount.getZone());
		TestCase.assertEquals(expectedIRODSAccount.getHomeDirectory(), actualIRODSAccount.getHomeDirectory());
		TestCase.assertEquals(expectedIRODSAccount.getHost(), actualIRODSAccount.getHost());
		TestCase.assertEquals(expectedIRODSAccount.getPassword(), actualIRODSAccount.getPassword());
		TestCase.assertEquals(expectedIRODSAccount.getPort(), actualIRODSAccount.getPort());
		TestCase.assertEquals(expectedIRODSAccount.getUserName(), actualIRODSAccount.getUserName());		
	}


	@Ignore
	public final void testIRODSAccountStringIntGSSCredential() {
		fail("Not yet implemented");
	}

	@Ignore
	public final void testIRODSAccountStringIntGSSCredentialStringString() {
		fail("Not yet implemented");
	}

	@Test
	public final void testSetDefaultStorageResource() {
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), expectedIRODSAccount.getDefaultStorageResource());
		actualIRODSAccount.setDefaultStorageResource("hellothere");
	}

	@Test
	public final void testGetDefaultStorageResource() {
		String expectedDefaultResource = "hellothere";
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), expectedIRODSAccount.getDefaultStorageResource());
		actualIRODSAccount.setDefaultStorageResource(expectedDefaultResource);
		String actualDefaultResource = actualIRODSAccount.getDefaultStorageResource();
		TestCase.assertEquals(expectedDefaultResource, actualDefaultResource);
		
	}
	
	// TODO: add test of IRODSAccountCreate with a contrived .irodsEnv file (need a generator) with no default storage resource, etc

}
