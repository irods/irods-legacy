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
public class IRODSAdmin
{

  //FIXME: static use of file system here, will not work on multi-user environment
  private IRODSFileSystem fileSystem;

  public final Domain ACCESS = new Domain(fileSystem, "access", "access_type", "");
  public final Domain ACTION = new Domain(fileSystem, "action", "action_type", "");
  public final Domain DATA = new Domain(fileSystem, "data", "data_type", "");
  public final Domain MAP = new Domain(fileSystem, "map", "coll_map", "");
  public final Domain OBJECT = new Domain(fileSystem, "object", "object_type", "");
  public final Domain RESOURCE_CLASS =
          new Domain(fileSystem, "resource class", "resc_class", "");
  public final Domain RULE = new Domain(fileSystem, "rule", "rulexec_type", "");
  public final Domain SCHEME =
          new Domain(fileSystem, "authorization", "auth_scheme_type", "");
  public final Domain ZONE = new Domain(fileSystem, "zone", "zone_type", "");
  public final Resource RESOURCE = new Resource(fileSystem);
  public final User USER = new User(fileSystem);

//------------------------------------------------------------------------
// Admin Types Enum, to make admin actions more obvious
// And keep method signatures from being
//  myMethod( String value1, String value2, String value3, ..., String valueN )
// As happened with the lack of a scheme before
//------------------------------------------------------------------------

  public IRODSAdmin( IRODSFileSystem fileSystem )
    throws IOException
  {
    this.fileSystem = fileSystem;
  }

  public String[] listUserGroups( )
    throws IOException
  {
    return fileSystem.commands.simpleQuery(
      "select user_name from r_user_main where user_type_name='rodsgroup'",
      null );
  }

  public String[] listUsers( )
    throws IOException
  {
    return USER.listSubjects();
  }

  public String[] listResources( )
    throws IOException
  {
    return RESOURCE.listSubjects();
  }

  public String[] listZones( )
    throws IOException
  {
    return fileSystem.commands.simpleQuery(
      "select zone_name from r_zone_main", null );
  }

//------------------------------------------------------------------------
  // mkgroup Name (make group)
  public void createGroup( String groupName )
    throws IOException
  {
/*
    <arg0>add</arg0>
<arg1>user</arg1>
<arg2>newgroup</arg2>
<arg3>rodsgroup</arg3>   ???
<arg4>tempZone</arg4>
*/
    String[] args = {
      "add", "user", groupName, "rodsgroup", fileSystem.getZone() };
    fileSystem.commands.admin( args );
  }

  // rmgroup Name (remove group)
  public void deleteGroup( String groupName )
    throws IOException
  {
    String[] args = { "rm", "user", groupName, fileSystem.getZone() };
    fileSystem.commands.admin( args );
  }

  // atg groupName userName (add to group - add a user to a group)
  public void addUserToGroup( String groupName, String userName )
    throws IOException
  {
    String[] args = { "modify", "group", groupName, "add", userName };
    fileSystem.commands.admin( args );
  }

  // rfg groupName userName (remove from group - remove a user from a group)
  public void removeUserFromGroup( String groupName, String userName )
    throws IOException
  {
    String[] args = { "modify", "group", groupName, "remove", userName };
    fileSystem.commands.admin( args );
  }

  // atrg resourceGroupName resourceName (add (resource) to resource group)
  public void addResourcetoGroup( String resourceGroup, String resourceName )
    throws IOException
  {
    String[] args = {
     "modify", "resourcegroup", resourceGroup, "add", resourceName };
    fileSystem.commands.admin( args );
  }

  // rfrg resourceGroupName resourceName (remove (resource) from resource group)
  public void removeResourceFromGroup(
    String resourceGroup, String resourceName )
    throws IOException
  {
    String[] args = {
      "modify", "resourcegroup", resourceGroup, "remove", resourceName };
    fileSystem.commands.admin( args );
  }

}

