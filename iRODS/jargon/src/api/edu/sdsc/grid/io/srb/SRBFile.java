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
//  SRBFile.java  -  edu.sdsc.grid.io.srb.SRBFile
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-.GeneralFile
//              |
//              +-.RemoteFile
//                    |
//                    +-.SRBFile
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.*;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.ConnectException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.Vector;
import java.util.Date;


/**
 * An abstract representation of file and directory pathnames on the SRB.
 *<P>
 * In the terminology of SRB, files are known as data sets. A data set is
 * a "stream-of-bytes" entity that can be uniquely identified. For example,
 * a file in HPSS or Unix is a data set, or a LOB stored in a SRB Vault
 * database is a data set. Importantly, note that a data set is not a
 * set of data objects/files. Each data set in SRB is given a unique
 * internal identifier by SRB. A dataset is associated with a collection.
 *<P>
 * A SRB collection is a logical name given to a set of data sets. All data
 * sets stored in SRB/MCAT are stored in some collection. A collection can
 * have sub-collections, and hence provides a hierarchical structure. A
 * collection in SRB/MCAT can be equated to a directory in a Unix file
 * system. But unlike a file system, a collection is not limited to a
 * single device (or partition). A collection is logical but the datsets
 * grouped under a collection can be stored in heterogeneous storage
 * devices. There is one obvious restriction, the name given to a data set
 * in a collection or sub-collection should be unique in that collection.
 *<P>
 * This class shares many similarities with the java.io.File class:
 * <p> User interfaces and operating systems use system-dependent <em>pathname
 * strings</em> to name files and directories.  This class presents an
 * abstract, SRB view of hierarchical pathnames.  An
 * <em>abstract pathname</em> has two components:
 *
 * <ol>
 * <li> An optional SRB <em>prefix</em> string, <code>"/"</code>&nbsp;
 * <li> A sequence of zero or more string <em>names</em>.
 * </ol>
 *
 * Each name in an abstract pathname except for the last denotes a directory;
 * the last name may denote either a directory or a file.  The <em>empty</em>
 * abstract pathname has no prefix and an empty name sequence.
 *<P>
 * When an abstract pathname is converted into a pathname string, each name
 * is separated from the next by a single copy of the default <em>separator
 * character</em>.
 *<P>
 * A pathname in string form may be either <em>absolute</em> or
 * <em>relative</em>. On construction the pathname is made absolute.
 * An absolute pathname is complete in that no other information is required in
 * order to locate the file that it denotes.  A relative pathname, in contrast,
 * must be interpreted in terms of information taken from some other pathname.
 * By default the classes in the <code>edu.sdsc.grid.io.srb</code> package always
 * resolve relative pathnames against the user home directory.  This directory
 * is named in the .MdasEnv file.
 *<P>
 * The prefix concept is used to handle root directories on the SRB is
 * the same as for UNIX platforms.
 * <p> For the SRB, the prefix of an absolute pathname is always
 * <code>"/"</code>.  Relative pathnames have no prefix.  The abstract pathname
 * denoting the root directory has the prefix <code>"/"</code> and an empty
 * name sequence.
 *<P>
 * Instances of the SRBFile class are immutable; that is, once created,
 * the abstract pathname represented by a SRBFile object will never change.
 *<P>
 * @author Lucas Gilbert, San Diego Supercomputer Center
 * @see  java.io.File
 * @see  edu.sdsc.grid.io.GeneralFile
 * @see  edu.sdsc.grid.io.RemoteFile
 * @since JARGON1.0
 */
public class SRBFile extends RemoteFile
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
  /**
   * Default SRB catalog type.
   */
  public static final int MDAS_CATALOG = 0;

  /**
   * A SRB catalog type.
   */
  public static final int LDAP_CATALOG = 1;

  /**
   * A SRB catalog type.
   */
  public static final int FILE_CATALOG = 2;


  //SRB proxy value
  static final int OPR_COPY = 0;


  /**
   * Maximum number of threads used for parallel operations.
   * It generally appears that increasing this beyond 4 threads raises
   * the overhead without speeding up the transfers.
   */
  static int MAX_NUMBER_OF_PARALLEL_THREADS = 4;
  /*protected */ static int MAX_NUMBER_OF_BULK_THREADS = 1;



  /**
   * Minimum thread size, in bytes, of threads for parallel file transfers.
   * If the file.length / NUMBER_OF_THREADS > THREAD_SIZE,
   * then the thread size is automatically increased.
   */
  /*protected */static int MIN_THREAD_SIZE = 30000000;


  /**
   * The default buffer size for bulk load transfers, in bytes. 8MB
   */
  static final int BULK_LOAD_BUFFER_SIZE = 2097152;

  /**
   * Files bigger then this shouldn't use the bulk tansfer
   * This number must be <= BULK_LOAD_BUFFER_SIZE
   */
  static final int MAX_BULK_FILE_SIZE = 2097152;

  /**
   * Maximum files registered by at a time by a thread during bulkLoad.
   */
  static final int MAX_REGISTRATION_FILES = 300;


  /** list the registered chksum value. */
  static final int LIST_CHECKSUM = 16; //#define l_FLAG 0x10

  /** compute chksum, but don't register. */
  static final int COMPUTE_CHECKSUM  = 128; //#define c_FLAG 0x80

  /** force compute and register of chksum even if one already exist. */
  static final int FORCE_CHECKSUM = 32; //#define f_FLAG 0x20

  /**
   * Location of the SRB Zone Authority. When given a zone name the
   * zone authority returns a xml document with the host and port
   * information for that zone.
   * see also, http://www.sdsc.edu/srb/cgi-bin/zoneList.cgi?zone=ZoneName
   */
  static String ZONE_AUTHORITY  =
    "http://www.sdsc.edu/srb/cgi-bin/zoneList.cgi?zone=";

//um, maybe not
static boolean USE_BULKCOPY = true;

  //MODIFY_DEFINABLE_METADATA_SEPARATOR
  //I thought why document, just put it all in the name...
  private static String MODIFY_DEFINABLE_METADATA_SEPARATOR = "|";


  /**
   * Whether this abstract pathname is a file or directory is
   * undetermined.
   */
  static final int PATH_IS_UNKNOWN = 0;

  /**
   * This abstract pathname is a file.
   */
  static final int PATH_IS_FILE = 1;

  /**
   * This abstract pathname is a directory.
   */
  static final int PATH_IS_DIRECTORY = 2;

  static final String LOCAL_CONTAINER_DIRECTORY = "container";
  static final String LOCAL_HOME_DIRECTORY = "home";
  static final String LOCAL_STYLES_DIRECTORY = "styles";
  static final String LOCAL_TRASH_DIRECTORY = "trash";

//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
  /**
   * Holds the server object used by this srb file. Same as the
   * <code>fileSystem</code> variable, but cast from GeneralFileSystem to
   * SRBFileSystem.
   */
  protected SRBFileSystem srbFileSystem;


  /**
   * The type of SRB catalog.
   * The <code>{@link #MDAS_CATALOG}</code> is the default catalog.
   */
  static int catalogType = MDAS_CATALOG;
  //I don't think the other catalogs are ever used...


  /**
   * The storage resource name.
   * A physical SRB resource is a system that is capable of storing data sets
   * and is accessible to the SRB. It is registered in SRB with its physical
   * characteristics such as its physical location, resource type, latency,
   * and maximum file size. For example, HPSS can be a resource, as can a
   * Unix file system.
   */
  String resource;


  /**
   * The data type of the file. The default value is "generic".
   */
  String dataType = "generic";



  /**
   * If delete on exit gets set.
   */
  boolean deleteOnExit = false;


  static Vector uriFileSystems = new Vector();


  /**
   * Since all filepaths get stored as canonical paths (to avoid errors),
   * this variable keeps the original value of the path given to the
   * constructor for use by the getPath() method.
   */
  String originalFilePath;

//private, containers only have internal replicas.
  private int replicaNumber = -1;
//also note, if this object refers to a certain replica
//fileName will include &COPY= replica number
//getName() and getAbsolutePath() will not return the replica number
//getCanonicalPath() will return the replica number
//using the setReplicaNumber is preferred, however
//using "fileName&COPY=#" in the constructor is also valid.


  /**
   * For list() should the entire contents be listed, or just
   * SRBFileSystem.DEFAULT_RECORDS_WANTED in number.
   */
  public boolean completeDirectoryList = true;


  /**
   * Whether this abstract pathname is actually a directory or a file.
   * Reduces the number of network calls.
   */
  int pathNameType = PATH_IS_UNKNOWN;


  /**
   * The local filepath of the data when logged on the SRB server.
   * <P>
   * The <code>serverLocalPath</code> points to an abstract filepath
   * on the SRB server. Most files are stored in the SRB Vault, but
   * this file can be stored anywhere on the server that you have
   * permission. You must have a user account on the SRB server machine,
   * and that account must have read/write permissions to this abstract
   * filepath.
   */
  String serverLocalPath = "";

  /**
   * If true, the cached value for the isDirectory or isFile methods
   * will always be used. The cache can be refresh by calling the
   * <code>isFile(true)</code> or <code>isDirectory(true)</code>.
   * When false, inside any particular SRBFile method isFile(true) or
   * isDirectory(true) is called only once. However if you know the status as
   * a normal file will not change and want to perform a lot of actions on
   * that file, then setting this value to false could save a number of
   * network calls. Care should be taken though, as most methods don't have a
   * refresh cache option, so not updating at the beginning of each method could
   * cause errors.
   */
  boolean useCache = false;

//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
  /**
   * Creates a new <code>SRBFile</code> instance by converting the given
   * pathname string into an abstract pathname.
   *
   *<P>
   * @param fileSystem  The connection to the SRB
   * @param filePath  The pathname string
   * @throws  NullPointerException  If the given string is null or
   * the empty string.
   */
  public SRBFile( SRBFileSystem srbFileSystem, String filePath )
    throws NullPointerException
  {
    this( srbFileSystem, "", filePath );
    checkResource();
    originalFilePath = filePath;
  }


  /**
   * Creates a new <code>SRBFile</code> instance from a parent pathname
   * string and a child pathname string.
   *<P>
   * If <code>parent</code> is <code>null</code> then the new
   * <code>SRBFile</code> instance is created as if by invoking the
   * single-argument <code>SRBFile</code> constructor on the given
   * <code>child</code> pathname string.
   *<P>
   * Otherwise the <code>parent</code> pathname string is taken to denote a
   * directory, and the <code>child</code> pathname string is taken to denote
   * either a directory or a file. If the <code>child</code> pathname string
   * is absolute then it is converted into a relative pathname in a
   * SRB pathname. If parent is the empty string then the new
   * <code>SRBFile</code> instance is created by converting <code>child</code>
   * into an abstract pathname and resolving the result against the user's
   * SRB default home directory. Otherwise each pathname string is
   * converted into an abstract pathname and the <code>child</code> abstract
   * pathname is resolved against the <code>parent</code>.
   *<P>
   * @param fileSystem  The connection to the SRB
   * @param parent  The parent pathname string
   * @param child    The child pathname string
   * @throws  NullPointerException  If the child string is null or
   * the empty string.
   */
  public SRBFile( SRBFileSystem srbFileSystem, String parent, String child )
    throws NullPointerException, IllegalArgumentException
  {
    super( srbFileSystem, parent, child );

    makePathCanonical( parent );
    checkResource();

    //just for getPath()
    if (parent != null) {
      if (!parent.equals( "" )) {
        originalFilePath = parent + separator + child;
      }
      else {
        originalFilePath = child;
      }
    }
    else {
      originalFilePath = child;
    }
  }


  /**
   * Creates a new <code>SRBFile</code> instance from a parent abstract
   * pathname and a child pathname string.
   *<P>
   * If parent is null then the new <code>SRBFile</code> instance is created
   * as if by invoking the single-argument <code>SRBFile</code> constructor
   * on the given child pathname string.
   *<P>
   * Otherwise the <code>parent</code> abstract pathname is taken to denote a
   * directory, and the <code>child</code> pathname string is taken to denote
   * either a directory or a file. If the <code>child</code> pathname string is
   * absolute then it is converted into a relative pathname in a
   * SRB pathname. If <code>parent</code> is the empty abstract
   * pathname then the new <code>SRBFile</code> instance is created by
   * converting <code>child</code> into an abstract pathname and resolving
   * the result against the user's SRB default home directory. Otherwise
   * each pathname string is converted into an abstract pathname and the
   * <code>child</code> abstract pathname is resolved against the
   * <code>parent</code>.
   *<P>
   * @param parent  The parent abstract pathname
   * @param child    The child pathname string
   * @throws  NullPointerException  If the child string is null or
   * the empty string.
   */
  public SRBFile( SRBFile parent, String child )
    throws NullPointerException
  {
    this( (SRBFileSystem) parent.getFileSystem(),  parent.getPath(),
      child );
  }


  /**
   * Creates a new GeneralFile instance by converting the given file: URI
   * into an abstract pathname.
   *<p>
   *  SRB URI protocol:<br>
   *  srb:// [ userName . domainHome [ : password ] @ ] host [ : port ][ / path ]
   *<P>
   * example:<br>
   * srb://testuser.sdsc:mypassword@srb.sdsc.edu:5555/home/testuser.sdsc/testfile.txt
   *
   * @param uri An absolute, hierarchical URI using the srb scheme.
   * @throws NullPointerException if <code>uri</code> is <code>null</code>.
   * @throws IllegalArgumentException If the preconditions on the parameter
   *    do not hold.
   * @throws IOException Can occur during the creation of the internal
   *     fileSystem object.
   */
  public SRBFile( URI uri )
    throws NullPointerException, IOException
  {
    super(uri);

    //srb://userName.mdasDomain:password@host:port/path
    if (!uri.getScheme().equals("srb")) {
      throw new IllegalArgumentException();
    }

    SRBAccount account = uriInitialAccount( uri );
    SRBFileSystem tempFileSystem = uriAccountTest( account );
    String zone = null;
    String path = uri.getPath();
    String query = uri.getQuery();
    originalFilePath = path;

    if (tempFileSystem != null) {
      setFileSystem( tempFileSystem );
    }
    else {
      //The fileSystem is used by the following query.
      setFileSystem( new SRBFileSystem( account ));

      zone = getLocalZone( srbFileSystem );
      srbFileSystem.setProxyMcatZone( zone );
      srbFileSystem.setMcatZone( zone );

      srbFileSystem.setDefaultStorageResource(
        getAvailableResource(  srbFileSystem ) );

      //established the filesystem parameters.
      //TODO uriTest no longer necessary with the new uriAccountTest?
      setFileSystem( uriTest( srbFileSystem ) );
    }

    if (query != null) {
      int index = query.indexOf("GUID=");
      String temp = null;
      if (index >= 0) {
        MetaDataCondition conditions[] = {
          getZoneCondition(),
          MetaDataSet.newCondition(
            SRBMetaDataSet.GUID, MetaDataCondition.EQUAL,
            query.substring(index+5) )//, query.indexOf('&',index) ) ),
            //maybe add multiple conditions eventually
            //I guess GUID wouldn't every need them though
        };
        String[] selectFieldNames = {
          GeneralMetaData.FILE_NAME,
          GeneralMetaData.DIRECTORY_NAME
        };
        MetaDataSelect selects[] =
          MetaDataSet.newSelection( selectFieldNames );

        MetaDataRecordList rl[] = fileSystem.query(conditions, selects);
        if (rl == null) {
          throw new FileNotFoundException( "Invalid GUID" );
        }

        path = rl[0].getValue(GeneralMetaData.DIRECTORY_NAME).toString();
        temp = rl[0].getValue(GeneralMetaData.FILE_NAME).toString();
        if (temp != null) path += separator+temp;
      }

      //set the filename whether or not something was found
      setFileName( path );
    }
    else if ((path == null) || (path == "")) {
      setFileName( separator );
    }
    else {
      setFileName( path );
    }

  }

  /**
   * Finalizes the object by explicitly letting go of each of
   * its internally held values.
   */
  protected void finalize( )
    throws Throwable
  {
    if (deleteOnExit)
      delete();

    super.finalize();

    if (resource != null)
      resource = null;

    if (dataType != null)
      dataType = null;
  }


//----------------------------------------------------------------------
// Initialization for URIs
//----------------------------------------------------------------------
  /**
   * Step one of SRBFile( uri )
   */
  static SRBAccount uriInitialAccount( URI uri )
    throws IOException
  {
    //
    //I guess eventually, move all this to some Handler.java for SRB URIs.
    //
    String host = uri.getHost();
    int port = uri.getPort();
    String userInfo = uri.getUserInfo();
    String userName = null, mdasDomain = null, password = null;
    String homeDirectory = null;
    int index = -1;

    if ((userInfo == null) || (userInfo == "")) {
      //anon. login
      userName = SRBAccount.PUBLIC_USERNAME;
      mdasDomain = SRBAccount.PUBLIC_DOMAINNAME;
      password = SRBAccount.PUBLIC_PASSWORD;
      homeDirectory = SRBAccount.PUBLIC_HOME_DIRECTORY;
    }
    else {
      index = userInfo.indexOf( "." );
      if (index < 0) {
        throw new MalformedURLException();
      }
      userName = userInfo.substring( 0, index );

      if (index < 0) {
        throw new MalformedURLException();
      }
      userInfo = userInfo.substring( index+1 );

      index = userInfo.indexOf( ":" );
      if (index > 0) {
        mdasDomain = userInfo.substring( 0, index );
        password = userInfo.substring( index+1 );
      }
      else {
        mdasDomain = userInfo;
      }
      //set the home directory to the local zone
      homeDirectory = "/home/"+userName+"."+mdasDomain;
    }

    index = host.indexOf( "." );
    if ( index < 0 ) {
      //use zone authority
      URL url = new URL( ZONE_AUTHORITY +  host );
      InputStream in = url.openConnection().getInputStream();

      //not the best, doesn't use the xml.
      int index2 = -1;
      String result = null;
      byte[] data = new byte[1000];

      in.read(data);
      result = new String( data );
      //? first read always stops at the 26th byte
      in.read(data);
      result += new String( data );

      index = result.indexOf( "ns1:server" );
      index2 = result.indexOf( "/ns1:server", index+11 );
      if ((index < 0) || (index2 < 0))
        throw new ConnectException( "Invalid zone name." );
      host = result.substring( index+11, index2-1 );

      index = result.indexOf( "ns1:port" );
      index2 = result.indexOf( "/ns1:port", index+9 );
      result = result.substring( index+9, index2-1 );
      if ((result != null) && (result.length() > 0))
        port = Integer.parseInt( result );
    }

    if ( port < 0 ) {
      port = 5544;
    }

    //Have to find a storage resource after connection.
    return new SRBAccount( host, port, userName, password,
      homeDirectory, mdasDomain, "" );
  }

  /**
   * Step two of SRBFile( uri )
   */
  static String getLocalZone( SRBFileSystem fs )
    throws IOException
  {
//TODO should I use the file zone? does it have one?
    //Query file system to determine this SRB user's zone,
    //then use that for the zone of its account object
    if (fs.getVersionNumber() >= 3)
    {
      //if null then file does not exist (or is dir?)
      //find what is the user the user can access, pick the first one.
      MetaDataCondition[] conditions = {
        MetaDataSet.newCondition( SRBMetaDataSet.ZONE_LOCALITY,
          MetaDataCondition.EQUAL, 1 ),
      };
      MetaDataSelect[] selects = {
        MetaDataSet.newSelection( SRBMetaDataSet.ZONE_NAME ) };
      MetaDataRecordList[] rl = fs.query( conditions, selects );
      if (rl != null) {
        return rl[0].getValue(  SRBMetaDataSet.ZONE_NAME ).toString();
      }
    }
    return null;
  }

  /**
   * Step three of SRBFile( uri )
   */
  static String getAvailableResource( SRBFileSystem fs )
    throws IOException
  {
    MetaDataRecordList[] rl = null;
    if (fs.getVersionNumber() >= 3)
    {
      String userName = fs.getUserName();
      String mdasDomain = fs.getDomainName();
      String zone = fs.getMcatZone();


      //Query file system to determine this SRBFile's storage resource,
      //find what resources the user can access, pick one at random
      //then use that for the default resource of its fileSystem object
      MetaDataCondition[] conditions = {
        MetaDataSet.newCondition( SRBMetaDataSet.RSRC_ACCESS_PRIVILEGE,
          MetaDataCondition.LIKE, "%write%" ),
        MetaDataSet.newCondition( SRBMetaDataSet.RSRC_ACCS_USER_NAME,
          MetaDataCondition.EQUAL, userName),
        MetaDataSet.newCondition( SRBMetaDataSet.RSRC_ACCS_USER_DOMAIN,
          MetaDataCondition.EQUAL, mdasDomain ),
        MetaDataSet.newCondition( SRBMetaDataSet.RSRC_ACCS_USER_ZONE,
          MetaDataCondition.EQUAL, zone ),
      };
      MetaDataSelect[] selects = {
        MetaDataSet.newSelection( SRBMetaDataSet.RESOURCE_NAME ) };
      rl = fs.query( conditions, selects );

      if (rl == null) {
        //Same as above, just no zone
        //Metadata to determine available resources was added only after SRB3
        rl = fs.query( SRBMetaDataSet.RESOURCE_NAME );
        if ((rl == null) && (!userName.equals("public"))) {
          //if null then file does not exist (or is dir?)
          //public user never has resources, so can't commit files, so it doesn't matter.
          throw new FileNotFoundException( "No resources available" );
        }
      }
    }
    if (rl != null) {
      //The first one was sometimes causing trouble,
      //pick a random one so it works some of the time at least
      int random = (int)Math.round((rl.length-1)*Math.random());
      return rl[random].getValue( SRBMetaDataSet.RESOURCE_NAME ).toString();
    }
    return "";
  }


  /**
   * Step four of SRBFile( uri )
   */
  static synchronized SRBFileSystem uriTest( SRBFileSystem fs )
  {
    //having multiple connections to the same filesystem is bad
    //users can open a thousand files, which will open 1000 filesystem
    //connections and might even crash the SRB.
    //TODO? synchronized
    SRBFileSystem uriTest = null;
    for (int i=0;i<uriFileSystems.size();i++) {
      uriTest = (SRBFileSystem) uriFileSystems.get( i );
      if ( fs.equals( uriTest ) ) {
        fs = uriTest;
        return fs;
      }
      else if ( !uriTest.isConnected() ) {
        uriFileSystems.remove( i );
      }
    }
    //add the new fileSystem to the list
    uriFileSystems.add( fs );

    //maybe unnecessary?
    return fs;
  }


  /**
   * Step four of SRBFile( uri )
   */
  static synchronized SRBFileSystem uriAccountTest( SRBAccount account )
  {
    //having multiple connections to the same filesystem is bad
    //users can open a thousand files, which will open 1000 filesystem
    //connections and might even crash the SRB.
    //TODO? synchronized
    SRBFileSystem uriTest = null;
    for (int i=0;i<uriFileSystems.size();i++) {
      uriTest = (SRBFileSystem) uriFileSystems.get( i );
      if ( account.equals( (SRBAccount)uriTest.getAccount(), false ) ) {
        if (uriTest.isConnected()) {
          return uriTest;
        }
      }
      else if ( !uriTest.isConnected() ) {
        uriFileSystems.remove( i );
      }
    }
    return null;
  }

  private void checkResource()
  {
    if (resource == null) {
      if (!exists())
        resource = srbFileSystem.getDefaultStorageResource();
      else {
//TODO throws IOException...
//also slows things down a bit with the added network call.
        //resource = getResource();
      }
    }
  }


