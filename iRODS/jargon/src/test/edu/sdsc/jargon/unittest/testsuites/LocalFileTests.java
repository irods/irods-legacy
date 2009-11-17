/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.local.LocalFileTest;

/**
 * Test suite for LocalFile operations
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)
/*@Suite.SuiteClasses({
  IRODSFileOutputStreamTest.class, IRODSFileTest.class
})*/

@Suite.SuiteClasses({
  LocalFileTest.class
})
public class LocalFileTests {
	
}
