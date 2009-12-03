package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.UserMetaData;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSManager;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

import junit.framework.TestCase;

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
	public void testBuildsRightUserDNQueryForOldIrods() throws Exception {
		MockGssCredential credential = new MockGssCredential();
		IRODSAccount account = new IRODSAccount(testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY),
				testingPropertiesHelper.getPortAsInt(testingProperties), credential);
		IRODSAccount dummyAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		// fake out file system just to get hands on target method
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(dummyAccount);
		irodsFileSystem.commands.setReportedIRODSVersion("rods2.1");
		
		MetaDataCondition condition = irodsFileSystem.buildMetaDataConditionForGSIUser(account);
		
		irodsFileSystem.close();

		TestCase.assertEquals("did not derive expected metadata name for this version", UserMetaData.USER_DN_2_1, condition.getField().getName());

	}
	
	@Test
	public void testBuildsRightUserDNQueryForNewIrods() throws Exception {
		MockGssCredential credential = new MockGssCredential();
		IRODSAccount account = new IRODSAccount(testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY),
				testingPropertiesHelper.getPortAsInt(testingProperties), credential);
		IRODSAccount dummyAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		// fake out file system just to get hands on target method
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(dummyAccount);
		irodsFileSystem.commands.setReportedIRODSVersion("rods2.2");
		
		MetaDataCondition condition = irodsFileSystem.buildMetaDataConditionForGSIUser(account);
		
		irodsFileSystem.close();

		TestCase.assertEquals("did not derive expected metadata name for this version", UserMetaData.USER_DN, condition.getField().getName());

	}
}