//----------------------------------------------------------------------
// Setters and Getters
//----------------------------------------------------------------------
  /**
   * Set the file name.
   * @param fleName The file name or fileName plus some or all of the
   * directory path.
   */
  protected void setFileName( String filePath )
  {
    //used when parsing the filepath
    int index;

    //in case they used the local separator
    //in the fileName instead of the srb separator.
    String localSeparator = File.separator;

    if ( filePath == null ) {
      throw new NullPointerException( "The file name cannot be null" );
    }

    //replace local separators with SRB separators.
    if (!localSeparator.equals(separator)) {
      index = filePath.lastIndexOf( localSeparator );
      while ((index >= 0) && ((filePath.substring( index + 1 ).length()) > 0)) {
        filePath = filePath.substring( 0, index ) + separator +
          filePath.substring( index + 1 );
        index = filePath.lastIndexOf( localSeparator );
      }
    }
    fileName = filePath;

    if (fileName.length() > 1) { //add to allow path = root "/"
      index = fileName.lastIndexOf( separator );
      while ((index == fileName.length()-1) && (index >= 0)) {
        //remove '/' at end of filename, if exists
        fileName =  fileName.substring( 0, index );
        index = fileName.lastIndexOf( separator );
      }

      //seperate directory and file
      if ((index >= 0) &&
        ((fileName.substring( index + 1 ).length()) > 0)) {
        //have to run setDirectory(...) again
        //because they put filepath info in the filename
        setDirectory( fileName.substring( 0, index + 1 ) );
        fileName =  fileName.substring( index + 1 );
      }
    }
  }


  /**
   * Set the directory.
   * @param dir  The directory path, need not be absolute.
   */
//though everything will be converted to a canonical path  to avoid errors
  protected void setDirectory( String dir )
  {
    if (directory == null) {
      directory = new Vector();
    }
    if (dir == null) {
      return;
    }

    //in case they used the local separator
    //in the fileName instead of the srb separator.
    String localSeparator = File.separator;
    int index = dir.lastIndexOf( localSeparator );
    if ((index >= 0) && ((dir.substring( index + 1 ).length()) > 0)) {
      dir = dir.substring( 0, index ) + separator +
        dir.substring( index + 1 );
      index = dir.lastIndexOf( localSeparator );
    }

    while ((directory.size() > 0) && //only if this is the dir cut from fileName
            dir.startsWith(separator))// &&  //strip these
//            (dir.length() > 1)) //but not if they only wanted
    {
      dir = dir.substring(1);
//problems if dir passed from filename starts with separator
    }

    //create directory vector
    index = dir.indexOf( separator );

    if (index >= 0) {
      do {
        directory.add( dir.substring( 0, index ) );
        do {
          dir = dir.substring( index + 1 );
          index = dir.indexOf( separator );
        } while (index == 0);
      } while (index >= 0);
    }
    //add the last path item
    if ((!dir.equals("")) && (dir != null)) {
      directory.add( dir );
    }
  }


  /**
   * Helper for setting the directory to an absolute path
   * @param dir Used to determine if the path is absolute.
   */
//Yes, this whole business is the most horrible thing you have ever seen.
//
//using "fileName&COPY=#" in the constructor should stay valid.
  void makePathCanonical( String dir )
  {
    int i = 0; //where to insert into the Vector
    boolean absolutePath = false;
    String canonicalTest = null;

    if (dir == null) {
      dir = "";
    }

    //In case this abstract path is supposed to be root
    if ((fileName.equals(SRBFileSystem.SRB_ROOT)) && (dir == "")) {
      return;
    }

    //In case this abstract path is supposed to be the home directory
    if (fileName.equals("") && dir.equals("")) {
      String home = fileSystem.getHomeDirectory();
      int index = home.lastIndexOf( separator );
      setDirectory( home.substring( 0, index ) );
      setFileName( home.substring( index+1 ) );
      return;
    }


    //if dir not absolute
    if (dir.startsWith(SRBFileSystem.SRB_ROOT))
      absolutePath = true;

    //if directory not already absolute
    if (directory.size() > 0) {
      if (directory.get(0).toString().length() == 0) {
        //The /'s were all striped when the vector was created
        //so if the first element of the vector is null
        //but the vector isn't null, then the first element
        //is really a /.
        absolutePath = true;
      }
    }
    if (!absolutePath) {
      String home = fileSystem.getHomeDirectory();
      int index = home.indexOf( separator );
      //allow the first index to = 0,
      //because otherwise separator won't get added in front.
      if (index >= 0) {
        do {
          directory.add( i, home.substring( 0, index ) );
          home = home.substring( index + 1 );
          index = home.indexOf( separator );
          i++;
        } while (index > 0);
      }
      if ((!home.equals("")) && (home != null)) {
        directory.add( i, home );
      }
    }


    //first, made absolute, then canonical
    for (i=0; i<directory.size(); i++) {
      canonicalTest = directory.get(i).toString();
      if (canonicalTest.equals( "." )) {
        directory.remove( i );
        i--;
      }
      else if ((canonicalTest.equals( ".." )) && (i >= 2)) {
        directory.remove( i );
        directory.remove( i-1 );
        i--;
        if (i > 0)
          i--;
      }
      else if (canonicalTest.equals( ".." )) {
        //at root, just remove the ..
        directory.remove( i );
        i--;
      }
      else if (canonicalTest.startsWith( separator )) {
        //if somebody put filepath as /foo//bar or /foo////bar
        do {
          canonicalTest = canonicalTest.substring( 1 );
        } while (canonicalTest.startsWith( separator ));
        directory.remove( i );
        directory.add( i, canonicalTest );
      }
    }
    //also must check fileName
    if (fileName.equals( "." )) {
      fileName = directory.get(directory.size()-1).toString();
      directory.remove( directory.size()-1 );
    }
    else if (fileName.equals( ".." )) {
      if (directory.size() > 1) {
        fileName = directory.get(directory.size()-2).toString();
        directory.remove( directory.size()-1 );
        directory.remove( directory.size()-1 );
      }
      else {
        //at root
        fileName = separator;
        directory.remove( directory.size()-1 );
      }
    }


    //maybe they tried to be smart and put the replica number on the filename
    //filename&COPY=#
    getReplicaNumber();
  }


  /**
   * Sets the SRB server used of this SRBFile object.
   *
   * @param fileSystem The SRB server to be used.
   * @throws IllegalArgumentException - if the argument is null.
   * @throws ClassCastException - if the argument is not a SRBFileSystem object.
   */
  protected void setFileSystem( GeneralFileSystem fileSystem )
    throws IllegalArgumentException
  {
    if ( fileSystem == null )
      throw new IllegalArgumentException("Illegal fileSystem, cannot be null");

    this.fileSystem = fileSystem;
    this.srbFileSystem = (SRBFileSystem) this.fileSystem;
  }



  /**
   * Sets the physical resource this SRBFile object will be stored on.
   *
   *
   * @param resource The name of resource to be used.
   * @throws NullPointerException If resourceName is null.
   * @throws IllegalArgumentException If resourceName is not a valid resource.
   * @throws IOException If an IOException occurs during the system change.
   */
  public void setResource( String resourceName )
    throws IOException, NullPointerException, IllegalArgumentException
  {
    if ( resourceName != null ) {

      resource = resourceName;
    }
    else if (false) {
      MetaDataCondition[] conditions = {
        getZoneCondition(),
        MetaDataSet.newCondition(
        SRBMetaDataSet.RESOURCE_NAME, MetaDataCondition.EQUAL, resourceName ) };
      MetaDataSelect[] selects = { MetaDataSet.newSelection(
        SRBMetaDataSet.RESOURCE_NAME ) };
      MetaDataRecordList[] resources =
        fileSystem.query( conditions, selects );

      if (resources == null)
        throw new IllegalArgumentException("Resource not found");

      //else resource exists
      resource = resourceName;

      if (isFile()) {
        boolean move = true;

        conditions = new MetaDataCondition[] {
//          getZoneCondition(),
          MetaDataSet.newCondition( GeneralMetaData.DIRECTORY_NAME,
            MetaDataCondition.EQUAL, getParent() ),
          MetaDataSet.newCondition( GeneralMetaData.FILE_NAME,
            MetaDataCondition.EQUAL, fileName )
        };
        resources = fileSystem.query( conditions, selects );

        if (resources != null) {
          for (int i=0;i<resources.length;i++) {
            if (resources[i].getStringValue(0) == resourceName) {
              move = false;
              i=resources.length;
            }
          }
        }
        if (move) {
//TODO maybe a better function available?
          srbFileSystem.srbObjMove( catalogType, fileName, getParent(),
            null, resource, serverLocalPath, null );
        }
      }
    }
    else
      throw new NullPointerException();
  }

  /**
   * Sets the dataType string of this SRBFile. If <code>dataTypeName</code>
   * is null, the default type of "generic", will be used.
   *
   * @throws IOException If an IOException occurs during the system change.
   */
  public void setDataType( String dataTypeName )
    throws IOException
  {
    if ( dataTypeName == null ) {
      dataTypeName = "generic";
    }

    if (isFile()) {
      if (getReplicaNumber() >= 0) {
        srbFileSystem.srbModifyDataset( catalogType,
          fileName,  getParent(), null, null, dataTypeName,
          null, SRBMetaDataSet.D_CHANGE_TYPE);
      }
      else {
        srbFileSystem.srbModifyDataset( catalogType, getName(),
          getParent(), null, null, dataTypeName, null,
          SRBMetaDataSet.D_CHANGE_TYPE);
      }
    }

    dataType = dataTypeName;
  }


  /**
   * Sets the catalogType string of this SRBFile. The default type is
   * <code>MDAS_CATALOG</code>
   */
/*
  public void setCatalogType( int catalog )
  {
    if ( catalog > 0 )
      this.catalogType = catalog;
    else
      this.catalogType = 0;
  }
*/


  /**
   * Sets the specific physical data replication refered to by this object.
   * ReplicaNumbers are always positive. setting a negative number will
   * remove the any specific replica reference.
   */
  public void setReplicaNumber( int replicaNumber )
  {
    if (replicaNumber >= 0) {
      //getName() strips the old number
      fileName = getName() + "&COPY=" + replicaNumber;
      this.replicaNumber = replicaNumber;
    }
    else {
      fileName = getName();
      this.replicaNumber = -1;
    }
  }
//note, if this object refers to a certain replica
//the global GeneralFile variable fileName will include &COPY= replica number
//getName() and getAbsolutePath() will not return the replica number
//getCanonicalPath() will return the replica number
//using the setReplicaNumber() is preferred, however
//using "fileName&COPY=#" in the constructor is also valid,
//because I don't want to parse it there.

//If users add &COPY= to the files on their own, bad stuff can happen?


  /**
   * The local filepath of the data when logged on the SRB server.
   * <P>
   * The <code>serverLocalPath</code> points to a filepath
   * on the SRB server (the directory plus filename.) Most files
   * are stored in the SRB Vault, but this file can be stored anywhere on
   * the server that you have permissions. You must have a user account on
   * the SRB server machine, and that account must have read/write
   * permissions to this abstract filepath.
   */
  public void setServerLocalPath( String serverLocalPath )
  {
    this.serverLocalPath = serverLocalPath;
  }

  /**
   * Returns a metadata condition that describes the current zone.
   * Used in federated queries, because the server needs to know which zone
   * is used.
   */
  MetaDataCondition getZoneCondition()
  {
    //skip root, get the next directory level
    String zone = (String) directory.get(1);
    if ((zone == null) ||
      (zone == LOCAL_CONTAINER_DIRECTORY) ||
      (zone == LOCAL_HOME_DIRECTORY) ||
      (zone == LOCAL_STYLES_DIRECTORY) ||
      (zone == LOCAL_TRASH_DIRECTORY))
    {
      return null;
    }

    return MetaDataSet.newCondition( SRBMetaDataSet.CURRENT_ZONE,
      MetaDataCondition.EQUAL, zone );
  }

  /**
   * Gets the specific physical data replication refered to by this object.
   *
   * @return The replica number refered to by this SRBFile object.
   * Returns a negative value if this SRBFile object does not refer to a
   * specific replica.
   */
  public int getReplicaNumber( )
  {
    if (replicaNumber >= 0) {
      return replicaNumber;
    }

    int index = fileName.indexOf( "&COPY=" );
    if (index >= 0) {
      replicaNumber = new Integer(fileName.substring( index+6 )).intValue();
      return replicaNumber;
    }
    else {
      return -1;
    }
  }


  /**
   * @return resource the physical resource where this SRBFile is stored.
   *    Will not query the SRB if this abstract pathname is a directory.
   *     Returns null if the file is a directory or does not exist.
   *
   * @throws IOException If an IOException occurs during the system query.
   */
  public String getResource( )
    throws IOException
  {
    if (isDirectory()) {
      return null;
    }

    if (resource != null) {
      return resource;
    }

    //if replicaNumber, filename, path will done in query()
    return firstQueryResult( SRBMetaDataSet.PHYSICAL_RESOURCE_NAME );
  }



  /**
   * @return dataType  The dataType string of this SRBFile.
   *    Will not query the SRB if this abstract pathname is a directory.
   *     Returns null if the file does not exist.
   *
   * @throws IOException If an IOException occurs during the system query.
   */
  public String getDataType( )
    throws IOException
  {
    if (isDirectory()) {
      return dataType;
    }

    return firstQueryResult( SRBMetaDataSet.FILE_TYPE_NAME );
  }


  /**
   * @return catalogType the catalog type int of this SRBFile.
   */
  public int getCatalogType( )
  {
    return catalogType;
  }


  /**
   * This method gets the path separator as defined by the SRB protocol.
   * @deprecated Use separator and pathSeparator
   */
  public final String getPathSeparator( )
  {
    return separator;
  }


  /**
   * This method gets the path separator char as defined by the SRB protocol.
   * @deprecated Use separatorChar and pathSeparatorChar
   */
  public final char getPathSeparatorChar( )
  {
    return separatorChar;
  }




  /**
   * @return fileSystem the SRBFileSystem object of this SRBFile.
   * @throws  NullPointerException  if fileSystem is null.
   */
  public GeneralFileSystem getFileSystem( )
    throws NullPointerException
  {
    if ( srbFileSystem != null )
      return srbFileSystem;

    throw new NullPointerException("fileSystem is null.");
  }

  /**
   * The local filepath of the data when logged on the SRB server.
   * <P>
   * The <code>serverLocalPath</code> points to a filepath
   * on the SRB server (the directory plus filename.) Most files
   * are stored in the SRB Vault, but this file can be stored anywhere on
   * the server that you have permissions. You must have a user account on
   * the SRB server machine, and that account must have read/write
   * permissions to this abstract filepath.
   */
  public String getServerLocalPath( )
  {
    if ((serverLocalPath == null) || (serverLocalPath.equals(""))) {
      try {
       return firstQueryResult( SRBMetaDataSet.PATH_NAME );
      } catch (IOException e) {
        if (SRBCommands.DEBUG > 0) {
          e.printStackTrace();
        }
      }
    }
    return serverLocalPath;
  }





