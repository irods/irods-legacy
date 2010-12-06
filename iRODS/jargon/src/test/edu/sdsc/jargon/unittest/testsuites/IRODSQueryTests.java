/**
 *
 */
package edu.sdsc.jargon.unittest.testsuites;

import org.irods.jargon.core.query.AVUQueryElementTest;
import org.irods.jargon.core.query.ExtensibleMetaDataMappingTest;
import org.irods.jargon.core.query.ExtensibleMetadataPropertiesSourceTest;
import org.irods.jargon.core.query.GenQueryClassicMidLevelServiceTest;
import org.irods.jargon.core.query.IRODSQueryResultRowTest;
import org.irods.jargon.core.query.IRODSQueryTest;
import org.irods.jargon.core.query.IRODSQueryTranslatorTest;
import org.irods.jargon.core.query.SelectFieldTest;
import org.irods.jargon.core.query.TranslatedIRODSQueryTest;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

@RunWith(Suite.class)

@Suite.SuiteClasses({
  IRODSQueryTranslatorTest.class,
 // IRODSQueryResultRowTest.class, TODO: fix mockito issues
  SelectFieldTest.class,
  TranslatedIRODSQueryTest.class, AVUQueryElementTest.class,
  ExtensibleMetaDataMappingTest.class,
  ExtensibleMetadataPropertiesSourceTest.class,
  GenQueryClassicMidLevelServiceTest.class,
  IRODSQueryResultRowTest.class,
  IRODSQueryTest.class,
  IRODSQueryTranslatorTest.class
  
})
public class IRODSQueryTests {
	
}
