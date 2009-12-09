/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IchksumCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IlsCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImetaCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.ImkdirCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommandTest;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IrmCommandTest;

import org.junit.runner.RunWith;

import org.junit.runners.Suite;


/**
 * Test suite for icommand wrapper libraries
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)
@Suite.SuiteClasses({IlsCommandTest.class,
    ImkdirCommandTest.class,
    IputCommandTest.class,
    IrmCommandTest.class,
    IlsCommandTest.class,
    ImetaCommandTest.class
    //IchksumCommandTest.class TODO: add after tests are implemented
})
public class ICommandInvokerTests {
}
