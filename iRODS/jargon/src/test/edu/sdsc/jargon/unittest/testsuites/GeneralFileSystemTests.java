/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.GeneralFileSystemTest;
import edu.sdsc.grid.io.MetaDataRecordListTest;

/**
 * Test suite for GeneralFileSystem operations
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)
/*@Suite.SuiteClasses({
  IRODSFileOutputStreamTest.class, IRODSFileTest.class
})*/

@Suite.SuiteClasses({
  GeneralFileSystemTest.class, MetaDataRecordListTest.class
})
public class GeneralFileSystemTests {
	
}
