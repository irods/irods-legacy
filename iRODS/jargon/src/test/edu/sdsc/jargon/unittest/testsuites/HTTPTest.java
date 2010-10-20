/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.http.HTTPFileTest;

/**
 * Test suite for http functionality within Jargon libraries
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)

@Suite.SuiteClasses({
  HTTPFileTest.class

})
public class HTTPTest {

}
