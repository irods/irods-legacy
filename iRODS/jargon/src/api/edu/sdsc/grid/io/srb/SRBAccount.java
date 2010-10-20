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
//  SRBAccount.java  -  edu.sdsc.grid.io.SRBAccount
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.GeneralAccount
//            |
//            +-.RemoteAccount
//                  |
//                  +-.srb.SRBAccount
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.*;

import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.File;
import java.util.HashMap;
import java.util.StringTokenizer;

import org.ietf.jgss.GSSCredential;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * This class extends the RemoteAccount class, adding those values necessary to
 * open a connection to the SRB. This class does not actually connect to a
 * filesystem. It only hold user connection information. Setting or getting this
 * information only refers to the contents of the object.
 *<P>
 * 
 * @author Lucas Gilbert, San Diego Supercomputer Center
 */
public class SRBAccount extends RemoteAccount {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------
	// anon. login
	static final String PUBLIC_USERNAME = "public";

	// anon. login
	static final String PUBLIC_DOMAINNAME = "npaci";

	// anon. login
	static final String PUBLIC_PASSWORD = "CANDO";

	// anon. login
	static final String PUBLIC_HOME_DIRECTORY = "/home/public.npaci/";

	// 48 = "0"
	/**
	 * Plain text password. Only supported in SRB version 1.1.8
	 */
	public static final int PASSWD_AUTH = 48;

	// 49 = "1"
	// public static final int SEA_AUTH = 49;
	// 50 = "2"
	// public static final int SEA_ENCRYPT = 50;

	// 51 = "3"
	/**
	 * GSI authentication protocol
	 */
	public static final int GSI_AUTH = 51;

	// 52 = "4"
	// public static final int GSI_SECURE_COMM = 52;

	// 53 = "5"
	/**
	 * Encrypted text password.
	 */
	public static final int ENCRYPT1 = 53;

	// 54 = "6"
	/**
	 * GSI delegated authentication protocol
	 */
	public static final int GSI_DELEGATE = 54;

	/**
	 * Send some extra info to the server. It will be stored in the SRB log.
	 * Might be useful for debugging or client version use tracking.
	 */
	static final String extraVersionInfo = "jargon";

	// *********************************************
	// Be sure to change setVersion() when adding new SRB versions
	// *********************************************
	/**
	 * SRB version 3.5 (same as SRB_VERSION_3_3_1)
	 */
	public static final String SRB_VERSION_3_5 = "SRB-3.5" + extraVersionInfo
			+ "&G";

	/**
	 * SRB version 3.4 (same as SRB_VERSION_3_3_1)
	 */
	public static final String SRB_VERSION_3_4 = "SRB-3.4" + extraVersionInfo
			+ "&G";

	/**
	 * SRB version 3.3.1
	 */
	public static final String SRB_VERSION_3_3_1 = "SRB-3.3.1"
			+ extraVersionInfo + "&G";

	/**
	 * SRB version 3.3
	 */
	public static final String SRB_VERSION_3_3 = "SRB-3.3" + extraVersionInfo
			+ "&G";

	/**
	 * SRB version 3.0.2 to version 3.2
	 */
	public static final String SRB_VERSION_3_0_2 = "SRB-3.0.2"
			+ extraVersionInfo + "&F";

	/**
	 * SRB version 3.0.0
	 */
	public static final String SRB_VERSION_3 = "SRB-3.0.0" + extraVersionInfo
			+ "&E";

	/**
	 * All SRB version 2
	 */
	public static final String SRB_VERSION_2 = "SRB2.0.0" + extraVersionInfo
			+ "&D";

	/**
	 * SRB version 1.1.8. The earliest SRB version tested for compatibility with
	 * this API.
	 */
	public static final String SRB_VERSION_1_1_8 = "SRB1.1.8"
			+ extraVersionInfo + "&C";

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 * The authorizing user name.
	 */
	protected String proxyUserName;

	/**
	 * The mdas domain home of the user.
	 */
	protected String proxyDomainName;

	/**
	 * The mdas domain home of the user. A domain is a string used to identify a
	 * site or project. Users are uniquely identified by their usernames
	 * combined with their domain 'smith.sdsc'. A SRB admin has the authority to
	 * create domains.
	 */
	protected String domainName;

