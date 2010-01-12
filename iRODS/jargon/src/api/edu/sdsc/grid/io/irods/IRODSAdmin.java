//
//  Copyright (c) 2008  San Diego Supercomputer Center (SDSC),
//  University of California, San Diego (UCSD), San Diego, CA, USA.
//
//  Users and possessors of this source code are hereby granted a
//  nonexclusive, royalty-free copyright and design patent license
//  to use this code in individual software.  License is not granted
//  for commercial resale, in whole or in part, without prior written
//  permission from SDSC/UCSD.  This source is provided "AS IS"
//  without express or implied warranty of any kind.
//
//
//  IRODSAdmin
//  Method.java  -  edu.sdsc.grid.io.irods.IRODSAdmin
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.irods.IRODSAdmin
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.io.IOException;

/**
 * For all your iRODS administration needs
 */
public class IRODSAdmin {

	private IRODSFileSystem fileSystem;

	public final Domain access;
	public final Domain action;
	public final Domain data;
	public final Domain map;
	public final Domain domainObject;
	public final Domain resourceClass;
	public final Domain rule;
	public final Domain scheme;
	public final Domain domainZone;
	public final Resource domainResource;
	public final User user;

	public IRODSAdmin(IRODSFileSystem fileSystem) throws IOException {
		this.fileSystem = fileSystem;
		access = new Domain(fileSystem, "access", "access_type", "");
		action = new Domain(fileSystem, "action", "action_type", "");
		data = new Domain(fileSystem, "data", "data_type", "");
		map = new Domain(fileSystem, "map", "coll_map", "");
		domainObject = new Domain(fileSystem, "object", "object_type", "");
		resourceClass = new Domain(fileSystem, "resource class", "resc_class",
				"");
		rule = new Domain(fileSystem, "rule", "rulexec_type", "");
		scheme = new Domain(fileSystem, "authorization", "auth_scheme_type", "");
		domainZone = new Domain(fileSystem, "zone", "zone_type", "");
		domainResource = new Resource(fileSystem);
		user = new User(fileSystem);
	}

	public String[] listUserGroups() throws IOException {
		return fileSystem.commands
				.simpleQuery(
						"select user_name from r_user_main where user_type_name='rodsgroup'",
						null);
	}

	public String[] listUsers() throws IOException {
		return user.listSubjects();
	}

	public String[] listResources() throws IOException {
		return domainResource.listSubjects();
	}

	public String[] listZones() throws IOException {
		return fileSystem.commands.simpleQuery(
				"select zone_name from r_zone_main", null);
	}

	// mkgroup Name (make group)
	public void createGroup(String groupName) throws IOException {
		/*
		 * <arg0>add</arg0> <arg1>user</arg1> <arg2>newgroup</arg2>
		 * <arg3>rodsgroup</arg3> ??? <arg4>tempZone</arg4>
		 */
		String[] args = { "add", "user", groupName, "rodsgroup",
				fileSystem.getZone() };
		fileSystem.commands.admin(args);
	}

	// rmgroup Name (remove group)
	public void deleteGroup(String groupName) throws IOException {
		String[] args = { "rm", "user", groupName, fileSystem.getZone() };
		fileSystem.commands.admin(args);
	}

	// atg groupName userName (add to group - add a user to a group)
	public void addUserToGroup(String groupName, String userName)
			throws IOException {
		String[] args = { "modify", "group", groupName, "add", userName };
		fileSystem.commands.admin(args);
	}

	// rfg groupName userName (remove from group - remove a user from a group)
	public void removeUserFromGroup(String groupName, String userName)
			throws IOException {
		String[] args = { "modify", "group", groupName, "remove", userName };
		fileSystem.commands.admin(args);
	}

	// atrg resourceGroupName resourceName (add (resource) to resource group)
	public void addResourcetoGroup(String resourceGroup, String resourceName)
			throws IOException {
		String[] args = { "modify", "resourcegroup", resourceGroup, "add",
				resourceName };
		fileSystem.commands.admin(args);
	}

	// rfrg resourceGroupName resourceName (remove (resource) from resource
	// group)
	public void removeResourceFromGroup(String resourceGroup,
			String resourceName) throws IOException {
		String[] args = { "modify", "resourcegroup", resourceGroup, "remove",
				resourceName };
		fileSystem.commands.admin(args);
	}

}
