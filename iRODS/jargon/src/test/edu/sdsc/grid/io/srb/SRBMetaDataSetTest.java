package edu.sdsc.grid.io.srb;

import static org.junit.Assert.*;
import junit.framework.Assert;
import junit.framework.TestCase;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class SRBMetaDataSetTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testSRBMetaDataSet() {
		SRBProtocol srbProtocol = new SRBProtocol();
		SRBMetaDataSet srbMetaDataSet = new SRBMetaDataSet(srbProtocol);
		Assert.assertNotNull(srbMetaDataSet);
	}

	@Test
	public final void testGetSRBID() {
		fail("Not yet implemented");
	}

	@Test
	public final void testGetSRBName() {
		fail("Not yet implemented");
	}

}