	/**
	 * The default storage resource.
	 */
	protected String defaultStorageResource;

	/**
	 * The srb options. Used to select an authorization scheme.
	 * <ul>
	 * <li>GSI_AUTH (not yet supported)
	 * <li>GSI_SECURE_COMM (not yet supported)
	 * <li>ENCRYPT1 (Default)
	 */
	protected int options = ENCRYPT1;

	// SRB 3.0
	/**
	 * The proxy mcat zone.
	 */
	protected String proxyMcatZone;

	/**
	 * The client mcat zone. Added as of SRB3.0. Used to address a user within
	 * federated MCATs. Federatation of SRBs allows multiple SRBs to communicate
	 * with each even if they use different MCAT database. use differ
	 */
	protected String clientMcatZone;

	/**
	 * The exec file.
	 */
	protected String execFile;

	// for GSI
	/**
	 * The certificate authority (CA) list. By default, the CA definition comes
	 * from the user's cog.properties file.
	 */
	protected String certificateAuthority;

	/**
	 * Stores the org.ietf.jgss.GSSCredential, used in GSI connections to the
	 * SRB.
	 */
	protected GSSCredential gssCredential;

	/**
	 * The srb version.
	 */
	protected static String version = SRB_VERSION_3_5;

	public static boolean defaultObfuscate = false;
	int obfuscate = 0;

	/**
	 * working with strings got annoying
	 */
	static HashMap versionNumber = new HashMap(10, 1);
	static {
		versionNumber.put(SRB_VERSION_3_5, new Float(3.5));
		versionNumber.put(SRB_VERSION_3_4, new Float(3.4));
		versionNumber.put(SRB_VERSION_3_3_1, new Float(3.31));
		versionNumber.put(SRB_VERSION_3_3, new Float(3.3));
		versionNumber.put(SRB_VERSION_3_0_2, new Float(3));
		versionNumber.put(SRB_VERSION_2, new Float(2));
		versionNumber.put(SRB_VERSION_1_1_8, new Float(1));

		systemPropertyVersion(null);
	}

	/**
	 * @returns true if the new version was different than the old, false
	 *          otherwise.
	 */
	static boolean systemPropertyVersion(String newVersion) {
		String temp = null;
		if (newVersion != null) {
			if (newVersion.equals(version)) {
				return false;
			} else {
				temp = newVersion;
			}
		} else {
			temp = System.getProperty("jargon.version");
			if (temp == null)
				return false;
		}
		if (temp.startsWith("SRB-3.5") || temp.startsWith("SRB3.5")
				|| temp.startsWith("3.5") || temp.startsWith("SRB-3.5")
				|| temp.startsWith("SRB3.5") || temp.startsWith("3.5"))

		{
			if (version != SRB_VERSION_3_5) {
				version = SRB_VERSION_3_5;
				return true;
			}
		} else if (temp.startsWith("SRB-3.4") || temp.startsWith("SRB3.4")
				|| temp.startsWith("3.4") || temp.startsWith("SRB-3.4")
				|| temp.startsWith("SRB3.4") || temp.startsWith("3.4"))

		{
			if (version != SRB_VERSION_3_4) {
				version = SRB_VERSION_3_4;
				return true;
			}
		} else if (temp.startsWith("SRB-3.3.1") || temp.startsWith("SRB3.3.1")
				|| temp.startsWith("3.3.1") || temp.startsWith("SRB-3.31")
				|| temp.startsWith("SRB3.31") || temp.startsWith("3.31")) {
			if (version != SRB_VERSION_3_3_1) {
				version = SRB_VERSION_3_3_1;
				return true;
			}
		} else if (temp.startsWith("SRB-3.3") || temp.startsWith("SRB3.3")
				|| temp.startsWith("3.3")) {
			if (version != SRB_VERSION_3_3) {
				version = SRB_VERSION_3_3;
				return true;
			}
		} else if (temp.startsWith("SRB-3.2") || temp.startsWith("SRB3.2")
				|| temp.startsWith("3.2")) {
			if (version != SRB_VERSION_3_0_2) {
				version = SRB_VERSION_3_0_2;
				return true;
			}
		} else if (temp.startsWith("SRB-3.0.2") || temp.startsWith("SRB3.0.2")
				|| temp.startsWith("3.0.2")) {
			if (version != SRB_VERSION_3_0_2) {
				version = SRB_VERSION_3_0_2;
				return true;
			}
		} else if (temp.startsWith("SRB-3") || temp.startsWith("SRB3")
				|| temp.startsWith("3")) {
			if (version != SRB_VERSION_3) {
				version = SRB_VERSION_3;
				return true;
			}
		} else if (temp.startsWith("SRB-2") || temp.startsWith("SRB2")
				|| temp.startsWith("2")) {
			if (version != SRB_VERSION_2) {
				version = SRB_VERSION_2;
				return true;
			}
		} else if (temp.startsWith("SRB-1") || temp.startsWith("SRB1")
				|| temp.startsWith("1")) {
			if (version != SRB_VERSION_1_1_8) {
				version = SRB_VERSION_1_1_8;
				return true;
			}
		}

		return false;
	}