//----------------------------------------------------------------------
// GeneralFile Methods
//----------------------------------------------------------------------
//TODO add ResultSet executeQuery( ... )

  /**
   * Queries metadata specific to this SRBFile object and selects one
   * metadata value, <code>fieldName</code>, to be returned.
   *
   * @param fieldName The string name used to form the select object.
   * @return The metadata values for this file refered to by
   * <code>fieldName</code>
   */
  public MetaDataRecordList[] query( String fieldName )
     throws IOException
  {
    MetaDataSelect[] temp = { MetaDataSet.newSelection( fieldName ) };
    return query( temp, fileSystem.DEFAULT_RECORDS_WANTED );
  }


  /**
   * Queries metadata specific to this SRBFile object.
   *
   * @param fieldNames The string names used to form the select objects.
   * @return The metadata values for this file refered to by
   * <code>fieldNames</code>
   */
  public MetaDataRecordList[] query( String[] fieldNames )
     throws IOException
  {
    return query( MetaDataSet.newSelection( fieldNames ),
      fileSystem.DEFAULT_RECORDS_WANTED );
  }


  /**
   * Queries metadata specific to this SRBFile object and selects one
   * metadata value, <code>select</code>, to be returned.
   */
  public MetaDataRecordList[] query( MetaDataSelect select )
     throws IOException
  {
    MetaDataSelect[] temp = { select };
    return query( temp, fileSystem.DEFAULT_RECORDS_WANTED );
  }


  /**
   * Queries metadata specific to this SRBFile object.
   */
  public MetaDataRecordList[] query( MetaDataSelect[] selects )
     throws IOException
  {
    return query( selects, fileSystem.DEFAULT_RECORDS_WANTED );
  }


  public MetaDataRecordList[] query( MetaDataSelect[] selects,
    int recordsWanted )
    throws IOException
  {
    //doesn't need to use getZoneCondition() because will always have a
    //DIRECTORY_NAME condition

    MetaDataCondition iConditions[] = null;
    String fieldName = null;
    int operator = MetaDataCondition.EQUAL;
    String value = null;

    if (isDirectory()) {
      iConditions = new MetaDataCondition[1];
      fieldName = GeneralMetaData.DIRECTORY_NAME;
      operator = MetaDataCondition.EQUAL;
      value = getAbsolutePath();
      iConditions[0] =
        MetaDataSet.newCondition( fieldName, operator, value );
    }
    else {
      iConditions = new MetaDataCondition[3];
      fieldName = GeneralMetaData.DIRECTORY_NAME;
      operator = MetaDataCondition.EQUAL;
      value = getParent();
      iConditions[0] =
        MetaDataSet.newCondition( fieldName, operator, value );

      fieldName = GeneralMetaData.FILE_NAME;
      value = getName();
      iConditions[1] =
        MetaDataSet.newCondition( fieldName, operator, value );

      if (getReplicaNumber() >= 0) {
        fieldName = SRBMetaDataSet.FILE_REPLICATION_ENUM;
        value = "" + replicaNumber;
        iConditions[2] =
          MetaDataSet.newCondition( fieldName, operator, value );
      }
      //else last condition = null, will be ignored
    }

    return fileSystem.query( iConditions, selects, recordsWanted );
  }


  public MetaDataRecordList[] query( MetaDataCondition[] conditions,
    MetaDataSelect[] selects )
    throws IOException
  {
    return query( conditions, selects, fileSystem.DEFAULT_RECORDS_WANTED );
  }


  public MetaDataRecordList[] query(  MetaDataCondition[] conditions,
    MetaDataSelect[] selects, int recordsWanted )
    throws IOException
  {
    //doesn't need to use getZoneCondition() because will always have a
    //DIRECTORY_NAME condition

    MetaDataCondition iConditions[] = null;
    String fieldName = null;
    int operator = MetaDataCondition.EQUAL;
    String value = null;

    if (isDirectory()) {
      iConditions = new MetaDataCondition[conditions.length+1];

      System.arraycopy( conditions, 0, iConditions, 0, conditions.length);

      fieldName = GeneralMetaData.DIRECTORY_NAME;
      operator = MetaDataCondition.EQUAL;
      value = getAbsolutePath();
      iConditions[conditions.length] =
        MetaDataSet.newCondition( fieldName, operator, value );
    }
    else {
      iConditions = new MetaDataCondition[conditions.length+3];

      System.arraycopy( conditions, 0, iConditions, 0, conditions.length);

      fieldName = GeneralMetaData.DIRECTORY_NAME;
      operator = MetaDataCondition.EQUAL;
      value = getParent();
      iConditions[conditions.length] =
        MetaDataSet.newCondition( fieldName, operator, value );

      fieldName = GeneralMetaData.FILE_NAME;
      value = fileName;
      iConditions[conditions.length+1] =
        MetaDataSet.newCondition( fieldName, operator, value );

      if (getReplicaNumber() >= 0) {
        fieldName = SRBMetaDataSet.FILE_REPLICATION_ENUM;
        value = "" +replicaNumber;
        iConditions[2] =
          MetaDataSet.newCondition( fieldName, operator, value );
      }
      //else last condition = null, will be ignored
    }

    return fileSystem.query( iConditions, selects, recordsWanted );
  }




  /**
   * Change the values of the metadata associated with this file object.
   * Not all metadata values can be modified. Also not all metadata value
   * types apply to every object, i.e. directories and files have different
   * metadata types.
   *
   * The following is a list of modifiable metadata types:
   * (see also SRBMetaDataSet)
   * CONTAINER_FOR_DIRECTORY<br>
   * DIRECTORY_ANNOTATION<br>
   * DIRECTORY_LINK_NUMBER<br>
   * DIRECTORY_NAME<br>
   * FILE_ANNOTATION<br>
   * FILE_ANNOTATION_POSITION<br>
   * FILE_AUDITFLAG<br>
   * FILE_CHECKSUM<br>
   * FILE_CLASS_NAME<br>
   * FILE_COMMENTS<br>
   * FILE_EXPIRE_DATE_2<br>
   * FILE_EXPIRY_DATE<br>
   * FILE_HIDE<br>
   * FILE_IS_COMPRESSED<br>
   * FILE_IS_ENCRYPTED<br>
   * FILE_LAST_ACCESS_TIMESTAMP<br>
   * FILE_LOCK_NUM<br>
   * FILE_NAME<br>
   * FILE_PIN_VAL<br>
   * FILE_REPLICATION_ENUM<br>
   * FILE_TYPE_NAME<br>
   * GUID<br>
   * INDEX_NAME_FOR_DATATYPE<br>
   * INDEX_NAME_FOR_DIRECTORY<br>
   * INDEX_NAME_FOR_FILE<br>
   * IS_DIRTY<br>
   * METHOD_NAME_FOR_DATATYPE<br>
   * METHOD_NAME_FOR_DIRECTORY<br>
   * METHOD_NAME_FOR_FILE<br>
   * MODIFICATION_DATE<br>
   * OFFSET<br>
   * OWNER<br>
   * PATH_NAME<br>
   * SIZE<br>
   * DEFINABLE_METADATA_FOR_FILES<br>
   * DEFINABLE_METADATA_FOR_DIRECTORIES<br>
   *<br>
   * note: Dates timestamps are Strings with the format: YYYY-MM-DD-HH.MM.SS
   *
   * @throws FileNotFoundException If the vault storage path can't be found.
   *    This can occur if the file was improperly deleted, removing the data
   *    on disk without removing the metadata.
   */
  public void modifyMetaData( MetaDataRecordList record )
    throws IOException
  {
//The srb C version, sort of,
//catalog/mdas-srb/srbC_mdas_library.c - modify_dataset_info(...)

    //the new metadata value, or various other uses...
    String dataValue1 = null, dataValue2 = null, dataValue3 = null;
    String fieldName = null;

    //Used by extensible modifications
    SRBProtocol protocol = new SRBProtocol();
    String extTableName = null, extAttributes = null, extValues = null;

    //This type of metadata only exists for files/dirs.
    boolean filesOnly = false;
    boolean dirsOnly = false;
    boolean isFile = isFile();

    //If there is no replica number,
    //Must get the actual file path on the machine which stores the data
    //and the storage resource.
    //Have to query first to get this.
    String vaultPathName = null;
    if ((serverLocalPath == null) || (serverLocalPath.equals(""))) {
      if ((getReplicaNumber() < 0) && isFile) {
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.PATH_NAME),
          MetaDataSet.newSelection(SRBMetaDataSet.RESOURCE_NAME) };
        MetaDataRecordList rl[] = query( selects );
        if (rl != null) {
          vaultPathName = rl[0].getValue( SRBMetaDataSet.PATH_NAME ).toString();
          resource = rl[0].getValue( SRBMetaDataSet.RESOURCE_NAME ).toString();
        }
        else {
          throw new FileNotFoundException( "Vault path/resource not found." );
        }
      }
    }
    else {
      vaultPathName = serverLocalPath;
    }
    //tells the srb what metadata is being changed
    int retractionType = -1;

    //if this is a MetaData type that can be modified.
//    boolean modifiable = false;

    //if the metadata is to be deleted, it uses a different retraction type.
    boolean delete = false;

    for (int i=0;i<record.getFieldCount();i++) {
      if (record.getValue(i) == null)
        delete = true;
      else
        delete = false;

      fieldName = record.getFieldName(i);
      if (fieldName == FileMetaData.FILE_COMMENTS)
      {
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_COMMENTS;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_COMMENTS;
          dataValue1 = record.getStringValue(i);
        }
      }
      else if (fieldName == SRBMetaDataSet.SIZE)
      {
        retractionType = SRBMetaDataSet.D_CHANGE_SIZE;
        dataValue1 = record.getStringValue(i);
        filesOnly = true;
      }
      else if (fieldName == SRBMetaDataSet.FILE_TYPE_NAME)
      {
        retractionType = SRBMetaDataSet.D_CHANGE_TYPE;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == GeneralMetaData.DIRECTORY_NAME)
      {
        retractionType = SRBMetaDataSet.D_CHANGE_GROUP;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.OFFSET)
      {
        retractionType = SRBMetaDataSet.D_CHANGE_OFFSET;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.IS_DIRTY)
      {
        retractionType = SRBMetaDataSet.D_CHANGE_DIRTY;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_REPLICATION_ENUM)
      {
//oldReplNum:newReplNum, oldVerNum:newVerNum  use -1 for new values for using
//internal  generated number
        retractionType = SRBMetaDataSet.D_CHANGE_REPLNUM_VERNUM;
        dataValue1 = record.getStringValue(i);
  //@@@
      }
      else if (fieldName == SRBMetaDataSet.FILE_LOCK_NUM)
      {
//accessName, not_used  use read, write  or null for accessName  read=
//readLock not even read is allowed  write= writeLock write not allowed
//null= remove lock.
        retractionType = SRBMetaDataSet.D_INSERT_LOCK;
        dataValue1 = record.getStringValue(i);
      //@@@
      }
      else if (fieldName == SRBMetaDataSet.FILE_CHECKSUM)
      {
        retractionType = SRBMetaDataSet.D_INSERT_DCHECKSUM;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_HIDE)
      {
        retractionType = SRBMetaDataSet.D_INSERT_DHIDE;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_AUDITFLAG)
      {
        retractionType = SRBMetaDataSet.D_INSERT_AUDITFLAG;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_PIN_VAL)
      {
//pinNumber, pinExpiryDate  pinNumber = 0 for no pin  pinExpiryDate = YYYY-MM-DD-HH.MM.SS
        retractionType = SRBMetaDataSet.D_UPDATE_PIN;
        dataValue1 = record.getStringValue(i);
      //@@@
      }
      else if (fieldName == SRBMetaDataSet.FILE_EXPIRY_DATE)
      {
//expiryDate = YYYY-MM-DD-HH.MM.SS
        retractionType = SRBMetaDataSet.D_UPDATE_DEXPIRE_DATE;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_EXPIRE_DATE_2)
      {
//expiryDate = YYYY-MM-DD-HH.MM.SS
        retractionType = SRBMetaDataSet.D_UPDATE_DEXPIRE_DATE_2;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_IS_COMPRESSED)
      {
//compressionInfo is a string
        retractionType = SRBMetaDataSet.D_UPDATE_DCOMPRESSED;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_IS_ENCRYPTED)
      {
//encryptionInfo is a string
        retractionType = SRBMetaDataSet.D_UPDATE_DENCRYPTED;
        dataValue1 = record.getStringValue(i);
      }
/*
  if (fieldName == SRBMetaDataSet.)
  {
//newCollName, newDataName  links the data to this srbObjectPath
    retractionType = SRBMetaDataSet.D_INSERT_LINK;
    dataValue1 = record.getStringValue(i);
  //@@@
  }
*/
      else if (fieldName == GeneralMetaData.FILE_NAME)
      {
        //same as renameTo(GeneralFile)
        retractionType = SRBMetaDataSet.D_CHANGE_DNAME;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_ANNOTATION)
      {
        Object temp = record.getValue(
          SRBMetaDataSet.FILE_ANNOTATION_TIMESTAMP);
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_ANNOTATIONS;
          if (temp != null) {
             dataValue1 = temp.toString();
          }
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_ANNOTATIONS;
          dataValue1 = record.getStringValue(i);
          if (temp != null) {
             dataValue2 = temp.toString();
          }
          temp = record.getValue(
            SRBMetaDataSet.FILE_ANNOTATION_POSITION);
          if (temp != null) {
             dataValue2 = temp.toString();
          }
        }
      }
      else if (fieldName == SRBMetaDataSet.DIRECTORY_ANNOTATION)
      {
        Object temp = record.getValue(
          SRBMetaDataSet.DIRECTORY_ANNOTATION_TIMESTAMP);
        if (delete) {
          retractionType = SRBMetaDataSet.C_DELETE_ANNOTATIONS;
          dataValue2 = record.getStringValue(i);
          if (temp != null) {
             dataValue1 = temp.toString();
          }
        }
        else {
          retractionType = SRBMetaDataSet.C_INSERT_ANNOTATIONS;
          dataValue1 = record.getStringValue(i);
          if (temp != null) {
             dataValue2 = temp.toString();
          }
        }
      }
      else if (fieldName == GeneralMetaData.OWNER)
      {
//newOwnerName, newOwnerDomain[|Zone]
        retractionType = SRBMetaDataSet.D_CHANGE_OWNER;
        dataValue1 = record.getStringValue(i);
      //@@@
      }
      else if (fieldName == SRBMetaDataSet.PATH_NAME)
      {
//newPathName, not_used  (be careful how you use this!)
        retractionType = SRBMetaDataSet.D_CHANGE_DPATH;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.MODIFICATION_DATE)
      {
//newTimeStamp, not_used  newTimeStamp=YYYY-MM-DD-HH.MM.SS
        retractionType = SRBMetaDataSet.D_CHANGE_MODIFY_TIMESTAMP;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_LAST_ACCESS_TIMESTAMP)
      {
//newAccessTime, not_used  newAccessTime=YYYY-MM-DD-HH.MM.SS
        retractionType = SRBMetaDataSet.D_CHANGE_LAST_ACCS_TIME;
        dataValue1 = record.getStringValue(i);
      }
      else if (fieldName == SRBMetaDataSet.FILE_CLASS_NAME)
      {
//classTypeString,classNameString  (user-defined)
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_CLASS;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_CLASS;
          dataValue1 = record.getStringValue(i);
        }
      //@@@
      }
      else if (fieldName == SRBMetaDataSet.INDEX_NAME_FOR_FILE)
      {
//srbObjName, not_used  srbObjName contains the index values  used by the data_name
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_INDEX_FOR_DATA;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_INDEX_FOR_DATA;
          dataValue1 = record.getStringValue(i);
        }
      }
      else if (fieldName == SRBMetaDataSet.INDEX_NAME_FOR_DATATYPE)
      {
//dataTypeName,not_used  dataType for which the data_name cane be  used as index
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_INDEX_FOR_DATATYPE;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_INDEX_FOR_DATATYPE;
          dataValue1 = record.getStringValue(i);
        }
      }
      else if (fieldName == SRBMetaDataSet.INDEX_NAME_FOR_DIRECTORY)
      {
//collectionName,not_used  Collection for which the data_name can be  used as index
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_INDEX_FOR_COLLECTION;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_INDEX_FOR_COLLECTION;
          dataValue1 = record.getStringValue(i);
        }
      }
      else if (fieldName == SRBMetaDataSet.METHOD_NAME_FOR_FILE)
      {
//srbObjName, not_used  srbObjName contains the method applicable  to  the data_name
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_METHOD_FOR_DATA;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_METHOD_FOR_DATA;
          dataValue1 = record.getStringValue(i);
        }
      }
      else if (fieldName == SRBMetaDataSet.METHOD_NAME_FOR_DATATYPE)
      {
//dataTypeName,not_used  dataType for which the data_name cane be  used as a method
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_METHOD_FOR_DATATYPE;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_METHOD_FOR_DATATYPE;
          dataValue1 = record.getStringValue(i);
        }
      }
      else if (fieldName == SRBMetaDataSet.METHOD_NAME_FOR_DIRECTORY)
      {
//collectionName, not_used  collection for whose data  the data_name
//can be used as a method
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_METHOD_FOR_COLLECTION;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_METHOD_FOR_COLLECTION;
          dataValue1 = record.getStringValue(i);
        }
      }
      //may error if try to use this attribute with a dir, or reverse
      else if (fieldName == SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES)
      {
        if (delete) {
          if (vaultPathName == null) {
            srbFileSystem.srbModifyDataset( catalogType,
              fileName, getParent(), null, null, "-1",
              "",  SRBMetaDataSet.D_DELETE_USER_DEFINED_STRING_META_DATA);
          }
          else {
            srbFileSystem.srbModifyDataset( catalogType,
              getName(), getParent(), resource, vaultPathName, "-1",
              "", SRBMetaDataSet.D_DELETE_USER_DEFINED_STRING_META_DATA);
          }
        }
        else {
          MetaDataTable table = record.getTableValue(i);
          dataValue2 = MODIFY_DEFINABLE_METADATA_SEPARATOR;
          for (int j=0;j<table.getRowCount();j++) {
            dataValue1 = "";
            int columns = table.getColumnCount();
            for (int k=0;k<columns;k++) {
              if (k == (columns - 1)) {
                dataValue1 += table.getStringValue(j,k);
              }
              else {
                dataValue1 += table.getStringValue(j,k) + dataValue2;
              }
            }
            if (vaultPathName == null) {
              srbFileSystem.srbModifyDataset( catalogType,
                fileName, getParent(), null, null, dataValue1,
                dataValue2,  SRBMetaDataSet.D_BULK_INSERT_UDEF_META_DATA_FOR_DATA);
            }
            else {
              srbFileSystem.srbModifyDataset( catalogType,
                getName(), getParent(), resource, vaultPathName, dataValue1,
                dataValue2, SRBMetaDataSet.D_BULK_INSERT_UDEF_META_DATA_FOR_DATA);
            }
          }
        }
      }
      else if (fieldName ==
        SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES)
      {
        if (delete) {
          srbFileSystem.srbModifyCollect( catalogType, getAbsolutePath(),
             "-1", dataValue2,  dataValue3,
            SRBMetaDataSet.C_DELETE_USER_DEFINED_COLL_STRING_META_DATA );
        }
        else {
          MetaDataTable table = record.getTableValue(i);
          dataValue2 = MODIFY_DEFINABLE_METADATA_SEPARATOR;
          for (int j=0;j<table.getRowCount();j++) {
            dataValue1 = "";
            int columns = table.getColumnCount();
            for (int k=0;k<columns;k++) {
              if (k == (columns - 1)) {
                dataValue1 += table.getStringValue(j,k);
              }
              else {
                dataValue1 += table.getStringValue(j,k) + dataValue2;
              }
            }
            srbFileSystem.srbModifyCollect( catalogType, getAbsolutePath(),
              dataValue1, dataValue2,  dataValue3,
              SRBMetaDataSet.C_BULK_INSERT_UDEF_META_DATA_FOR_COLL );
          }
        }
      }
/*TODO not really in API
      else if (fieldName == SRBMetaDataSet.same as for STRING)
      {
        if (delete) {
          retractionType =
            SRBMetaDataSet.D_DELETE_USER_DEFINED_INTEGER_META_DATA;
        }
        else
        {
          retractionType =
            SRBMetaDataSet.D_INSERT_USER_DEFINED_INTEGER_META_DATA;
          dataValue1 = record.getStringValue(i);
        }
      }
*/
      else if (fieldName == SRBMetaDataSet.CONTAINER_FOR_DIRECTORY)
      {
//oldContainerName, not_used  if the oldContainerName is used as a
//container forsome collection(s) then  the data_name  will replace it.
//(used internally by SRB server)
        retractionType = SRBMetaDataSet.D_UPDATE_CONTAINER_FOR_COLLECTION;
        dataValue1 = record.getStringValue(i);
      }
/*different class or method
      else if (fieldName == SRBMetaDataSet.)
      {
//newContainerCollName, newContainerDataName  copies access control
//(and some other metadata) from data_name to the new one.
//(used internally by SRB server)
        retractionType =
          SRBMetaDataSet.D_COPY_META_DATA_FROM_CONTAINER_TO_NEW_CONTAINER;
        dataValue1 = record.getStringValue(i);
      //@@@
      }
      else if (fieldName == SRBMetaDataSet.)
      {
//sourceCollName, sourceDataName  copies user-defined metadata from  source to data_name.
        retractionType = SRBMetaDataSet.D_COPY_META_DATA_FROM_DATA_TO_DATA;
        dataValue1 = record.getStringValue(i);
      //@@@
      }
      else if (fieldName == SRBMetaDataSet.)
      {
//sourceCollName, not_used  copies user-defined metadata from  source to data_name.
        retractionType = SRBMetaDataSet.D_COPY_META_DATA_FROM_COLL_TO_DATA;
        dataValue1 = record.getStringValue(i);
      }
*/

