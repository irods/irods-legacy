/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.irods.jargon.core.genupdate.GenUpdateProcessorTest;
import org.irods.jargon.core.query.ExtensibleMetaDataMappingTest;
import org.irods.jargon.core.query.ExtensibleMetadataPropertiesSourceTest;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

import edu.sdsc.grid.io.http.HTTPFileTest;
import edu.sdsc.grid.io.irods.DomainTest;
import edu.sdsc.grid.io.irods.IRODSAccountTest;
import edu.sdsc.grid.io.irods.IRODSAdminTest;
import edu.sdsc.grid.io.irods.IRODSAvuTest;
import edu.sdsc.grid.io.irods.IRODSCommandsCopyToTest;
import edu.sdsc.grid.io.irods.IRODSCommandsDeleteTest;
import edu.sdsc.grid.io.irods.IRODSCommandsGetTest;
import edu.sdsc.grid.io.irods.IRODSCommandsMiscTest;
import edu.sdsc.grid.io.irods.IRODSCommandsPutTest;
import edu.sdsc.grid.io.irods.IRODSCommandsQueryTest;
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
import edu.sdsc.grid.io.irods.IRODSExtensibleMetaDataTest;
import edu.sdsc.grid.io.irods.IRODSMultiThreadGetAndPutTest;
import edu.sdsc.grid.io.irods.IRODSRandomAccessFileTest;
import edu.sdsc.grid.io.irods.IRODSResourceQueryTest;
import edu.sdsc.grid.io.irods.IRODSThousandFilesTest;
import edu.sdsc.grid.io.irods.ResourceTest;
import edu.sdsc.grid.io.irods.RuleTest;
import edu.sdsc.grid.io.irods.UserTest;

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
