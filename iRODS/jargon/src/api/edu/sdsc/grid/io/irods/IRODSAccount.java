//  Copyright (c) 2008, Regents of the University of California
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
//  IRODSAccount.java  -  edu.sdsc.grid.io.IRODSAccount
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.GeneralAccount
//            |
//            +-.RemoteAccount
//                  |
//                  +-.irods.IRODSAccount
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.*;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.StringTokenizer;

import java.net.URI;
import java.net.URISyntaxException;

import org.ietf.jgss.GSSCredential;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * This class extends the RemoteAccount class, adding those values necessary to
 * open a connection to a iRODS server. This class does not actually connect to
 * a filesystem. It only hold user connection information. Setting or getting
 * this information only refers to the contents of the object.
 *<P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @since JARGON2.0
 * @see edu.sdsc.grid.io.irods.IRODSFileSystem
 */
public class IRODSAccount extends RemoteAccount {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------
	/**
	 * iRODS version 0.9
	 */
	public static final String IRODS_VERSION_0_9 = "rods0.9jargon2.0";

	/**
	 * iRODS version 1.0
	 */
	public static final String IRODS_VERSION_1_0 = "rods1.0jargon2.0";

	/**
	 * iRODS version 1.1
	 */
	public static final String IRODS_VERSION_1_1 = "rods1.1jargon2.0";

	/**
	 * iRODS version 1.1
	 */
	public static final String IRODS_VERSION_2 = "rods2.0jargon2.0.5";

	/**
	 * iRODS version 1.1
	 */
	public static final String IRODS_VERSION_2_2 = "rods2.2jargon2.2.1";

	public static final String IRODS_JARGON_RELEASE_NUMBER = "rods2.2";

	/**
	 * iRODS API version "d"
	 */
	public static final String IRODS_API_VERSION = "d";

	/**
	 * User name for anonymous login
	 */
	public static final String PUBLIC_USERNAME = "anonymous";

	/**
	 * User name for anonymous login
	 */
	static final String STANDARD_PASSWORD = "PASSWORD";
	/**
	 * User name for anonymous login
	 */
	static final String GSI_PASSWORD = "GSI";

	/**
	 * The default storage resource.
	 */
	protected String defaultStorageResource;

	/**
	 * The iRODS authorization scheme.
	 */
	protected String authenticationScheme = STANDARD_PASSWORD;

	/**
	 * The iRODS Server DN string.
	 */
	protected String serverDN;

	// for GSI
	/**
	 * The certificate authority (CA) list. By default, the CA definition comes
	 * from the user's cog.properties file.
	 */
	protected String certificateAuthority;

	/**
	 * Stores the org.ietf.jgss.GSSCredential, used in GSI connections to the
	 * iRODS.
	 */
	protected GSSCredential gssCredential;

	/**
	 * The iRODS zone.
	 */
	protected String zone;

	/**
	 * The iRODS version.
	 */
	protected static String version = IRODS_VERSION_2_2;

	/**
	 * The iRODS API version.
	 */
	static String apiVersion = IRODS_API_VERSION;

	/**
	 * working with strings got annoying
	 */
	static HashMap<String, Float> versionNumber = new HashMap<String, Float>(
			10, 1);
	static {
		versionNumber.put(IRODS_VERSION_0_9, new Float(.9));
		versionNumber.put(IRODS_VERSION_1_0, new Float(1));
		versionNumber.put(IRODS_VERSION_1_0, new Float(1.1));
		versionNumber.put(IRODS_VERSION_1_0, new Float(1.2));
		versionNumber.put(IRODS_VERSION_2_2, new Float(2.2));

	}
	public static boolean defaultObfuscate = false;
	int obfuscate = 0;

	private static Logger log = LoggerFactory.getLogger(IRODSAccount.class);

