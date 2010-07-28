/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.irods.jargon.core.accessobject.IRODSAccessObjectFactoryImplTest;
import org.irods.jargon.core.accessobject.IRODSGenQueryExecutorImplTest;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

@RunWith(Suite.class)

@Suite.SuiteClasses({
  IRODSAccessObjectFactoryImplTest.class,
  IRODSGenQueryExecutorImplTest.class
})
public class IRODSAccessObjectTests {
	
}
