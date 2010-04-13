/*
 *   Copyright (c) Oct 9, 2008  DICE Research,
 *   University of California, San Diego (UCSD), San Diego, CA, USA.
 *
 *   Users and possessors of this source code are hereby granted a
 *   nonexclusive, royalty-free copyright and design patent license
 *   to use this code in individual software.  License is not granted
 *   for commercial resale, in whole or in part, without prior written
 *   permission from SDSC/UCSD.  This source is provided "AS IS"
 *   without express or implied warranty of any kind.
 *
 *
 *   User
 *   User.java  -  edu.sdsc.grid.io.irods.User
 *
 *   CLASS HIERARCHY
 *   java.lang.Object
 *       |
 *       +-edu.sdsc.grid.io.irods.Domain
 *          |
 *          +-edu.sdsc.grid.io.irods.User
 *
 *
 *   PRINCIPAL AUTHOR
 *   Lucas Gilbert, SDSC/UCSD
 */

package edu.sdsc.grid.io.irods;

import java.io.IOException;

/**
 * 
 * @author iktome
 */
public class User extends Domain {
	public User(IRODSFileSystem fileSystem) {
		super(fileSystem, "user", "user_type", "r_user_main");
	}

	/**
	 * Queries the fileSystem to aqcuire all the values for this domain. So the
	 * user domain returns all the users.
	 * 
	 * @return
	 */
	public String[] listSubjects() throws IOException {
		return irodsFileSystem.commands
				.simpleQuery(
						"select user_name from r_user_main where zone_name=? and user_type_name != 'rodsgroup'",
						irodsFileSystem.getZone());
	}

	// ------------------------------------------------------------------------
	// moduser Name [ type | zone | DN | comment | info | password ] newValue
	// user-type: rodsAdmin, normal, group, public, ...

	/**
	 * 
	 * @param userName
	 * @param userType
	 *            rodsgroup, rodsadmin, rodsuser, domainadmin, groupadmin,
	 *            storageadmin, rodscurators, ...
	 * @throws java.io.IOException
	 */
	public void addUser(String userName, String userType) throws IOException {
		addUser(userName, userType, "");
	}

	/**
	 * 
	 * @param userName
	 * @param userType
	 *            rodsgroup, rodsadmin, rodsuser, domainadmin, groupadmin,
	 *            storageadmin, rodscurators, ...
	 * @param DN
	 * @throws java.io.IOException
	 */
	public void addUser(String userName, String userType, String DN)
			throws IOException {
		if (DN == null)
			DN = "";

		// 5th arg is zone, which is blank to take default
		String[] args = { "add", "user", userName, userType, "", DN };
		irodsFileSystem.commands.admin(args);
	}

	// rmuser Name (remove user, where userName: name[@department][#zone])
	/**
	 * 
	 * @param userName
	 *            name[@department][#zone]
	 * @throws java.io.IOException
	 */
	public void deleteUser(String userName) throws IOException {
		String[] args = { "rm", "user", userName };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyZone(String userName, String newValue) throws IOException {
		String[] args = { "modify", "user", userName, "zone", newValue };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyDN(String userName, String newValue) throws IOException {
		if (irodsFileSystem.commands.getReportedIRODSVersion()
				.equals("rods2.1")) {
			// String[] args = { "modify", "user", userName, "dn", newValue };
			// irodsFileSystem.commands.admin(args);
			throw new UnsupportedOperationException(
					"operation not supported in 2.1");
		} else {
			String[] args = { "modify", "user", userName, "addAuth", newValue };
			irodsFileSystem.commands.admin(args);
		}
	}

	public void modifyComment(String userName, String newValue)
			throws IOException {
		String[] args = { "modify", "user", userName, "comment", newValue };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyInfo(String userName, String newValue) throws IOException {
		String[] args = { "modify", "user", userName, "info", newValue };
		irodsFileSystem.commands.admin(args);
	}

	/**
	 * This method is currently dead code, and will need enhancements to obfucate the password information in an upcoming version, per 
	 *  Bug 83 -  user.modifyPassword fails in Jargon
	 * @param userName
	 * @param newValue
	 * @throws IOException
	 */
	public void modifyPassword(String userName, String newValue)
			throws IOException {
		throw new UnsupportedOperationException("method currently lacks obfuscation code and will requre an upgrade");
		//String[] args = { "modify", "user", userName, "password", newValue };
		//irodsFileSystem.commands.admin(args);
	}
}
