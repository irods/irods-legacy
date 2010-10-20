package edu.sdsc.grid.io.irods;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.UserMetaData;
import edu.sdsc.grid.io.irods.mocks.MockGssCredential;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSFileSystemGsiHandlingTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileSystemMGsiHandlingTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		
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
	 *  BUG 33 -  GSI Support in Jargon 
	 * @throws Exception
	 */
	@Test
	public void testBuildsRightUserDNQueryForOldIrods() throws Exception {
		MockGssCredential credential = new MockGssCredential();
		IRODSAccount account = new IRODSAccount(testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY),
				testingPropertiesHelper.getPortAsInt(testingProperties), credential);
		IRODSAccount dummyAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		// fake out file system just to get hands on target method
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(dummyAccount);
	
		IRODSServerProperties irodsServerProperties = IRODSServerProperties.instance(IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 0, "rods2.1", "d", "test");
		irodsFileSystem.commands.setIrodsServerProperties(irodsServerProperties);
		MetaDataCondition condition = irodsFileSystem.buildMetaDataConditionForGSIUser(account);
		
		irodsFileSystem.close();

		Assert.assertEquals("did not derive expected metadata name for this version", UserMetaData.USER_DN_2_1, condition.getField().getName());

	}
	
	/**
	 *  BUG 33 -  GSI Support in Jargon 
	 * @throws Exception
	 */
	@Test
	public void testBuildsRightUserDNQueryForNewIrods() throws Exception {
		MockGssCredential credential = new MockGssCredential();
		IRODSAccount account = new IRODSAccount(testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY),
				testingPropertiesHelper.getPortAsInt(testingProperties), credential);
		IRODSAccount dummyAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		// fake out file system just to get hands on target method
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(dummyAccount);
		IRODSServerProperties irodsServerProperties = IRODSServerProperties.instance(IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 0, "rods2.2", "d", "test");
		irodsFileSystem.commands.setIrodsServerProperties(irodsServerProperties);
		
		MetaDataCondition condition = irodsFileSystem.buildMetaDataConditionForGSIUser(account);
		
		irodsFileSystem.close();

		Assert.assertEquals("did not derive expected metadata name for this version", UserMetaData.USER_DN, condition.getField().getName());

	}
	

	/**
	 *  BUG 33 -  GSI Support in Jargon 
	 * @throws Exception
	 */
	@Test 
	public void testGetOldIrodsMetaDataid() throws Exception {
		String id = IRODSMetaDataSet.getID(UserMetaData.USER_DN_2_1);
		Assert.assertEquals("did not find prev version metadata id", "205", id);
		
	
	}
	

	/**
	 *  BUG 33 -  GSI Support in Jargon 
	 * @throws Exception
	 */
	@Test 
	public void testGetNewIrodsMetaDataid() throws Exception {
		String id = IRODSMetaDataSet.getID(UserMetaData.USER_DN);
		Assert.assertEquals("did not find prev version metadata id", "1601", id);
		
	
	}
	
}
