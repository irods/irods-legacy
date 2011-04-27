/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.irods.jargon.core.accessobject.BulkFileOperationsAOImplTest;
import org.irods.jargon.core.accessobject.FileCatalogObjectAOImplTest;
import org.irods.jargon.core.accessobject.IRODSAccessObjectFactoryImplTest;
import org.irods.jargon.core.accessobject.IRODSGenQueryExecutorImplTest;
import org.irods.jargon.core.accessobject.RemoteExecutionOfCommandsAOImplTest;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

@RunWith(Suite.class)
@Suite.SuiteClasses({ IRODSAccessObjectFactoryImplTest.class,
		IRODSGenQueryExecutorImplTest.class,
		RemoteExecutionOfCommandsAOImplTest.class,
		BulkFileOperationsAOImplTest.class, FileCatalogObjectAOImplTest.class })
public class IRODSAccessObjectTests {

}
