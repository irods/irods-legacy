/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.irods.IRODSFileOutputStreamTest;
import edu.sdsc.grid.io.irods.IRODSFileTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IlsCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IrmCommandTest;

/**
 * Test suite for icommand wrapper libraries
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)
/*@Suite.SuiteClasses({
  IRODSFileOutputStreamTest.class, IRODSFileTest.class
})*/

@Suite.SuiteClasses({
  IlsCommandTest.class, ImkdirCommandTest.class, IputCommandTest.class, IrmCommandTest.class
})
public class ICommandInvokerTests {
	
}
