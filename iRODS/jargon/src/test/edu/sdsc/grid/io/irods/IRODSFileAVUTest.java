package edu.sdsc.grid.io.irods;


import java.util.Properties;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSFileAVUTest {
	 private static Properties testingProperties = new Properties();
	    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	    private static ScratchFileUtils scratchFileUtils = null;
	    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	    private static AssertionHelper assertionHelper = null;
	    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileAVUTest";


	    @BeforeClass
	    public static void setUpBeforeClass() throws Exception {
	        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
	        testingProperties = testingPropertiesLoader.getTestProperties();
	        scratchFileUtils = new ScratchFileUtils(testingProperties);
	        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
	        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
	        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
	        assertionHelper = new AssertionHelper();
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

}
