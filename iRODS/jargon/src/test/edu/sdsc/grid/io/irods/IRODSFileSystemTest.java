package edu.sdsc.grid.io.irods;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.*;
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

import java.util.Properties;

import junit.framework.TestCase;


public class IRODSFileSystemTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSFileSystemTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
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
    public void testBasicCanCreateValidIrodsFileSystem() throws Exception {
    	
    	 IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
         IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
         String testFileName = "testBasicCanCreateValidIrodsFileSystem.txt";
         
         // generate a file and put into irods
         String fullPathToTestFile = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                 GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);
         
         IputCommand iputCommand = new IputCommand();
         iputCommand.setLocalFileName(fullPathToTestFile);
         iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH ));
         iputCommand.setForceOverride(true);

         IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
         IcommandInvoker invoker = new IcommandInvoker(invocationContext);
         invoker.invokeCommandAndGetResultAsString(iputCommand);
          
         IRODSFile irodsFile = new IRODSFile(irodsFileSystem, 
        		 testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH) + '/' + testFileName);
        
    	 TestCase.assertTrue("file system not aware this is a file", irodsFile.isFile());
    	 TestCase.assertFalse("thinks this is a file and a directory", irodsFile.isDirectory());
    	 TestCase.assertEquals("does not have the resource from the account", account.getDefaultStorageResource(), irodsFile.getResource());
    	 irodsFileSystem.close();
         
    }
    
    @Test(expected=SecurityException.class)
    public void testCreateWithDefaultConstructorNoGsi() throws Exception {
    	IRODSFileSystem IRODSFileSystem = new IRODSFileSystem();
    }
    
    @Test
    public void testBasicCanCreateValidIrodsFileSystemAsCollection() throws Exception {
    	
    	 IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
         IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
         String testFileName = "testBasicCanCreateValidIrodsFileSystem.txt";
         
         IRODSFile irodsFile = new IRODSFile(irodsFileSystem, 
        		 testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH));
        
    	 TestCase.assertFalse("file system thinks this is a file", irodsFile.isFile());
    	 TestCase.assertTrue("file system does not think this is a directory", irodsFile.isDirectory());
    	 TestCase.assertEquals("does not have the resource from the account", account.getDefaultStorageResource(), irodsFile.getResource());
    	 irodsFileSystem.close();
    }
    
    /**
     * This is related to bug 24, given a null resource in the account, IRODSFile or IRODSFileSystem do not attempt to discern the default resource
     * @throws Exception
     * BUG: 24
     */
    @Test(expected=NullPointerException.class)
    public void testDoesntIgnoreNullResource() throws Exception {
    	
    	 Properties noResourceProperties = (Properties) testingProperties.clone();
    	 noResourceProperties.remove(IRODS_RESOURCE_KEY);
    	 IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(noResourceProperties);
         IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
         String testFileName = "testDoesntIgnoreNullResource.txt";
         
         // generate a file and put into irods
         String fullPathToTestFile = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                 GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);
         
         IputCommand iputCommand = new IputCommand();
         iputCommand.setLocalFileName(fullPathToTestFile);
         iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH ));
         iputCommand.setForceOverride(true);

         IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
         IcommandInvoker invoker = new IcommandInvoker(invocationContext);
         invoker.invokeCommandAndGetResultAsString(iputCommand);
           
         IRODSFile irodsFile = new IRODSFile(irodsFileSystem, 
        		 testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH) + '/' + testFileName);
        
    	 TestCase.assertTrue("file system not aware this is a file", irodsFile.isFile());
    	 TestCase.assertFalse("thinks this is a file and a directory", irodsFile.isDirectory());
    	 TestCase.assertEquals("does not have the resource from the account", testingProperties.getProperty(IRODS_RESOURCE_KEY), irodsFile.getResource());
    	 irodsFileSystem.close();
         
    }
    
    /**
     * This is related to bug 24, given a null resource in the account, IRODSFile or IRODSFileSystem do not attempt to discern the default resource
     * @throws Exception
     * BUG: 24 
     */
    @Ignore
    public void testDoesntIgnoreBogusResource() throws Exception {
    	
    	 IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
    	 account.setDefaultStorageResource("bogusrescource");
         IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
         String testFileName = "testDoesntIgnoreBogusResource.txt";
         
         // generate a file and put into irods
         String fullPathToTestFile = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                 GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);
         
         IputCommand iputCommand = new IputCommand();
         iputCommand.setLocalFileName(fullPathToTestFile);
         iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH ));
         iputCommand.setForceOverride(true);

         IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
         IcommandInvoker invoker = new IcommandInvoker(invocationContext);
         invoker.invokeCommandAndGetResultAsString(iputCommand);
           
         IRODSFile irodsFile = new IRODSFile(irodsFileSystem, 
        		 testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH) + '/' + testFileName);
        
    	 TestCase.assertTrue("file system not aware this is a file", irodsFile.isFile());
    	 TestCase.assertFalse("thinks this is a file and a directory", irodsFile.isDirectory());
    	 TestCase.assertEquals("does not have the resource from the account", testingProperties.getProperty(IRODS_RESOURCE_KEY), irodsFile.getResource());
    	 irodsFileSystem.close();    
    }
    
}