//-----------------------
//SRB3.2
//----------------------
      else if (fieldName == SRBMetaDataSet.GUID)
      {
        if (delete) {
          retractionType = SRBMetaDataSet.D_DELETE_GUID;
        }
        else {
          //have to delete the old GUID
          if (vaultPathName == null) {
            //has replica number
            srbFileSystem.srbModifyDataset( catalogType, fileName,
              getParent(), null, null, dataValue1, dataValue2,
              SRBMetaDataSet.D_DELETE_GUID );
          }
          else {
            srbFileSystem.srbModifyDataset( catalogType, getName(),
              getParent(), resource, vaultPathName, dataValue1,
              dataValue2, SRBMetaDataSet.D_DELETE_GUID  );
          }
          retractionType = SRBMetaDataSet.D_INSERT_GUID;
          dataValue1 = record.getStringValue(i);
          dataValue2 = "1";
           /* guid_flag == 0 means it is made by SRB and hence can be
            overwritten */
          /* guid_flag >= 1 means it is made by external generatort
            and hence cannot be overwritten only if data_value_2 is
            greater than the i*/

        }
      }

//-----------------------
//SRB3.3.1
//----------------------
      else if (fieldName == SRBMetaDataSet.DIRECTORY_LINK_NUMBER)
      {
        if (delete) {
          retractionType = SRBMetaDataSet.C_CHANGE_ACL_INHERITANCE_BIT;
          dataValue1 = "0";
        }
        else {
          if (record.getStringValue(i).equals("0") || 
              record.getStringValue(i).equals("false")) {
            retractionType = SRBMetaDataSet.C_CHANGE_ACL_INHERITANCE_BIT;
            dataValue1 = "0";
          }
          else if (record.getStringValue(i).toLowerCase().equals("recursive") ||
              record.getStringValue(i).toLowerCase().equals("r")) {
            retractionType = SRBMetaDataSet.C_CHANGE_ACL_INHERITANCE_BIT_RECUR;
            dataValue1 = "1";
          }
          else {
            retractionType = SRBMetaDataSet.C_CHANGE_ACL_INHERITANCE_BIT;
            dataValue1 = "1";
          }
        }
      }
      else if (record.getField(i).isExtensible(protocol)) {
        if (extTableName == null) {
          extTableName = record.getField(i).getExtensibleName(protocol);
        }
        //else if
        //TODO they are using fields with different extensible?

        if (extAttributes == null) {
          extAttributes = "DATA_ID";
        }
        extAttributes += "|" + record.getFieldName(i).toUpperCase();
        if (extValues == null) {
          extValues = "0";
        }
        extValues += "|'" + record.getStringValue(i)+"'";
      }
      


      if (retractionType >= 0) {
        if (isFile) {
          if (!dirsOnly) {
            if (vaultPathName == null) {
              //has replica number
              srbFileSystem.srbModifyDataset( catalogType, fileName,
                getParent(), null,  null,  dataValue1, dataValue2,
                retractionType );
            }
            else {
              srbFileSystem.srbModifyDataset( catalogType, getName(),
                getParent(), resource, vaultPathName, dataValue1,
                dataValue2, retractionType );
            }
          }
        }
        else if (!filesOnly) {
          srbFileSystem.srbModifyCollect( catalogType, getAbsolutePath(),
            dataValue1, dataValue2,  dataValue3, retractionType );
        }
        retractionType = -1;
      }
    }

    //for extensible modifications,
    //run after all the fields have been accummulated
    if (extTableName != null) {
      if (isFile(false))
      {
        if (delete) {
          srbFileSystem.srbModifyExtMetaData( catalogType,
            fileName, getParent(), extTableName,
            extAttributes, extValues,
            "", "", SRBMetaDataSet.D_DELETE_FROM_EXTMD_TABLE);
        }
        else {
          srbFileSystem.srbModifyExtMetaData( catalogType,
            fileName, getParent(), extTableName,
            extAttributes, extValues, "", "",
            SRBMetaDataSet.D_INSERT_INTO_EXTMD_TABLE);
        }
      }
      else if (isDirectory(false))
      {
        if (delete) {
          srbFileSystem.srbModifyExtMetaData( catalogType,
            null, getAbsolutePath(), extTableName,
            extAttributes, extValues,
            "", "", SRBMetaDataSet.D_DELETE_FROM_EXTMD_TABLE);
        }
        else {
          srbFileSystem.srbModifyExtMetaData( catalogType,
            null, getAbsolutePath(), extTableName,
            extAttributes, extValues,
            "", "", SRBMetaDataSet.D_INSERT_INTO_EXTMD_TABLE);
       }
      }
    }
  }




  /**
   * Copies this file to another file. This object is the source file.
   * The destination file is given as the argument.
   * If the destination file, does not exist a new one will be created.
   * Otherwise the source file will be appended to the destination file.
   * Directories will be copied recursively.
   *<P>
   * note: Files will be transferred using the SRB parallel transfer protocol.
   *  However, appending a file cannot use the parallel copy method.
   *  Also, the parallel method may be blocked by some firewalls,
   *  see also SRBFileSystem.setFirewallPorts( int, int )
   *
   * @param file The file to receive the data.
   * @param forceOverwrite If the file exists, force it to be overwritten.
   *     If the file cannot be overwritten throw IOException.
   * @throws  NullPointerException If file is null.
   * @throws IOException If an IOException occurs.
   */
  public void copyTo( GeneralFile file, boolean forceOverwrite )
    throws IOException
  {
    copyTo( file, forceOverwrite, USE_BULKCOPY );
  }


  /**
   * Copies this file to another file. This object is the source file.
   * The destination file is given as the argument.
   * If the destination file, does not exist a new one will be created.
   * Otherwise the source file will be appended to the destination file.
   * Directories will be copied recursively.
   *<P>
   * note: Files will be transferred using the SRB parallel transfer protocol.
   *  However, appending a file cannot use the parallel copy method.
   *  Also, the parallel method may be blocked by some firewalls,
   *  see also SRBFileSystem.setFirewallPorts( int, int )
   *
   * @param file The file to receive the data.
   * @param forceOverwrite If the file exists, force it to be overwritten.
   *     If the file cannot be overwritten throw IOException.
   * @param bulkCopy If true, bulk copy: Default option, new ports may be
   *     opened. Files copied in parallel or bulk as appropriate. <br>
   *     If false, parallel copy: new ports may be opened, files copied in
   *     parallel but not in bulk.
   * @throws  NullPointerException If file is null.
   * @throws IOException If an IOException occurs.
   */
  public void copyTo( GeneralFile file, boolean forceOverwrite,
    boolean bulkCopy )
    throws IOException
  {
    if (file == null) {
      throw new NullPointerException();
    }

    //make sure all the parent directories exist
//java.io.File.mkdirs doesn't work with relative path
    FileFactory.newFile( file.getFileSystem(),
      file.getAbsolutePath() ).getParentFile().mkdirs();

    //Versions before SRB2.0 can't run parallel
    if (srbFileSystem.getVersionNumber() < 2 ) {
      super.copyTo( file, forceOverwrite );
      return;
    }

    if (isDirectory()) {
      if (( file instanceof LocalFile ) &&
        ((srbFileSystem.getVersionNumber() >= 3 )
          && bulkCopy
          && (USE_BULKCOPY || bulkCopy)))
      {
        //TODO a query for big files to be done in parallel,
        //server-side changes?
        if(bulkCopy) {
          if (forceOverwrite)
            if (!file.delete())
            {
              if (file.exists())
              {
                throw new IOException(file+" cannot be removed");
              }
            }

          bulkUnload( (LocalFile) file );
          return;
        }
        //if the directory to be copied contains files over MAX_BULK_FILE_SIZE
        //if is better to use parallel copy. The files can't be copied
        //some in bulk, some in parallel, like in copyFrom(). So instead
        //just query and if big files are found, copy them all in parallel.
        MetaDataCondition conditions[] = { MetaDataSet.newCondition(
          GeneralMetaData.DIRECTORY_NAME, MetaDataCondition.LIKE,
          getAbsolutePath()+"*" ) };
        MetaDataSelect selects[] = {
          MetaDataSet.newSelection( GeneralMetaData.SIZE ) };
        MetaDataRecordList rl[] = query( conditions, selects );
        if (rl != null) {
          do {
            for (int i=0;i<rl.length;i++) {
              if (rl[i].getIntValue(0) > MAX_BULK_FILE_SIZE) {
                bulkCopy = false;
                break;
              }
            }
            rl = rl[0].getMoreResults();
          } while (rl != null && (bulkCopy && !rl[0].isQueryComplete()));
        }

        if (bulkCopy) {
          bulkUnload( (LocalFile) file );
          return;
        }
      }

      //recursive copy
      GeneralFile fileList[] = listFiles();

      file.mkdir();
      if (fileList != null) {
        for (int i=0;i<fileList.length;i++) {
          fileList[i].copyTo(
            FileFactory.newFile( file.getFileSystem(),
              file.getAbsolutePath(),  fileList[i].getName()),
              forceOverwrite );
        }
      }
    }
    else if (isFile(false)) {
      if (file.isDirectory()) {
        //change the destination from a directory to a file
        file = FileFactory.newFile( file, getName() );
      }

      if (!forceOverwrite && file.exists()) {
        //the source file will be appended to the destination file in serial
        super.copyTo( file, forceOverwrite );
        return;
      }

      if ( file instanceof LocalFile ) {
        int numThreads = (int) length()/MIN_THREAD_SIZE;
        if (numThreads > MAX_NUMBER_OF_PARALLEL_THREADS)
          numThreads = MAX_NUMBER_OF_PARALLEL_THREADS;

        //Note: Removed support for before SRB version 3.0
        srbFileSystem.srbObjGetClientInitiated(
          fileName, getParent(), file, 0, numThreads, forceOverwrite );
      }
      else if ( file instanceof SRBFile ) {
        //have to just delete it or it will error
        //already know file doesn't exist in case of !overwrite,
        //so delete() doesn't matter
        if (forceOverwrite) {
          if (!file.delete())
          {
            if (file.exists())
            {
              throw new IOException(file+" cannot be removed");
            }
          }
        }
        srbFileSystem.srbObjCopy( fileName, getParent(), ((SRBFile) file).fileName,
          file.getParent(), ((SRBFile) file).getResource() );
      }
      else {
        super.copyTo( file, forceOverwrite );
      }
    }
    else {
      throw new FileNotFoundException( "Source file is not valid: "+ this );
    }
  }


  /**
   * Copies this file to another file. This object is the source file.
   * The destination file is given as the argument.
   * If the destination file, does not exist a new one will be created.
   * Otherwise the source file will be appended to the destination file.
   * Directories will be copied recursively.
   *<P>
   * note: Files will be transferred using the SRB parallel transfer protocol.
   *  However, appending a file cannot use the parallel copy method.
   *  Also, the parallel method may be blocked by some firewalls,
   *  see also SRBFileSystem.setFirewallPorts( int, int )
   *
   * @param file  The file to receive the data.
   * @throws  NullPointerException If file is null.
   * @throws IOException If an IOException occurs.
   */
  public void copyFrom( GeneralFile file, boolean forceOverwrite )
    throws IOException
  {
    copyFrom( file, forceOverwrite, USE_BULKCOPY );
  }

  /**
   * Copies this file to another file. This object is the source file.
   * The destination file is given as the argument.
   * If the destination file, does not exist a new one will be created.
   * Otherwise the source file will be appended to the destination file.
   * Directories will be copied recursively.
   *<P>
   * note: Files will be transferred using the SRB parallel transfer protocol.
   *  However, appending a file cannot use the parallel copy method.
   *  Also, the parallel method may be blocked by some firewalls,
   *  see also SRBFileSystem.setFirewallPorts( int, int )
   *
   * @param file  The file to receive the data.
   * @param bulkCopy If true, bulk copy: Default option, new ports may be
   *     opened. Files copied in parallel or bulk as appropriate. <br>
   *     If false, parallel copy: new ports may be opened, files copied in
   *     parallel but not in bulk.
   * @throws  NullPointerException If file is null.
   * @throws IOException If an IOException occurs.
   */
  public void copyFrom( GeneralFile file, boolean forceOverwrite,
    boolean bulkCopy )
    throws IOException
  {
    if (file == null) {
      throw new NullPointerException();
    }

    //make sure all the parent directories exist
    getParentFile().mkdirs();

    //Versions before SRB2.0 can't run parallel
    if (srbFileSystem.getVersionNumber() < 2 ) {
        super.copyFrom( file, forceOverwrite );
        return;
    }


    if (file.isDirectory()) {
      if (( file instanceof LocalFile ) &&
        (srbFileSystem.getVersionNumber() >= 3 )
          && bulkCopy
          && (USE_BULKCOPY || bulkCopy))
      {
        //This somewhat confusing situation matches the behavior of the
        //commandline 'cp' command
        LocalFile[] files = null;
        if (isDirectory()) {
          files = new LocalFile[1];
          files[0] = (LocalFile) file;
        }
        else {
          files = (LocalFile[]) file.listFiles();
        }
        bulkLoad( files, forceOverwrite );
      }
      else {
        //recursive copy
        GeneralFile fileList[] = file.listFiles();

        mkdir();
        if (fileList != null) {
          for (int i=0;i<fileList.length;i++) {
            FileFactory.newFile( this, fileList[i].getName() ).copyFrom(
              fileList[i], forceOverwrite );
          }
        }
      }
    }
    else if (file.isFile()) {
      if (isDirectory()) {
        //change the destination from a directory to a file
        GeneralFile subFile = FileFactory.newFile( this, file.getName() );
        subFile.copyFrom( file, forceOverwrite );
        return;
      }

      if (!forceOverwrite && exists()) {
        super.copyFrom( file, forceOverwrite );
        return;
      }

      if ( file instanceof LocalFile ) {
        if (forceOverwrite) {
          if (!delete())
          {
            if (exists())
            {
              throw new IOException(file+" cannot be removed");
            }
          }
        }

        long length = file.length();
        int numThreads = 1;
        int ONE_THREAD_SIZE = 50000000;
        long value;
        if ( length / ONE_THREAD_SIZE > MAX_NUMBER_OF_PARALLEL_THREADS-1) {
          numThreads = MAX_NUMBER_OF_PARALLEL_THREADS;
        }
        else {
          numThreads = (int) length / ONE_THREAD_SIZE + 1;
        } 
        if (forceOverwrite) {
          try {
            value = srbFileSystem.srbObjPutClientInitiated( getName(),
              getParent(), resource, "", serverLocalPath , file.getAbsolutePath(),
              length, 1, numThreads );
          } catch(ConnectException e) {        
            if (SRBCommands.DEBUG > -1) e.printStackTrace();

            //Sometimes the connection drops after many repeated copies. 
            srbFileSystem = new SRBFileSystem((SRBAccount)fileSystem.getAccount());
            fileSystem = srbFileSystem;
            value = srbFileSystem.srbObjPutClientInitiated( getName(),
              getParent(), resource, "", serverLocalPath , file.getAbsolutePath(),
              length, 1, numThreads );
          }
        }
        else {
          try {
            value = srbFileSystem.srbObjPutClientInitiated( getName(),
              getParent(), resource, "", serverLocalPath , file.getAbsolutePath(),
              length, 0, numThreads );
          } catch(ConnectException e) {
            if (SRBCommands.DEBUG > -1) e.printStackTrace(); 
            
            //Sometimes the connection drops after many repeated copies.
            fileSystem = new SRBFileSystem((SRBAccount)fileSystem.getAccount());
            value = srbFileSystem.srbObjPutClientInitiated( getName(),
              getParent(), resource, "", serverLocalPath , file.getAbsolutePath(),
              length, 0, numThreads );
          }
        }

        if (value == SRBCommands.MSG_USE_SINGLE_PORT) {
          //server doesn't allow opening new ports
          super.copyFrom( file, forceOverwrite );
        }
        else if (value < 0) {
          throw new SRBException( "", (int)  value );
        }
      }
      else if ( file instanceof SRBFile ) {
        //have to just delete it or it will error
        //already know file doesn't exist in case of !overwrite,
        //so delete() doesn't matter
        if (forceOverwrite) {
          if (!delete())
          {
            if (exists())
            {
              throw new IOException(file+" cannot be removed");
            }
          }
        }
        srbFileSystem.srbObjCopy( ((SRBFile) file).fileName, file.getParent(),
          fileName, getParent(), getResource() );
      }
      else {
        super.copyFrom( file, forceOverwrite );
      }
    }
    else {
      throw new FileNotFoundException( "Source File is not valid: "+ file );
    }
  }


//----------------------------------------------------------------------
// RemoteFile Methods
//----------------------------------------------------------------------





//----------------------------------------------------------------------
// SRBFile Methods
//----------------------------------------------------------------------
  /**
   * The number of bytes transfered so far by a currently executing
   * SRBFile.copyTo/copyFrom command.
   *
   * @return the number of bytes that have been transfered so far.
   */
  public long fileCopyStatus( )
  {
    return srbFileSystem.fileCopyStatus();
  }

  /**
   * Retrieves the platform independent stat structure.
   * This is basically the POSIX stat definition.
   *<P>
   * Returns:<br>
   * st_size = 0. File size in bytes (long).<br>
   * st_dev = 1. Device.<br>
   * st_ino = 2. Inode.<br>
   * st_mode = 3. File mode.<br>
   * st_nlink = 4. Number of links.<br>
   * st_uid = 5. User ID of the file's owner.<br>
   * st_gid = 6. Group ID of the file's group.<br>
   * st_rdev = 7. ID of device.<br>
   *         This entry is defined only for character or block special files.<br>
   * st_atim = 8. Time of last access.<br>
   *       Use st_atim instead of st_atime because in solaris, st_atime is<br>
   *       defined to be somthing else.<br>
   * st_spare1 = 9.<br>
   * st_mtime = 10. Time of last data modification.<br>
   * st_spare2 = 11.<br>
   * st_ctim = 12. Time of last file status change.<br>
   * st_spare3 = 13.<br>
   *      Time measured in seconds since 00:00:00 GMT, Jan. 1, 1970.<br>
   * st_blksize = 14. Optimal blocksize for file system i/o ops.<br>
   * st_blocks = 15. Actual number of blocks allocated in DEV_BSIZE blocks.<br>
   * st_vfstype = 16. Type of fs (see vnode.h).<br>
   * st_vfs = 17. Vfs number.<br>
   * st_type = 18. Vnode type.<br>
   * st_gen = 19. Inode generation number.<br>
   * st_flag = 20. Flag word.<br>
   * Reserved1 = 21. Reserved.<br>
   * Reserved2 = 22. Reserved.<br>
   * st_access = 23. Process' access to file.<br>
   * st_dummy = 24. pat to 32 bit boundary.<br>
   * st_spare4 = 25+. Reserved.
   *
   * @throws IOException If an IOException occurs.
   */
  // see also,  the SRB C client back/srbStat.h
  public long[] getStat() throws IOException
  {
    long stat[] = null;

    if (!exists()) return null;

    if (isDirectory()) {
      //@param myType  file or dir state : IS_UNKNOWN, IS_FILE, IS_DIR_1,
      //      IS_DIR_2, IS_DIR_3, IS_DIR_4.
      stat = srbFileSystem.srbObjStat(
        0, getAbsolutePath(), 3 );//IS_DIR_3?
    }
    else {
      stat = srbFileSystem.srbObjStat(
        0, getAbsolutePath(), 0 );
    }

    if ( stat != null ) {
/*can maybe use this stuff
      long st_size = stat[0]; // File size in bytes (long)

      long st_dev = stat[1]; // Device
      long st_ino = stat[2]; // Inode


      long st_mode = stat[3]; // File mode

      long st_nlink = stat[4]; // Number of links
      long st_uid = stat[5];  // User ID of the file's owner
      long st_gid = stat[6];  // Group ID of the file's group
      long st_rdev = stat[7]; // ID of device
         //   This entry is defined only for
         //   character or block special files
      long st_atim = stat[8]; // Time of last access
         // Use st_atim instead of st_atime
         // because in solaris, st_atime is
         // defined to be somthing else
      long st_spare1 = stat[9];


      long st_mtime = stat[10]; // Time of last data modification

      long st_spare2 = stat[11];
      long st_ctim = stat[12]; // Time of last file status change
      long st_spare3 = stat[13];
        // Time measured in seconds since
        //   00:00:00 GMT, Jan. 1, 1970

      long st_blksize = stat[14]; // Optimal blocksize for file system
           //i/o ops
      long st_blocks = stat[15]; // Actual number of blocks allocated
           //in DEV_BSIZE blocks
      long st_vfstype = stat[16]; // Type of fs (see vnode.h)
      long st_vfs = stat[17];  // Vfs number
      long st_type = stat[18]; // Vnode type
      long st_gen = stat[19];  // Inode generation number
      long st_flag = stat[20]; // Flag word

      long Reserved1 = stat[21]; // Reserved
      long Reserved2 = stat[22]; // Reserved
      long st_access = stat[23];      // Process' access to file
      long st_dummy = stat[24];       // pat to 32 bit boundary
      long st_spare4[] = new long[25];
      for (int i=0;i<st_spare4.length;i++)
        st_spare4[i] = stat[i+25];   // Reserved
*/ return stat;
    }
    else
      throw new IOException( "Unable to obtain file stat" );
  }


