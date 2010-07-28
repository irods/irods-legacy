package edu.sdsc.grid.io;

import junit.framework.TestCase;

import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSMetaDataRecordList;
import edu.sdsc.grid.io.irods.IRODSProtocol;

public class MetaDataRecordListTest {

	@Test
	public void testMetaDataRecordListMetaDataFieldLong() {
		long longVal = 100000000L;
		MetaDataRecordList testList = new IRODSMetaDataRecordList(new MetaDataField("name", "desc", MetaDataField.LONG, new IRODSProtocol()), longVal);
		TestCase.assertNotNull(testList);
	}
	

	@Test
	public void testGetLongValueWhenSetAsString() {
		String longVal = "100000000";
		long longValAsLong = 100000000L;

		MetaDataRecordList testList = new IRODSMetaDataRecordList(new MetaDataField("name", "desc", MetaDataField.STRING, new IRODSProtocol()), longVal);
		TestCase.assertNotNull(testList);
		long actualLongVal = testList.getLongValue(0);
		TestCase.assertEquals(longValAsLong, actualLongVal);
	}

	@Test
	public void testGetLongValue() {
		long longVal = 100000000L;
		MetaDataRecordList testList = new IRODSMetaDataRecordList(new MetaDataField("name", "desc", MetaDataField.LONG, new IRODSProtocol()), longVal);
		long actualLongVal = testList.getLongValue(0);
		TestCase.assertEquals(longVal, actualLongVal);
	}

}
