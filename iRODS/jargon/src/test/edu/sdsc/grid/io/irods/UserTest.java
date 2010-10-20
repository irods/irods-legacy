package edu.sdsc.grid.io.irods;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandException;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.AddUserCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ListUserDnCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ListUsersCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.RemoveUserCommand;

public class UserTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
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

	@Test
	public final void testListSubjects() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		User user = new User(irodsFileSystem);
		String[] subjects = user.listSubjects();
		Assert.assertTrue("no subjects returned", subjects.length > 0);
	}

	@Test
	public final void testAddUserStringString() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		String testUser = "addUserTestUser";

		// setup, delete user if it exists

		try {
			RemoveUserCommand command = new RemoveUserCommand();
			command.setUserName(testUser);
			IrodsInvocationContext invocationContext = testingPropertiesHelper
					.buildIRODSInvocationContextFromTestProperties(testingProperties);
			IcommandInvoker invoker = new IcommandInvoker(invocationContext);
			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore exception, user may not exist
		}

		User user = new User(irodsFileSystem);
		user.addUser(testUser, "rodsuser");
		irodsFileSystem.close();
		// make sure the user was added
		// set up a test dir to remove via command
		ListUsersCommand command = new ListUsersCommand();
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		String result = invoker.invokeCommandAndGetResultAsString(command);
		Assert.assertTrue("did not find user I just added", result
				.indexOf(testUser) > -1);

		// cleanup...delete the user, also provides an additional check that the
		// user had indeed been added

		invoker.invokeCommandAndGetResultAsString(command);

	}

	@Test
	public final void testAddUserStringStringString() throws Exception {
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
		String testUser = "addUserTestUserDn";
		String expectedDn = "testDn";

		// setup, delete user if it exists

		try {
			RemoveUserCommand command = new RemoveUserCommand();
			command.setUserName(testUser);
			IrodsInvocationContext invocationContext = testingPropertiesHelper
					.buildIRODSInvocationContextFromTestProperties(testingProperties);
			IcommandInvoker invoker = new IcommandInvoker(invocationContext);
			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore exception, user may not exist
		}

		User user = new User(irodsFileSystem);
		user.addUser(testUser, "rodsuser", expectedDn);
		irodsFileSystem.close();
		// make sure the user was added
		// set up a test dir to remove via command
		ListUsersCommand command = new ListUsersCommand();
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		String result = invoker.invokeCommandAndGetResultAsString(command);
		Assert.assertTrue("did not find user I just added", result
				.indexOf(testUser) > -1);

		// skip this for irods2.1, icommand does not work

		if (!irodsFileSystem.commands.getIrodsServerProperties().getRelVersion().equals(
				"rods2.1")) {

			// verify the DN
			ListUserDnCommand dnCommand = new ListUserDnCommand();
			dnCommand.setUserDn(expectedDn);
			result = invoker.invokeCommandAndGetResultAsString(dnCommand);
			Assert.assertTrue("did not find user dn", result
					.indexOf(expectedDn) > -1);
		}

		// cleanup...delete the user, also provides an additional check that the
		// user had indeed been added

		RemoveUserCommand removeCommand = new RemoveUserCommand();
		removeCommand.setUserName(testUser);
		result = invoker.invokeCommandAndGetResultAsString(removeCommand);
	}

	@Test
	public final void testDeleteUser() throws Exception {
		String testUser = "testDeleteUser";
		String testUserType = "rodsuser";
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		// add the user, may already be there so ignore any error
		try {
			AddUserCommand command = new AddUserCommand();
			command.setUserName(testUser);
			command.setUserType(testUserType);
			IrodsInvocationContext invocationContext = testingPropertiesHelper
					.buildIRODSInvocationContextFromTestProperties(testingProperties);
			IcommandInvoker invoker = new IcommandInvoker(invocationContext);
			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore, might already be present
		}

		// now delete
		User user = new User(irodsFileSystem);
		user.deleteUser(testUser);
		irodsFileSystem.close();

		// user should not be present
		ListUsersCommand command = new ListUsersCommand();
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		String result = invoker.invokeCommandAndGetResultAsString(command);
		Assert.assertTrue("user was not deleted",
				result.indexOf(testUser) == -1);

	}

	@Test
	public final void testModifyDN() throws Exception {
		String testUser = "testModDnUser";
		String testUserType = "rodsuser";
		String expectedDn = "dnTestModifyDn";
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// add the user, may already be there so ignore any error
		try {
			AddUserCommand command = new AddUserCommand();
			command.setUserName(testUser);
			command.setUserType(testUserType);

			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore, might already be present
		}
		
		//  currently not working with 2.1
		
		if (irodsFileSystem.commands.getIrodsServerProperties().getRelVersion().equals("rods2.1")) {
			return;
		}

		// modify the DN for the user
		User user = new User(irodsFileSystem);
		user.modifyDN(testUser, expectedDn);
		irodsFileSystem.close();

		// verify
		ListUserDnCommand dnCommand = new ListUserDnCommand();
		dnCommand.setUserDn(expectedDn);
		String result = invoker.invokeCommandAndGetResultAsString(dnCommand);
		Assert.assertTrue("did not find user dn",
				result.indexOf(expectedDn) > -1);

		// cleanup, delete user
		RemoveUserCommand removeCommand = new RemoveUserCommand();
		removeCommand.setUserName(testUser);
		result = invoker.invokeCommandAndGetResultAsString(removeCommand);

	}

	@Test
	public final void testModifyComment() throws Exception {
		String testUser = "testModCommentUser";
		String testUserType = "rodsuser";
		String expectedComment = "I am testing that I can modify this comment";
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// add the user, may already be there so ignore any error
		try {
			AddUserCommand command = new AddUserCommand();
			command.setUserName(testUser);
			command.setUserType(testUserType);

			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore, might already be present
		}

		// modify the DN for the user
		User user = new User(irodsFileSystem);
		user.modifyComment(testUser, expectedComment);
		irodsFileSystem.close();

		// verify

		ListUsersCommand command = new ListUsersCommand();
		command.setUserName(testUser);
		String result = invoker.invokeCommandAndGetResultAsString(command);
		Assert.assertTrue("did not find new comment", result
				.indexOf(expectedComment) > -1);

		// cleanup, delete user
		RemoveUserCommand removeCommand = new RemoveUserCommand();
		removeCommand.setUserName(testUser);
		result = invoker.invokeCommandAndGetResultAsString(removeCommand);
	}

	@Test
	public final void testModifyInfo() throws Exception {
		String testUser = "testModInfoUser";
		String testUserType = "rodsuser";
		String expectedInfo = "I am testing that I can modify this info";
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// add the user, may already be there so ignore any error
		try {
			AddUserCommand command = new AddUserCommand();
			command.setUserName(testUser);
			command.setUserType(testUserType);

			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore, might already be present
		}

		// modify the DN for the user
		User user = new User(irodsFileSystem);
		user.modifyInfo(testUser, expectedInfo);
		irodsFileSystem.close();

		// verify

		ListUsersCommand command = new ListUsersCommand();
		command.setUserName(testUser);
		String result = invoker.invokeCommandAndGetResultAsString(command);
		Assert.assertTrue("did not find new comment", result
				.indexOf(expectedInfo) > -1);

		// cleanup, delete user
		RemoveUserCommand removeCommand = new RemoveUserCommand();
		removeCommand.setUserName(testUser);
		result = invoker.invokeCommandAndGetResultAsString(removeCommand);
	}
	
	/**
	 *  Bug 83 -  user.modifyPassword fails in Jargon
	 *  
	 *  Currently ignored as this 
	 *  
	 * @throws Exception
	 */
	@Ignore
	public final void testModifyPassword() throws Exception {
		String testUser = "testModPasswordUser";
		String testUserType = "rodsuser";
		String testPassword = "password";
		IRODSAccount testAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IcommandInvoker invoker = new IcommandInvoker(invocationContext);

		// add the user, may already be there so ignore any error
		try {
			AddUserCommand command = new AddUserCommand();
			command.setUserName(testUser);
			command.setUserType(testUserType);

			String result = invoker.invokeCommandAndGetResultAsString(command);
		} catch (IcommandException ice) {
			// ignore, might already be present
		}

		// modify the password for the user
		User user = new User(irodsFileSystem);
		user.modifyPassword(testUser, testPassword);
		irodsFileSystem.close();

		// verify by trying to log in as this user

		String host = testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY);
		int port = testingPropertiesHelper.getPortAsInt(testingProperties);
		String zone = testingProperties.getProperty(TestingPropertiesHelper.IRODS_ZONE_KEY);
		String resource = testingProperties.getProperty(TestingPropertiesHelper.IRODS_RESOURCE_KEY);
		String homeDir = '/' + zone + "/home/" + testUser + '/';
		
		// do a login with the modified password
		IRODSAccount loginAccount = new IRODSAccount(host, port, testUser, testPassword, homeDir,zone, resource);
		IRODSFileSystem testLoginFileSystem = new IRODSFileSystem(loginAccount);
		testLoginFileSystem.close();
		
		// cleanup, delete user
		RemoveUserCommand removeCommand = new RemoveUserCommand();
		removeCommand.setUserName(testUser);
		invoker.invokeCommandAndGetResultAsString(removeCommand);
	}
	
}
