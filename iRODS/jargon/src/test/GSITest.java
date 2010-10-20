import edu.sdsc.grid.io.srb.SRBAccount;
import edu.sdsc.grid.io.srb.SRBFileSystem;
import edu.sdsc.grid.io.srb.SRBFile;

import edu.sdsc.grid.io.irods.*;
import edu.sdsc.grid.io.*;


import java.io.*;


public class GSITest
{
  /**
   * Example of connecting with iRODS GSI authentication
   * @param args 
   *    args[0] =   "irods"
   *    args[1] =   path to gsi proxyfile
   *    args[2] =   filepath of test file
   *    args[3+]=   filepaths of certificates Authorities
   * 
   * @throws java.io.IOException
   */
  static void irodsGSITest( String args[] ) 
    throws IOException
  {
    /*
      If you use the .irodsEnv, 
        set the value, 
          irodsAuthScheme 'GSI'
        Defined the CA locations in your cog.properties file
        Set the .irodsA password to the proxy file's path.
        Then you can use the default constructor.
     */ 
    IRODSAccount irodsAccount = new IRODSAccount();    
    
     
    /* 
      If you have a GSSCredential use:
        irodsAccount.setGSSCredential(gssCredential); 
    */

    
    //Otherwise:
    
    if (args.length > 1) {
      //Filepath to proxy file
      irodsAccount.setPassword( args[1] );


      //CA locations are not defined in your cog.properties file,
      //create a comma seperated list:
      String certificatesAuthorities = "";
      for (int i=3;i<args.length;i++) {
        certificatesAuthorities += args[i];
        if (i < args.length-1)
          certificatesAuthorities+=",";
      }
      irodsAccount.setCertificateAuthority(certificatesAuthorities);
    }
    
    GeneralFileSystem fileSystem = FileFactory.newFileSystem( irodsAccount );

    GeneralFile file = FileFactory.newFile( fileSystem, "/" );
    System.out.println( "True if connected: "+ file.exists() );
  }
  
//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
	/**
	 * Testing SRB functions
	 */
	public static void main(String args[])
	{
		SRBAccount srbAccount = null;
		SRBFileSystem srbFileSystem = null;

		SRBFile srbFile = null;
   

		/**
		 * If error occurs exit with this variable;
		 */
		int err = 0; 

		try{
			if (args.length >= 1) {        
        if (args[0].startsWith("irods")) {
//----------------------------------------------------------------------
// iRODS GSI Test
//----------------------------------------------------------------------
          irodsGSITest(args);
          return;
        }
        
        if ((args.length > 1) && (args[0].equals("-uri")))
        {          
          //set the password option to GSI_AUTH
          srbAccount.setOptions( SRBAccount.GSI_AUTH );

          //give the file path of your proxy file instead of the a password
          srbAccount.setPassword( args[2] );

          if (args.length > 3) {
            String certificatesAuthorities = "";
            for (int i=3;i<args.length;i++) {
              certificatesAuthorities += args[i];
              if (i < args.length-1)
                certificatesAuthorities+=",";
            }

            //If the CA locations are not defined in your cog.properties file:
            srbAccount.setCertificateAuthority(	certificatesAuthorities );
          }
        }
        else {
  				//uses the ~/.srb/MdasEnv user info file
    			srbAccount = new SRBAccount( );

//**********************************************************************
//For GSI authentication:
//**********************************************************************
          //set the password option to GSI_AUTH
          srbAccount.setOptions( SRBAccount.GSI_AUTH );

          //give the file path of your proxy file instead of the a password
          srbAccount.setPassword( args[0] );

          if (args.length > 1) {
            String certificatesAuthorities = "";
            for (int i=1;i<args.length;i++) {
              certificatesAuthorities += args[i];
              if (i < args.length-1)
                certificatesAuthorities+=",";
            }

            //If the CA locations are not defined in your cog.properties file:
            srbAccount.setCertificateAuthority(	certificatesAuthorities );
          }

//**********************************************************************
//
//**********************************************************************
          
          
        }
			}
			else if (args.length == 0) {
				//The GSI setting that work for me:

				//uses the ~/.srb/MdasEnv user info file
				srbAccount = new SRBAccount( );

				//set the password option to GSI_AUTH
				srbAccount.setOptions( SRBAccount.GSI_AUTH );

				//give the file path of your proxy file instead of the a password
				srbAccount.setPassword( "i:\\x509up_u28227" );

				//If the CA locations are not defined in your cog.properties file.
        //The signing polices should also be in the same directory.
				srbAccount.setCertificateAuthority(
					"/etc/grid-security/certificates/b89793e4.0, "+
					"/etc/grid-security/certificates/3deda549.0, "+
					"/etc/grid-security/certificates/42864e48.0" );
			}
			else {
				throw new IllegalArgumentException(
					"Wrong number of arguments sent to Test." );
			}

/*
      Object cred = GSIAuth.getCredential(srbAccount);
      srbAccount = new SRBAccount( srbAccount.getHost(),srbAccount.getPort(),cred);
      
System.out.println(srbAccount+ " "+cred+" "+srbAccount.getGSSCredential());
*/
      srbFileSystem = new SRBFileSystem( srbAccount );

			srbFile = new SRBFile( srbFileSystem, "/home" );
			System.out.println( "True if connected: "+ srbFile.exists() );
		}
		catch ( Throwable e ) {
			e.printStackTrace();

			Throwable chain = e.getCause();
			while (chain != null) {
				chain.printStackTrace();
				chain = chain.getCause();
			}
			err = 1;
		}


		System.exit(err);
	}
}
