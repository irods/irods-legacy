package edu.sdsc.grid.io.irods;

import static org.junit.Assert.fail;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.query.ExtensibleMetaDataSource;
import org.irods.jargon.core.query.ExtensibleMetadataPropertiesSource;
import org.irods.jargon.core.query.GenQueryClassicMidLevelService;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataField;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSExtensibleMetaDataTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsExensibleMetaDataTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
		assertionHelper = new AssertionHelper();
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Ignore
	public final void testGetField() {
		fail("Not yet implemented");
	}

	@Test
	public final void testIRODSMetaDataSetIRODSProtocol() {
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		IRODSMetaDataSet irodsMetaDataSet = new IRODSMetaDataSet(irodsProtocol);
		Assert.assertNotNull(irodsMetaDataSet);
	}

	@Test
	public final void findExtensibleMetaDataId() throws Exception {
		// generate metadata mapping from test source
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource(
				"test_extended_icat_data.properties");
		source.generateExtensibleMetaDataMapping();
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		IRODSMetaDataSet irodsMetaDataSet = new IRODSMetaDataSet(irodsProtocol);
		String id = IRODSMetaDataSet.getIDFromExtensibleMetaData("COL_TEST_ID");
		Assert.assertEquals("10001", id);

	}

	@Test
	public final void findAvuVal() throws Exception {
		// generate metadata mapping from test source
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource(
				"test_extended_icat_data.properties");
		source.generateExtensibleMetaDataMapping();
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		IRODSMetaDataSet irodsMetaDataSet = new IRODSMetaDataSet(irodsProtocol);
		String id = IRODSMetaDataSet.getID("IAmAnAvu");
		Assert.assertEquals("IAmAnAvu", id);

	}

	@Test
	public final void testQueryBuiltFromExtensibleMetaDataAttributes()
			throws Exception {
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource(
				"test_extended_icat_data.properties");
		source.generateExtensibleMetaDataMapping();
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		IRODSMetaDataSet irodsMetaDataSet = new IRODSMetaDataSet(irodsProtocol);

		// add a file and set two metadata values
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		// build a query, then look at the tags that are created

		IRODSCommands commands = irodsFileSystem.commands;

		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = MetaDataSet.newCondition("COLL_TEST_ID",
				MetaDataCondition.EQUAL, "123");

		String[] fileds = { "COLL_TEST_ID" };
		MetaDataSelect[] select = MetaDataSet.newSelection(fileds);
		
		GenQueryClassicMidLevelService genQueryMidLevelService = GenQueryClassicMidLevelService.instance(commands);

		Tag queryTag = genQueryMidLevelService.buildQueryTag(condition, select, 100, 0,
				Namespace.FILE, true);

		irodsFileSystem.close();

		Assert.assertNotNull("null queryTag means message not built",
				queryTag);

	}

	@Test
	public final void testIRODSToJargonLookupForExtensibleMetaDataValue()
			throws Exception {
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource(
				"test_extended_icat_data.properties");
		source.generateExtensibleMetaDataMapping();
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		IRODSMetaDataSet irodsMetaDataSet = new IRODSMetaDataSet(irodsProtocol);

		String fieldId = IRODSMetaDataSet.getIDFromExtensibleMetaData("COL_TEST_ID");
		MetaDataField actualReverseField = IRODSMetaDataSet.getField(fieldId);
		String actualFieldName = actualReverseField.getName();

		Assert.assertEquals("expected to get field name from reverse lookup", "COL_TEST_ID", actualFieldName);

	}

}
