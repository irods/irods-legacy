//  Copyright (c) 2005, Regents of the University of California
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//    * Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//    * Neither the name of the University of California, San Diego (UCSD) nor
//  the names of its contributors may be used to endorse or promote products
//  derived from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//  SRBCommands.java  -  edu.sdsc.grid.io.srb.SRBCommands
//
//  CLASS HIERARCHY
//  java.lang.Object
//     |
//     +-.SRBCommands
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.*;

import java.net.Socket;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.globus.common.CoGProperties;
import org.globus.gsi.GlobusCredential;
import org.globus.gsi.gssapi.GlobusGSSCredentialImpl;
import org.globus.gsi.gssapi.net.impl.GSIGssInputStream;
import org.globus.gsi.gssapi.net.impl.GSIGssOutputStream;
import org.globus.gsi.gssapi.net.impl.GSIGssSocket;
import org.gridforum.jgss.ExtendedGSSCredential;
import org.gridforum.jgss.ExtendedGSSManager;
import org.ietf.jgss.GSSContext;
import org.ietf.jgss.GSSCredential;
import org.ietf.jgss.GSSException;



/**
 * Instances of this class support socket I/O to a Srb server.
 *<P>
 * Handles socket level protocol for interacting with the SRB.
 *<P>
 * See also: <a href="doc-files/SRBProtocol.htm">SRB protocol</a>
 *
 * <P>
 * @author  Lucas Gilbert, San Diego Supercomputer Center
 */
//This is the working GSIAuth class. If you have the GSI libraries,
//(Which should be available at the JARGON website)
//overwrite GSIAuth.java with this file.
//This class and those libraries will allow you to compile Jargon to include
//All functionality of Jargon, including GSI authentication.
class GSIAuth
{
  /**
   * Checks the authentication using GSI of a SRB connection which has already
   * passed the uner info part of the handshake.
   *<P>
   * @param account the SRB connection information
   * @param srbConnection The open socket to the SRB.
   * @param out The output stream from that socket.
   * @param in The input stream from that socket.
   * @throws IOException If the authentication to the SRB fails.
   */
  GSIAuth( SRBAccount account,
    Socket srbConnection, OutputStream out, InputStream in )
    throws IOException
  {
    sendGSIAuth( account, srbConnection, out, in );
  }
  
  /**
   * GSI authorization method. Makes a connection to the SRB using
   * the GSI authorization scheme.
   *
   * @param account the SRB connection information
   * @param srbConnection The open socket to the SRB.
   * @param out The output stream from that socket.
   * @param in The input stream from that socket.
   * @throws IOException If the authentication to the SRB fails.
   */
  void sendGSIAuth( SRBAccount account, 
    Socket srbConnection, OutputStream out, InputStream in )
    throws IOException
  {
    CoGProperties cog = null;
    String defaultCA = null;
    GSSCredential credential = null;
    String caLocations = account.getCertificateAuthority();
    
    ExtendedGSSManager manager =
      (ExtendedGSSManager)ExtendedGSSManager.getInstance();

    try {
      credential = getCredential( account );

      if (caLocations != null) {
//there is no other way to do this.
//so I'm overwriting the default then changing it back.
        cog = CoGProperties.getDefault();
        defaultCA = cog.getCaCertLocations();
        cog.setCaCertLocations( caLocations );
      }

      GSSContext context = null;
      GSIGssOutputStream gssout = null;
      GSIGssInputStream gssin = null;

      context = manager.createContext(null,
        null,
        credential,
        GSSContext.DEFAULT_LIFETIME);

      context.requestCredDeleg(false);
      context.requestMutualAuth(true);

      GSIGssSocket ggSocket = new GSIGssSocket( srbConnection, context );
      gssout = new GSIGssOutputStream(out, context);
      gssin = new GSIGssInputStream(in, context);

      byte [] inToken = new byte[0];
      byte [] outToken = null;

      while( !context.isEstablished() ) {
        outToken = context.initSecContext(inToken, 0, inToken.length);

        if (outToken != null) {
          gssout.writeToken(outToken);
        }

        if (!context.isEstablished()) {
          inToken = gssin.readHandshakeToken();
        }
      }
    } catch ( GSSException e ) {
      SecurityException gsiException = null;
      String message = e.getMessage();
      if (message.indexOf("Invalid buffer") >= 0) {
        gsiException = new SecurityException(
          "GSI Authentication Failed - Invalid Proxy File" );
        gsiException.initCause(e);
      }
      else if (message.indexOf("Unknown CA") >= 0) {
        gsiException = new SecurityException(
          "GSI Authentication Failed - Cannot find "+
          "Certificate Authority (CA)" );
        gsiException.initCause(e);
      }
      else {
        gsiException = new SecurityException(
          "GSI Authentication Failed" );
        gsiException.initCause(e);
      }
      throw gsiException;
    }
    catch ( Throwable e ) {
      SecurityException exception = new SecurityException(
        "GSI Authentication Failed" );
      exception.initCause(e);
      throw exception;
    }
    finally {
      if (defaultCA != null) {
        cog.setCaCertLocations( defaultCA );
      }
    }
  }

