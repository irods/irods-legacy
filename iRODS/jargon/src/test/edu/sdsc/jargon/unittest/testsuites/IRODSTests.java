/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.irods.IRODSAccountTest;
import edu.sdsc.grid.io.irods.IRODSAdminTest;
import edu.sdsc.grid.io.irods.IRODSCommandsCopyToTest;
import edu.sdsc.grid.io.irods.IRODSCommandsDeleteTest;
import edu.sdsc.grid.io.irods.IRODSCommandsGetTest;
import edu.sdsc.grid.io.irods.IRODSCommandsMiscTest;
import edu.sdsc.grid.io.irods.IRODSCommandsPutTest;
import edu.sdsc.grid.io.irods.IRODSExecuteCommandsTest;
import edu.sdsc.grid.io.irods.IRODSFileAVUTest;
import edu.sdsc.grid.io.irods.IRODSFileCommandsTest;
import edu.sdsc.grid.io.irods.IRODSFileDeleteTest;
import edu.sdsc.grid.io.irods.IRODSFileInputStreamTest;
import edu.sdsc.grid.io.irods.IRODSFileOutputStreamTest;
import edu.sdsc.grid.io.irods.IRODSFileSystemCreateTarTest;
import edu.sdsc.grid.io.irods.IRODSFileSystemGsiHandlingTest;
import edu.sdsc.grid.io.irods.IRODSFileSystemMetadataQueryTest;
import edu.sdsc.grid.io.irods.IRODSFileSystemModifyMetadataTest;
import edu.sdsc.grid.io.irods.IRODSFileSystemTest;
import edu.sdsc.grid.io.irods.IRODSFileTest;
import edu.sdsc.grid.io.irods.IRODSMultiThreadGetAndPutTest;
import edu.sdsc.grid.io.irods.IRODSRandomAccessFileTest;
import edu.sdsc.grid.io.irods.IRODSResourceQueryTest;
import edu.sdsc.grid.io.irods.ResourceTest;
import edu.sdsc.grid.io.irods.RuleTest;
import edu.sdsc.grid.io.irods.UserTest;

/**
 * Test suite for Irods functionality within Jargon libraries
 * @author Mike Conway, DICE
 * @since 10/10/2009
 */
@RunWith(Suite.class)

@Suite.SuiteClasses({
  IRODSFileTest.class,
  IRODSResourceQueryTest.class,
  IRODSCommandsCopyToTest.class,
  IRODSCommandsDeleteTest.class,
  IRODSFileSystemMetadataQueryTest.class,
  IRODSFileSystemTest.class,
  IRODSFileCommandsTest.class,
  IRODSAccountTest.class,
  IRODSFileSystemCreateTarTest.class,
  ResourceTest.class,
  IRODSFileSystemGsiHandlingTest.class,
  IRODSFileInputStreamTest.class,
  IRODSRandomAccessFileTest.class,
  IRODSFileAVUTest.class,
  RuleTest.class,
  IRODSCommandsGetTest.class,
  IRODSFileDeleteTest.class,
  IRODSFileOutputStreamTest.class,
  UserTest.class,
  IRODSAdminTest.class,
  IRODSCommandsPutTest.class,
  IRODSExecuteCommandsTest.class,
  IRODSCommandsMiscTest.class,
  IRODSFileSystemModifyMetadataTest.class,
  IRODSMultiThreadGetAndPutTest.class
})
public class IRODSTests {

}