	/**
	 * This constructor uses the default info found in the Mdas files in the
	 * user's local home directory.
	 * 
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount() throws FileNotFoundException, IOException {
		// Can't actually do anything until the .Mdas files have been read.
		super("", 0, "", "", "");

		LocalFile info = new LocalFile(System.getProperty("user.home")
				+ "/.srb/");
		if (!info.exists()) {
			// Windows Scommands doesn't setup as "."
			info = new LocalFile(System.getProperty("user.home") + "/srb/");
		}

		if (!info.exists())
			throw new FileNotFoundException(
					"Cannot find default srb account info");

		setUserInfo(info);
	}

	/**
	 * Creates an object to hold SRB account information.
	 * <P>
	 * 
	 * @param userInfoDirectory
	 *            the local directory holding the .Mdas files
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount(String userInfoDirectory) throws FileNotFoundException,
			IOException {
		this(new LocalFile(userInfoDirectory));
	}

	/**
	 * Creates an object to hold SRB account information.
	 * <P>
	 * 
	 * @param userInfoDirectory
	 *            directory holding the .Mdas files
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount(File userInfoDirectory) throws FileNotFoundException,
			IOException {
		this(new LocalFile(userInfoDirectory));
	}

	/**
	 * Creates an object to hold SRB account information.
	 * <P>
	 * 
	 * @param userInfoDirectory
	 *            directory holding the .Mdas files
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount(GeneralFile userInfoDirectory)
			throws FileNotFoundException, IOException {
		// Can't actually do anything until the .Mdas files have been read.
		super("", 0, "", "", "");

		if (userInfoDirectory.equals(null))
			throw new NullPointerException("UserInfoDirectory cannot be null");

		setUserInfo(userInfoDirectory);
	}

	/**
	 * Creates an object to hold SRB account information.
	 * <P>
	 * 
	 * @param mdasEnvFile
	 *            Location of the ".MdasEnv" file.
	 * @param mdasAuthFile
	 *            Location of the ".MdasAuth" file.
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount(String mdasEnvFile, String mdasAuthFile)
			throws FileNotFoundException, IOException {
		this(new LocalFile(mdasEnvFile), new LocalFile(mdasAuthFile));
	}

	/**
	 * Creates an object to hold SRB account information.
	 * <P>
	 * 
	 * @param mdasEnvFile
	 *            Location of the ".MdasEnv" file.
	 * @param mdasAuthFile
	 *            Location of the ".MdasAuth" file.
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount(File mdasEnvFile, File mdasAuthFile)
			throws FileNotFoundException, IOException {
		this(new LocalFile(mdasEnvFile), new LocalFile(mdasAuthFile));
	}

	/**
	 * Creates an object to hold SRB account information.
	 * <P>
	 * 
	 * @param mdasEnvFile
	 *            Location of the ".MdasEnv" file.
	 * @param mdasAuthFile
	 *            Location of the ".MdasAuth" file.
	 * @throws FileNotFoundException
	 *             if the user info cannot be found.
	 * @throws IOException
	 *             if the user info exists but cannot be opened or created for
	 *             any other reason.
	 */
	public SRBAccount(GeneralFile mdasEnvFile, GeneralFile mdasAuthFile)
			throws FileNotFoundException, IOException {
		// Can't actually do anything until the .Mdas files have been read.
		super("", 0, "", "", "");

		if (mdasEnvFile.equals(null) || mdasAuthFile.equals(null))
			throw new NullPointerException("Mdas files cannot be null");

		setMdasUserInfo(mdasEnvFile);
		readMdasAuth(mdasAuthFile);
	}

