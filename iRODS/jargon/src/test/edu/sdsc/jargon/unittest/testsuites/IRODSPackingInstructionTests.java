/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.irods.jargon.core.packinstr.AbstractIRODSPackingInstructionTest;
import org.irods.jargon.core.packinstr.CollInpTest;
import org.irods.jargon.core.packinstr.DataObjInpTest;
import org.irods.jargon.core.packinstr.GenQueryInp_PITest;
import org.irods.jargon.core.packinstr.GenUpdateInpTest;
import org.irods.jargon.core.packinstr.ModAvuMetadataInpTest;
import org.irods.jargon.core.packinstr.OpenedDataObjInpTest;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

/**
 * Test suite for Irods functionality within Jargon libraries
 * @author Mike Conway, DICE
 */
@RunWith(Suite.class)

@Suite.SuiteClasses({
  
  GenUpdateInpTest.class, AbstractIRODSPackingInstructionTest.class,
  CollInpTest.class, DataObjInpTest.class, GenQueryInp_PITest.class,
  GenUpdateInpTest.class, ModAvuMetadataInpTest.class, OpenedDataObjInpTest.class
  
})
public class IRODSPackingInstructionTests {

}
