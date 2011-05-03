package org.irods.jargon.core.query;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.mockito.Mockito;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.StandardMetaData;
import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.Tag;
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
	
	@Ignore //fixme: need to retest with updated iRODS
	public final void testBuildQueryTagUsingAnInInTheWhereClause() throws Exception {
		String testFileName1 = "testfilename1";
		String testFileName2 = "testfilename2";
		IRODSCommands irodsCommands = Mockito.mock(IRODSCommands.class);
		Mockito.when(irodsCommands.isConnected()).thenReturn(true);
		IRODSServerProperties irodsServerProperties = IRODSServerProperties.instance(IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.5", "d", "test1");
		Mockito.when(irodsCommands.getIrodsServerProperties()).thenReturn(irodsServerProperties);
		GenQueryClassicMidLevelService genQueryService = GenQueryClassicMidLevelService.instance(irodsCommands);
		// now query
		MetaDataCondition[] condition = new MetaDataCondition[1];
		String fileNames[] = new String[2];
		fileNames[0] = testFileName1;
		fileNames[1] = testFileName2;
		condition[0] = MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
				MetaDataCondition.IN, fileNames);
		
		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		Tag query = genQueryService.buildQueryTag(condition, select, 50, 0, Namespace.FILE, true);
		TestCase.assertNotNull("null tag was returned", query);
		String tagAsString = query.parseTag();
		TestCase.assertTrue("did not correctly format the in", tagAsString.indexOf("in 'testfilename1' 'testfilename2'") > -1);
	}
	
	@Ignore //fixme: need to retest with updated iRODS
	public final void testBuildQueryTagUsingABetweenInWhereClause() throws Exception {
		String size1 = "0";
		String size2 = "1000";
		IRODSCommands irodsCommands = Mockito.mock(IRODSCommands.class);
		Mockito.when(irodsCommands.isConnected()).thenReturn(true);
		IRODSServerProperties irodsServerProperties = IRODSServerProperties.instance(IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.5", "d", "test1");
		Mockito.when(irodsCommands.getIrodsServerProperties()).thenReturn(irodsServerProperties);
		GenQueryClassicMidLevelService genQueryService = GenQueryClassicMidLevelService.instance(irodsCommands);
		// now query
		MetaDataCondition[] condition = new MetaDataCondition[1];
		
		condition[0] = MetaDataSet.newCondition(StandardMetaData.FILE_NAME,
				MetaDataCondition.BETWEEN, size1, size2);
		
		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		Tag query = genQueryService.buildQueryTag(condition, select, 50, 0, Namespace.FILE, true);
		TestCase.assertNotNull("null tag was returned", query);
		String tagAsString = query.parseTag();
		TestCase.assertTrue("did not correctly format the in", tagAsString.indexOf("in 'testfilename1' 'testfilename2'") > -1);
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
		Assert.assertNotNull("mid level query service not created", service);
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
