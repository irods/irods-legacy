/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.irods.jargon.core.genupdate.GenUpdateProcessorTest;
import org.irods.jargon.core.query.GenQueryClassicMidLevelServiceTest;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

/**
 * Test suite for Irods functionality within Jargon libraries
 * @author Mike Conway, DICE
 */
@RunWith(Suite.class)

@Suite.SuiteClasses({
  
  GenQueryClassicMidLevelServiceTest.class
  
})
public class IRODSMidLevelServicesTest {

}
