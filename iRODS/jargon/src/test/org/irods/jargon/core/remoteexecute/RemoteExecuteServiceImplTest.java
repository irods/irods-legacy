package org.irods.jargon.core.remoteexecute;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Properties;

import junit.framework.Assert;

import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class RemoteExecuteServiceImplTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "RemoteExecuteServiceImplTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;


	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils
				.clearAndReinitializeScratchDirectory(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testInstance() throws Exception {

		String cmd = "hello";
		String args = "";
		String host = "host";
		String absPath = "/an/abs/path";

		IRODSCommands irodsCommands = Mockito.mock(IRODSCommands.class);

		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl
				.instance(irodsCommands, cmd, args, host, absPath);
		Assert.assertNotNull(remoteExecuteService);

	}

	@Test
	public final void testExecuteHello() throws Exception {

		String cmd = "hello";
		String args = "";
		String host = "";
		String absPath = "";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl
				.instance(irodsCommands, cmd, args, host, absPath);

		InputStream inputStream = remoteExecuteService.execute();

		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("did not successfully execute hello command",
				"Hello world  from irods".trim(), result.trim());

	}
	
	@Test
	public final void testExecuteHelloWithHost() throws Exception {

		String cmd = "hello";
		String args = "";
		String host = testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY);
		String absPath = "";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl
				.instance(irodsCommands, cmd, args, host, absPath);

		InputStream inputStream = remoteExecuteService.execute();

		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("did not successfully execute hello command",
				"Hello world  from irods".trim(), result.trim());

	}
	
	@Test(expected=JargonException.class)
	public final void testExecuteHelloWithBadHost() throws Exception {

		String cmd = "hello";
		String args = "";
		String host = "ImNotAHostWhyAreYouLookingAtMe";
		String absPath = "";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl
				.instance(irodsCommands, cmd, args, host, absPath);

	 remoteExecuteService.execute();

	}
	
	@Test
	public final void testExecuteHelloWithBadPath() throws Exception {

		String cmd = "hello";
		String args = "";
		String host = "";
		String absPath = "/I/am/not/a/path.txt";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl
				.instance(irodsCommands, cmd, args, host, absPath);

		InputStream inputStream = remoteExecuteService.execute();
		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("I should not have returned anything as the path was bad",
				"", result.trim());

	}
	
	@Test
	public final void testExecuteHelloWithPath() throws Exception {

		String cmd = "hello";
		String args = "";
		String host = "";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		
		String testFileName = "testExecuteHelloWithPath.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 300);

		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(localFileName);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);
		
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);
		IRODSCommands irodsCommands = irodsFileSystem.getCommands();
		RemoteExecutionService remoteExecuteService = RemoteExecuteServiceImpl
				.instance(irodsCommands, cmd, args, host, targetIrodsFile);

		InputStream inputStream = remoteExecuteService.execute();

		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("did not successfully execute hello command",
				"Hello world  from irods".trim(), result.trim());

	}

}
