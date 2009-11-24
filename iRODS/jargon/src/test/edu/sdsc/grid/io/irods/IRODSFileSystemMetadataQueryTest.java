package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

import junit.framework.TestCase;

public class IRODSFileSystemMetadataQueryTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileSystemMetadataQueryTest";
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

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testCreateAndModifyDate() throws Exception {
		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryModDate1.txt";

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);
		IRODSFile irodsFile = new IRODSFile(irodsFileSystem,
				testingPropertiesHelper
						.buildIRODSCollectionAbsolutePathFromTestProperties(
								testingProperties, IRODS_TEST_SUBDIR_PATH));

		String filename = irodsFile.getName();
		long size = irodsFile.length();

		// query(String[]) defined in GeneralFile
		// will call query(String[] conditions, String[] selects) in IRODSFile,
		// with null in conditions and these
		// values in selects. The values in the selects are abstract data names
		// that are translated
		// into IRODS-specific query symbols in IRODSMetaDataSet

		// trickles down to IRODSCommands.query()

		MetaDataRecordList[] lists = irodsFile.query(new String[] {
				IRODSMetaDataSet.CREATION_DATE,
				IRODSMetaDataSet.MODIFICATION_DATE });

		for (MetaDataRecordList l : lists) {
			String createDate = l.getStringValue(0);
			String modDate = l.getStringValue(1);

			// demonstration code for getting real dates from irods date, left for reference
			TimeZone timeZone = TimeZone.getTimeZone("GMT");
			TimeZone easternTimeZone = TimeZone.getTimeZone("GMT-5:00");
			DateFormat dateFormat = DateFormat.getDateTimeInstance();
			dateFormat.setTimeZone(timeZone);
			Calendar calendar = Calendar.getInstance();
			// calendar.setTimeZone(timeZone);
			calendar.setTimeInMillis(0L);
			//System.out.println("epoch date: " + dateFormat.format(calendar.getTime()));
			calendar.add(Calendar.SECOND, Integer.parseInt(createDate));
			Date computedDate = calendar.getTime();
			dateFormat.setTimeZone(easternTimeZone);
			dateFormat.format(computedDate);
			//System.out.println("create date: " + dateFormat.format(computedDate.getTime()));

			TestCase.assertEquals("create and mod date not equal", createDate,
					modDate);
		}

		irodsFileSystem.close();

	}

	@Test
	public void testQueryFileNameWithLike() throws Exception {

		IRODSAccount account = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		String testFileName = "testQueryFileName1.txt";

		// generate a file and put into irods
		String fullPathToTestFile = FileGenerator
				.generateFileOfFixedLengthGivenName(testingProperties
						.getProperty(GENERATED_FILE_DIRECTORY_KEY)
						+ IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);

		IputCommand iputCommand = new IputCommand();
		iputCommand.setLocalFileName(fullPathToTestFile);
		iputCommand.setIrodsFileName(testingPropertiesHelper
				.buildIRODSCollectionRelativePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		iputCommand.setForceOverride(true);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		String keyword = "testQueryFileName1%";
		MetaDataCondition[] condition = new MetaDataCondition[1];
		condition[0] = IRODSMetaDataSet.newCondition(
				IRODSMetaDataSet.FILE_NAME, MetaDataCondition.LIKE, keyword);
		String[] fileds = { IRODSMetaDataSet.FILE_NAME,
				IRODSMetaDataSet.DIRECTORY_NAME, IRODSMetaDataSet.SIZE };
		MetaDataSelect[] select = IRODSMetaDataSet.newSelection(fileds);
		MetaDataRecordList[] fileList = irodsFileSystem.query(condition,
				select, 100);
		TestCase.assertTrue(
				"did not return query result for file I just added",
				fileList != null);

		irodsFileSystem.close();

	}

}
