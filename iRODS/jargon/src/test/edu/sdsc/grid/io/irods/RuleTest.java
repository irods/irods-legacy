package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.io.FileNotFoundException;
import java.io.StringBufferInputStream;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;

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
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class RuleTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "RuleTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
	private static AssertionHelper assertionHelper = null;

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
	}

	@After
	public void tearDown() throws Exception {
	}

	@Test
	public void testExecuteRuleViaStreamWithEqualSign() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		
		String ruleString = "List Available MS||msiListEnabledMS(*KVPairs)##writeKeyValPairs(stdout,*KVPairs, \": \")|nop\n*A=hello\n ruleExecOut";
		StringBufferInputStream sbis = new StringBufferInputStream(ruleString);
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		// if I complete, it tolerated the = sign, this rule is just to check
		// parsing of attributes
	}

	/**
	 * Bug 41 - Exec rule with = sign causes IllegalArgumentException
	 * 
	 * @throws Exception
	 */
	@Test
	public void testExecuteRuleViaStreamWithNoEqualSign() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		String ruleString = "List Available MS||msiListEnabledMS(*KVPairs)##writeKeyValPairs(stdout,*KVPairs, \": \")|nop\nnull\n ruleExecOut";
		StringBufferInputStream sbis = new StringBufferInputStream(ruleString);
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		// if I complete, it tolerated the = sign, this rule is just to check
		// parsing of attributes
	}

	/**
	 * Bug 41 - Exec rule with = sign causes IllegalArgumentException
	 * 
	 * @throws Exception
	 */
	@Test
	public void testExecuteRuleViaStreamWithNoEqualSignSayHello()
			throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		String ruleString = "printHello||print_hello|nop\nnull\nruleExecOut";
		StringBufferInputStream sbis = new StringBufferInputStream(ruleString);
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();

		TestCase.assertNotNull("null response, no data back from rule", result);
		TestCase.assertTrue(
				"I expected to get the ruleExec out from this hello command",
				result.length > 0);
		TestCase.assertTrue("did not get hello in response", result[0]
				.getStringValue().indexOf("Hello") > -1);

	}

	@Test
	public void testParseWithEqualsInCondition() throws Exception {
		String attribTestLine = "*Condition=COLL_NAME='/Bergen/home/csaba/01f760ac-7084-4233-93db-659a8b2d5c9f'";
		StringTokenizer tokenizer = new StringTokenizer(attribTestLine);
		Vector<Parameter> parameters = new Vector<Parameter>();
		Rule.processRuleAttributesLine(tokenizer, parameters);
		TestCase.assertTrue(parameters.size() > 0);
		TestCase.assertEquals("did not parse out parameter name", "*Condition",
				parameters.get(0).getUniqueName());
		TestCase
				.assertEquals(
						"Did not get entire condition as value",
						"COLL_NAME='/Bergen/home/csaba/01f760ac-7084-4233-93db-659a8b2d5c9f'",
						parameters.get(0).getStringValue());
	}

	@Test
	public void testRuleContainsConditionWithEqualsInAttrib() throws Exception {

		// put a collection out to do a checksum on
		String testFileName = "testRuleChecksum1.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				100);

		// put scratch file into irods in the right place
		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		StringBuilder fileNameAndPath = new StringBuilder();
		fileNameAndPath.append(absPath);

		fileNameAndPath.append(testFileName);

		iputCommand.setLocalFileName(fileNameAndPath.toString());
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		StringBuilder ruleBuilder = new StringBuilder();
		ruleBuilder
				.append("myTestRule||acGetIcatResults(*Action,*Condition,*B)##forEachExec(*B,msiGetValByKey(*B,RESC_LOC,*R)##remoteExec(*R,null,msiDataObjChksum(*B,*Operation,*C),nop)##msiGetValByKey(*B,DATA_NAME,*D)##msiGetValByKey(*B,COLL_NAME,*E)##writeLine(stdout,CheckSum of *E/*D at *R is *C),nop)|nop##nop\n");
		ruleBuilder.append("*Action=chksumRescLoc%*Condition=COLL_NAME = '");
		ruleBuilder.append(iputCommand.getIrodsFileName());
		ruleBuilder.append("'%*Operation=ChksumAll\n");
		ruleBuilder.append("*Action%*Condition%*Operation%*C%ruleExecOut");
		String ruleString = ruleBuilder.toString();

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		StringBufferInputStream sbis = new StringBufferInputStream(ruleString);
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		TestCase.assertNotNull("did not get a response", result);
		TestCase.assertEquals("did not get results for each output parameter",
				5, result.length);

		// I need to find the parameter for the condition, so I can see if the
		// full condition with the = sign was preserved
		boolean foundCondition = false;
		for (int i = 0; i < result.length; i++) {
			if (result[i].getUniqueName().equals("*Condition")) {
				foundCondition = true;
				TestCase.assertEquals(
						"did not get the condition, including the = sign",
						"COLL_NAME = '" + iputCommand.getIrodsFileName() + "'",
						result[i].getStringValue());

			}
		}

		TestCase
				.assertTrue(
						"did not find the 'condition' returned as output from the rule",
						foundCondition);
	}

	@Test
	public void testExecuteGenQueryProcessAsArray() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		StringBuilder builder = new StringBuilder();
		builder
				.append("testExecReturnArray||msiMakeGenQuery(\"RESC_NAME, RESC_LOC\",*Condition,*GenQInp)##msiExecGenQuery(*GenQInp, *GenQOut)|nop\n");
		builder.append("*Condition=RESC_NAME > 'a'\n");
		builder.append("*GenQOut");
		StringBufferInputStream sbis = new StringBufferInputStream(builder
				.toString());
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		TestCase.assertNotNull("null response, no data back from rule", result);
		TestCase.assertEquals(result[0].getType(), "GenQueryOut_PI");

	}
	
	
	@Test
	public void testFileSystemExecuteRuleReturnString() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		StringBuilder builder = new StringBuilder();
		builder
				.append("testExecReturnArray||msiMakeGenQuery(\"RESC_NAME, RESC_LOC\",*Condition,*GenQInp)##msiExecGenQuery(*GenQInp, *GenQOut)|nop\n");
		builder.append("*Condition=RESC_NAME > 'a'\n");
		builder.append("*GenQOut");
		StringBufferInputStream sbis = new StringBufferInputStream(builder
				.toString());
		
		Map<String,String> response = irodsFileSystem.executeRule(sbis);
		
		//FIXME:  just got the string "GenQueryOut_PI", is this right?
		
		irodsFileSystem.close();
		TestCase.assertNotNull("null response, no data back from rule", response);

	}
	

	@Test
	public void testExecuteRequestClientActionGetStringValue()
			throws Exception {
		// create a local file to put
		// put a collection out to do a checksum on
		String testFileName = "testClientAction.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String putFileName = FileGenerator.generateFileOfFixedLengthGivenName(
				absPath, testFileName, 1);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		StringBuilder builder = new StringBuilder();
		builder.append("testClientAction||msiDataObjPut(");
		builder.append(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		builder.append('/');
		builder.append("foo.txt,null,");
		builder.append(putFileName);
		builder.append(",*status)|nop\n");
		builder.append("*A=null\n");
		builder.append("*ruleExecOut");
		StringBufferInputStream sbis = new StringBufferInputStream(builder
				.toString());
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		TestCase.assertEquals("expected the result of the put in the parms", 1,
				result.length);
		String resVal = result[0].getStringValue();
		TestCase.assertTrue("should have the uri of the file", resVal
				.indexOf("foo.txt") > -1);

	}
	
	@Test
	public void testExecuteRequestClientActionGetObjectValue()
			throws Exception {
		// create a local file to put
		// put a collection out to do a checksum on
		String testFileName = "testClientActionGetObject.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String putFileName = FileGenerator.generateFileOfFixedLengthGivenName(
				absPath, testFileName, 1);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		StringBuilder builder = new StringBuilder();
		builder.append("testClientAction||msiDataObjPut(");
		builder.append(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		builder.append('/');
		builder.append("bar.txt,null,");
		builder.append(putFileName);
		builder.append(",*status)|nop\n");
		builder.append("*A=null\n");
		builder.append("*ruleExecOut");
		StringBufferInputStream sbis = new StringBufferInputStream(builder
				.toString());
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		TestCase.assertEquals("expected the result of the put in the parms", 1,
				result.length);
		Object resVal = result[0].getValue();
		TestCase.assertTrue("I should have an irods file, which is my request result", resVal instanceof IRODSFile);
		

	}

	@Test(expected = FileNotFoundException.class)
	public void testExecuteQueryRequestClientPutActionUserFileNotExists()
			throws Exception {
		// create a local file to put
		// put a collection out to do a checksum on

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));

		StringBuilder builder = new StringBuilder();
		builder.append("testClientAction||msiDataObjPut(");
		builder.append(testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH));
		builder.append('/');
		builder.append("foo.txt,null,");
		builder.append("/nope/notachance.jpg");
		builder.append(",*status)|nop\n");
		builder.append("*A=null\n");
		builder.append("*ruleExecOut");

		StringBufferInputStream sbis = new StringBufferInputStream(builder
				.toString());

		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();

		TestCase.assertNotNull("null response, no data back from rule", result);

	}

}