//0=empty, -1=loading, 1=ready to load
private volatile int loadBufferCount = 0;

//Thread currently being registered
int activeRegistrationThread = 0;

//filesReadyToRegister and which still need to be.
int filesReadyToRegister = 0;

  /**
   * Loads the local files on to the SRB in this container in the
   * <code>srbDirectory</code>.
   *
   */
  void bulkLoad( LocalFile[] files, boolean forceOverwrite )
    throws IOException
  {
/*
which container or resource, get size and MaxSize of container
open container or temp file for upload
create two 8MB byte buffers
create new threads
main thread starts loading local files into buffer
  -one buffer gets filled well the other buffer is used in transfer
also loads myResultStruct (upto 300 files)
  -somehow->dataName,collectionName,offset
  -once full create registration thread
make sure not to overflow container

only one transfer thread
waits on buffer to have data
  -main gives signal
check enough space, otherwise create new container
uses regular srbobjwrite
udates status and wait again

upto 4 registration threads
*/

    long maxSize = -1;
    long containerOffset = 0;

    //buffer for loading to transfering files
    byte[] loadBuffer = new byte[BULK_LOAD_BUFFER_SIZE];

    //the temporary files to receive the data during transfer
    GeneralFile[] tempFile = new GeneralFile[MAX_NUMBER_OF_BULK_THREADS];
    GeneralRandomAccessFile[] raf =
      new GeneralRandomAccessFile[MAX_NUMBER_OF_BULK_THREADS];

    LoadThread load = null;

    Vector parallelSourceFiles = new Vector();
    Vector parallelDestination = new Vector();//have to be SRBFile

    //if this object isn't a dir, make it one.
    mkdir();


    //create a temp file for each registration thread
    for (int i=0;i<MAX_NUMBER_OF_BULK_THREADS;i++) {
      //just a placeholder for some crud, does not get created.
      tempFile[i] = FileFactory.newFile( this, "placeholder" );
    }
    tempFile[0] = SRBFile.createTempFile(
      "BLoad", ".tmp", this );

    raf[0] = FileFactory.newRandomAccessFile( tempFile[0], "rw" );

    //threads to tell MCAT about the metadata of the files.
    RegistrationThread[] registration =
      new RegistrationThread[MAX_NUMBER_OF_BULK_THREADS];
    Thread[] registrationThreads = new Thread[MAX_NUMBER_OF_BULK_THREADS];

    for (int i=0;i<MAX_NUMBER_OF_BULK_THREADS;i++) {
      registration[i] = new RegistrationThread(
        tempFile[i].getAbsolutePath(), this );
    }
    registrationThreads[0] = new Thread( registration[0] );


    //thread to transfer all the data
//TODO use parallel put?
    load = new LoadThread( raf, loadBuffer, this );

    // Now start copying
    for ( int i=0;i<files.length;i++ ) {
      loadBuffer( files[i], load, tempFile, registrationThreads,
        registration, null, parallelSourceFiles, parallelDestination );
    }

    //singal the load thread to stop
    load.keepLoading = false;
    load.sendBuffer(0);

    try {
      //close, register, then delete all remaining temp files
      for ( int i=0;i<MAX_NUMBER_OF_BULK_THREADS;i++ ) {
        if (raf[i] != null) {
          raf[i].close();
        }
        if ( registration[i] != null ) {
          //TODO
          //The last bulkFile wasn't getting its filepath set.
          //Causing a -3201 DATA_NOT_IN_CAT srb error, when the old bulkFile
          //(that had already been registered) attempted to get re-registered
          if (new SRBFile(srbFileSystem, registration[i].bloadFilePath).exists()) {
                    registration[i].register();
          }
          else {
            registration[i].bloadFilePath = tempFile[i].getAbsolutePath();
            registration[i].register();
          }
        }

        if (tempFile[i] != null) {
          ((SRBFile)tempFile[i]).delete(true);
        }
      }

      //end of registration threads
      for ( int i=0;i<MAX_NUMBER_OF_BULK_THREADS;i++ ) {
        if ( registrationThreads[i] != null ) {
          if ( registrationThreads[i].isAlive() ) {
            registrationThreads[i].join();
          }
        }
      }
    }
    catch(InterruptedException e) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    //large files copied in parallel after the bulk load
    if (parallelSourceFiles.size() > 0) {
      for (int i=0;i<parallelSourceFiles.size();i++) {
        ((SRBFile) parallelDestination.get(i)).copyFrom(
          ((GeneralFile) parallelSourceFiles.get(i)), forceOverwrite, false );
      }
    }
  }


  //loads a file (or a directory recursively)
  //into the buffer for transfer to the SRB.
  private void loadBuffer( GeneralFile file, LoadThread load,
    GeneralFile[] tempFile, Thread[] registrationThreads,
    RegistrationThread[] registration, String relativePath,
    Vector parallelSourceFiles, Vector parallelDestination )
    throws IOException
  {
    long length = file.length();
    boolean exist = file.exists();
    boolean isDirectory = file.isDirectory();
    String name = file.getName();

    if (( file == null ) || (!exist)) {
      return;
    }

    //recursively loads the directory,
    //unfortunately this means a lot of logic has to be in this method
    //instead of bulkLoad(). Which explains all the method variables.
    if ( isDirectory ) {
      //this relativePath variable keeps track of the matching of the
      //local directory and the SRB directory. It is keeps the subdirectory
      //structure but keeps the files from being registered further up the
      //tree than this SRBFile object.
      //Solves the problem of if they use a relative path like
      //../../../../../../src and a SRB bulkLoad dir of /home/testuser
      if ( (relativePath == null) || (relativePath.equals("")) ) {
        relativePath = name;
        //empty directories weren't getting bulkloaded
        new SRBFile( srbFileSystem,
          getAbsolutePath(), relativePath ).mkdir();
      }
      else {
        relativePath += separator + name;
        //empty directories weren't getting bulkloaded
        new SRBFile( srbFileSystem,
          getAbsolutePath(), relativePath ).mkdir();
      }

/*
I guess, if a LocalFile is a file:
  GeneralFile[] files = file.listFiles();

  sometimes throws this exception

  java.lang.InternalError: Unable to bind "someLocalFile" to parent
   at sun.awt.shell.Win32ShellFolder2.getIShellFolder(Unknown Source)
   at sun.awt.shell.Win32ShellFolder2.listFiles(Unknown Source)
   at sun.awt.shell.ShellFolder.listFiles(Unknown Source)
   at edu.sdsc.grid.io.local.LocalFile.listFiles(Unknown Source)
   at edu.sdsc.grid.io.srb.SRBFile.loadBuffer(Unknown Source)
   at edu.sdsc.grid.io.srb.SRBFile.bulkLoad(Unknown Source)
   at edu.sdsc.grid.io.srb.SRBFile.copyFrom(Unknown Source)
   at edu.sdsc.grid.io.srb.SRBFile.copyFrom(Unknown Source)
   at edu.sdsc.grid.io.srb.SRBFile.copyFrom(Unknown Source)
*/
      GeneralFile[] files = file.listFiles();

      if (files == null)
        return;

      for (int i=0;i<files.length;i++) {
        loadBuffer( files[i], load, tempFile,
          registrationThreads, registration, relativePath,
          parallelSourceFiles, parallelDestination );
      }
    }
    else if (length > MAX_BULK_FILE_SIZE) {
      //copy files larger then 2MB in parallel. after the bulkload finishes

      //keeps the subdirectory structure
      if ( (relativePath == null) || (relativePath.equals("")) ) {
        relativePath = name;
      }
      else {
        relativePath += separator + name;
      }

      //copy into proper directory
      parallelSourceFiles.add( file );
      parallelDestination.add( FileFactory.newFile( this, relativePath ) );
    }
    else {
      //temp buffer to read in source file
      int zxcv=0;

      //note: wait if all four threads are full&in use handled at the bottom
      //this registration is up here because the last file in a block
      //wasn't getting registered
      filesReadyToRegister++;

      //(when not using containers) files can't be registered until
      //after they have been uploaded. this adds the file to the current
      //registration thread list.
      if ( relativePath == null ) {
        registration[activeRegistrationThread].addFile( file, "" );
      }
      else {
        registration[activeRegistrationThread].addFile( file, relativePath );
      }


      long toRead = length;
      int temp = 0;
      GeneralFileInputStream readFile = null;
      try {
        readFile = FileFactory.newFileInputStream( file );
      } catch ( SecurityException e ) {
        //file I/O problem, maybe it will recover...
        if (SRBCommands.DEBUG > 0) e.printStackTrace();
        return;
      } catch ( IOException e ) {
        //TODO better to catch the specific SRBException that means
        //"trying to open a file that does not exist"

        //file I/O problem, maybe it will recover...
        if (SRBCommands.DEBUG > 0) e.printStackTrace();
        return;
      }

      //this loop: copies a file, waits if load buffers are full
      while (toRead > 0) {
        if ( (toRead+load.loadBufferLength) <= BULK_LOAD_BUFFER_SIZE ) {
          synchronized ( this ) {
            loadBufferCount = -1;
          }
          //read file into loadBuffer
          temp = readFile.read( load.loadBuffer, load.loadBufferLength,
            (int) toRead );

          if (temp > 0)  {
            toRead -= temp;
            zxcv+=temp;
            load.loadBufferLength += temp;
          }
        }
        //loadBuffer is full
        if ((load.loadBufferLength+toRead) >= BULK_LOAD_BUFFER_SIZE) {
          load.sendBuffer(0);
        }
      }
      readFile.close();


      //every MAX_REGISTRATION_FILES
      if ( (filesReadyToRegister % MAX_REGISTRATION_FILES) == 0 )
      {
        //if buffer has data make sure it gets loaded first.
        load.sendBuffer(0);
        load.out[load.randomIndex].close();

        //start registration
        registrationThreads[activeRegistrationThread].start();


        //increment to next registration thread
        activeRegistrationThread++;
        if (activeRegistrationThread >= MAX_NUMBER_OF_BULK_THREADS) {
          activeRegistrationThread = 0;
        }

        //start a new registration thread or reuse old one
        if (registrationThreads[activeRegistrationThread] == null) {
          registrationThreads[activeRegistrationThread] =
            new Thread(registration[activeRegistrationThread]);
        }
        else {
          //do not reuse old threads that are not finished
          try {
            if ( registrationThreads[activeRegistrationThread].isAlive() ) {
              //with 4+ threads this probably won't ever happen?
              //but having a wait here can cause error
              registrationThreads[activeRegistrationThread].join();
            }
          } catch(InterruptedException e) {
            if (SRBCommands.DEBUG > 0) e.printStackTrace();
          }

          registrationThreads[activeRegistrationThread] =
            new Thread(registration[activeRegistrationThread]);
        }

        //new tempFile for registration and load threads
        tempFile[activeRegistrationThread] = SRBFile.createTempFile(
          "BLoad", ".tmp", tempFile[activeRegistrationThread].getParentFile() );
        registration[activeRegistrationThread].setBloadFilePath(
          tempFile[activeRegistrationThread].getAbsolutePath() );

        load.restartRandom = true;
        load.newRandom = tempFile[activeRegistrationThread];
        load.randomIndex = activeRegistrationThread;
      }
    }
  }


  /**
   * Uses two buffers to upload data to the SRB, switching back and forth
   * transfering the full buffer while the main thread loads the other
   * buffer.
   */
  class LoadThread implements Runnable
  {
    GeneralRandomAccessFile[] out;
    byte[] loadBuffer;
    byte[] buffer2;
    boolean writeLoadBuffer = false;
    int loadBufferLength = 0;
    boolean keepLoading = true;

    boolean restartRandom = false;
    GeneralFile newRandom;
    int randomIndex;

    //keep thread monitor
    Object mainThread;

    LoadThread( GeneralRandomAccessFile[] out, byte[] loadBuffer,
      Object mainThread )
    {
      this.out = out;
      this.loadBuffer = loadBuffer;
      this.mainThread = mainThread;
    }

    public void run( )
    {
      int oldBuf = Integer.MIN_VALUE;
    }

    /**
     * switch to a new file so the old file can be registered.
     * only have MAX_NUMBER_OF_BULK_THREADS open at a time, when one
     * fills up with MAX_REGISTRATION_FILES, the temp file must be closed
     * to allow it to be registered. Need to open a new one to keep loading
     * data.
     */
    void restartRandomAccessFile( )//int fileIndex, GeneralFile tempFile )
      throws IOException
    {
      out[randomIndex] = FileFactory.newRandomAccessFile( newRandom, "rw" );
      restartRandom = false;
    }

    int sendBuffer( int oldBuf )
      throws IOException
    {
      if (loadBufferLength <= 0) {
        return oldBuf;
      }

      synchronized (this) {
        if ( restartRandom ) {
          restartRandomAccessFile();
        }
      }

      //if loadBuffer is full, transfer it.
      //Must transfer buffers in the order they were loaded.
      //a zero buffer means nothing loaded, safe to load the other one.
      out[activeRegistrationThread].write(loadBuffer, 0, loadBufferLength);
      loadBufferLength = 0;
      //only place loadBufferCount is set to 0
      loadBufferCount = 0;
      return oldBuf;
    }
  }



  /**
   * One of a number of threads that tell metadata to the SRB-MCAT.
   */
  class RegistrationThread implements Runnable
  {
    //directory to load into on the SRB
    String bloadFilePath;

    //keep thread monitor
    Object mainThread;

    //list of files to be registered
    Vector files = new Vector();
    //list of SRB relative paths of those files
    Vector paths = new Vector();

    RegistrationThread( String bloadFilePath,  Object mainThread )
      throws IOException
    {
      this.bloadFilePath = bloadFilePath;
      this.mainThread = mainThread;
    }

    void setBloadFilePath( String bloadFilePath )
    {
      this.bloadFilePath = bloadFilePath;
    }

    void addFile( GeneralFile file, String relativePath )
    {
      files.add(file);
      paths.add(relativePath);
    }

    public void run( )
    {
      try {
        register();
      } catch (IOException e) {
        if (SRBCommands.DEBUG > 0) e.printStackTrace();
      }
    }

    //get the files that are ready to be registered
    SRBMetaDataRecordList[] getFileRegistry(int numFiles)
      throws IOException
    {
      SRBMetaDataRecordList rl = null;
      SRBMetaDataRecordList[] recordLists =
        new SRBMetaDataRecordList[numFiles];

      GeneralFile tempFile = null, tempFile2 = null;
      String tempName = null, tempName2 = null;

      String dirName = null; //SRBParentFile???
      long size = 0;
      int offset = 0;

      //create the registry for the files loaded so far.
      for (int i=0;i<numFiles;i++) {
        tempFile = (GeneralFile) files.get(i);
        offset += size;//TODO ???
        size = tempFile.length();
        dirName = (String) paths.get(i);
        if ((dirName == null) || (dirName.equals(""))) {
          dirName = getAbsolutePath();
        }
        else {
          dirName = getAbsolutePath() + separator + dirName;
        }

        rl = new SRBMetaDataRecordList(
          SRBMetaDataSet.getField( GeneralMetaData.FILE_NAME ),
            tempFile.getName());
        rl.addRecord(
          SRBMetaDataSet.getField( GeneralMetaData.DIRECTORY_NAME ),  dirName);
        rl.addRecord(
          SRBMetaDataSet.getField( GeneralMetaData.SIZE ), size);
        rl.addRecord(
          SRBMetaDataSet.getField( SRBMetaDataSet.OFFSET ), offset);

        recordLists[i] = rl;
      }

      for (int i=0;i<numFiles;i++) {
try {//HACK ?
        files.remove(0);
        paths.remove(0);
} catch (ArrayIndexOutOfBoundsException e) {
  if (SRBCommands.DEBUG > 0) e.printStackTrace();
  break;
}
      }

      return recordLists;
    }

    void register( )
      throws IOException
    {
      int numFiles = files.size();
      if (numFiles > 0) {
//I think registration blocks?
        synchronized( mainThread )
        {
          //wake up the main thread if it is waiting for a buffer to empty
          mainThread.notify();
        }
        srbFileSystem.srbBulkLoad(
          catalogType, bloadFilePath, getFileRegistry(numFiles) );
      }
      synchronized( mainThread )
      {
        //wake up the main thread if it is waiting for a buffer to empty
        mainThread.notify();
      }
    }
  }


  /**
   * Loads this SRB directory into this local directory.
   */
  void bulkUnload( LocalFile dir )
    throws IOException
  {
    //If the flag is set to this value,
    //it will unload the inContainer files as well as the normal files.
    //#define BUL_CONT       0x8000000
    int flag = 134217728;

    if ( exists() ) {
      //This somewhat confusing situation matches the behavior of the
      //commandline 'cp' command
      if (dir.exists()) {
        dir = new LocalFile( dir, getName() );
      }
      else {
        dir.mkdir();
      }
      srbFileSystem.srbBulkUnload( catalogType, flag, getAbsolutePath(),
        dir.getAbsolutePath() );
    }

    //make sure empty directories get downloaded/created
    String fileDir, allDir;
    boolean copied = false;

    MetaDataCondition[] conditions = { MetaDataSet.newCondition(
      GeneralMetaData.DIRECTORY_NAME, MetaDataCondition.LIKE,
      getAbsolutePath()+"*"),
    };
    MetaDataSelect[] selects = {
      MetaDataSet.newSelection( GeneralMetaData.DIRECTORY_NAME ),
      MetaDataSet.newSelection( GeneralMetaData.FILE_NAME ),
    };

    MetaDataRecordList[] rl = fileSystem.query( conditions, selects );

    selects[1] = null;

    MetaDataRecordList[] rl2 = fileSystem.query( conditions, selects );

    if ((rl != null) && (rl2 != null)) {
      for (int i=0;i<rl2.length;i++) {
        allDir =
          rl2[i].getStringValue(0).substring(getAbsolutePath().length());
        for (int j=0;j<rl.length;j++) {
          fileDir =
            rl[j].getStringValue(1).substring(getAbsolutePath().length());
          if (fileDir.equals(allDir)) {
            copied = true;
          }
        }
        if (!copied) {
          new LocalFile( dir, allDir ).mkdirs();
        }
        copied = false;
      }
    }
  }
