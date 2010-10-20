package edu.sdsc.grid.io.irods.mocks;

import java.util.Properties;

import org.ietf.jgss.GSSException;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.Oid;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;


public class MockGssName implements GSSName {
	
	private TestingPropertiesHelper testingPropertiesHelper;
	private Properties testingProperties;

	public MockGssName() {
		testingPropertiesHelper = new TestingPropertiesHelper();
		try {
		testingProperties = testingPropertiesHelper.getTestProperties();
		} catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}
	}

	public GSSName canonicalize(Oid mech) throws GSSException {
		return null;
	}

	public boolean equals(GSSName another) throws GSSException {
		return false;
	}

	public byte[] export() throws GSSException {
		return null;
	}

	public Oid getStringNameType() throws GSSException {
		return null;
	}

	
	public boolean isAnonymous() {
		return false;
	}

	public boolean isMN() {
		return false;
	}
	
	@Override
	public String toString() {
		return testingProperties.getProperty(TestingPropertiesHelper.IRODS_USER_DN_KEY);
	}
	

}