	/**
	 * Creates an object to hold SRB account information. This constructor does
	 * not use any default info.
	 * <P>
	 * 
	 * @param host
	 *            the SRB server domain name
	 * @param port
	 *            the port on the SRB server
	 * @param userName
	 *            the user name
	 * @param password
	 *            the password
	 * @param homeDirectory
	 *            home directory on the SRB
	 * @param mdasDomainName
	 *            the mdas home domain
	 * @param defaultStorageResource
	 *            default storage resource
	 */
	public SRBAccount(String host, int port, String userName, String password,
			String homeDirectory, String mdasDomainName,
			String defaultStorageResource) {
		super(host, port, userName, password, homeDirectory);

		setProxyUserName(userName);
		setProxyDomainName(mdasDomainName);
		setDomainName(mdasDomainName);
		setDefaultStorageResource(defaultStorageResource);
	}

	/**
	 * Creates an object to hold SRB account information. This constructor does
	 * not use any default info.
	 * <P>
	 * 
	 * @param host
	 *            the SRB server domain name
	 * @param port
	 *            the port on the SRB server
	 * @param userName
	 *            the user name
	 * @param password
	 *            the password
	 * @param homeDirectory
	 *            home directory on the SRB
	 * @param mdasDomainName
	 *            the mdas home domain
	 * @param defaultStorageResource
	 *            default storage resource
	 * @param mcatZone
	 *            mcat zone
	 */
	public SRBAccount(String host, int port, String userName, String password,
			String homeDirectory, String mdasDomainName,
			String defaultStorageResource, String mcatZone) {
		super(host, port, userName, password, homeDirectory);

		setProxyUserName(userName);
		setProxyDomainName(mdasDomainName);
		setDomainName(mdasDomainName);
		setDefaultStorageResource(defaultStorageResource);
		setProxyMcatZone(mcatZone);
		setMcatZone(mcatZone);
	}

	/**
	 * Creates an object to hold SRB account information. Uses the GSSCredential
	 * to discover the connection information. Sets the authentication option to
	 * GSI_AUTH.
	 * 
	 * @param host
	 *            the SRB server domain name
	 * @param port
	 *            the port on the SRB server
	 * @param gssCredential
	 *            the org.ietf.jgss.GSSCredential object
	 */
	public SRBAccount(String host, int port, GSSCredential gssCredential) {
		super(host, port, null, null, "");

		setGSSCredential(gssCredential);
	}

