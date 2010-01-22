package edu.sdsc.grid.io.irods;

import java.net.URI;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalFileSystem;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSMultiThreadGetAndPutTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsMultiThreadGetAndPutTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;
	
	private static Logger log = LoggerFactory.getLogger(IRODSMultiThreadGetAndPutTest.class);


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

	@Before
	public void setUp() throws Exception {
		log.debug(">>>>>>>>>>>>>>>>>clearing test scratch directory area");
		scratchFileUtils.clearAndReinitializeScratchDirectory(IRODS_TEST_SUBDIR_PATH);
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testMultiThreadGetAndPut() throws Exception {
		// sleep is here to pause while profiler fires up
		Thread.sleep(30000);
		
		int nbrThreads = 6;
		// generate a set of test files in a common directory
		int nbrTestFiles = 80;
		int testFileLengthMin = 100;
		int testFileLengthMax = 1000;
		String testFileNamePrefix = "test";
		String testFileNameSuffix = ".abc";
		String sourceFileDir = "sourceFiles";
		String threadDirPrefix = "threaddir";
		List<String> sourceFileNames = new ArrayList<String>();
		List<TestPutLoop> threads = new ArrayList<TestPutLoop>();
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH
						+ '/' + sourceFileDir);

		String genFileName = "";
		String localFileName = "";

		// n number of random files in the source directory, with a random
		// length between the min and max

		for (int i = 0; i < nbrTestFiles; i++) {
			genFileName = testFileNamePrefix + i + testFileNameSuffix;
			localFileName = FileGenerator.generateFileOfFixedLengthGivenName(
					absPath, genFileName, FileGenerator.generateRandomNumber(
							testFileLengthMin, testFileLengthMax));
			sourceFileNames.add(localFileName);
		}

		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// spawn the threads, put each file to a per thread directory, then get
		// it back to a per thread return directory
		Thread testThread;
		String writeBackLocalDir;
		String threadIrodsTargetCollection;
		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);
		for (int i = 0; i < nbrThreads; i++) {
			writeBackLocalDir = scratchFileUtils
			.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH
					+ '/' + threadDirPrefix + i);
			threadIrodsTargetCollection = targetIrodsCollection + '/'
					+ threadDirPrefix + i;
			TestPutLoop testPutLoop = new TestPutLoop(irodsFileSystem,
					nbrTestFiles, writeBackLocalDir,
					threadIrodsTargetCollection, sourceFileNames);
			testThread = new Thread(testPutLoop);
			threads.add(testPutLoop);
			testThread.start();
		}

		// wait for stuff to finish
		boolean keepRunning = true;
		while (keepRunning) {
			log.debug(">>>>>>>>>>>>>>>>>>>>>>>>looping thru threads looking for finished");
			keepRunning = false;
			for (TestPutLoop testPutLoop : threads) {
				if (!testPutLoop.isFinished()) {
					log.debug(">>>>>>>>>>>>>>>>>>>>>>>>found an unfinished thread, wait some more");
					keepRunning = true;
					break;
				}
			}
			
			Thread.sleep(5000);
		}
		
		log.debug(">>>>>>>>>>>>>>>>>>>>>>>>im all finished");

		// check for any exceptions in the individual loops and die on first one
		for (TestPutLoop testPutLoop : threads) {
			if (testPutLoop.caughtException != null) {
				log.error("error in put/get loop", testPutLoop.caughtException);
				throw testPutLoop.caughtException;
			}
		}

		// now check each thread dir

	}

	class TestPutLoop implements Runnable {
		private final IRODSFileSystem irodsFileSystem;
		private final int iterations;
		private final String threadDir;
		private boolean finished = false;
		private Exception caughtException = null;
		private final List<String> sourceFileNames;
		private final String irodsTargetCollection;

		public TestPutLoop(IRODSFileSystem irodsFileSystem, int iterations,
				String threadDir, String irodsTargetCollection,
				List<String> sourceFileNames) {
			this.irodsFileSystem = irodsFileSystem;
			this.iterations = iterations;
			this.threadDir = threadDir;
			this.sourceFileNames = sourceFileNames;
			this.irodsTargetCollection = irodsTargetCollection;
		}

		public synchronized boolean isFinished() {
			return finished;
		}

		public synchronized void setFinished(boolean finished) {
			this.finished = finished;
		}

		public void run() {
			IRODSFile irodsFile = null;
			GeneralFile generalFile = null;
			
			try {
				
				// create the thread's collection
				irodsFile = new IRODSFile(irodsFileSystem, irodsTargetCollection);
				irodsFile.mkdirs();
				int ctr = 0;
				for (String fileName : sourceFileNames) {
					log.debug(">>>>>>>>>>>>>>>>>>>>>>>>" + Thread.currentThread().getName()
							+ " working on:" + fileName);

					irodsFile = new IRODSFile(irodsFileSystem, irodsTargetCollection + "/file" + ctr);
					generalFile = FileFactory.newFile(new URI("file:///" + fileName));
					irodsFile.copyFrom(generalFile, true);
					
					// now get the file and stick it back in local scratch for this particular thread
					generalFile = FileFactory.newFile(new URI("file:///" + threadDir + '/' + "/file" + ctr++));
					//generalFile.copyFrom(irodsFile);
					irodsFile.copyTo(generalFile, true);
					log.debug(">>>>>>>>irods file copied back at:" + generalFile.getAbsolutePath());
					
				}
				setFinished(true);
				log.debug(">>>>>>>>>>>>>>>>>>>>>>>>thread finished:" + Thread.currentThread().getName());
			} catch (Exception e) {
				setFinished(true);
				log.error("exception within put/get operation", e);
				log.error(".....irods file:" + irodsFile.getAbsolutePath());
				log.error(".....local file:" + generalFile.getAbsolutePath());
				caughtException = e;
			}
		}
	}

}