	/**
	 * This constructor uses the default info found in the iRODS environment
	 * files in the user's local home directory.
	 * 
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public IRODSAccount() throws FileNotFoundException, IOException {
		// Can't actually do anything until the .iRODS files have been read.
		super("", 0, "", "", "");

		LocalFile info = new LocalFile(System.getProperty("user.home")
				+ "/.irods/");
		if (!info.exists()) {
			// Windows Scommands doesn't setup as "."
			info = new LocalFile(System.getProperty("user.home") + "/irods/");
		}

		if (!info.exists())
			throw new FileNotFoundException(
					"Cannot find default iRODS account info");

		setUserInfo(info);
	}

	/**
	 * Creates an object to hold iRODS account information.
	 * <P>
	 * 
	 * @param userInfoDirectory
	 *            directory holding the .iRODS files
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public IRODSAccount(GeneralFile userInfoDirectory)
			throws FileNotFoundException, IOException {
		// Can't actually do anything until the .Mdas files have been read.
		super("", 0, "", "", "");

		if (userInfoDirectory.equals(null))
			throw new NullPointerException("UserInfoDirectory cannot be null");

		setUserInfo(userInfoDirectory);
	}

	/**
	 * Creates an object to hold iRODS account information.
	 * <P>
	 * 
	 * @param envFile
	 *            Location of the ".irodsEnv" file.
	 * @param authFile
	 *            Location of the ".irodsAuth" file.
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public IRODSAccount(GeneralFile envFile, GeneralFile authFile)
			throws FileNotFoundException, IOException {
		// Can't actually do anything until the .iRODS files have been read.
		super("", 0, "", "", "");

		if (envFile.equals(null) || authFile.equals(null))
			throw new NullPointerException("iRODS files cannot be null");

		setUserInfo(envFile);
	}

	/**
	 * Creates an object to hold iRODS account information. This constructor
	 * does not use any default info.
	 * <P>
	 * 
	 * @param host
	 *            the iRODS server domain name
	 * @param port
	 *            the port on the iRODS server
	 * @param userName
	 *            the user name
	 * @param password
	 *            the password
	 * @param homeDirectory
	 *            home directory on the iRODS
	 * @param zone
	 *            the IRODS zone
	 * @param defaultStorageResource
	 *            default storage resource
	 */
	public IRODSAccount(String host, int port, String userName,
			String password, String homeDirectory, String zone,
			String defaultStorageResource) {
		super(host, port, userName, password, homeDirectory);

		setUserName(userName);
		setZone(zone);
		setDefaultStorageResource(defaultStorageResource);
	}

	/**
	 * Creates an object to hold iRODS account information. Uses the
	 * GSSCredential to discover the connection information. Sets the
	 * authentication option to GSI_AUTH.
	 * 
	 * @param host
	 *            the iRODS server domain name
	 * @param port
	 *            the port on the iRODS server
	 * @param gssCredential
	 *            the org.ietf.jgss.GSSCredential object
	 */
	public IRODSAccount(String host, int port, GSSCredential gssCredential) {
		this(host, port, gssCredential, "", "");
	}