//----------------------------------------------------------------------------------------
//end bulkLoad methods/classes------------------------------------------------------------
//----------------------------------------------------------------------------------------

  /**
   * Get the permissions of the current user for this SRBFile:
   * write, read, all, annotate or null.
   * <P>
   * @throws IOException If an IOException occurs.
   */
  public String getPermissions( )
    throws IOException
  {
    //can't just call getPermissions(false) since we need to get
    //different values if it is directory or file.
    MetaDataRecordList[] rl = null;
    String userName = srbFileSystem.getUserName();
    String userDomain = srbFileSystem.getDomainName();

    if (isDirectory()) {
      MetaDataCondition conditions[] = {
//        getZoneCondition(),
        MetaDataSet.newCondition(
          SRBMetaDataSet.ACCESS_DIRECTORY_NAME, MetaDataCondition.EQUAL,
            getAbsolutePath() ),
        MetaDataSet.newCondition(
          UserMetaData.USER_NAME, MetaDataCondition.EQUAL, userName ),
        MetaDataSet.newCondition(
          SRBMetaDataSet.USER_DOMAIN, MetaDataCondition.EQUAL, userDomain ),
      };
      MetaDataSelect selects[] = {
        MetaDataSet.newSelection( SRBMetaDataSet.ACCESS_DIRECTORY_NAME ),
        MetaDataSet.newSelection( SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT ),
        MetaDataSet.newSelection( UserMetaData.USER_NAME ),
        MetaDataSet.newSelection( SRBMetaDataSet.USER_DOMAIN ),
      };
      rl = fileSystem.query( conditions, selects );

      if (rl != null) {
        for (int i=0;i<rl.length;i++) {
          if (rl[i].getValue( UserMetaData.USER_NAME ).equals( userName ) &&
            rl[i].getValue( SRBMetaDataSet.USER_DOMAIN ).equals( userDomain ))
          {
            return rl[i].getValue(
              SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT ).toString();
          }
        }
      }
    }
    else {
      MetaDataCondition conditions[] = {
        MetaDataSet.newCondition(
          UserMetaData.USER_NAME, MetaDataCondition.EQUAL, userName ),
        MetaDataSet.newCondition(
          SRBMetaDataSet.USER_DOMAIN, MetaDataCondition.EQUAL, userDomain ),
      };
      MetaDataSelect selects[] = {
        MetaDataSet.newSelection( SRBMetaDataSet.ACCESS_CONSTRAINT ),
        MetaDataSet.newSelection( UserMetaData.USER_NAME ),
        MetaDataSet.newSelection( SRBMetaDataSet.USER_DOMAIN ),
      };
      rl = query( conditions, selects );

      if (rl != null) {
        for (int i=0;i<rl.length;i++) {
          if (rl[i].getValue( UserMetaData.USER_NAME ).equals( userName ) &&
            rl[i].getValue( SRBMetaDataSet.USER_DOMAIN ).equals( userDomain ))
          {
            return rl[i].getValue(
              SRBMetaDataSet.ACCESS_CONSTRAINT ).toString();
          }
        }
      }
    }

    return null;
  }


  /**
   * Gets all the non-null permissions of all SRB users for this SRBFile:
   * write, read, all, annotate or null.
   *<P>
   * @throws IOException If an IOException occurs.
   */
  public MetaDataRecordList[] getPermissions( boolean allUsers )
    throws IOException
  {
    if (allUsers) {
      if (isDirectory()) {
        MetaDataCondition conditions[] = {
//          getZoneCondition(),
          MetaDataSet.newCondition(
            SRBMetaDataSet.ACCESS_DIRECTORY_NAME, MetaDataCondition.EQUAL,
            getAbsolutePath() ),
          };
        MetaDataSelect selects[] = {
          MetaDataSet.newSelection( SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT ),
          MetaDataSet.newSelection( UserMetaData.USER_NAME ),
          MetaDataSet.newSelection( SRBMetaDataSet.USER_DOMAIN )
        };
        return fileSystem.query( conditions, selects );
      }
      else {
        MetaDataSelect selects[] = {
          MetaDataSet.newSelection( SRBMetaDataSet.ACCESS_CONSTRAINT ),
          MetaDataSet.newSelection( UserMetaData.USER_NAME ),
          MetaDataSet.newSelection( SRBMetaDataSet.USER_DOMAIN ),
        };
        return query( selects );
      }
    }
    else {
      String userName = srbFileSystem.getUserName();
      String userDomain = srbFileSystem.getDomainName();

      if (isDirectory()) {
        MetaDataCondition conditions[] = {
//          getZoneCondition(),
          MetaDataSet.newCondition(
            SRBMetaDataSet.ACCESS_DIRECTORY_NAME, MetaDataCondition.EQUAL,
              getAbsolutePath() ),
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, MetaDataCondition.EQUAL, userName ),
          MetaDataSet.newCondition(
            SRBMetaDataSet.USER_DOMAIN, MetaDataCondition.EQUAL, userDomain ),
        };
        MetaDataSelect selects[] = {
          MetaDataSet.newSelection( SRBMetaDataSet.ACCESS_DIRECTORY_NAME ),
          MetaDataSet.newSelection( SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT ),
          MetaDataSet.newSelection( UserMetaData.USER_NAME ),
          MetaDataSet.newSelection( SRBMetaDataSet.USER_DOMAIN ),
        };
        return fileSystem.query( conditions, selects );
      }
      else {
        MetaDataCondition conditions[] = {
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, MetaDataCondition.EQUAL, userName ),
          MetaDataSet.newCondition(
            SRBMetaDataSet.USER_DOMAIN, MetaDataCondition.EQUAL, userDomain ),
        };
        MetaDataSelect selects[] = {
          MetaDataSet.newSelection( SRBMetaDataSet.ACCESS_CONSTRAINT ),
          MetaDataSet.newSelection( UserMetaData.USER_NAME ),
          MetaDataSet.newSelection( SRBMetaDataSet.USER_DOMAIN ),
        };
        return query( conditions, selects );
      }
    }
  }


  /**
   * Change the permissions for this SRBFile.
   * <P>
   * @param permission "w" - write;"r" - read;"rw" or "all" - read/write;
   *   "n" - null;"t" - annotate;"o" - owner;"c" - curate
   * @param newUserName The permissions are changed for this user,
   * @param userMdasDomain at this Mdas domain.
   * @throws IOException If an IOException occurs.
   */
  public void changePermissions(
    String permission, String newUserName, String userMdasDomain )
    throws IOException
  {
    changePermissions( permission, newUserName, userMdasDomain, false );
  }


  /**
   * Change the permissions for this SRBFile.
   * <P>
   * @param permission "w" - write;"r" - read;"rw" or "all" - read/write;
   *   "n" - null;"t" - annotate;"o" - owner;"c" - curate
   * @param newUserName The permissions are changed for this user,
   * @param userMdasDomain at this Mdas domain.
   * @param recursive Changes this and all subdirectories
   * @throws IOException If an IOException occurs.
   */
  public void changePermissions( String permission, String newUserName,
    String userMdasDomain, boolean recursive )
    throws IOException
  {
    int retractionType = -1;

    if (permission == null) {
      permission = "";
    }

    permission = permission.toLowerCase();

    if (permission.equals("n") || permission.equals("null")
      || permission.equals(""))
    {
      permission = "";
    }
    else if (permission.equals("r") || permission.equals("read")) {
      permission = "read";
    }
    else if (permission.equals("w") || permission.equals("write")) {
      permission = "write";
    }
    else if (permission.equals("rw")) {
      permission = "all";
    }
    else if (permission.equals("all") || permission.equals("ownership")) {
      permission = "all";
    }
    else if (permission.equals("t") || permission.equals("annotate")) {
      permission = "annotate";
    }
    else if (permission.equals("c") || permission.equals("curate")) {
      permission = "curate";
    }
    else {
      //permission = "";
      throw new IllegalArgumentException(
        "Permission type not valid: "+permission );
    }

    if (isDirectory()) {
      if ( permission == "" ) {
        if (recursive) {
          retractionType = SRBMetaDataSet.D_DELETE_INCOLL_ACCS_RECUR;
        }
        else {
          retractionType = SRBMetaDataSet.D_DELETE_COLL_ACCS;
        }
      }
      else {
        if (recursive) {
          retractionType = SRBMetaDataSet.D_INSERT_INCOLL_ACCS_RECUR;
        }
        else {
          retractionType = SRBMetaDataSet.D_INSERT_COLL_ACCS;
        }
      }

      srbFileSystem.srbModifyCollect( catalogType, getAbsolutePath(),
        newUserName, userMdasDomain, permission, retractionType );
    }
    else {
      if ( permission == "" ) {
        retractionType = SRBMetaDataSet.D_DELETE_ACCS;
      }
      else {
        retractionType = SRBMetaDataSet.D_INSERT_ACCS;
      }

      srbFileSystem.srbModifyDataset( catalogType, fileName,
        getParent(), "", "", newUserName+"@"+userMdasDomain, permission,
        retractionType );
    }
  }


  /**
   * Replicates this SRBFile to a new resource. Each replicant will increment
   * its replication number by 1 from the last replication.
   * Directories/collections will be recursively replicated.
   *<P>
   * In SRB, one can make copies of a data set and store the copies in
   * different locations. But, all these copies in SRB are considered to be
   * identifiable by the same identifier. That is, each copy is considered to
   * be equivalent to each other.
   *<P>
   * When a user reads a replicated data set, SRB cycles through all the
   * copies of the datset and reads the one that is accessible at that time.
   * It uses a simple replica identification mechanism to order this list of
   * replicated datasets.
   *
   * @param  newResource The storage resource name of the new copy.
   * @throws IOException If an IOException occurs.
   */
  public void replicate( String newResource )
    throws IOException
  {
    if (isDirectory()) {
      SRBFile list[] = (SRBFile[])listFiles();

      for (int i=0;i<list.length;i++) {
        if (list[i].isFile(false)) {
          srbFileSystem.srbObjReplicate( catalogType, fileName,
            getParent(), newResource, null );
        }
        else {
          list[i].replicate( newResource, false );
        }
      }
    }
    else {
      srbFileSystem.srbObjReplicate( catalogType, fileName, getParent(),
        newResource, null );
    }
  }

  /**
   * Replicates this SRBFile to a new resource. Each replicant will increment
   * its replication number by 1 from the last replication.
   * Directories/collections will be recursively replicated.
   *<P>
   * Used internally when recursively replicating a directory.
   *
   * @param  newResource The storage resource name of the new copy.
   * @throws IOException If an IOException occurs.
   */
  private void replicate( String newResource, boolean update )
    throws IOException
  {
//Why am I doing recursive this way?  Isn't there a single server call?
//answer: no.

    //false, won't update when recursively replicating
    if (isDirectory(update)) {
      SRBFile list[] = (SRBFile[])listFiles();

      for (int i=0;i<list.length;i++) {
        if (list[i].isFile(false)) {
          srbFileSystem.srbObjReplicate( catalogType, fileName,
            getParent(), newResource, null );
        }
        else {
          list[i].replicate( newResource, false );
        }
      }
    }
    else {
      srbFileSystem.srbObjReplicate( catalogType, fileName, getParent(),
        newResource, null );
    }
  }


  /**
   * Backup a data object. Make a replica to the backup
   * resource. Skip it if a good copy already exist.
   *
   * @param backupResource - The backup resource
   */
  public void backup( String backupResource )
    throws IOException
  {
    if (isDirectory()) {
      String list[] = list();

      for (int i=0;i<list.length;i++) {
        if (list[i].startsWith("/")) {
          new SRBFile( srbFileSystem, list[i] ).backup(
            backupResource );
        }
        else {
          new SRBFile( srbFileSystem, getAbsolutePath() + "/" +
            list[i] ).backup( backupResource );
        }
      }
    }
    else {
      srbFileSystem.srbBackupData( catalogType, fileName, getParent(),
        backupResource, 0);//flag? );
    }
  }


  /**
   * Checksum a SRB data file. If the chksum already already exists,
   * do nothing and return the chksum value. If the chksum does not exist,
   * compute and register it.
   *
   * @return the checksum value. Returns null if this SRBFile object is a
   *     directory.
   */
  public String checksum( )
    throws IOException
  {
    if (isFile()) {
      byte[] checksum = srbFileSystem.srbObjChksum( getName(), getParent(),
        LIST_CHECKSUM, null );

      //No checksum registered, probably
      if (checksum == null) {
        return checksum( true, true );
      }

      return new String( checksum );
    }

    return null;
  }


  /**
   * Checksum a SRB data file. By default, if the chksum
   * already exists, do nothing and return the chksum value.
   * If the chksum does not exist, compute and register it.
   *
   * @param force If true force compute and register of chksum even if one
   *    already exists. If false compute chksum, but don't register if one
   *     already exists.
   *
   * @return the checksum value. Returns null if this SRBFile object is a
   *     directory.
   */
  public String checksum( boolean force )
    throws IOException
  {
    return checksum( false, true );
  }

  /**
   * Checksum a SRB data file. By default, if the chksum
   * already exists, do nothing and return the chksum value.
   * If the chksum does not exist, compute and register it.
   *
   * @param force If true force compute and register of chksum even if one
   *    already exists. If false compute chksum, but don't register if one
   *     already exists.
   * @param update Reduces repetitive network calls, such as isFile again.
   *
   * @return the checksum value. Returns null if this SRBFile object is a
   *     directory.
   */
  private String checksum( boolean force, boolean update )
    throws IOException
  {
    if (isFile(update)) {
      if (force) {
        return new String(
          srbFileSystem.srbObjChksum( getName(), getParent(),
            FORCE_CHECKSUM, null ) );
      }
      else {
        return new String(
          srbFileSystem.srbObjChksum( getName(), getParent(),
            COMPUTE_CHECKSUM, null ) );
      }
    }

//TODO what is going on here, if update is false, then always return null?
    return null;
  }


  /**
   * Registers the location of a file on a SRB physical resource.
   * This SRBFile represents the logical location on the SRB.
   * it will be linked to its physical storage resource at the path
   * specified by <code>registeringObjectPath</code>.
   *<br>
   * Registers files that are created outside SRB but are accesible
   * by SRB and are as yet unregistered. The files are not modified.
   * The registeringObjectPath can be a path name in the physical resource
   * file hierarchy. SRB should have at least 'read/write' access permission
   * for registeringObjectPath. This is required as a security measure that
   * objects are registered only by owners who are able to grant SRB write
   * priveleges. One can always remove the 'write' permission once the object
   * has been registered.
   *<br>
   * The second synopsis allows one to register another copy of an already
   * existing (or registered) srbObjectName at the new location
   * registeringObjectPath.
   *<br>
   * The TargetName can be a path name in the collection hierarchy. The
   * object creation is done in the current collection, if TargetName is
   * just an object-name. If a relative or absolute collection is given
   * in TargetName, then the object is stored in that collection. The user
   * should have 'write' access permission for the collection. '.' can be
   * used as TargetName to denote the current collection.
   *<br>
   * If TargetName is a collection, then register(...) uses the names of the
   * local files as SRB object names. The directory path of
   * registeringObjectPath is not used in making the SRB object name.
   *<br>
   * If TargetName is an object-name (possibly with a collection path) and
   * there are more than one local file to be copied then the TargetName is
   * appended to the front of the local file names to make SRB object names.
   *<br>
   * The correct size parameter is needed for many other SRB operations.
   * Hence we suggest that you give the correct size of the file
   * using this option. If the size is not given, it will try to get it
   * through a stat() call assuming the file is local. If this stat() failed,
   * a value of 0 is chosen.
   *<br>
   * @see #setResource(java.lang.String)
   * @param registeringObjectPath The file/DB path of the data.
   * @param dataSize The size of the dataset if known. 0 = unknown.
   */
  public void register( String registeringObjectPath, long dataSize )
    throws IOException
  {
    if (dataSize < 0) dataSize = 0;
    srbFileSystem.srbRegisterDataset( catalogType, getName(), dataType,
      resource, getParent(), registeringObjectPath, dataSize );
  }

  /**
   * Proxy Operation that executes a command. The results of the command
   * will be returned by the InputStream. The protocol of the return
   * value on the InputStream depends on the command that was run.
   * The InputStream is opened on a different port than the main SRB
   * connection. It can be read independently of other SRB calls.
   *<br>
   * Calls the SRBFileSystem function of the same name.
   *
   * @param command       The command to run.
   * @param commandArgs   The command argument string.
   *
   * @return any byte stream output.
   * @throws IOException  If an IOException occurs.
   *
   * @see edu.sdsc.grid.io.srb.SRBFileSystem#executeProxyCommand
   */
  public InputStream executeProxyCommand( String command, String commandArgs )
    throws IOException
  {
    return srbFileSystem.executeProxyCommand( command, commandArgs,
      null, getPath(), -1);
  }

  /**
   * Links <code>newLink</code> with this object as the source.
   * The user should have at least 'read' access permission for the target.
   */
  public void link( SRBFile newLink )
    throws IOException
  {
    if (isDirectory()) {
      //recursive copy
      GeneralFile[] fileList = listFiles();
      SRBFile temp;
      newLink.mkdir();

      for (int i=0;i<fileList.length;i++) {
        temp = new SRBFile( newLink,  fileList[i].getName() );
        ((SRBFile)fileList[i]).link( temp );
      }
    }
    else {
      srbFileSystem.srbModifyDataset(0, fileName, getParent(), "", "",
        newLink.getName(), newLink.getParent(), SRBMetaDataSet.D_INSERT_LINK);
    }
  }


//----------------------------------------------------------------------
// java.io.File-like Methods
//----------------------------------------------------------------------
  /**
   * Tests whether the application can read the file denoted by
   * this abstract pathname.
   *
   * @return  <code>true</code> if and only if the file specified by this
   *   abstract pathname exists <em>and</em> can be read; otherwise
   *  <code>false</code>.
   */
  public boolean canRead( )
  {
    MetaDataRecordList[] canRead = null;
    String readable = null;
    String userName = srbFileSystem.getUserName();
    int operator = MetaDataCondition.EQUAL;

    try {
      if (isDirectory()) {
        MetaDataCondition[] conditions = {
//          getZoneCondition(),
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            SRBMetaDataSet.ACCESS_DIRECTORY_NAME, operator, getAbsolutePath())
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT) };
        canRead = fileSystem.query( conditions, selects, 3 );

        if (canRead == null)
          return false;

        for (int i=0;i<canRead.length;i++) {
          if (canRead[i].getValue(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT)
            != null)
          {
            readable = canRead[i].getValue(
              SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT).toString();
            if ( readable.equals("all") || readable.equals("write")
                || readable.equals("read") )
            {
              return true;
            }
          }
        }
      }
      else if (isFile(false)) {
        MetaDataCondition[] conditions = {
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            GeneralMetaData.DIRECTORY_NAME, operator, getParent() ),
          MetaDataSet.newCondition(
            GeneralMetaData.FILE_NAME, operator, getName() )
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.ACCESS_CONSTRAINT) };
        canRead = fileSystem.query( conditions, selects, 3 );

        if (canRead == null)
          return false;

        //only can be one recordlist returned
        readable = canRead[0].getValue(
            SRBMetaDataSet.ACCESS_CONSTRAINT).toString();
        if ( readable.equals("all") || readable.equals("write")
            || readable.equals("read") )
        {
          return true;
        }
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }

  /**
   * Tests whether the application can read the file denoted by
   * this abstract pathname. This version of the method is just to
   * reduce repeat server queries for actions internal to the package.
   *
   * @return  <code>true</code> if and only if the file specified by this
   *   abstract pathname exists <em>and</em> can be read; otherwise
   *  <code>false</code>.
   */
