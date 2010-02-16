package edu.sdsc.grid.io.irods;

//FIXME: add to suite

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSAvuTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSAvuTest";
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
	public void testCheckAvuNoMetadataInAvu() throws Exception {
		String[] fileds = { IRODSMetaDataSet.FILE_NAME,
				IRODSMetaDataSet.DIRECTORY_NAME };
		MetaDataSelect[] selects = IRODSMetaDataSet.newSelection(fileds);
		MetaDataCondition[] conditions = new MetaDataCondition[1];
		conditions[0] = IRODSMetaDataSet.newCondition("attr1", MetaDataCondition.EQUAL, "val1");
		String[] selectedAVU = new String[selects.length];


		IRODSAvu.checkForAVU(conditions, selects, Namespace.FILE, selectedAVU);
		TestCase.assertEquals("did not get the selects back", selects.length, selectedAVU.length);
		

	}

}