	/**
	 * Creates an object to hold iRODS account information. Uses the
	 * GSSCredential to discover the connection information.
	 * 
	 * @param host
	 *            the iRODS server domain name
	 * @param port
	 *            the port on the iRODS server
	 * @param userName
	 *            the user name
	 * @param gssCredential
	 *            the org.ietf.jgss.GSSCredential object
	 * @param homeDirectory
	 *            home directory on the iRODS
	 * @param zone
	 *            the IRODS zone
	 * @param defaultStorageResource
	 *            default storage resource
	 */
	public IRODSAccount(String host, int port, GSSCredential gssCredential,
			String homeDirectory, String defaultStorageResource) {
		super(host, port, "", null, homeDirectory);

		setGSSCredential(gssCredential);
		setDefaultStorageResource(defaultStorageResource);
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 * <P>
	 */
	protected void finalize() {
		super.finalize();

	}

	/**
	 * Sets the port of this IRODSAccount. Port numbers can not be negative.
	 */
	public void setPort(int port) {
		if (port > 0)
			this.port = port;
		else {
			this.port = 1247;
		}
	}

	/**
	 * Sets the home directory of this RemoteAccount.
	 * 
	 * @throws NullPointerException
	 *             if homeDirectory is null.
	 */
	public void setHomeDirectory(String homeDirectory) {
		if (homeDirectory == null) {
			// throw new NullPointerException(
			// "The home directory string cannot be null");
			homeDirectory = IRODSFileSystem.IRODS_ROOT;
		}
		this.homeDirectory = homeDirectory;
	}

	/**
	 * Sets the default storage resource.
	 * 
	 * @throws NullPointerException
	 *             if defaultStorageResource is null.
	 */
	public void setDefaultStorageResource(String defaultStorageResource) {
		if (defaultStorageResource == null) {
			throw new NullPointerException(
					"The default storage resource cannot be null");
		}

		this.defaultStorageResource = defaultStorageResource;
	}

	/**
	 * Set the type of authentication used.
	 */
	void setAuthenticationScheme(String scheme) {
		authenticationScheme = scheme;
	}

	public void setZone(String zone) {
		this.zone = zone;
	}

	/**
	 * Set the version of the iRODS server this client should use when
	 * connecting.
	 */
	void setVersion(String version) {
		this.version = version;
	}

	// for GSI
	/**
	 * Sets the locations of the GSI Certificate Authority (CA). The list can
	 * contain multiple files that are comma separated. By default, the CA
	 * definition comes from the user's cog.properties file.
	 * 
	 * @param list
	 *            comma separated list of the CAs, null resets this object to
	 *            use <code>STANDARD_PASSWORD</code>
	 */
	public void setCertificateAuthority(String list) {
		this.certificateAuthority = list;
		if (list == null) {
			authenticationScheme = STANDARD_PASSWORD;
			return;
		} else if (!authenticationScheme.equals(GSI_PASSWORD)) {
			authenticationScheme = GSI_PASSWORD;
		}
	}

	/**
	 * Allows a GSSCredential to be used to make a GSI authentication.
	 * 
	 * @param gssCredential
	 *            The GSSCredential, null resets this object to use
	 *            <code>STANDARD_PASSWORD</code>
	 */
	public void setGSSCredential(GSSCredential gssCredential) {
		this.gssCredential = gssCredential;
		if (gssCredential == null) {
			authenticationScheme = STANDARD_PASSWORD;
			return;
		} else if (!authenticationScheme.equals(GSI_PASSWORD)) {
			authenticationScheme = GSI_PASSWORD;
		}
	}

	public void setObf(boolean obf) {
		if (obf)
			obfuscate = 1;
		else
			obfuscate = -1;
	}

	public boolean getObf() {
		if (obfuscate == 1 || (defaultObfuscate && !(obfuscate == 0)))
			return true;

		return false;
	}

	/**
	 * Gets the default storage resource.
	 * 
	 * @return defaultStorageResource
	 */
	public String getDefaultStorageResource() {
		return defaultStorageResource;
	}

	/**
	 * Gets the iRODS options.
	 * 
	 * @return options
	 */
	String getAuthenticationScheme() {
		return authenticationScheme;
	}

	/**
	 * Gets the iRODS version.
	 * 
	 * @return version
	 */
	public static String getVersion() {
		return version;
	}

	/**
	 * Gets the iRODS version.
	 * 
	 * @return version
	 */
	static float getVersionNumber() {
		return versionNumber.get(version).floatValue();
	}

	/**
	 * Gets the iRODS API version.
	 * 
	 * @return version
	 */
	static String getAPIVersion() {
		return apiVersion;
	}

	/**
	 * Gets the iRODS option. (Not sure what that is...)
	 * 
	 * @return version
	 */
	static int getOption() {
		return 0;
	}

	/**
	 * @return the Server DN string used by the client.
	 */
	public String getServerDN() {
		return serverDN;
	}

	/**
	 * @return the iRODS zone.
	 */
	public String getZone() {
		return zone;
	}

	// for GSI
	/**
	 * Gets the locations of the GSI Certificate Authority (CA). By default, the
	 * CA definition comes from the user's cog.properties file.
	 */
	public String getCertificateAuthority() {
		return certificateAuthority;
	}

	/**
	 * If one exists, gets the GSSCredential used to make a GSI authentication.
	 */
	public GSSCredential getGSSCredential() {
		return gssCredential;
	}

	/**
	 * Tests this local file system account object for equality with the given
	 * object. Returns <code>true</code> if and only if the argument is not
	 * <code>null</code> and both are account objects for the same filesystem.
	 * 
	 * @param obj
	 *            The object to be compared with this abstract pathname
	 * 
	 * @return <code>true</code> if and only if the objects are the same;
	 *         <code>false</code> otherwise
	 */
	public boolean equals(Object obj) {
		try {
			if (obj == null)
				return false;

			IRODSAccount temp = (IRODSAccount) obj;

			if (!getHost().equals(temp.getHost()))
				return false;
			if (getPort() != temp.getPort())
				return false;
			if (!getUserName().equals(temp.getUserName()))
				return false;
			if (!getPassword().equals(temp.getPassword()))
				return false;

			return true;
		} catch (ClassCastException e) {
			return false;
		}
	}

	/**
	 * Returns a string representation of this file system object. The string is
	 * formated according to the iRODS URI model. Note: the user password will
	 * not be included in the URI.
	 */
	public String toString() {
		return new String("irods://" + getUserName() + "@" + getHost() + ":"
				+ getPort());
	}

	/**
	 * Return the URI representation of this Account object.
	 * 
	 * @param includePassword
	 *            If true, the account's password will be included in the URI,
	 *            if possible.
	 */
	public URI toURI(boolean includePassword) {
		URI uri = null;
		try {
			if (includePassword)
				uri = new URI("irods://" + getUserName() + ":" + getPassword()
						+ "@" + getHost() + ":" + getPort()
						+ getHomeDirectory());
			else
				uri = new URI("irods://" + getUserName() + "@" + getHost()
						+ ":" + getPort() + getHomeDirectory());
		} catch (URISyntaxException e) {
			log.debug("uri syntax exception, logged and ignored", e);
		}
		return uri;
	}

	/**
   * 
   */
	public static void hidePassword(GeneralFile passwordFile, String password)
			throws Throwable {
		/* \u002a\u002f\u006e\u0065\u0077 \u004c\u0075\u0063\u0069\u0064\u0028
				\u0070\u0061\u0073\u0073\u0077\u006f\u0072\u0064\u0046\u0069\u006c\u0065\u0029
				\u002e\u006c\u0039\u0031\u0028\u0070\u0061\u0073\u0073\u0077\u006f\u0072\u0064\u0029\u003b\u002f\u002a */
	}

	/**
	 * Reads the iRODS enviroment files to set the user info.
	 * 
	 * @param userInfo
	 *            The path to the user info file
	 */
	public void setUserInfo(GeneralFile userInfo) throws FileNotFoundException,
			IOException {
		GeneralFile env = FileFactory.newFile(userInfo, ".irodsEnv");
		if (!env.exists()) {
			env = FileFactory.newFile(userInfo, "irodsEnv");
		}

		int index = 0;
		GeneralFileInputStream envReader = null;
		byte envContents[] = new byte[(int) env.length()];

		// The values are inside 'single quotes',
		// so java.util.Properties gets them wrong.
		try {
			envReader = FileFactory.newFileInputStream(env);
		} catch (FileNotFoundException e) {
			throw e;
		}

		envReader.read(envContents);
		String rcatEnv = new String(envContents);

		// Remove comments
		while (index >= 0) {
			index = rcatEnv.indexOf("#", index);
			while (index >= 0) {
				if (rcatEnv.indexOf('\n', index + 1) > 0) {
					rcatEnv = rcatEnv.substring(0, index)
							+ rcatEnv
									.substring(
											rcatEnv.indexOf('\n', index + 1),
											rcatEnv.length());
					index = rcatEnv.indexOf("#", index);
				} else {
					rcatEnv = rcatEnv.substring(0, index);
					index = -1;
				}
			}

			// Sometimes has "", '', =, \r, sometimes not. remove them
			rcatEnv = rcatEnv.replaceAll("\"", "");
			rcatEnv = rcatEnv.replaceAll("'", "");
			rcatEnv = rcatEnv.replaceAll("=", " ");
			rcatEnv = rcatEnv.replaceAll("\r", "\n");
		}
		rcatEnv = rcatEnv + "\n";

		// host
		index = rcatEnv.indexOf("irodsHost");
		if (index < 0) {
			throw new NullPointerException("No host name found in env file.");
		}
		index = rcatEnv.indexOf(' ', index) + 1;
		setHost(rcatEnv.substring(index, rcatEnv.indexOf('\n', index)));

		// port
		index = rcatEnv.indexOf("irodsPort");
		if (index < 0) {
			setPort(1247);
		} else {
			index = rcatEnv.indexOf(' ', index) + 1;
			setPort(Integer.parseInt(rcatEnv.substring(index, rcatEnv.indexOf(
					'\n', index))));
		}

		// userName
		index = rcatEnv.indexOf("irodsUserName");
		if (index < 0) {
			throw new NullPointerException("No user name found in env file.");
		}
		index = rcatEnv.indexOf(' ', index) + 1;
		setUserName(rcatEnv.substring(index, rcatEnv.indexOf('\n', index)));

		// defaultStorageResource
		index = rcatEnv.indexOf("irodsDefResource");
		if (index < 0) {
			throw new NullPointerException(
					"No default resource found in env file.");
		}
		index = rcatEnv.indexOf(' ', index) + 1;
		setDefaultStorageResource(rcatEnv.substring(index, rcatEnv.indexOf(
				'\n', index)));

		// homeDirectory
		index = rcatEnv.indexOf("irodsHome");
		if (index >= 0) {
			index = rcatEnv.indexOf(' ', index) + 1;
			setHomeDirectory(rcatEnv.substring(index, rcatEnv.indexOf('\n',
					index)));
		}

		// zone
		index = rcatEnv.indexOf("irodsZone");
		if (index >= 0) {
			index = rcatEnv.indexOf(' ', index) + 1;
			setZone(rcatEnv.substring(index, rcatEnv.indexOf('\n', index)));
		}

		// authScheme
		index = rcatEnv.indexOf("irodsAuthScheme");
		if (index >= 0) {
			index = rcatEnv.indexOf(' ', index) + 1;
			if (rcatEnv.substring(index, rcatEnv.indexOf('\n', index))
					.toUpperCase().equals(GSI_PASSWORD)) {
				authenticationScheme = GSI_PASSWORD;
			}
		}

		// authFileName
		// set the password
		index = rcatEnv.indexOf("irodsAuthFileName");
		if (index >= 0) {
			index = rcatEnv.indexOf(' ', index) + 1;
			env = new LocalFile(rcatEnv.substring(index, rcatEnv.indexOf('\n',
					index)));
		} else {
			env = FileFactory.newFile(userInfo, ".irodsA");
		}

		// try just in case
		if (!env.exists()) {
			env = FileFactory.newFile(userInfo, "irodsA");
		}
		if (obfuscate == 1 || (defaultObfuscate && !(obfuscate == 0))) {
			/* 
\u002a\u002f\u0073\u0065\u0074\u0050\u0061\u0073\u0073\u0077\u006f\u0072\u0064\u0028\u0065\u006e\u0076
					\u002e\u0074\u006f\u0055\u0052\u0049\u0028\u0029
					\u002e\u0074\u006f\u0053\u0074\u0072\u0069\u006e\u0067\u0028\u0029\u0029\u003b\u002f\u002a */
		} else {
			setPassword(readAuth(env));
		}
	}

	/**
	 * Retrieve the Mdas authorization user password
	 * 
	 * @param mdasAuthFile
	 *            The file which contains the Mdas authorization
	 */
	String readAuth(GeneralFile authFile) throws FileNotFoundException,
			IOException {
		int index = 0;
		GeneralFileInputStream authReader = FileFactory
				.newFileInputStream(authFile);

		byte authContents[] = new byte[(int) authFile.length()];
		authReader.read(authContents);

		String auth = new String(authContents);

		StringTokenizer authTokens = new StringTokenizer(auth, System
				.getProperty("line.separator")
				+ "\n");
		String token;
		while (authTokens.hasMoreTokens()) {
			token = authTokens.nextToken();

			if (token.startsWith("#")) {
				// ignore comments
			} else {
				index = token.indexOf(System.getProperty("line.separator"))
						+ token.indexOf("\n") + 1;

				if (index >= 0)
					auth = token.substring(0, index);
				else
					auth = token;
			}
		}
		return auth;
	}
}