  static String getDN( SRBAccount account ) 
    throws IOException
  {
    StringBuffer dn = null;
    int index = -1, index2 = -1;
    try {
       GlobusGSSCredentialImpl credential = 
        ((GlobusGSSCredentialImpl) getCredential( account ));
      dn = new StringBuffer( credential.getName().toString() );
    } catch ( GSSException e ) {
      throw new IllegalArgumentException( "Invalid or missing credentials" );//, e);
    }

    //remove the extra /CN if exists
    index = dn.indexOf("UID");
    if ( index >= 0 ) {
      index2 = dn.lastIndexOf("CN");
      if ( index2 > index ) {
        dn = dn.delete( index2-1, dn.length() );
      }
    }    

    //The DN gets returned with commas.
    index = dn.indexOf(",");
    while (index >= 0) {
      dn = dn.replace( index, index+1, "/" );
      index = dn.indexOf(",");
    }

    //add / to front if necessary
    if (dn.indexOf("/") != 0) {
      return "/"+dn;
    }
    else {
      return dn.toString();
    }
  }

  static GSSCredential getCredential( SRBAccount account )
    throws GSSException, IOException
  {    
    byte[] data = null;
    GSSCredential credential = (GSSCredential) account.getGSSCredential();
    if (credential != null) {
      if(credential.getRemainingLifetime() <= 0) 
        throw new GSSException(GSSException.CREDENTIALS_EXPIRED );

      return credential;
    }

    String password = account.getPassword();
    ExtendedGSSManager manager =
      (ExtendedGSSManager)ExtendedGSSManager.getInstance();

    if (password == null) {
      throw new IllegalArgumentException(
        "Password/Proxyfile and GSSCredential cannot be null." );
    }
    else if (password.startsWith( "-----BEGIN CERTIFICATE-----" )) {
      data = password.getBytes();

      credential = manager.createCredential(
        data, ExtendedGSSCredential.IMPEXP_OPAQUE,
        GSSCredential.DEFAULT_LIFETIME,
        null, GSSCredential.INITIATE_AND_ACCEPT);
    }
    else {
      LocalFile f = new LocalFile(password);
      if (f.exists()) {
        GeneralRandomAccessFile inputFile =
          FileFactory.newRandomAccessFile(f, "r");
        data = new byte[(int)f.length()];
        // read in the credential data
        inputFile.read(data);
        inputFile.close();
      }
      else {
        throw new IOException( "Proxy file path invalid" );
      }

      credential = manager.createCredential(
        data, ExtendedGSSCredential.IMPEXP_OPAQUE,
        GSSCredential.DEFAULT_LIFETIME,
        null, GSSCredential.INITIATE_AND_ACCEPT);
    }    

    if(credential.getRemainingLifetime() <= 0) 
      throw new GSSException(GSSException.CREDENTIALS_EXPIRED );

    return credential;
  }
}
