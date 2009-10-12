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
  //Why static? Well, how many admin accounts do you have? 
  //It is also harder to get confused which one you are working with.
  //Also I was maybe lazy.
  static IRODSFileSystem fileSystem;
  
  public final static Domain ACCESS = new Domain("access", "access_type", "");
  public final static Domain ACTION = new Domain("action", "action_type", "");
  public final static Domain DATA = new Domain("data", "data_type", "");
  public final static Domain MAP = new Domain("map", "coll_map", "");
  public final static Domain OBJECT = new Domain("object", "object_type", "");
  public final static Domain RESOURCE_CLASS = 
          new Domain("resource class", "resc_class", "");
  public final static Domain RULE = new Domain("rule", "rulexec_type", "");
  public final static Domain SCHEME = 
          new Domain("authorization", "auth_scheme_type", "");
  public final static Domain ZONE = new Domain("zone", "zone_type", "");
  public final static Resource RESOURCE = new Resource();
  public final static User USER = new User();
  
//------------------------------------------------------------------------ 
// Admin Types Enum, to make admin actions more obvious
// And keep method signatures from being 
//  myMethod( String value1, String value2, String value3, ..., String valueN )
// As happened with the lack of a scheme before 
//------------------------------------------------------------------------   

  
  
  
//------------------------------------------------------------------------  
// Constructors
//------------------------------------------------------------------------  
  public IRODSAdmin( IRODSFileSystem fileSystem )
    throws IOException
  {
    this.fileSystem = fileSystem;
  }

  

  
//------------------------------------------------------------------------  
// Helpful listing of metadata
//------------------------------------------------------------------------  
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
  
/*  
  // pv [date-time] [repeat-time(minutes)] 
  //(initiate a periodic rule to vacuum the DB)
  public void scheduleVacuum( no idea... )
    throws IOException
  {
    String[] args = { "rm", "user", userName };
    fileSystem.commands.admin( args );
  }
*/
  
 
 

  /*
A blank execute line invokes the interactive mode, where it
prompts and executes commands until 'quit' or 'q' is entered.
Single or double quotes can be used to enter items with blanks.
Commands are:
 lu [name] (list user info; details if name entered)
 lt [name] [subname] (list token info)
 lr [name] (list resource info)
 ls [name] (list directory: subdirs and files)
 lz [name] (list zone info)
 lg [name] (list group info (user member list))
 lgd name  (list group details)
 lrg [name] (list resource group info)
 lf DataId (list file details; DataId is the number (from ls))
 mkuser Name Type [DN] (make user, where userName: name[@department][#zone])
 moduser Name [ type | zone | DN | comment | info | password ] newValue
 rmuser Name (remove user, where userName: name[@department][#zone])
 mkdir Name (make directory(collection))
 rmdir Name (remove directory)
 mkresc Name Type Class Host Path (make Resource)
 modresc Name [type, class, host, path, comment, info, freespace] Value (mod Resc)
 rmresc Name (remove resource)
 mkgroup Name (make group)
 rmgroup Name (remove group)
 atg groupName userName (add to group - add a user to a group)
 rfg groupName userName (remove from group - remove a user from a group)
 atrg resourceGroupName resourceName (add (resource) to resource group)
 rfrg resourceGroupName resourceName (remove (resource) from resource group)
 at tokenNamespace Name Value [Value2] [Value3] (add token)
 rt tokenNamespace Name Value (remove token)
 spass Password Key (print a scrambled form of a password for DB)
 dspass Password Key (descramble a password and print it)
 pv [date-time] [repeat-time(minutes)] (initiate a periodic rule to vacuum the DB)
 ctime Time (convert an iRODS time value (integer) to local time)
 help (or h) [command] (this help, or more details on a command)
Also see 'irmtrash -M -u user' for the admin mode of removing trash.

   */
}

