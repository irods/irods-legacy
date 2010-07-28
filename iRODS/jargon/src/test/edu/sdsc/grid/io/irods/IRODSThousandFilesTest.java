package edu.sdsc.grid.io.irods;

import java.util.Properties;

import junit.framework.TestCase;

import org.irods.jargon.core.query.GenQueryClassicMidLevelService;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class IRODSThousandFilesTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsThousandFilesTestParent";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	public static final String collDir = "coll";

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils.clearAndReinitializeScratchDirectory(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);

		// put in the thousand files
		String testFilePrefix = "thousandFileTest";
		String testFileSuffix = ".txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);

		FileGenerator.generateManyFilesInGivenDirectory(IRODS_TEST_SUBDIR_PATH
				+ '/' + collDir, testFilePrefix, testFileSuffix, 1000, 20, 500);  

		// put scratch files into irods in the right place
		// 1000 files from 20-500K size
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// make the put subdir
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		ImkdirCommand iMkdirCommand = new ImkdirCommand();
		iMkdirCommand.setCollectionName(targetIrodsCollection);
		invoker.invokeCommandAndGetResultAsString(iMkdirCommand);

		// put the files by putting the collection
		IputCommand iputCommand = new IputCommand();
		iputCommand.setForceOverride(true);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setLocalFileName(absPath + collDir);
		iputCommand.setRecursive(true);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		// now add avu's to each
		addAVUsToEachFile();

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

	public static final void addAVUsToEachFile() throws Exception {

		String avu1Attrib = "avu1";
		String avu1Value = "avu1value";
		String avu2Attrib = "avu2";
		int avuIncr = 0;

		String[] metaData = new String[2];
		metaData[0] = avu1Attrib;
		metaData[1] = avu1Value;

		String[] metaData2 = new String[2];

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
										+ collDir));

		// get a list of files underneath the top-level directory, and add some
		// avu's to each one

		String[] fileList = irodsFile.list();
		IRODSFile subFile = null;

		for (int i = 0; i < fileList.length; i++) {
			subFile = new IRODSFile(irodsFileSystem, irodsFile
					.getAbsolutePath()
					+ '/' + fileList[i]);
			subFile.modifyMetaData(metaData);

			metaData2[0] = avu2Attrib;
			metaData2[1] = String.valueOf(avuIncr++);
			subFile.modifyMetaData(metaData2);
		}

		irodsFileSystem.close();
	}

	@Test
	public void testSearchForAvuFiles() throws Exception {
		String avu1Attrib = "avu1";
		String avu1Value = "avu1value";

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		
		MetaDataSelect avu = IRODSMetaDataSet.newSelection(avu1Attrib);
		MetaDataSelect objNameSel = IRODSMetaDataSet.newSelection(IRODSMetaDataSet.FILE_NAME);
		MetaDataSelect[] sels = new MetaDataSelect[2];
		sels[0] = avu;
		sels[1] = objNameSel;
		MetaDataRecordList[] lists = irodsFileSystem.query(sels, 1000);
		// should have 1000 in this batch
		TestCase.assertEquals("did not get back the 1000 rows I requested", 1000, lists.length);
		// last entry should have a continuation
		MetaDataRecordList last = lists[999];
		TestCase.assertFalse("last row did not have expected continuation", last.isQueryComplete());
		// now test getting the next batch
		lists = last.getMoreResults(1000);
		// last entry has continuation again
		TestCase.assertEquals("did not get back the 1000 rows I requested", 1000, lists.length);
		// last entry should have a continuation
		last = lists[999];
		TestCase.assertFalse("last row did not have expected continuation", last.isQueryComplete()); 
		// requery again
		lists = last.getMoreResults(1000);
		irodsFileSystem.close();
		
	}
	
	@Test
	public void testSearchForAvuFilesWithPartialStart() throws Exception {
		String avu1Attrib = "avu1";
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		GenQueryClassicMidLevelService genQuery = GenQueryClassicMidLevelService.instance(irodsFileSystem.commands);
		MetaDataSelect avu = IRODSMetaDataSet.newSelection(avu1Attrib);
		MetaDataSelect objNameSel = IRODSMetaDataSet.newSelection(IRODSMetaDataSet.FILE_NAME);
		MetaDataSelect[] sels = new MetaDataSelect[2];
		sels[0] = avu;
		sels[1] = objNameSel;
		MetaDataCondition[] condition = new MetaDataCondition[0];
		MetaDataRecordList[] lists =genQuery.queryWithPartialStart(condition, sels, 1000, 0, Namespace.FILE, true);
		// should have 1000 in this batch
		TestCase.assertEquals("did not get back the 1000 rows I requested", 1000, lists.length);
		irodsFileSystem.close();
		
	}
	
	@Test
	public void testSearchForAvuFilesGetAllInOneLump() throws Exception {
		String avu1Attrib = "avu1";
		String avu1Value = "avu1value";

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		
		MetaDataSelect avu = IRODSMetaDataSet.newSelection(avu1Attrib);
		MetaDataSelect objNameSel = IRODSMetaDataSet.newSelection(IRODSMetaDataSet.FILE_NAME);
		MetaDataSelect[] sels = new MetaDataSelect[2];
		sels[0] = avu;
		sels[1] = objNameSel;
		MetaDataRecordList[] lists = irodsFileSystem.query(sels, 10000);
		// should have 1000 in this batch
		TestCase.assertTrue("did not get back the rows I requested", lists.length > 1000);
		// last entry should have a continuation
		MetaDataRecordList last = lists[lists.length -1];
		TestCase.assertFalse("last row had a continuation, should have all of them", last.isQueryComplete());
		irodsFileSystem.close();
		
	}

}
