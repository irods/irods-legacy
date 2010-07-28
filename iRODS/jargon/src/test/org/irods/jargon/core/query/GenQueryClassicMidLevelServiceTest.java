package org.irods.jargon.core.query;

import static org.junit.Assert.*;

import java.util.Properties;

import junit.framework.TestCase;

import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class GenQueryClassicMidLevelServiceTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "GenQueryClassicMidLevelServiceTest";
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

	@Test
	public final void testGenQueryClassicMidLevelService() throws Exception {
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource("test_extended_icat_data.properties");
		ExtensibleMetaDataMapping mapping = source.generateExtensibleMetaDataMapping();
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		GenQueryClassicMidLevelService service = GenQueryClassicMidLevelService.instance(irodsFileSystem.getCommands());
		irodsFileSystem.close();
		TestCase.assertNotNull("mid level query service not created", service);
	}
	
	
	@Test(expected=JargonException.class)
	public final void testGenQueryClassicMidLevelServiceNullCommands() throws Exception {
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource("test_extended_icat_data.properties");
		ExtensibleMetaDataMapping mapping = source.generateExtensibleMetaDataMapping();
		GenQueryClassicMidLevelService.instance(null);		
	}
	
	@Test(expected=JargonException.class)
	public final void testGenQueryClassicMidLevelServiceClosedConnection() throws Exception {
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource("test_extended_icat_data.properties");
		ExtensibleMetaDataMapping mapping = source.generateExtensibleMetaDataMapping();
		IRODSAccount account = testingPropertiesHelper
		.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		irodsFileSystem.close();
		GenQueryClassicMidLevelService.instance(irodsFileSystem.getCommands());
	}
	
}
