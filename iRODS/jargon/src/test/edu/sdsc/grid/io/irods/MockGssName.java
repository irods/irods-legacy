package edu.sdsc.grid.io.irods;

import java.util.Properties;

import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.Oid;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;


public class MockGssName implements GSSName {
	
	private TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private Properties testingProperties;

	public MockGssName() {
		try {
		testingProperties = testingPropertiesHelper.getTestProperties();
		} catch (Exception e) {
			throw new RuntimeException(e.getMessage());
		}
	}

	@Override
	public GSSName canonicalize(Oid mech) throws GSSException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean equals(GSSName another) throws GSSException {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public byte[] export() throws GSSException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Oid getStringNameType() throws GSSException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean isAnonymous() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean isMN() {
		// TODO Auto-generated method stub
		return false;
	}
	

}
