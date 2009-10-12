/*
 *   Copyright (c) Oct 13, 2008  DICE Research,
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
 *   iadmin
 *   iadmin.java  -  .iadmin
 * 
 *   CLASS HIERARCHY
 *   java.lang.Object
 *       |
 *       +-.iadmin
 * 
 * 
 *   PRINCIPAL AUTHOR
 *   Lucas Gilbert, SDSC/UCSD
 */


import edu.sdsc.grid.io.irods.*;
import edu.sdsc.grid.io.*;
import java.io.*;
import java.net.*;



/**
 *
 * @author iktome
 */
public class iadmin
{
  /**
   * An example usage of iRODS rules
   */
  static void example(IRODSFileSystem fileSystem)
    throws IOException
  {
    IRODSAdmin admin = new IRODSAdmin(fileSystem);

    /*
     * Available areas for administrators:
         ACCESS, ACTION, DATA, MAP, OBJECT, RESOURCE_CLASS, 
         RULE, SCHEME, ZONE, RESOURCE, USER
     */
  
    String fakeType = "myFakeJargonUserType";
    String fakeUser = "myFakeJargonUserName";
        
    //Add a new type
    admin.USER.addType(fakeType);
    
    //list the types
    String[] types = admin.USER.listTypes();
    //print results
    System.out.println("\n");
    for (String t : types)
      System.out.println(t);

    
    
    //list the users
    String[] users = admin.listUsers();
    //print results
    System.out.println("\n");
    for (String user : users ) {
      System.out.println(user);
    }
    
    //alternate listing of the users, 'list the subjects'
    users = admin.USER.listSubjects();
    //print results
    System.out.println("\n");
    for (String user : users ) {
      System.out.println(user);
    }
    
    //add a user
    admin.USER.addUser(fakeUser, fakeType);
    //print results
    users = admin.listUsers();
    System.out.println("\n");
    for (String user : users ) {
      System.out.println(user);
    }
    
    //delete a user
    admin.USER.deleteUser(fakeUser);
    //print results
    users = admin.listUsers();
    System.out.println("\n");
    for (String user : users ) {
      System.out.println(user);
    }
    
    
    
    //delete a type
    admin.USER.deleteType(fakeType);   
    //print results
    System.out.println("\n");
    for (String s : admin.USER.listTypes())
      System.out.println(s);
  }
  
  
	/**
	 * Testing
	 */
	public static void main(String args[])
	{
    
		try{
			if ((args.length == 2) && args[0].equals("-uri")) {
        example((IRODSFileSystem)
                FileFactory.newFileSystem( new URI( args[1] ) ));
			}
			else if (args.length == 0) {
        example(new IRODSFileSystem());
			}
			else {
				throw new IllegalArgumentException(
					"\nUsage: irule rule_filepath" +
          "\nUsage: irule -uri irods://... rule_filepath" );
			}
		}
		catch ( Throwable e ) {
			e.printStackTrace();

			Throwable chain = e.getCause();
			while (chain != null) {
				chain.printStackTrace();
				chain = chain.getCause();
			}
      
      System.out.println(((IRODSException)e).getType());
      System.exit(1);
		}

		System.exit(0);
	}
}
