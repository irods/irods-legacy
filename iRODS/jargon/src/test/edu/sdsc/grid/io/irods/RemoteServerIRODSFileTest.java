package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_HOST_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PASSWORD_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PORT_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_ZONE_KEY;

import java.net.URI;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class RemoteServerIRODSFileTest {
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
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
	 * Bug 78 - issues with getHomeDirectory after IRODS2.3 upgrade
	 */
	@Test
	public final void testGetHomeDir() throws Exception {
		
		String host = "seagrass.man.poznan.pl";
		int port = 1247;
		String user = "mike";
		String password = "testuser123";
		String resource = "demoResc";
		String zone = "PSNCZone";
		String homeDir = "/PSNCZone/home/mike";
		
		IRODSAccount account = new IRODSAccount(host, port, user, password, homeDir, zone, resource);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
		IRODSFile irodsFile = (IRODSFile) FileFactory.newFile(irodsFileSystem,
				irodsFileSystem.getHomeDirectory());
		boolean canWrite = irodsFile.canWrite();
		irodsFileSystem.close();
		TestCase.assertTrue("file should be writable", canWrite);

	}

	/**
	 * Bug 78 - issues with getHomeDirectory after IRODS2.3 upgrade
	 */
	@Test
	public final void testGetHomeDirMultipleTimes() throws Exception {

		String host = "seagrass.man.poznan.pl";
		int port = 1247;
		String user = "mike";
		String password = "testuser123";
		String resource = "demoResc";
		String zone = "PSNCZone";
		String homeDir = "/PSNCZone/home/mike";
		
		IRODSAccount account = new IRODSAccount(host, port, user, password, homeDir, zone, resource);
		
		int numberOfTimes = 30;

		for (int i = 0; i < numberOfTimes; i++) {

			IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);

			IRODSFile irodsFile = (IRODSFile) FileFactory.newFile(
					irodsFileSystem, irodsFileSystem.getHomeDirectory());
			boolean canWrite = irodsFile.canWrite();
			irodsFileSystem.close();
			TestCase.assertTrue("file should be writable", canWrite);
		}

	}

}
