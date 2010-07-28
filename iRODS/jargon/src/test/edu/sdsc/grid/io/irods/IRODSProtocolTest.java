package edu.sdsc.grid.io.irods;

import static org.junit.Assert.fail;
import junit.framework.TestCase;

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
		TestCase.assertNotNull(irodsProtocol);
	}

	@Test
	public final void testGetMetaDataSet() {
		IRODSProtocol irodsProtocol = new IRODSProtocol();
		TestCase.assertNotNull(irodsProtocol.getMetaDataSet());
	}

}
