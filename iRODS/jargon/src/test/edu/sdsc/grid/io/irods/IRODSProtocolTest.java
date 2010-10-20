package edu.sdsc.grid.io.irods;

import junit.framework.Assert;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class IRODSProtocolTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testIRODSProtocol() throws Exception {
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		Assert.assertNotNull(irodsProtocol);
	}

	@Test
	public final void testGetMetaDataSet() {
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		Assert.assertNotNull(irodsProtocol.getMetaDataSet());
	}

}
