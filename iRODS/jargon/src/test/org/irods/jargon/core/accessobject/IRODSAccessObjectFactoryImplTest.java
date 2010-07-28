package org.irods.jargon.core.accessobject;

import static org.junit.Assert.*;
import static org.mockito.Mockito.*;
import junit.framework.TestCase;

import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSCommands;

// TODO: add to suite

public class IRODSAccessObjectFactoryImplTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testInstance() throws Exception {
		IRODSCommands irodsCommands = mock(IRODSCommands.class);
		when(irodsCommands.isConnected()).thenReturn(true);
		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl.instance(irodsCommands);
		TestCase.assertNotNull("did not create a factory", irodsAccessObjectFactory);
	}

	@Test(expected=JargonException.class)
	public final void testInstanceNotConnected() throws Exception {
		IRODSCommands irodsCommands = mock(IRODSCommands.class);
		when(irodsCommands.isConnected()).thenReturn(false);
		 IRODSAccessObjectFactoryImpl.instance(irodsCommands);
	}
	
	@Test(expected=JargonException.class)
	public final void testInstanceNullIRODSCommands() throws Exception {
		 IRODSAccessObjectFactoryImpl.instance(null);
	}
}
