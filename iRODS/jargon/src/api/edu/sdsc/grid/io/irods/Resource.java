/** 
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
 *   IRODSAdmin
 *   Resource.java  -  edu.sdsc.grid.io.irods.Resource
 * 
 *   CLASS HIERARCHY
 *   java.lang.Object
 *       |
 *       +-edu.sdsc.grid.io.irods.Domain
 *          |
 *          +-edu.sdsc.grid.io.irods.Resource
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
public class Resource extends Domain
{ 
  static final String iName = "resource";
  Resource( )
  {
    super("resource", "resc_type", "r_resc_main");    
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
      "select resc_name from r_resc_main", 
      null ); 
  }
  
  
   
  

//------------------------------------------------------------------------  
  // mkresc Name Type Class Host Path (make Resource)
  public void addResource( String resourceName, String resourceType,
    String resourceClass, String host, String vaultFilePath )
    throws IOException
  {
    String[] args = { "add", name, resourceName, resourceType.toString(), 
      resourceClass, host, vaultFilePath };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  // rmresc Name (remove resource)
  public void deleteResource( String resourceName )
    throws IOException
  {
    String[] args = { "rm", name, resourceName };
    IRODSAdmin.fileSystem.commands.admin( args );
  }  
  
  
//------------------------------------------------------------------------ 
  // modresc Name [type, class, host, path, comment, info, freespace] Value 
  //(mod Resc)
  public static void modifyClass( String resourceName, String newClass )
    throws IOException
  {
    String[] args = { "modify", iName, resourceName, "class", newClass  };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  public static void modifyHost( String resourceName, String newHost )
    throws IOException
  {
    String[] args = { "modify", iName, resourceName, "host", newHost  };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  public static void modifyPath( String resourceName, String newPath )
    throws IOException
  {
    String[] args = { "modify", iName, resourceName, "path", newPath  };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  public static void modifyComment( String resourceName, String newComment )
    throws IOException
  {
    String[] args = { 
      "modify", iName, resourceName, "comment", newComment };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  public static void modifyInfo( String resourceName, String newInfo )
    throws IOException
  {
    String[] args = { "modify", iName, resourceName, "info", newInfo  };
    IRODSAdmin.fileSystem.commands.admin( args );
  }

  public static void modifyFreespace( String resourceName, String newValue )
    throws IOException
  {
    String[] args = { "modify", iName, resourceName, "type", newValue  };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
}