//used internal to reduce calls to the SRB when certain info is already known
  boolean canRead( String kindOfFile )
  {
    MetaDataRecordList[] canRead = null;
    String readable = null;
    String userName = srbFileSystem.getUserName();
    int operator = MetaDataCondition.EQUAL;

    try {
      if (kindOfFile.equals("isDir")) {
        MetaDataCondition[] conditions = {
//          getZoneCondition(),
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            SRBMetaDataSet.ACCESS_DIRECTORY_NAME, operator, getAbsolutePath())
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT) };
        canRead = fileSystem.query( conditions, selects, 3 );

        if (canRead == null)
          return false;

        for (int i=0;i<canRead.length;i++) {
          if (canRead[i].getValue(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT)
            != null)
          {
            readable = canRead[i].getValue(
              SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT).toString();
            if ( readable.equals("all") || readable.equals("write")
                || readable.equals("read") )
            {
              return true;
            }
          }
        }
      }
      else if (kindOfFile.equals("isFile")) {
        MetaDataCondition[] conditions = {
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            GeneralMetaData.DIRECTORY_NAME, operator, getParent() ),
          MetaDataSet.newCondition(
            GeneralMetaData.FILE_NAME, operator, getName() )
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.ACCESS_CONSTRAINT) };
        canRead = fileSystem.query( conditions, selects, 3 );

        if (canRead == null)
          return false;

        //only can be one recordlist returned
        readable = canRead[0].getValue(
            SRBMetaDataSet.ACCESS_CONSTRAINT).toString();
        if ( readable.equals("all") || readable.equals("write")
            || readable.equals("read") )
        {
          return true;
        }
      }
      else {
        return canRead();
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }


  /**
   * Tests whether the application can modify to the file denoted by
   * this abstract pathname.
   *
   * @return  <code>true</code> if and only if the file system actually
   *   contains a file denoted by this abstract pathname <em>and</em>
   *   the application is allowed to write to the file; otherwise
   * <code>false</code>.
   */
  public boolean canWrite( )
  {
    MetaDataRecordList[] canWrite = null;
    String writeable = null;
    String userName = srbFileSystem.getUserName();
    int operator = MetaDataCondition.EQUAL;

    try {
      if (isDirectory()) {
        MetaDataCondition[] conditions = {
//          getZoneCondition(),
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            SRBMetaDataSet.ACCESS_DIRECTORY_NAME, operator, getAbsolutePath())
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT) };
        canWrite = fileSystem.query( conditions, selects, 3 );

        if (canWrite == null)
          return false;

        for (int i=0;i<canWrite.length;i++) {
          if (canWrite[i].getValue(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT)
            != null)
          {
            writeable = canWrite[i].getValue(
              SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT).toString();
            if (( writeable.equals("all")) || ( writeable.equals("write"))) {
              return true;
            }
          }
        }
      }
      else if (isFile(false)) {
        MetaDataCondition[] conditions = {
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            GeneralMetaData.DIRECTORY_NAME, operator, getParent() ),
          MetaDataSet.newCondition(
            GeneralMetaData.FILE_NAME, operator, getName() )
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.ACCESS_CONSTRAINT) };
        canWrite = fileSystem.query( conditions, selects, 3 );

        if (canWrite == null)
          return false;

        //only can be one recordlist returned
        writeable = canWrite[0].getValue(
            SRBMetaDataSet.ACCESS_CONSTRAINT).toString();
        if (( writeable.equals("all")) || ( writeable.equals("write"))) {
          return true;
        }
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }

  /**
   * Tests whether the application can modify to the file denoted by
   * this abstract pathname. This version of the method is just to
   * reduce repeat server queries for actions internal to the package.
   *
   * @return  <code>true</code> if and only if the file system actually
   *   contains a file denoted by this abstract pathname <em>and</em>
   *   the application is allowed to write to the file; otherwise
   * <code>false</code>.
   */
