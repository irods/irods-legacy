package edu.sdsc.jargon.unittest.testsuites;


import org.junit.runner.RunWith;
import org.junit.runners.Suite;

/**
 * Main test suite to run all Jargon unit tests, as well as long running and functional tests
 * This is the whole keilbasa
 * @author Mike Conway, DICE
 * @since 10/10/2009
 *
 */

@RunWith(Suite.class)
@Suite.SuiteClasses({
  AllTests.class, 
  LongRunningTests.class 
})

public class AllTestsIncludingLongAndFunctional {

}
