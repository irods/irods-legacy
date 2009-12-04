package edu.sdsc.grid.io.irods.mocks;

import java.util.Properties;

import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.Oid;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;


public class MockGssCredential implements GSSCredential {
	
	
	
	public void add(GSSName name, int initLifetime, int acceptLifetime,
			Oid mech, int usage) throws GSSException {
		
	}

	
	public void dispose() throws GSSException {
		
	}

	
	public Oid[] getMechs() throws GSSException {
		return null;
	}

	
	public GSSName getName() throws GSSException {
		return new MockGssName();
	}

	
	public GSSName getName(Oid mech) throws GSSException {
		return null;
	}

	
	public int getRemainingAcceptLifetime(Oid mech) throws GSSException {
		return 0;
	}

	
	public int getRemainingInitLifetime(Oid mech) throws GSSException {
		return 0;
	}

	public int getRemainingLifetime() throws GSSException {
		return 0;
	}

	
	public int getUsage() throws GSSException {
		return 0;
	}

	
	public int getUsage(Oid mech) throws GSSException {
		return 0;
	}
	
	
	

}
