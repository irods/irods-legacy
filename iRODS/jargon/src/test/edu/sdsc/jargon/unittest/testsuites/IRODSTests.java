/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.irods.IRODSAccountTest;
import edu.sdsc.grid.io.irods.IRODSCommandsMetadataTest;
import edu.sdsc.grid.io.irods.IRODSFileOutputStreamTest;
import edu.sdsc.grid.io.irods.IRODSFileSystemTest;
import edu.sdsc.grid.io.irods.IRODSFileTest;

/**
 * Test suite for Irods functionality within Jargon libraries
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)
/*@Suite.SuiteClasses({
  IRODSFileOutputStreamTest.class, IRODSFileTest.class
})*/

@Suite.SuiteClasses({
  IRODSFileTest.class, 
  IRODSCommandsMetadataTest.class,
  //IRODSFileOutputStreamTest.class,
  IRODSFileSystemTest.class,
  IRODSAccountTest.class
})
public class IRODSTests {
	
}
