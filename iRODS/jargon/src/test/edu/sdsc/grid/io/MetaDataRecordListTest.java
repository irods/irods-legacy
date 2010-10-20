package edu.sdsc.grid.io;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSMetaDataRecordList;
import edu.sdsc.grid.io.irods.IRODSProtocol;

public class MetaDataRecordListTest {

	@Test
	public void testMetaDataRecordListMetaDataFieldLong() {
		long longVal = 100000000L;
		MetaDataRecordList testList = new IRODSMetaDataRecordList(new MetaDataField("name", "desc", MetaDataField.LONG, new IRODSProtocol()), longVal);
		Assert.assertNotNull(testList);
	}
	

	@Test
	public void testGetLongValueWhenSetAsString() {
		String longVal = "100000000";
		long longValAsLong = 100000000L;

		MetaDataRecordList testList = new IRODSMetaDataRecordList(new MetaDataField("name", "desc", MetaDataField.STRING, new IRODSProtocol()), longVal);
		Assert.assertNotNull(testList);
		long actualLongVal = testList.getLongValue(0);
		Assert.assertEquals(longValAsLong, actualLongVal);
	}

	@Test
	public void testGetLongValue() {
		long longVal = 100000000L;
		MetaDataRecordList testList = new IRODSMetaDataRecordList(new MetaDataField("name", "desc", MetaDataField.LONG, new IRODSProtocol()), longVal);
		long actualLongVal = testList.getLongValue(0);
		Assert.assertEquals(longVal, actualLongVal);
	}

}
