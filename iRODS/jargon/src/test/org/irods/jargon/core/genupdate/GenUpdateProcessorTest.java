/**
 * 
 */
package org.irods.jargon.core.genupdate;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.query.ExtensibleMetaDataMapping;
import org.irods.jargon.core.query.ExtensibleMetaDataSource;
import org.irods.jargon.core.query.ExtensibleMetadataPropertiesSource;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;

/**
 * Test the gen update processor
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class GenUpdateProcessorTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		assertionHelper = new AssertionHelper();
	}

	/**
	 * @throws java.lang.Exception
	 */
	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public void testCreateInstance() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		GenUpdateProcessor genUpdateProcessor = GenUpdateProcessor
				.instance(irodsFileSystem.getCommands());
		Assert.assertNotNull(genUpdateProcessor);
	}

	@Ignore //FIXME: currently fails, checking on packing instruction....
	public void testInsertOneField() throws Exception {
		ExtensibleMetaDataSource source = new ExtensibleMetadataPropertiesSource(
				"test_extended_icat_data.properties");
		ExtensibleMetaDataMapping mapping = source
				.generateExtensibleMetaDataMapping();
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		GenUpdateProcessor genUpdateProcessor = GenUpdateProcessor
				.instance(irodsFileSystem.getCommands());
		
		List<GenUpdateFieldValuePair> fvps = new ArrayList<GenUpdateFieldValuePair>();
		fvps.add(GenUpdateFieldValuePair.instance("COL_TEST_NAME", "name"));
		
		genUpdateProcessor.processGeneralUpdateOrInsert(fvps);		
		irodsFileSystem.close();

	}

}
