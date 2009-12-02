package edu.sdsc.grid.io;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;
import static org.junit.Assert.*;

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.irods.IRODSMetaDataSet;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class GeneralFileSystemTest {

	protected static Properties testingProperties = new Properties();
	protected static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	protected static ScratchFileUtils scratchFileUtils = null;
	public static String IRODS_TEST_SUBDIR_PATH = "GeneralFileSystemTest";
	protected static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);

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
	 * test relevant to BUG: 36 resource added with icommand does not show up in
	 * Jargon query
	 *
	 * FIXME: this test really needs to add the resource in code, requiring login of 'rods' user?
	 * @throws Exception
	 */
	@Test
	public final void testQueryMetaDataSelectArray() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		GeneralFileSystem irodsFileSystem = FileFactory.newFileSystem(account);

		MetaDataRecordList[] lists = irodsFileSystem
				.query(new String[] { IRODSMetaDataSet.COLL_RESOURCE_NAME });

		boolean foundResc1 = false;
		boolean foundResc2 = false;

		for (MetaDataRecordList l : lists) {
			String resource = l.getStringValue(0);

			if (resource.equals(testingProperties
					.getProperty(IRODS_RESOURCE_KEY))) {
				foundResc1 = true;
			} else if (resource.equals(testingProperties
					.getProperty(IRODS_SECONDARY_RESOURCE_KEY))) {
				foundResc2 = true;
			}
		}

		TestCase.assertTrue("did not find first resource", foundResc1);
		TestCase.assertTrue("did not find second resource", foundResc2);
	}

}
