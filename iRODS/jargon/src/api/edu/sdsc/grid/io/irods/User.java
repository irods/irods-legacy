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
public class User extends Domain
{  
  User( ) 
  { 
    super("user", "user_type", "r_user_main" );
  }
  
  
  
  /**
   * Queries the fileSystem to aqcuire all the values for this domain.
   * So the user domain returns all the users.
   * @return
   */
  public String[] listSubjects( ) 
    throws IOException
  {
    return IRODSAdmin.fileSystem.commands.simpleQuery(       
      "select user_name from r_user_main where user_type_name != 'rodsgroup'", 
      null ); 
  }
  
  
  
  //------------------------------------------------------------------------    
  // moduser Name [ type | zone | DN | comment | info | password ] newValue
  //user-type: rodsAdmin, normal, group, public, ...  
  
  /**
   * 
   * @param userName
   * @param userType rodsgroup, rodsadmin, rodsuser, domainadmin, groupadmin,
   *    storageadmin, rodscurators, ... 
   * @throws java.io.IOException
   */
  public static void addUser( String userName, String userType )
    throws IOException
  {    
    addUser( userName, userType, "" );
  }
  
  /**
   * 
   * @param userName
   * @param userType rodsgroup, rodsadmin, rodsuser, domainadmin, groupadmin,
   *    storageadmin, rodscurators, ... 
   * @param DN 
   * @throws java.io.IOException
   */
  public static void addUser( String userName, String userType, String DN )
    throws IOException
  {    
    if (DN == null) DN = "";
    
    String[] args = { "add", "user", userName, userType, DN };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  // rmuser Name (remove user, where userName: name[@department][#zone])
  /**
   * 
   * @param userName name[@department][#zone]
   * @throws java.io.IOException
   */
  public static void deleteUser( String userName )
    throws IOException
  {
    String[] args = { "rm", "user", userName };
    IRODSAdmin.fileSystem.commands.admin( args );
  }

  public static void modifyZone( 
          String userName, String newValue )
    throws IOException
  {
    String[] args = { 
      "modify", "user", userName, "zone", newValue };
    IRODSAdmin.fileSystem.commands.admin( args );
  }  
  
  public static void modifyDN( 
          String userName, String newValue )
    throws IOException
  {
    String[] args = { 
      "modify", "user", userName, "dn", newValue };
    IRODSAdmin.fileSystem.commands.admin( args );
  }  
  
  public static void modifyComment( 
          String userName, String newValue )
    throws IOException
  {
    String[] args = { 
      "modify", "user", userName, "comment", newValue };
    IRODSAdmin.fileSystem.commands.admin( args );
  }  
  
  public static void modifyInfo( 
          String userName, String newValue )
    throws IOException
  {
    String[] args = { 
      "modify", "user", userName, "info", newValue };
    IRODSAdmin.fileSystem.commands.admin( args );
  }  
  
  public static void modifyPassword( 
          String userName, String newValue )
    throws IOException
  {
    String[] args = { 
      "modify", "user", userName, "password", newValue };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
}
