package org.irods.jargon.core.functionaltest;

import java.io.File;
import java.io.FileInputStream;
import java.util.Date;
import java.util.Properties;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSFile;
import edu.sdsc.grid.io.irods.IRODSFileOutputStream;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class CreateModAVUReplicateRenameFunctionalTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "CreateModAVUReplicateRenameFunctionalTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		
		String targetIrodsCollection = testingPropertiesHelper
		.buildIRODSCollectionAbsolutePathFromTestProperties(
				testingProperties, IRODS_TEST_SUBDIR_PATH);
		
		IRODSFile testColl = new IRODSFile(irodsFileSystem, targetIrodsCollection);
		testColl.delete(true);
		testColl.mkdirs();
	
		//irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		//irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		//irodsTestSetupUtilities
		//		.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
		//assertionHelper = new AssertionHelper();
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

	/*
	 * [#160] Files with no ACL in complex create new, stream, set metadata,
	 * delay exec
	 */
	@Test
	public final void testCollectionLooseACLScenario() throws Exception {
		// generate a local scratch file
		String testFileNamePrefix = "testCollectionLooseACLScenario";
		String testAvuNamePrefix = "testCollectionLooseACLScenarioAVUName";
		String testAvuValuePrefix = "testCollectionLooseACLScenarioAVUValue";
		String testAvuValuePrefixNew = "newTestCollectionLooseACLScenarioAVUValue";

		String tempPrefix = "temp";
		int numberIterations = 1000;
		int fileLength = 20 * 1024;

		// + new Date().getTime() +
		String testFileNameSuffix = ".txt";
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		/*
		 * Runnable r=new Runnable() { public void run() {
		 * 
		 * 
		 * 
		 * 
		 * } }; Thread t=new Thread(r); t.start();
		 */

		long fileId = 0;
		String testFileNameTemp;
		String testFileNamePerm;
		IRODSFileOutputStream fos = null;
		File localFile = null;
		FileInputStream localfis = null;
		IRODSFile irodsFile = null;
		IRODSFile permIrodsFile = null;
		for (int i = 0; i < numberIterations; i++) {
			fileId = new Date().getTime();
			testFileNameTemp = testFileNamePrefix + fileId + tempPrefix
					+ testFileNameSuffix;
			testFileNamePerm = testFileNamePrefix + fileId + testFileNameSuffix;
			FileGenerator.generateFileOfFixedLengthGivenName(absPath,
					testFileNameTemp, fileLength);
			FileGenerator.generateFileOfFixedLengthGivenName(absPath,
					testFileNamePerm, fileLength);
			permIrodsFile = new IRODSFile(irodsFileSystem,
					targetIrodsCollection + '/' + testFileNamePerm);
			localFile = new File(absPath + "/" + testFileNamePerm);

			// create a permanent file that the temp will delete and replace
			permIrodsFile.copyFrom(new LocalFile(localFile));

			// 1. create (using IRODSFile.createNewFile()) a temporary empty
			// file, say temp1
			irodsFile = new IRODSFile(irodsFileSystem, targetIrodsCollection
					+ '/' + testFileNameTemp);
			irodsFile.createNewFile();
			fos = new IRODSFileOutputStream(irodsFile);
			localFile = new File(absPath + "/" + testFileNameTemp);
			localfis = new FileInputStream(localFile);

			// 2. stream data to the empty file temp1

			int offset = 0;
			int numRead = 0;
			byte[] buff = new byte[1024];
			while ((numRead = localfis.read(buff, offset, buff.length - offset)) >= 0) {
				fos.write(buff, 0, numRead);
			}

			// Close our input stream
			localfis.close();
			fos.close();

			// 3. add file meta data (AVU's) to file temp1
			String[] metaData = { testAvuNamePrefix, testAvuValuePrefix };
			irodsFile.modifyMetaData(metaData);

			// 4. if permanent file already exists i.e. a file update
			// delete (using IRODSFile.delete(true)) the current corresponding
			// permanent
			permIrodsFile.delete(true);

			// 5. rename (using IRODSFile.renameTo(...)) temp1 to perm1
			irodsFile.renameTo(permIrodsFile);

			// 6. replicate (using IRODSFileSystem.executeRule(REPLICATION RULE

			/*
			 * delayExec(<EF>5m DOUBLE UNTIL SUCCESS OR 10
			 * TIMES</EF>,msiDataObjRepl(*filename, *resource,*status),nop)
			 * fileName=<filename>%*resource=destRescName=<resource group
			 * name>++++all=ruleExecOut
			 */

			StringBuilder ruleBuilder = new StringBuilder();
			ruleBuilder
					.append("testdelay||delayExec(<PLUSET>1m</PLUSET> ,msiDataObjRepl(*filename, *resource,*status),nop)|nop\n");
			ruleBuilder.append("*filename=");
			ruleBuilder.append(permIrodsFile.getAbsolutePath());
			ruleBuilder.append("%*resource=");
			ruleBuilder
					.append(testingProperties
							.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));

			ruleBuilder.append("\n");
			ruleBuilder.append("*ruleExecOut");
			ruleBuilder.toString();

			// Map<String,String> ruleResult =
			// irodsFileSystem.executeRule(ruleString);

			// 7. update an AVU (file_status from 'not_ready' to 'ready') for
			// perm1
			String[] newMetaData = { testAvuNamePrefix, testAvuValuePrefixNew };
			permIrodsFile.modifyMetaData(newMetaData);
		}

		irodsFileSystem.close();
	}

}
