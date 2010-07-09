package edu.sdsc.jargon.unittest.testsuites;


import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.irods.IRODSCommandsBigCopyTest;
import edu.sdsc.grid.io.irods.IRODSCommandsDeleteBigTest;
import edu.sdsc.grid.io.irods.IRODSFileInputStreamParallelTest;
import edu.sdsc.grid.io.irods.IRODSFileOutputStreamParallelTest;

/**
 * Main test suite to run all Jargon unit tests,including ones that take a while to run
 * @author Mike Conway, DICE
 * @since 10/10/2009
 *
 */

@RunWith(Suite.class)
@Suite.SuiteClasses({
  IRODSCommandsDeleteBigTest.class, 
  IRODSFileOutputStreamParallelTest.class,
  IRODSFileInputStreamParallelTest.class,
  IRODSCommandsBigCopyTest.class
})

public class LongRunningTests {

}
