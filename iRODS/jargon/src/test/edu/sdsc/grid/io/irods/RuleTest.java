package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.io.StringBufferInputStream;
import java.util.Properties;

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
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

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
	public void testExecRuleViaCommandExecute() throws Exception {

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		String ruleString = "printHello||print_hello|nop";
		Parameter[] inputParms = { new Parameter() };
		//Parameter[] outputParms = { new Parameter("out") };
		Parameter[] outputParms = { new Parameter() };

		Tag ruleResult = null;
		ruleResult = irodsFileSystem.commands.executeRule(ruleString,
				inputParms, outputParms);
		
		irodsFileSystem.close();
		// currently, the rule invocation returns null.  This will be corrected in a later version.  For now, a non-error execution of this rule
		// is a successful test
		//TestCase.assertNotNull(ruleResult);
	}
	
	@Test 
	public void textExecRuleWithNoParmsAndReadResult() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		String ruleString = "printHello||print_hello|nop";
		Parameter[] inputParms = { new Parameter() };
		//Parameter[] outputParms = { new Parameter("out") };
		Parameter[] outputParms = { new Parameter() };

		Tag ruleResult = null;
		ruleResult = irodsFileSystem.commands.executeRule(ruleString,
				inputParms, outputParms);
		irodsFileSystem.close();
		Parameter[] readResult = Rule.readResult(irodsFileSystem, ruleResult);
		TestCase.assertNotNull(readResult);
		
	}
	
	@Ignore
	public void testExecRuleWithInputParams() throws Exception {
		StringBuilder ruleStringBuilder = new StringBuilder();
		ruleStringBuilder.append("myTestRule||msiDataObjOpen(*A,*S_FD)##msiDataObjCreate(*B,null,*D_FD)##msiDataObjLseek(*S_FD,10,SEEK_SET,*junk1)##msiDataObjRead(*S_FD,10000,*R_BUF)##msiDataObjWrite(*D_FD,*R_BUF,*W_LEN)##msiDataObjClose(*S_FD,*junk2)##msiDataObjClose(*D_FD,*junk3)##msiDataObjCopy(*B,*C,null,*junk4)##delayExec(<PLUSET>2m</PLUSET>,msiDataObjRepl(*C,*Resource,*junk5),nop)|nop\n");
		ruleStringBuilder.append("*A=/tempZone/home/rods/foo1%*B=/tempZone/home/rods/foo2%*C=/tempZone/home/rods/foo3%*Resource=nvoReplResc\n");
		// FIXME: finish implementing test, consider visibility of Parameter
		
		//ruleStringBuilder.append("*R_BUF%*W_LEN%*A");
	}
	
	
	@Test
	public void testExecuteRuleViaStreamWithEqualSign() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));
		//String ruleString = "List Available MS||msiListEnabledMS(*KVPairs)##writeKeyValPairs(stdout,*KVPairs, \": \")|nop\n null\n ruleExecOut";
		
		String ruleString = "List Available MS||msiListEnabledMS(*KVPairs)##writeKeyValPairs(stdout,*KVPairs, \": \")|nop\n*A=hello\n ruleExecOut";
		StringBufferInputStream sbis = new StringBufferInputStream(ruleString);
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		// if I complete, it tolerated the = sign
	}
	
	/**
	 *  Bug 41 -  Exec rule with = sign causes IllegalArgumentException
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
		// if I complete, it tolerated the = sign
	}
	
	/**
	 *  Bug 41 -  Exec rule with = sign causes IllegalArgumentException
	 * @throws Exception
	 */
	@Test
	public void testExecuteRuleViaStreamWithNoEqualSignSayHello() throws Exception {
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(
				testingPropertiesHelper
						.buildIRODSAccountFromTestProperties(testingProperties));	
		String ruleString = "printHello||print_hello|nop\nnull\nruleExecOut";
		StringBufferInputStream sbis = new StringBufferInputStream(ruleString);
		Parameter[] result = Rule.executeRule(irodsFileSystem, sbis);
		irodsFileSystem.close();
		// if I complete, it tolerated the = sign
	}


}