//used internal to reduce calls to the SRB when certain info is already known
  boolean canWrite( String kindOfFile )
  {
    MetaDataRecordList[] canWrite = null;
    String writeable = null;
    String userName = srbFileSystem.getUserName();
    int operator = MetaDataCondition.EQUAL;

    try {
      if (kindOfFile.equals("isDir")) {
        MetaDataCondition[] conditions = {
//          getZoneCondition(),
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            SRBMetaDataSet.ACCESS_DIRECTORY_NAME, operator, getAbsolutePath())
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT) };
        canWrite = fileSystem.query( conditions, selects, 3 );

        if (canWrite == null)
          return false;

        for (int i=0;i<canWrite.length;i++) {
          if (canWrite[i].getValue(SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT)
            != null)
          {
            writeable = canWrite[i].getValue(
              SRBMetaDataSet.DIRECTORY_ACCESS_CONSTRAINT).toString();
            if (( writeable.equals("all")) || ( writeable.equals("write"))) {
              return true;
            }
          }
        }
      }
      else if (kindOfFile.equals("isFile")) {
        MetaDataCondition[] conditions = {
          MetaDataSet.newCondition(
            UserMetaData.USER_NAME, operator, userName),
          MetaDataSet.newCondition(
            GeneralMetaData.DIRECTORY_NAME, operator, getParent() ),
          MetaDataSet.newCondition(
            GeneralMetaData.FILE_NAME, operator, getName() )
        };
        MetaDataSelect[] selects = {
          MetaDataSet.newSelection(SRBMetaDataSet.ACCESS_CONSTRAINT) };
        canWrite = fileSystem.query( conditions, selects, 3 );

        if (canWrite == null)
          return false;

        //only can be one recordlist returned
        writeable = canWrite[0].getValue(
            SRBMetaDataSet.ACCESS_CONSTRAINT).toString();
        if (( writeable.equals("all")) || ( writeable.equals("write"))) {
          return true;
        }
      }
      else {
        return canWrite();
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }


  /**
   * Atomically creates a new, empty file named by this abstract pathname if
   * and only if a file with this name does not yet exist.  The check for the
   * existence of the file and the creation of the file if it does not exist
   * are a single operation that is atomic with respect to all other
   * filesystem activities that might affect the file.
   * <P>
   * Note: this method should <i>not</i> be used for file-locking, as
   * the resulting protocol cannot be made to work reliably.
   *
   * @return  <code>true</code> if the named file does not exist and was
   *          successfully created; <code>false</code> if the named file
   *          already exists
   *
   * @throws  IOException If an I/O error occurred
   */
  public boolean createNewFile() throws IOException
  {
    try {
      if (!isFile()) {
        getParentFile().mkdirs();

        int fd = srbFileSystem.srbObjCreate(
          catalogType, getName(), dataType, resource, getParent(),
          serverLocalPath, -1 );

        //Be sure to close files after a create() or open().
        srbFileSystem.srbObjClose( fd );

        return true;
      }
    } catch (SRBException e) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      //catch already exists and just return false
      if (e.getType() != -3210)
        throw e;
    }

    return false;
  }


  /**
   * <p> Creates a new empty file in the specified directory, using the
   * given prefix and suffix strings to generate its name.  If this method
   * returns successfully then it is guaranteed that:
   *
   * <ol>
   * <li> The file denoted by the returned abstract pathname did not exist
   *      before this method was invoked, and
   * <li> Neither this method nor any of its variants will return the same
   *      abstract pathname again in the current invocation of the virtual
   *      machine.
   * </ol>
   *
   * This method provides only part of a temporary-file facility.  To arrange
   * for a file created by this method to be deleted automatically, use the
   * <code>{@link #deleteOnExit}</code> method.
   *
   * <p> The <code>prefix</code> argument must be at least three characters
   * long.  It is recommended that the prefix be a short, meaningful string
   * such as <code>"hjb"</code> or <code>"mail"</code>.  The
   * <code>suffix</code> argument may be <code>null</code>, in which case the
   * suffix <code>".tmp"</code> will be used.
   *
   * <p> To create the new file, the prefix and the suffix may first be
   * adjusted to fit the limitations of the underlying platform.  If the
   * prefix is too long then it will be truncated, but its first three
   * characters will always be preserved.  If the suffix is too long then it
   * too will be truncated, but if it begins with a period character
   * (<code>'.'</code>) then the period and the first three characters
   * following it will always be preserved.  Once these adjustments have been
   * made the name of the new file will be generated by concatenating the
   * prefix, five or more internally-generated characters, and the suffix.
   *
   * <p> If the <code>directory</code> argument is <code>null</code> then the
   * default temporary-file directory will be used. Since the SRB does not
   * have a standard temporary directory, files will be placed in a temp/
   * directory in the user's SRB home directory.
   * There are certain difficulties creating a static connection to the SRB.
   * For this static method to connect to the SRB, .Mdas files must be
   * available in the local home directory/.srb. That is the information that
   * will be used when storing the temporary file. This comprimise is
   * necessary to maintain the designs unity with the java.io.File class.
   *
   * @param  prefix     The prefix string to be used in generating the file's
   *                    name; must be at least three characters long
   *
   * @param  suffix     The suffix string to be used in generating the file's
   *                    name; may be <code>null</code>, in which case the
   *                    suffix <code>".tmp"</code> will be used
   *
   * @param  directory  The directory in which the file is to be created, or
   *                    <code>null</code> if the default temporary-file
   *                    directory is to be used
   *
   * @return  An abstract pathname denoting a newly-created empty file
   *
   * @throws  IllegalArgumentException
   *          If the <code>prefix</code> argument contains fewer than three
   *          characters
   *
   * @throws  IOException  If a file could not be created
   */
  public static GeneralFile createTempFile(
    String prefix, String suffix, GeneralFile directory)
    throws IOException, IllegalArgumentException
  {
    String randomChars = "";
    for (int i=0;i<8;i++)
      randomChars += ((char) (65 + Math.random() * 25));

    if (prefix == null)
      throw new NullPointerException();
    if (prefix.length() < 3)
      throw new IllegalArgumentException("Prefix string too short");

    if (suffix == null)
      suffix = ".tmp";

    if (directory == null) {
      SRBFileSystem fs = new SRBFileSystem();
      directory = FileFactory.newFile( fs, fs.getHomeDirectory(), "temp" );
      directory.mkdir();
    }


    GeneralFile temp = FileFactory.newFile( directory,
      prefix+randomChars+suffix );

    if ( temp.createNewFile() )
      return temp;
    else {
      throw new IOException("The temp file already exists.");
    }
  }


  /**
   * Deletes the file or directory denoted by this abstract pathname.  If
   * this pathname denotes a directory, then the directory must be empty in
   * order to be deleted.
   *
   * @return  <code>true</code> if and only if the file or directory is
   *          successfully deleted; <code>false</code> otherwise
   */
  public boolean delete( )
  {
    return delete( false );
  }

  /**
   * Deletes the file or directory denoted by this abstract pathname.  If
   * this pathname denotes a directory, then the directory must be empty in
   * order to be deleted.
   *
   * @return  <code>true</code> if and only if the file or directory is
   *          successfully deleted; <code>false</code> otherwise
   */
  public boolean delete( boolean force )
  {
    try {
      //Trashcan new as of SRB3.1
      if (!force &&  (srbFileSystem.getVersionNumber() > 3 ))
      {
        if (isDirectory()) {
          srbFileSystem.srbModifyCollect( catalogType, getAbsolutePath(),
            "", "", "", SRBMetaDataSet.D_DELETE_COLL );

//          This way no good, is recursive
//          srbFileSystem.srbModifyCollect ( catalogType,  getAbsolutePath(),
//            "", "", "", SRBMetaDataSet.C_MOVE_COLL_TO_TRASH );
          return true;
        }
        else if (isFile(false) && (getReplicaNumber() < 0)) {
          srbFileSystem.srbModifyDataset ( catalogType, fileName, getParent(),
            "", "", "", "", SRBMetaDataSet.D_MOVE_DATA_TO_TRASH);
          return true;
        }
        else {
          srbFileSystem.srbObjUnlink( fileName, getParent() );
          return true;
        }
      }
      else {
        if (isDirectory()) {
          srbFileSystem.srbModifyCollect( catalogType, getAbsolutePath(),
            "", "", "", SRBMetaDataSet.D_DELETE_COLL );
          return true;
        }
        else if (isFile(false)) {
          srbFileSystem.srbObjUnlink( fileName, getParent() );
          return true;
        }
      }
    } catch( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      return false;
    }
    return false;
  }


  /**
   * Requests that the file or directory denoted by this abstract
   * pathname be deleted when the virtual machine terminates.
   * Deletion will be attempted only for normal termination of the
   * virtual machine, as defined by the Java Language Specification.
   *
   * <p> Once deletion has been requested, it is not possible to cancel the
   * request.  This method should therefore be used with care.
   *
   * <P>
   * Note: this method should <i>not</i> be used for file-locking, as
   * the resulting protocol cannot be made to work reliably.
   */
  public void deleteOnExit( )
  {
    deleteOnExit = true;
  }


  /**
   * Tests this abstract pathname for equality with the given object.
   * Returns <code>true</code> if and only if the argument is not
   * <code>null</code> and is an abstract pathname that denotes the same file
   * or directory as this abstract pathname on the same filesystem.
   * 
   *
   * @param   obj   The object to be compared with this abstract pathname
   *
   * @return  <code>true</code> if and only if the objects are the same;
   *          <code>false</code> otherwise
   */  
  public boolean equals( Object obj )
  { 
//Does not compare other user or host information of the filesystems?
    try {
      if (obj == null)
        return false;

      if (obj instanceof SRBFile) {
        SRBFile temp = (SRBFile) obj;
//TODO March 18, 2008, 
//should maybe compare SRBFileSystems? or just zone?
/*        if (temp.srbFileSystem.getMcatZone().equals(
          srbFileSystem.getMcatZone())) 
        {
 */
        if (temp.srbFileSystem.equals(srbFileSystem)) 
        { 
          return getAbsolutePath().equals(temp.getAbsolutePath());         
        } 
      }
    } catch (ClassCastException e) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }
    return false;
  }


  /**
   * Tests whether the file denoted by this abstract pathname exists.
   *
   * @return  <code>true</code> if and only if the file denoted by this
   *   abstract pathname exists; <code>false</code> otherwise
   */
  public boolean exists( )
  {
    try {
      MetaDataRecordList[] rl = null;
      int operator = MetaDataCondition.EQUAL;

      //if it is a file
      MetaDataCondition conditions[] = null;

      if (getReplicaNumber() >= 0) {
        conditions = new MetaDataCondition[2];
        conditions[0] = MetaDataSet.newCondition(
          GeneralMetaData.DIRECTORY_NAME, operator, getParent() );
        conditions[1] = MetaDataSet.newCondition(
          GeneralMetaData.FILE_NAME, operator, getName() );
        conditions[1] = MetaDataSet.newCondition(
          SRBMetaDataSet.FILE_REPLICATION_ENUM, operator, replicaNumber );
      }
      else {
        conditions = new MetaDataCondition[2];
        conditions[0] = MetaDataSet.newCondition(
          GeneralMetaData.DIRECTORY_NAME, operator, getParent() );
        conditions[1] = MetaDataSet.newCondition(
          GeneralMetaData.FILE_NAME, operator, getName() );
      }

      MetaDataSelect selects[] = {
        MetaDataSet.newSelection( GeneralMetaData.FILE_NAME )
      };

      rl = fileSystem.query( conditions, selects, 3 );

      if (rl != null)
        return true;


      //if it is a directory
      conditions = new MetaDataCondition[1];
      conditions[0] =
        MetaDataSet.newCondition(
          GeneralMetaData.DIRECTORY_NAME, operator, getAbsolutePath() );
      selects[0] =
        MetaDataSet.newSelection( GeneralMetaData.DIRECTORY_NAME );
      rl = fileSystem.query( conditions, selects, 3 );

      if (rl != null)
        return true;

    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }


  /**
   * Returns the canonical pathname string of this abstract pathname.
   *
   * @return  The canonical pathname string denoting the same file or
   *          directory as this abstract pathname
   *
   * @throws  IOException
   *          If an I/O error occurs, which is possible because the
   *          construction of the canonical pathname may require
   *          filesystem queries
   */
  public String getCanonicalPath( )
    throws IOException
  {
    if (( directory != null ) && (!directory.isEmpty())) {
      int size = directory.size();
      String path = (String) directory.firstElement();
      int i = 1;

      while (i < size ) {
        path += separator + directory.get( i );
        i++;
      }

      return path + separator + fileName;
    }

    return fileName;
  }


  /**
   * @return The name of the file or directory denoted by this
   *   abstract pathname.
   */
  public String getName( )
  {
    //strip &COPY=
    int index = fileName.indexOf( "&COPY=" );
    if (index >= 0) {
      return fileName.substring( 0, index );
    }
    else {
      return fileName;
    }
  }



  /**
   * @return This abstract pathname as a pathname string.
   */
  public String getPath( )
  {
    //The path always gets converted to absolute form on construction
    return originalFilePath;
  }

  /**
   * Computes a hash code for this abstract pathname. The hash code of
   * an abstract pathname is equal to the exclusive <em>or</em> of its
   * pathname string and the decimal value <code>1234321</code>.
   *
   * @return  A hash code for this abstract pathname
   */
  public int hashCode( )
  {
    return getAbsolutePath().toLowerCase().hashCode() ^ 1234321;
  }


  /**
   * Tests whether this abstract pathname is absolute. A pathname is
   * absolute if its prefix is <code>"/"</code>.
   *
   * @return  <code>true</code> if this abstract pathname is absolute,
   *          <code>false</code> otherwise
   */
  public boolean isAbsolute( )
  {
    //all path names are made absolute at construction.
    return true;
  }


  /**
   * Tests whether the file denoted by this abstract pathname is a
   * SRB container.
   *<P>
   * @return <code>true</code> if and only if the file denoted by this
   *          abstract pathname exists <em>and</em> is a container;
   *          <code>false</code> otherwise
   */
  public boolean isContainer( )
  {
    return false;
  }


  /**
   * Tests whether the file denoted by this abstract pathname is a directory.
   * Also known on the SRB as a collection.
   *<P>
   * A SRB collection is a logical name given to a set of data sets. All data
   * sets stored in SRB/MCAT are stored in some collection. A collection can
   * have sub-collections, and hence provides a hierarchical structure. A
   * collection in SRB/MCAT can be equated to a directory in a Unix file
   * system. But unlike a file system, a collection is not limited to a
   * single device (or partition). A collection is logical but the datsets
   * grouped under a collection can be stored in heterogeneous storage
   * devices. There is one obvious restriction, the name given to a data set
   * in a collection or sub-collection should be unique in that collection.
   *
   * @return <code>true</code> if and only if the file denoted by this
   *          abstract pathname exists <em>and</em> is a directory;
   *          <code>false</code> otherwise
   */
  public boolean isDirectory( )
  {
    if ( useCache ) {
      return isDirectory( false );
    }
    else {
      return isDirectory( true );
    }
  }


  /**
   * Tests whether the file denoted by this abstract pathname is a directory.
   * Also known on the SRB as a collection.
   *<P>
   * @param update If true, send a new query to the SRB to determine if
   *    this abstract pathname refers to a directory. If false, this
   *    method will return a previously stored value. Also queries the SRB
   *    if the value is not already stored with this object.
   * @return <code>true</code> if and only if the file denoted by this
   *          abstract pathname exists <em>and</em> is a directory;
   *          <code>false</code> otherwise
   */
  public boolean isDirectory( boolean update )
  {
    if (update || (pathNameType == PATH_IS_UNKNOWN)) {
      //run the code below
    }
    else if (pathNameType == PATH_IS_FILE) {
      return false;
    }
    else if (pathNameType == PATH_IS_DIRECTORY) {
      return true;
    }

    MetaDataRecordList[] rl = null;
    MetaDataCondition[] conditions = {
      MetaDataSet.newCondition( GeneralMetaData.DIRECTORY_NAME,
        MetaDataCondition.EQUAL, getAbsolutePath() ) };
    MetaDataSelect[] selects = {
      MetaDataSet.newSelection( GeneralMetaData.DIRECTORY_NAME ) };

    try {
      rl = fileSystem.query( conditions, selects, 3 );

      if ( rl != null ) {
        pathNameType = PATH_IS_DIRECTORY;
        return true;
      }

    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }


  /**
   * Tests whether the file denoted by this abstract pathname is a normal
   * file. A file is <em>normal</em> if it is not a directory or a container.
   * Any non-directory or other subclass of SRBFile, such as a SRBContainer,
   * file created by a Java application is guaranteed to be a normal file.
   *<P>
   * In the terminology of SRB, files are known as data sets. A data set is
   * a "stream-of-bytes" entity that can be uniquely identified. For example,
   * a file in HPSS or Unix is a data set, or a LOB stored in a SRB Vault
   * database is a data set. Importantly, note that a data set is not a
   * set of data objects/files. Each data set in SRB is given a unique
   * internal identifier by SRB. A dataset is associated with a collection.
   *
   * @return  <code>true</code> if and only if the file denoted by this
   *          abstract pathname exists <em>and</em> is a normal file;
   *          <code>false</code> otherwise
   */
  public boolean isFile( )
  {
    if ( useCache ) {
      return isFile( false );
    }
    else {
      return isFile( true );
    }
  }

  /**
   * Tests whether the file denoted by this abstract pathname is a file.
   * Also known on the SRB as a dataset.
   *<P>
   * @param update If true, send a new query to the SRB to determine if
   *    this abstract pathname refers to a file. If false, this
   *    method will return a previously stored value. Also queries the SRB
   *    if the value is not already stored with this object.
   * @return <code>true</code> if and only if the file denoted by this
   *          abstract pathname exists <em>and</em> is a directory;
   *          <code>false</code> otherwise
   */
  public boolean isFile( boolean update )
  {
    if ((pathNameType == PATH_IS_UNKNOWN) || update) {
      //run the code below
    }
    else if (pathNameType == PATH_IS_FILE) {
      return true;
    }
    else if (pathNameType == PATH_IS_DIRECTORY) {
      return false;
    }

    MetaDataRecordList[] rl = null;
    MetaDataCondition[] conditions = {
      MetaDataSet.newCondition( GeneralMetaData.DIRECTORY_NAME,
        MetaDataCondition.EQUAL, getParent() ),
      MetaDataSet.newCondition( GeneralMetaData.FILE_NAME,
        MetaDataCondition.EQUAL, getName() ) };
    MetaDataSelect[] selects = {
      MetaDataSet.newSelection( GeneralMetaData.FILE_NAME ) };

    try {
      rl = fileSystem.query( conditions, selects, 3 );

      if( rl != null ) {
        pathNameType = PATH_IS_FILE;
        return true;
      }

    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }



  /**
   * Tests whether the file named by this abstract pathname is a hidden file.
   *
   * @return  <code>true</code> if and only if the file denoted by this
   *          abstract pathname is hidden.
   */
  public boolean isHidden( )
  {
    return false; //SRB files can't be hidden
  }


  /**
   * Returns the time that the file denoted by this abstract pathname
   * was last modified.
   *
   * @return  A <code>long</code> value representing the time the file was
   *          last modified, measured in milliseconds since the epoch
   *          (00:00:00 GMT, January 1, 1970), or <code>0L</code> if the
   *          file does not exist or if an I/O error occurs
   */
  public long lastModified( )
  {
    long lastModified = 0;
    String result = null;
    try {
      result = firstQueryResult( GeneralMetaData.MODIFICATION_DATE );
      if (result != null) {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd-HH.mm.ss");
        lastModified = format.parse( result ).getTime();
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      return 0;
    } catch ( ParseException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      return 0;
    }
    return lastModified;
  }


  /**
   * Returns an array of strings naming the files and directories in
   * the directory denoted by this abstract pathname.
   *<P>
   * There is no guarantee that the name strings in the resulting
   * array will appear in any specific order; they are not, in particular,
   * guaranteed to appear in alphabetical order.
   *<P>
   * If this SRBFile object denotes a file, the directory containing
   * that file will be listed instead.
   *<P>
   * This method will return all the files in the directory. Listing
   * directories with a large number of files may take a very long time.
   * The more generic SRBFile.query() method could be used to iterate
   * through the file list piecewise.
   *
   * @return  An array of strings naming the files and directories in the
   *          directory denoted by this abstract pathname. The array will be
   *           empty if the directory is empty. Returns null if an I/O error
   *           occurs.
   */
  public String[] list( )
  {
    MetaDataCondition conditions[] = new MetaDataCondition[1];
    MetaDataSelect selects[] = {
      MetaDataSet.newSelection( GeneralMetaData.FILE_NAME ) };
    MetaDataRecordList[] rl1 = null;
    MetaDataRecordList[] rl2 = null;
    MetaDataRecordList[] temp = null;
    Vector list = null;
    String path = null;


    try {
      //Have to do two queries, one for files and one for directories.
      if (isDirectory()) {
        path = getAbsolutePath();
      }
      else {
        path = getParent();
      }

      //get all the files
      conditions[0] = MetaDataSet.newCondition(
        GeneralMetaData.DIRECTORY_NAME, MetaDataCondition.EQUAL, path );
      rl1 = fileSystem.query(
        conditions, selects, fileSystem.DEFAULT_RECORDS_WANTED );
      if (completeDirectoryList) {
        rl1 = MetaDataRecordList.getAllResults( rl1 );
      }

      //get all the sub-directories
      selects[0] = MetaDataSet.newSelection( GeneralMetaData.DIRECTORY_NAME );
      conditions[0] = MetaDataSet.newCondition(
        DirectoryMetaData.PARENT_DIRECTORY_NAME, MetaDataCondition.EQUAL, path );
      rl2 = fileSystem.query(
        conditions, selects, fileSystem.DEFAULT_RECORDS_WANTED );
      if (completeDirectoryList) {
        rl2 = MetaDataRecordList.getAllResults( rl2 );
      }

      //change to relative path
      if (rl2 != null) {
        String absolutePath = null;
        String relativePath = null;
        for (int i=0;i<rl2.length;i++) {
          //only one record per rl
          absolutePath = rl2[i].getStringValue(0);
          relativePath = absolutePath.substring(
            absolutePath.lastIndexOf( "/" )+1 );
          rl2[i].setValue( 0, relativePath );
        }
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      return null;
    }


    if (( rl1 != null ) && (rl2 != null)) {
      //length of previous query + (new query - table and attribute names)
      temp = new SRBMetaDataRecordList[rl1.length+rl2.length];
      //copy files
      System.arraycopy( rl1, 0, temp, 0, rl1.length );
      System.arraycopy( rl2, 0, temp, rl1.length, rl2.length );
    }
    else if (rl1 != null) {
      temp = rl1;
    }
    else if (rl2 != null) {
      temp = rl2;
    }
    else {
      return new String[0];
    }

    list = new Vector();
    for (int i=0;i<temp.length;i++) {
      if (temp[i].getStringValue(0) != null) {
        //only one record per rl
        list.add(temp[i].getStringValue(0));
      }
    }

    return (String[]) list.toArray(new String[0]);
  }


  /**
   * Returns the array of strings naming the files and directories in
   * the directory denoted by this abstract pathname and which match a
   * query formed using these <code>conditions</code>.
   *
   * @see list()
   */
  public String[] list( MetaDataCondition[] conditions )
  {
    if (conditions == null)
      return list();

    MetaDataCondition tempConditions[] = null;
    MetaDataSelect selects[] = {
      MetaDataSet.newSelection( GeneralMetaData.FILE_NAME ) };
    MetaDataRecordList[] rl1 = null;
    MetaDataRecordList[] rl2 = null;
    MetaDataRecordList[] temp = null;
    Vector list = null;
    String path = null;


    try {
      //Have to do two queries, one for files and one for directories.
      if (isDirectory()) {
        path = getAbsolutePath();
      }
      else {
        path = getParent();
      }
      tempConditions = MetaDataSet.mergeConditions(
        MetaDataSet.newCondition(  GeneralMetaData.DIRECTORY_NAME,
          MetaDataCondition.EQUAL, path ),
        conditions );

      //get all the files
      rl1 = fileSystem.query( tempConditions, selects );
      if (completeDirectoryList) {
        rl1 = MetaDataRecordList.getAllResults( rl1 );
      }

      //get all the sub-directories
      selects[0] = MetaDataSet.newSelection( GeneralMetaData.DIRECTORY_NAME );
      tempConditions = MetaDataSet.mergeConditions( MetaDataSet.newCondition(
        DirectoryMetaData.PARENT_DIRECTORY_NAME, MetaDataCondition.EQUAL, path ),
        (MetaDataCondition)null );
      rl2 = fileSystem.query( tempConditions, selects );
      if (completeDirectoryList) {
        rl2 = MetaDataRecordList.getAllResults( rl2 );
      }

      //change to relative path
      if (rl2 != null) {
        String absolutePath = null;
        String relativePath = null;
        for (int i=0;i<rl2.length;i++) {
          //only one record per rl
          absolutePath = rl2[i].getStringValue(0);
          relativePath = absolutePath.substring(
            absolutePath.lastIndexOf( "/" )+1 );
          rl2[i].setValue( 0, relativePath );
        }
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      return null;
    }


    if (( rl1 != null ) && (rl2 != null)) {
      //length of previous query + (new query - table and attribute names)
      temp = new MetaDataRecordList[rl1.length+rl2.length];
      //copy files
      System.arraycopy( rl1, 0, temp, 0, rl1.length );
      System.arraycopy( rl2, 0, temp, rl1.length, rl2.length );
    }
    else if (rl1 != null) {
      temp = rl1;
    }
    else if (rl2 != null) {
      temp = rl2;
    }
    else {
      return new String[0];
    }

    list = new Vector();
    for (int i=0;i<temp.length;i++) {
      if (temp[i].getStringValue(0) != null) {
        //only one record per rl
        list.add(temp[i].getStringValue(0));
      }
    }

    return (String[]) list.toArray(new String[0]);
  }


  /**
   * Creates the directory named by this abstract pathname.
   */
  public boolean mkdir( )
  {
    try {
      if (!isDirectory()) {
        srbFileSystem.srbCreateCollect( catalogType, getParent(), getName() );
        return true;
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return false;
  }



  /**
   * Renames the file denoted by this abstract pathname. Will attempt to
   * overwrite existing files with the same name at the destination.
   *<P>
   * Whether or not this method can move a file from one filesystem to
   * another is platform-dependent. The return value should always be
   * checked to make sure that the rename operation was successful.
   *  
   * After an unsuccessful attempt, some errors may cause a whole/partial copy
   * of this file/directory to be left at <code>dest</code>.
   * 
   * After a successful move, this file object is no longer
   * valid, only the <code>dest</code> file object should be used.
   *
   * @param  dest  The new abstract pathname for the named file
   * @throws NullPointerException - If dest is null
   */
  public boolean renameTo(GeneralFile dest)
    throws IllegalArgumentException
  {
    if (dest instanceof SRBFile) {
      try {
        if (isFile()) {
/*TODO          if (!dest.getServerLocalPath.equals( serverLocalPath )) {

          }
          else */if (getParent().equals(dest.getParent())) {
            //only renaming data
            srbFileSystem.srbModifyDataset( catalogType, fileName, getParent(),
              "", serverLocalPath, dest.getName(), "", SRBMetaDataSet.D_CHANGE_DNAME);
          }
          else if (getName().equals(dest.getName())) {
            //only moving to new collection
            srbFileSystem.srbModifyDataset( catalogType, fileName, getParent(),
              "", serverLocalPath, dest.getParent(), "", SRBMetaDataSet.D_CHANGE_GROUP);
          }
          else {
            //changing name of object as well as its collection
            String tempName = dest.getName() + (long) Math.random();

            //first change the name to a temp name
            srbFileSystem.srbModifyDataset( catalogType, fileName, getParent(),
              "", serverLocalPath, tempName, "", SRBMetaDataSet.D_CHANGE_DNAME);


            try {
              //then change directory name
              srbFileSystem.srbModifyDataset( catalogType, tempName,
                getParent(), "",serverLocalPath, dest.getParent(), "",
                SRBMetaDataSet.D_CHANGE_GROUP);
            } catch ( IOException e ) {
              if (SRBCommands.DEBUG > 0) e.printStackTrace();
              //change the name back
              srbFileSystem.srbModifyDataset( catalogType, tempName,
                getParent(), "",serverLocalPath, fileName, "",
                SRBMetaDataSet.D_CHANGE_DNAME);
            }

            try { //then change the temp name to the new name
              srbFileSystem.srbModifyDataset( catalogType, tempName,
                dest.getParent(),  "",serverLocalPath, dest.getName(), "",
                SRBMetaDataSet.D_CHANGE_DNAME);
            } catch ( IOException e ) {
              if (SRBCommands.DEBUG > 0) e.printStackTrace();
              //change the it back
              srbFileSystem.srbModifyDataset( catalogType, tempName,
                dest.getParent(),  "",serverLocalPath, getParent(), "",
                SRBMetaDataSet.D_CHANGE_GROUP);
              srbFileSystem.srbModifyDataset( catalogType, tempName,
                getParent(), "",serverLocalPath, fileName, "",
                SRBMetaDataSet.D_CHANGE_DNAME);
            }
          }
          directory = new Vector();
          setFileName( dest.getAbsolutePath() );
          return true;
        }
        else if (isDirectory(false)) {
          srbFileSystem.srbModifyCollect( catalogType,
            getAbsolutePath(), dest.getAbsolutePath(), null, null,
            SRBMetaDataSet.D_CHANGE_COLL_NAME);
          directory = new Vector();
          setFileName( dest.getAbsolutePath() );
          return true;
        }
        else if (!exists()) {
          directory = new Vector();
          setFileName( dest.getAbsolutePath() );
          return true;
        }
      } catch ( IOException e ) {
        if (SRBCommands.DEBUG > 0) e.printStackTrace();
      }
    }
    else {
      return super.renameTo(dest);
    }

    return false;
  }


  /*
   * Sets the last-modified time of the file or directory named by
   * this abstract pathname.
   *<P>
   * All platforms support file-modification times to the nearest second,
   * but some provide more precision. The argument will be truncated to fit
   * the supported precision. If the operation succeeds and no intervening
   * operations on the file take place, then the next invocation of the
   * lastModified() method will return the (possibly truncated) time argument
   * that was passed to this method.
   *
   * The SRB does not support this operation.
   * This method will only change the lastModified time to the current time.
   * The SRB will change the last modified date of a file, but then
   * immediately recognize a change to the file has occured and change
   * the last modified time to the current time.
   *
   * @param  time  The new last-modified time, measured in milliseconds since
   *               the epoch (00:00:00 GMT, January 1, 1970)
   *
   * @throws  IllegalArgumentException  If the argument is negative
   */
  /**
   * This method will only change the lastModified time to the current time.
   * The SRB will overwrite the input from this method
   */
  public boolean setLastModified(long time)
    throws IllegalArgumentException
  {
    //can't do this in the SRB.
    throw new UnsupportedOperationException();
  }


  /**
   * Marks the file or directory named by this abstract pathname so that
   * only read operations are allowed.  After invoking this method the file
   * or directory is guaranteed not to change until it is either deleted or
   * marked to allow write access.  Whether or not a read-only file or
   * directory may be deleted depends upon the underlying system.
   *
   * This operation is not possible on the SRB. File permissions can only
   * be changed on a per user basis. Changing the file permissions for
   * everyone is not possible.
   */
  public boolean setReadOnly( )
  {
    //can't do this in the SRB.
    throw new UnsupportedOperationException();
  }


  /**
   * Constructs a <tt>srb:</tt> URI that represents this abstract pathname.
   *
   * <p> The exact form of the URI is according to the SRB.  If it can be
   * determined that the file denoted by this abstract pathname is a
   * directory, then the resulting URI will end with a slash.
   *
   * <p> For a given abstract pathname <i>f</i>, it is guaranteed that
   *
   * <blockquote><tt>
   * new {@link #SRBFile(java.net.URI) SRBFile}
   * (</tt><i>&nbsp;f</i><tt>.toURI()).equals(</tt><i>&nbsp;f</i><tt>)
   * </tt></blockquote>
   *
   * so long as the original abstract pathname, the URI, and the new abstract
   * pathname are all created in (possibly different invocations of) the same
   * Java virtual machine.  However, this relationship typically does not hold
   * when a <tt>srb:</tt> URI that is created in a virtual machine on one
   * operating system is converted into an abstract pathname in a virtual
   * machine on a different operating system.
   *
   * @return  An absolute, hierarchical URI with a scheme equal to
   *          <tt>"srb"</tt>, a path representing this abstract pathname,
   *          and undefined authority, query, and fragment components
   *
   * @see #SRBFile(java.net.URI)
   * @see java.net.URI
   * @see java.net.URI#toURL()
   */
  public URI toURI( )
  {
    URI uri = null;

    try {
      if (isDirectory()) {
        uri = new URI( "srb",
          srbFileSystem.getUserName() + "." + srbFileSystem.getDomainName(),
          srbFileSystem.getHost(), srbFileSystem.getPort(), getAbsolutePath() +
          "/", "", "");
      }
      else {
        uri = new URI( "srb",
          srbFileSystem.getUserName() + "." + srbFileSystem.getDomainName(),
          srbFileSystem.getHost(),  srbFileSystem.getPort(), getAbsolutePath(),
          "", "");
      }
    } catch ( URISyntaxException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return uri;
  }
  
  /**
   * Constructs a <tt>srb:</tt> URI that represents this abstract pathname.
   *
   * <p> The exact form of the URI is according to the SRB.  If it can be
   * determined that the file denoted by this abstract pathname is a
   * directory, then the resulting URI will end with a slash.
   *
   * <p> For a given abstract pathname <i>f</i>, it is guaranteed that
   *
   * <blockquote><tt>
   * new {@link #SRBFile(java.net.URI) SRBFile}
   * (</tt><i>&nbsp;f</i><tt>.toURI()).equals(</tt><i>&nbsp;f</i><tt>)
   * </tt></blockquote>
   *
   * so long as the original abstract pathname, the URI, and the new abstract
   * pathname are all created in (possibly different invocations of) the same
   * Java virtual machine.  However, this relationship typically does not hold
   * when a <tt>srb:</tt> URI that is created in a virtual machine on one
   * operating system is converted into an abstract pathname in a virtual
   * machine on a different operating system.
   *
   * @return  An absolute, hierarchical URI with a scheme equal to
   *          <tt>"srb"</tt>, a path representing this abstract pathname,
   *          and undefined authority, query, and fragment components
   *
   * @see #SRBFile(java.net.URI)
   * @see java.net.URI
   * @see java.net.URI#toURL()
   */
  public URI toURI( boolean includePassword )
  {
    if (!includePassword)
      return toURI();
    
    URI uri = null;

    try {
      if (isDirectory()) {
        uri = new URI( "srb",
          srbFileSystem.getUserName() + "." + srbFileSystem.getDomainName() +
            ":"+srbFileSystem.getPassword(),
          srbFileSystem.getHost(), srbFileSystem.getPort(), getAbsolutePath() +
          "/", "", "");
      }
      else {
        uri = new URI( "srb",
          srbFileSystem.getUserName() + "." + srbFileSystem.getDomainName() +
            ":"+srbFileSystem.getPassword(),
          srbFileSystem.getHost(),  srbFileSystem.getPort(), getAbsolutePath(),
          "", "");
      }
    } catch ( URISyntaxException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return uri;
  }


  /**
   * Converts this abstract pathname into a <code>srb:</code> URL.  The
   * exact form of the URL is is according to the SRB.  If it can be
   * determined that the file denoted by this abstract pathname is a
   * directory, then the resulting URL will end with a slash.
   *
   * <p> <b>Usage note:</b> This method does not automatically escape
   * characters that are illegal in URLs.  It is recommended that new code
   * convert an abstract pathname into a URL by first converting it into a
   * URI, via the {@link #toURI() toURI} method, and then converting the URI
   * into a URL via the {@link java.net.URI#toURL() URI.toURL} method.
   *
   * @return  A URL object representing the equivalent file URL
   *
   * @throws  MalformedURLException
   *          If the path cannot be parsed as a URL
   *
   * @see     #toURI()
   * @see     java.net.URI
   * @see     java.net.URI#toURL()
   * @see     java.net.URL
   */
  public URL toURL() throws MalformedURLException
  {
    URL url = null;

    if (isDirectory()) {
      url = new URL( "srb://" + srbFileSystem.getUserName() + "." +
        srbFileSystem.getDomainName() + "@" + srbFileSystem.getHost() + ":" +
        srbFileSystem.getPort() + getAbsolutePath() + "/" );
    }
    else {
      url = new URL( "srb://" + srbFileSystem.getUserName() + "." +
        srbFileSystem.getDomainName() + "@" + srbFileSystem.getHost() + ":" +
        srbFileSystem.getPort() + getAbsolutePath() );
    }

    return url;
  }


  /**
   * Returns a string representation of this file object.
   * The string is formated according to the SRB URI model.
   * Note: the user password will not be included in the URI.
   */
  public String toString( )
  {
    return new String( "srb://"+srbFileSystem.getUserName()+
      "."+srbFileSystem.getDomainName()+"@"+
      srbFileSystem.getHost()+":"+
      srbFileSystem.getPort() + getAbsolutePath() );
  }
}

