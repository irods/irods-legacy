package edu.sdsc.grid.io.irods;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.exception.JargonRuntimeException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.StandardMetaData;
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

	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public void testAddConditionsBasedOnAVUSelects() throws Exception {
		List<IRODSMetaDataSelectWrapper> translatedSelects = new ArrayList<IRODSMetaDataSelectWrapper>();
		IRODSMetaDataSelectWrapper irodsMetaDataSelectWrapper = null;

		String[] fileds = { StandardMetaData.FILE_NAME,
				StandardMetaData.DIRECTORY_NAME };
		MetaDataSelect[] selects = MetaDataSet.newSelection(fileds);

		for (int i = 0; i < selects.length; i++) {
			try {
				irodsMetaDataSelectWrapper = new IRODSMetaDataSelectWrapper(
						selects[i]);
				irodsMetaDataSelectWrapper.setSelectType(IRODSMetaDataSelectWrapper.SelectType.IRODS_GEN_QUERY_METADATA);
				irodsMetaDataSelectWrapper.setTranslatedMetaDataNumber("600");
				translatedSelects.add(irodsMetaDataSelectWrapper);
			} catch (JargonException e) {
				String msg = "error translating an IRODS select into an IRODSMetaDataSelectWrapper";
				throw new JargonRuntimeException(msg, e);
			}

		}
		
	}
	
}