	/**
	 * Creates an object to hold SRB account information. Uses the GSSCredential
	 * to discover the connection information.
	 * 
	 * @param host
	 *            the SRB server domain name
	 * @param port
	 *            the port on the SRB server
	 * @param gssCredential
	 *            the org.ietf.jgss.GSSCredential object
	 * @param homeDirectory
	 *            home directory on the SRB
	 * @param defaultStorageResource
	 *            default storage resource
	 * @param options
	 *            authentication protocol, e.g GSI_AUTH
	 */
	public SRBAccount(String host, int port, GSSCredential gssCredential,
			String homeDirectory, String defaultStorageResource, int options) {
		super(host, port, null, null, homeDirectory);

		setGSSCredential(gssCredential);
		setOptions(options);
		setDefaultStorageResource(defaultStorageResource);
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 * <P>
	 */
	protected void finalize() {
		super.finalize();
		proxyUserName = null;
		proxyDomainName = null;
		domainName = null;
		defaultStorageResource = null;
	}

	/**
	 * Sets the home directory of this RemoteAccount.
	 * 
	 * @throws NullPointerException
	 *             if homeDirectory is null.
	 */
	public void setHomeDirectory(String homeDirectory) {
		if (homeDirectory == null)
			throw new NullPointerException(
					"The home directory string cannot be null");

		this.homeDirectory = homeDirectory;
	}

	/**
	 * Returns the homeDirectory used by this SRBAccount.
	 * 
	 * @return homeDirectory
	 */
	public String getHomeDirectory() throws NullPointerException {
		if (homeDirectory != null)
			return homeDirectory;

		String dir = "/home/";
		if (userName != null)
			if (domainName != null)
				dir += userName + "." + domainName;

		return dir;
	}

	// From RemoteAccount
	/**
	 * Sets the port of this SRBAccount. Port numbers can not be negative.
	 * Default value is 5544
	 */
	public void setPort(int port) {
		if (port > 0)
			this.port = port;
		else {
			this.port = 5544;
		}
	}

	/**
	 * Sets the proxy user name.
	 * 
	 * @throws NullPointerException
	 *             if proxyUserName is null.
	 */
	protected void setProxyUserName(String proxyUserName) {
		if (proxyUserName == null)
			throw new NullPointerException("The proxy user name cannot be null");

		this.proxyUserName = proxyUserName;
	}

	/**
	 * Sets the proxy domain name.
	 * 
	 * @throws NullPointerException
	 *             if proxyDomainName is null.
	 */
	protected void setProxyDomainName(String proxyDomainName) {
		if (proxyDomainName == null)
			throw new NullPointerException(
					"The proxy domain name cannot be null");

		this.proxyDomainName = proxyDomainName;
	}

	/**
	 * Sets the domain name used by the client.
	 * 
	 * @throws NullPointerException
	 *             if domainName is null.
	 */
	public void setDomainName(String domainName) {
		if (domainName == null)
			throw new NullPointerException("The domain name cannot be null");

		this.domainName = domainName;
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
	 * Set the type of authentication used, e.g. ENCRYPT1, GSI_AUTH.
	 */
	public void setOptions(int options) {
		this.options = options;
	}

	/**
	 * Sets the version.
	 * 
	 * @throws NullPointerException
	 *             if version is null.
	 */
	public static void setVersion(String srbVersion) {
		if (srbVersion == null)
			throw new IllegalArgumentException("Invalid version");

		if (versionNumber.get(srbVersion) != null) {
			version = srbVersion;
		} else {
			throw new IllegalArgumentException("Invalid version");
		}
	}

	// SRB 3.0
	/**
	 * Sets the proxy mcat zone.
	 */
	public void setProxyMcatZone(String proxyMcatZone) {
		this.proxyMcatZone = proxyMcatZone;
	}

	/**
	 * Sets the client mcat zone.
	 */
	public void setMcatZone(String clientMcatZone) {
		this.clientMcatZone = clientMcatZone;
	}

	/**
	 * Sets the exec file.
	 */
	public void setExecFile(String execFile) {
		this.execFile = execFile;
	}

	// for GSI
	/**
	 * Sets the locations of the GSI Certificate Authority (CA). The list can
	 * contain multiple files that are comma separated. By default, the CA
	 * definition comes from the user's cog.properties file.
	 */
	public void setCertificateAuthority(String list) {
		certificateAuthority = list;
	}

	/**
	 * Allows a GSSCredential to be used to make a GSI authentication.
	 */
	public void setGSSCredential(GSSCredential gssCredential) {
		this.gssCredential = gssCredential;
		if (gssCredential == null) {
			return;
		}

		if ((options != GSI_AUTH) || (options != GSI_DELEGATE)) {
			options = GSI_AUTH;
		}
	}

	public void setObf(boolean obf) {
		if (obf)
			obfuscate = 1;
		else
			obfuscate = -1;
	}

	public boolean getObf() {
		if (obfuscate == 1 || (obfuscate == 0 && defaultObfuscate))
			return true;

		return false;
	}

	/**
	 * Gets the SRB proxyUserName.
	 * 
	 * @return proxyUserName
	 */
	protected String getProxyUserName() {
		return proxyUserName;
	}

	/**
	 * Gets the SRB proxyDomainName.
	 * 
	 * @return proxyDomainName
	 */
	protected String getProxyDomainName() {
		return proxyDomainName;
	}

	/**
	 * Gets the domain name used by the client. Only different from the
	 * proxyDomainName for ticketed users.
	 */
	public String getDomainName() {
		return domainName;
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
	 * Gets the SRB options.
	 * 
	 * @return options
	 */
	public int getOptions() {
		return options;
	}

	/**
	 * Gets the SRB version.
	 * 
	 * @return version
	 */
	public static String getVersion() {
		return version;
	}

	/**
	 * Gets the SRB version.
	 * 
	 * @return version
	 */
	static float getVersionNumber() {
		return ((Float) versionNumber.get(version)).floatValue();
	}

	// SRB 3.0
	/**
	 * @return the proxy mcat zone.
	 */
	public String getProxyMcatZone() {
		return proxyMcatZone;
	}

	/**
	 * @return the client mcat zone.
	 */
	public String getMcatZone() {
		return clientMcatZone;
	}

	/**
	 * @return the exec file.
	 */
	public String getExecFile() {
		return execFile;
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
	 * Tests this account object for equality with the given object. Returns
	 * <code>true</code> if and only if the argument is not <code>null</code>
	 * and both are account objects with equivalent connection parameters.
	 * 
	 * @param obj
	 *            The object to be compared with this abstract pathname
	 * 
	 * @return <code>true</code> if and only if the objects are the same;
	 *         <code>false</code> otherwise
	 */
	public boolean equals(Object obj) {
		return equals(obj, true);
	}

	/**
	 * The zone is uniquely determined by username.domain. So SRBFile(uri) does
	 * some fancy stuff, but doesn't have the zone yet.
	 */
	boolean equals(Object obj, boolean checkZone) {
		try {
			if (obj == null)
				return false;

			SRBAccount temp = (SRBAccount) obj;

			if (!getHost().equals(temp.getHost()))
				return false;
			if (getPort() != temp.getPort())
				return false;
			if (!getUserName().equals(temp.getUserName()))
				return false;
			if (!getPassword().equals(temp.getPassword()))
				return false;
			if ((getOptions() == GSI_AUTH) || (getOptions() == GSI_DELEGATE)) {
				if (!getCertificateAuthority().equals(
						temp.getCertificateAuthority()))
					return false;
			}

			if (!getProxyUserName().equals(temp.getProxyUserName()))
				return false;
			if (!getProxyDomainName().equals(temp.getProxyDomainName()))
				return false;
			if (!getDomainName().equals(temp.getDomainName()))
				return false;

			if (!getVersion().equals(SRB_VERSION_2)
					&& !getVersion().equals(SRB_VERSION_1_1_8) && checkZone) {
				if (getProxyMcatZone() != null) {
					if (temp.getProxyMcatZone() != null) {
						if (getMcatZone() != null) {
							if (temp.getMcatZone() != null) {
								// above make sure zone has actually been
								// obtained

								if (!getProxyMcatZone().equals(
										temp.getProxyMcatZone())) {
									if (!getMcatZone().equals(
											temp.getMcatZone())) {
										return false;
									}
								}
							}
						}
					}
				}
			}

			// else //everything is equal
			return true;
		} catch (ClassCastException e) {
			return false;
		}
	}

	/**
	 * Returns a string representation of this file system object. The string is
	 * formated according to the SRB URI model. Note: the user password will not
	 * be included in the URI.
	 */
	public String toString() {
		return new String("srb://" + getUserName() + "." + getDomainName()
				+ "@" + getHost() + ":" + getPort());
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
			return new URI("srb://" + getUserName() + "." + getDomainName()
					+ ":" + getPassword() + "@" + getHost() + ":" + getPort()
					+ getHomeDirectory());
		} catch (URISyntaxException e) {
			if (SRBCommands.DEBUG > 0)
				e.printStackTrace();
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
	 * Reads the Mdas enviroment files, then set the user info.
	 * 
	 * @param userInfoDirectory
	 *            The path to the user info files
	 */
	public void setUserInfo(GeneralFile userInfoDirectory)
			throws FileNotFoundException, IOException {
		GeneralFile mdas = FileFactory.newFile(userInfoDirectory, ".MdasEnv");
		if (!mdas.exists()) {
			mdas = FileFactory.newFile(userInfoDirectory, "MdasEnv");
		}
		setMdasUserInfo(mdas);

		mdas = FileFactory.newFile(userInfoDirectory, ".MdasAuth");
		if (!mdas.exists()) {
			mdas = FileFactory.newFile(userInfoDirectory, "MdasAuth");
		}
		readMdasAuth(mdas);
	}

	/**
	 * Reads the Mdas enviroment file, then sets the user info variables.
	 * 
	 * @param mdasEnvFile
	 *            The mdas file which stores the user info
	 */
	public void setMdasUserInfo(GeneralFile mdasEnvFile)
			throws FileNotFoundException, IOException {
		int index = 0;
		GeneralFileInputStream mdasEnvReader = null;

		// The values are inside 'single quotes',
		// so java.util.Properties gets them wrong.
		try {
			mdasEnvReader = FileFactory.newFileInputStream(mdasEnvFile);
		} catch (FileNotFoundException e) {
			if (!version.equals(SRB_VERSION_2)
					&& !version.equals(SRB_VERSION_1_1_8)) {
				// Ticketed users don't have an SRB account. This default
				// account
				// was created after SRB3.0.1 to allow anyone access to the
				// information stored in public@npaci.
				setProxyUserName("ticketuser");
				// clientUserName
				setUserName("ticketuser");
				setHomeDirectory("/home/public.npaci");

				setPassword("");
				return;
			} else {
				throw e;
			}
		}

		byte mdasEnvContents[] = new byte[(int) mdasEnvFile.length()];
		mdasEnvReader.read(mdasEnvContents);
		String mdasEnv = new String(mdasEnvContents);

		// Remove comments
		while (index >= 0) {
			index = mdasEnv.indexOf("#", index);
			if (index >= 0) {
				mdasEnv = mdasEnv.substring(0, index)
						+ mdasEnv.substring(mdasEnv.indexOf('\n', index + 1),
								mdasEnv.length());
			}
		}

		// Default host
		index = mdasEnv.indexOf("srbHost");
		if (index < 0) {
			throw new NullPointerException("No host name found in Mdas file.");
		}
		index = mdasEnv.indexOf('\'', index) + 1;
		setHost(mdasEnv.substring(index, mdasEnv.indexOf('\'', index)));

		// Default port
		index = mdasEnv.indexOf("srbPort");
		if (index < 0) {
			setPort(5544);
		} else {
			index = mdasEnv.indexOf('\'', index) + 1;
			setPort(Integer.parseInt(mdasEnv.substring(index, mdasEnv.indexOf(
					'\'', index))));
		}

		// Default proxyUserName
		index = mdasEnv.indexOf("srbUser");
		if (index < 0) {
			throw new NullPointerException("No user name found in Mdas file.");
		}
		index = mdasEnv.indexOf('\'', index) + 1;
		setProxyUserName(mdasEnv.substring(index, mdasEnv.indexOf('\'', index)));

		// clientUserName
		setUserName(proxyUserName);

		// proxyDomainName
		index = mdasEnv.indexOf("mdasDomain");
		if (index < 0) {
			throw new NullPointerException(
					"No home domain name found in Mdas file.");
		}
		index = mdasEnv.indexOf('\'', index) + 1;
		setProxyDomainName(mdasEnv.substring(index, mdasEnv
				.indexOf('\'', index)));

		// domainName
		setDomainName(proxyDomainName);

		// defaultStorageResource
		index = mdasEnv.indexOf("defaultResource");
		if (index < 0) {
			throw new NullPointerException(
					"No default resource found in Mdas file.");
		}
		index = mdasEnv.indexOf('\'', index) + 1;
		setDefaultStorageResource(mdasEnv.substring(index, mdasEnv.indexOf(
				'\'', index)));

		// options (password type)
		index = mdasEnv.indexOf("AUTH_SCHEME");
		if (index < 0) {
			setOptions(ENCRYPT1);
		} else {
			String option = null;
			index = mdasEnv.indexOf('\'', index) + 1;
			option = mdasEnv.substring(index, mdasEnv.indexOf('\'', index));
			if (option.equals("PASSWD_AUTH")) {
				setOptions(PASSWD_AUTH);
			} else if (option.equals("GSI_AUTH")) {
				setOptions(GSI_AUTH);
			} else if (option.equals("ENCRYPT1")) {
				setOptions(ENCRYPT1);
			} else if (option.equals("SEA_AUTH")) {
				throw new IllegalArgumentException("SEA_AUTH not supported");
			} else if (option.equals("SEA_ENCRYPT")) {
				throw new IllegalArgumentException("SEA_ENCRYPT not supported");
			} else if (option.equals("GSI_SECURE_COMM")) {
				throw new IllegalArgumentException(
						"GSI_SECURE_COMM not supported");
			}
		}

		// SRB 3.0
		// proxyMcatZone
		index = mdasEnv.indexOf("mcatZone");
		if (index >= 0) {
			index = mdasEnv.indexOf('\'', index) + 1;
			setProxyMcatZone(mdasEnv.substring(index, mdasEnv.indexOf('\'',
					index)));
		}

		// clientMcatZone
		setMcatZone(proxyMcatZone);

		// execFile
		index = mdasEnv.indexOf("execFile");
		if (index >= 0) {
			index = mdasEnv.indexOf('\'', index) + 1;
			setDefaultStorageResource(mdasEnv.substring(index, mdasEnv.indexOf(
					'\'', index)));
		}

		// had to move down after 3.0
		// homeDirectory
		index = mdasEnv.indexOf("mdasCollection");
		if (index < 0) {
			if (proxyMcatZone == null) {
				// = /zone/home/user.domain
				setHomeDirectory(SRBFile.separator + proxyMcatZone
						+ SRBFile.separator + "home" + SRBFile.separator
						+ userName + "." + domainName);
			} else {
				// = /home/user.domain
				setHomeDirectory(SRBFile.separator + "home" + SRBFile.separator
						+ userName + "." + domainName);
			}
		} else {
			index = mdasEnv.indexOf('\'', index) + 1;
			setHomeDirectory(mdasEnv.substring(index, mdasEnv.indexOf('\'',
					index)));
		}

		// srbVersion (for JARGON only)
		index = mdasEnv.indexOf("srbVersion");
		if (index > 0) {
			index = mdasEnv.indexOf('\'', index) + 1;
			setVersion(mdasEnv.substring(index, mdasEnv.indexOf('\'', index)));
		}
	}

	/**
	 * Retrieve the Mdas authorization user password
	 * 
	 * @param mdasAuthFile
	 *            The file which contains the Mdas authorization
	 */
	public void readMdasAuth(GeneralFile mdasAuthFile)
			throws FileNotFoundException, IOException {
		if (obfuscate == 1 || (obfuscate == 0 && defaultObfuscate)) {
			/* 
      \u002a\u002f\u0073\u0065\u0074\u0050\u0061\u0073\u0073\u0077\u006f\u0072\u0064\u0028\u006d\u0064\u0061\u0073\u0041\u0075\u0074\u0068\u0046\u0069\u006c\u0065
					\u002e\u0074\u006f\u0055\u0052\u0049\u0028\u0029
					\u002e\u0074\u006f\u0053\u0074\u0072\u0069\u006e\u0067\u0028\u0029\u0029\u003b
			\u0072\u0065\u0074\u0075\u0072\u006e\u003b\u002f\u002a */
		}
		int index = 0;
		GeneralFileInputStream mdasAuthReader = FileFactory
				.newFileInputStream(mdasAuthFile);

		byte mdasAuthContents[] = new byte[(int) mdasAuthFile.length()];
		mdasAuthReader.read(mdasAuthContents);

		String mdasAuth = new String(mdasAuthContents);

		StringTokenizer authTokens = new StringTokenizer(mdasAuth, System
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
					mdasAuth = token.substring(0, index);
				else
					mdasAuth = token;
			}
		}

		setPassword(mdasAuth);
	}
}
