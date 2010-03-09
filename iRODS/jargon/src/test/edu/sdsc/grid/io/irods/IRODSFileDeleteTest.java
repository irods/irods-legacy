package edu.sdsc.grid.io.irods;

import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.AssertionHelper;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandException;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IlsCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IreplCommand;

public class IRODSFileDeleteTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IrodsFileDeleteTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;
    private static AssertionHelper assertionHelper = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
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

    /**
     *  Bug 48 -  NPE on Delete
     * @throws Exception
     */
    @Test
    public void testExistsOnDeletedDirectory() throws Exception {
    	
    	String testNewDir = "testExistsOnDeletedNoForcedDir";
    	String testNewDirIrodsPath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			IRODS_TEST_SUBDIR_PATH + '/' + testNewDir);
    	String testNewFileName = "testExistsOnDeletedFile.csv";
    	
    	
    	IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
    	IRODSFile newFolder = new IRODSFile(irodsFileSystem, testNewDirIrodsPath);
    	IRODSFile newFile = new IRODSFile(irodsFileSystem, newFolder.getAbsolutePath(), testNewFileName);

    	newFolder.mkdir();
    	TestCase.assertTrue("directory I just created should exist", newFolder.exists());
    	newFile.createNewFile();
    	TestCase.assertTrue("file I just created should exist", newFile.exists());

    	TestCase.assertTrue("could not delete file I just created", newFile.delete());
    	TestCase.assertFalse("file I just deleted should not exist", newFile.exists());

    	TestCase.assertTrue("unable to delete folder I just created", newFolder.delete());
    	TestCase.assertFalse("folder I just deleted should not exist", newFolder.exists());
    	irodsFileSystem.close();

    }
    
    /**
     *   Bug 50 -  forced delete on directory does not work, moves files to trash
     * @throws Exception
     */
    @Test
    public void testForcedDeleteDirectory() throws Exception {
    	
    	String testNewDir = "testForcedDeleteDir";
    	String testNewDirIrodsPath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			testNewDir);
    	String testNewFileName = "testExistsForcedDelete.csv";
    	
    	
    	IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
    	IRODSFile newFolder = new IRODSFile(irodsFileSystem, testNewDirIrodsPath);
    	IRODSFile newFile = new IRODSFile(irodsFileSystem, newFolder.getAbsolutePath(), testNewFileName);

    	newFolder.mkdir();
    	TestCase.assertTrue("directory I just created should exist", newFolder.exists());
    	newFile.createNewFile();
    	TestCase.assertTrue("file I just created should exist", newFile.exists());

    	TestCase.assertTrue("unable to delete folder I just created", newFolder.delete(true));
    	TestCase.assertFalse("folder I just deleted should not exist", newFolder.exists());
    	irodsFileSystem.close();

    }
    
    @Test
    public void testUnForcedDeleteDirectory() throws Exception {
    	
    	String testNewDir = "testUnforcedDeleteDir";
    	String testNewDirIrodsPath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, 
    			testNewDir);
    	String testNewFileName = "testExistsUnforcedDelete.csv";
    	
    	
    	IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
    	IRODSFile newFolder = new IRODSFile(irodsFileSystem, testNewDirIrodsPath);
    	IRODSFile newFile = new IRODSFile(irodsFileSystem, newFolder.getAbsolutePath(), testNewFileName);

    	newFolder.mkdir();
    	TestCase.assertTrue("directory I just created should exist", newFolder.exists());
    	newFile.createNewFile();
    	TestCase.assertTrue("file I just created should exist", newFile.exists());

    	TestCase.assertTrue("unable to delete folder I just created", newFolder.delete(false));
    	TestCase.assertFalse("folder I just deleted should not exist", newFolder.exists());
    	irodsFileSystem.close();

    }
    
    /**
     * Bug 63 - Jargon irm equivalent, IRODSFile.delete() not removing all replicas
     * @throws Exception
     */
    @Test 
    public void testDeleteFileOnTwoResources() throws Exception {
    	
    	// create a file and place on two resources
    	String testFileName = "testDeleteFileOnTwoResc.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

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
		
		String irodsObjectAbsolutePath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
		
		// replicate this file to the second resource
		IreplCommand iReplCommand = new IreplCommand();
		iReplCommand.setObjectToReplicate(irodsObjectAbsolutePath);
		iReplCommand.setDestResource(testingProperties.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));
		invoker.invokeCommandAndGetResultAsString(iReplCommand);
		
		// now delete using IRODSFile and make sure no replicas are left

    	IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
    	IRODSFile delFile = new IRODSFile(irodsFileSystem, irodsObjectAbsolutePath);
    	TestCase.assertTrue("the test file does not exist!", delFile.exists());
    	
    	delFile.delete(true);
    	
    	irodsFileSystem.close();
    	
    	// do an ils -L and make sure the the file is on neither resource    	
    	IlsCommand ilsCommand = new IlsCommand();
    	ilsCommand.setIlsBasePath(irodsObjectAbsolutePath);
    	ilsCommand.setLongFormat(true);
    	
    	boolean notFound = false;
    	String ilsResult = "";
    	// i expect an exception using ils, as the file should not be found anywhere
    	
    	try {
    		ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
    	} catch (IcommandException ice) {
    		TestCase.assertTrue("was not the expected 'not found' condition", ice.getMessage().indexOf("does not exist") > -1);
    		notFound = true;
    	}

    	TestCase.assertTrue("found the deleted file with ils", notFound);
    	

    }
    
    /**
     * Bug 63 - Jargon irm equivalent, IRODSFile.delete() not removing all replicas
     * @throws Exception
     */
    @Test 
    public void testDeleteFileOnTwoResourcesNoForceOption() throws Exception {
    	
    	// create a file and place on two resources
    	String testFileName = "testDeleteFileOnTwoRescNoForce.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		FileGenerator.generateFileOfFixedLengthGivenName(absPath, testFileName,
				8);

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
		
		String irodsObjectAbsolutePath = testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH + '/' + testFileName);
		
		// replicate this file to the second resource
		IreplCommand iReplCommand = new IreplCommand();
		iReplCommand.setObjectToReplicate(irodsObjectAbsolutePath);
		iReplCommand.setDestResource(testingProperties.getProperty(TestingPropertiesHelper.IRODS_SECONDARY_RESOURCE_KEY));
		invoker.invokeCommandAndGetResultAsString(iReplCommand);
		
		// now delete using IRODSFile and make sure no replicas are left

    	IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
    	IRODSFile delFile = new IRODSFile(irodsFileSystem, irodsObjectAbsolutePath);
    	TestCase.assertTrue("the test file does not exist!", delFile.exists());
    	
    	delFile.delete(false);
    	
    	irodsFileSystem.close();
    	
    	// do an ils -L and make sure the the file is on neither resource    	
    	IlsCommand ilsCommand = new IlsCommand();
    	ilsCommand.setIlsBasePath(irodsObjectAbsolutePath);
    	ilsCommand.setLongFormat(true);
    	
    	boolean notFound = false;
    	String ilsResult = "";
    	// i expect an exception using ils, as the file should not be found anywhere
    	
    	try {
    		ilsResult = invoker.invokeCommandAndGetResultAsString(ilsCommand);
    	} catch (IcommandException ice) {
    		TestCase.assertTrue("was not the expected 'not found' condition", ice.getMessage().indexOf("does not exist") > -1);
    		notFound = true;
    	}

    	TestCase.assertTrue("found the deleted file with ils", notFound);
    	

    }
    
    
    
}
