package edu.sdsc.grid.io.irods;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;


public class IRODSExecuteCommandsTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
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
    public void testExecuteHello() throws Exception {
    	
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
        InputStream inputStream = irodsFileSystem.commands.executeCommand("hello", "", "localhost", "");
        
        BufferedReader br = new BufferedReader(new InputStreamReader(inputStream));
        StringBuilder sb = new StringBuilder();
        String line = null;

        while ((line = br.readLine()) != null) {
        sb.append(line + "\n");
        }

        br.close();
        String result = sb.toString();
        irodsFileSystem.close();

	    TestCase.assertEquals("did not successfully execute hello command", "Hello world  from irods".trim(), result.trim());
        
    }
    
    @Test
    public void testExecuteHelloViaFileSystem() throws Exception {
    	
        IRODSAccount testAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
        IRODSFileSystem irodsFileSystem = new IRODSFileSystem(testAccount);
        
        InputStream inputStream = irodsFileSystem.executeProxyCommand("hello", "");
                
        BufferedReader br = new BufferedReader(new InputStreamReader(inputStream));
        StringBuilder sb = new StringBuilder();
        String line = null;

        while ((line = br.readLine()) != null) {
        sb.append(line + "\n");
        }

        br.close();
        String result = sb.toString();
        irodsFileSystem.close();

	    TestCase.assertEquals("did not successfully execute hello command", "Hello world  from irods".trim(), result.trim());
        
    }
    
   

}
