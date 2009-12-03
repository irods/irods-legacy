package edu.sdsc.grid.io.irods;

import java.util.Properties;

import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.Oid;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;


public class MockGssCredential implements GSSCredential {
	
	
	@Override
	public void add(GSSName name, int initLifetime, int acceptLifetime,
			Oid mech, int usage) throws GSSException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void dispose() throws GSSException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public Oid[] getMechs() throws GSSException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public GSSName getName() throws GSSException {
		return new MockGssName();
	}

	@Override
	public GSSName getName(Oid mech) throws GSSException {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public int getRemainingAcceptLifetime(Oid mech) throws GSSException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getRemainingInitLifetime(Oid mech) throws GSSException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getRemainingLifetime() throws GSSException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getUsage() throws GSSException {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int getUsage(Oid mech) throws GSSException {
		// TODO Auto-generated method stub
		return 0;
	}
	
	
	

}
