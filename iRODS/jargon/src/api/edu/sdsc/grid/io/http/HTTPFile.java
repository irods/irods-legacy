//	Copyright (c) 2006, Regents of the University of California
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are
//	met:
//
//	  * Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//	  * Redistributions in binary form must reproduce the above copyright
//	notice, this list of conditions and the following disclaimer in the
//	documentation and/or other materials provided with the distribution.
//	  * Neither the name of the University of California, San Diego (UCSD) nor
//	the names of its contributors may be used to endorse or promote products
//	derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//	RemoteFile.java	-  edu.sdsc.grid.io.RemoteFile
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-.GeneralFile
//	            |
//	            +-.RemoteFile
//                 |
//                 +-.http.HTTPFile
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.http;

import java.io.*;
import java.net.*;
import java.util.*;

import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.*;

//consider instead http://www.innovation.ch/java/HTTPClient/

/**
 * An abstract representation of file and directory pathnames on a
 * http server.
 *<P>
 * Shares many similarities with the java.io.File class:
 * User interfaces and operating systems use system-dependent pathname
 * strings to name files and directories. This class presents an abstract,
 * system-independent view of hierarchical pathnames. An abstract pathname
 * has two components:
 *<P>
 * Instances of the HTTPFile class are immutable; that is, once created,
 * the abstract pathname represented by a HTTPFile object will never change.
 *<P>
 * @author	Lucas Gilbert, San Diego Supercomputer Center
 * @see	java.io.File
 * @see	edu.sdsc.grid.io.GeneralFile
 * @since   Jargon2.0
 */
public class HTTPFile extends RemoteFile
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------
	/**
	 * Standard HTTP path separator character represented as a string for
	 * convenience. This string contains a single character, namely
   * <code>{@link #PATH_SEPARATOR_CHAR}</code>.
   */
	public static final String PATH_SEPARATOR = "/";

	/**
	 * The HTTP path separator character, '/'.
	 */
	public static final char PATH_SEPARATOR_CHAR = '/';

//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
  /**
   * Connection to the http server
   */
  HTTPFileSystem httpFileSystem;

//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
	/**
	 * Creates a new HTTPFile instance by converting the given pathname string
	 * into an abstract pathname.
	 *<P>
	 * @param fileSystem	The connection to the http server
	 * @param filePath	A pathname string
	 */
	public HTTPFile( HTTPFileSystem fileSystem, String filePath )
		throws IOException
	{
		this( fileSystem, filePath, "" );
	}

	/**
	 * Creates a new HTTPFile instance from a parent pathname string and
	 * a child pathname string.
	 *<P>
	 * If parent is null then the new HTTPFile instance is created as if by
	 * invoking the single-argument HTTPFile constructor on the given child
	 * pathname string.
	 *<P>
	 * Otherwise the parent pathname string is taken to denote a directory,
	 * and the child pathname string is taken to denote either a directory
	 * or a file. If the child pathname string is absolute then it is
	 * converted into a relative pathname in a system-dependent way.
	 * If parent is the empty string then the new RemoteFile instance is created
	 * by converting child into an abstract pathname and resolving the result
	 * against a system-dependent default directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the child abstract
	 * pathname is resolved against the parent.
	 *<P>
	 * @param fileSystem	The connection to the http server
	 * @param parent	The parent pathname string
	 * @param child		The child pathname string
	 */
	public HTTPFile( HTTPFileSystem fileSystem, String parent, String child )
		throws IOException
	{
		super( fileSystem, parent, child );   
    httpFileSystem = fileSystem;
	}

	/**
	 * Creates a new HTTPFile instance from a parent abstract pathname
	 * and a child pathname string.
	 *<P>
	 * If parent is null then the new HTTPFile instance is created as if
	 * by invoking the single-argument HTTPFile constructor on the given
	 * child pathname string.
	 *<P>
	 * Otherwise the parent abstract pathname is taken to denote a directory,
	 * and the child pathname string is taken to denote either a directory or
	 * a file. If the child pathname string is absolute then it is converted
	 * into a relative pathname in a system-dependent way. If parent is the
	 * empty abstract pathname then the new RemoteFile instance is created by
	 * converting child into an abstract pathname and resolving the result
	 * against a system-dependent default directory. Otherwise each pathname
	 * string is converted into an abstract pathname and the child abstract
	 * pathname is resolved against the parent.
	 *<P>
	 * @param parent	The parent abstract pathname
	 * @param child		The child pathname string
	 */
	public HTTPFile( HTTPFile parent, String child )
		throws IOException
	{
		this( (HTTPFileSystem) parent.getFileSystem(), 
      parent.getAbsolutePath(), child );
	}



	/**
	 * Creates a new HTTPFile instance by converting the given file: URI
	 * into an abstract pathname.
	 *<P>
   *  HTTP URI protocol:<br>
   *  http:// [ userName [ : password ] @ ] host [ : port ][ / path ]
   *<P>
   * example:<br>
   * http://http@http.sdsc.edu:21/pub/testfile.txt 
	 *
	 * @param uri An absolute, hierarchical URI using a supported scheme.
	 * @throws NullPointerException if <code>uri</code> is <code>null</code>.
	 * @throws IllegalArgumentException If the preconditions on the parameter
	 *		do not hold.
	 */
	public HTTPFile( URI uri )
		throws IOException, URISyntaxException
	{
		super( uri );
//TODO just allow anything handled by URLConnection?    
//    if (uri.getScheme().equals( "http" )) {
      setFileSystem( new HTTPFileSystem(uri) );
      setFileName( uri.getPath() );
      httpFileSystem = (HTTPFileSystem)fileSystem;
/*    }
    else {
      throw new URISyntaxException(uri.toString(), "Wrong URI scheme");
    }
 */
	}



//----------------------------------------------------------------------
// Setters and Getters
//----------------------------------------------------------------------

  
  
  
//----------------------------------------------------------------------
// GeneralFile Methods
//----------------------------------------------------------------------
	/**
	 * Copies this file to another file. This object is the source file.
	 * The destination file is given as the argument.
	 * If the destination file, does not exist a new one will be created.
	 * Otherwise the source file will be appended to the destination file.
	 * Directories will be copied recursively.
	 *
	 * @param file	The file to receive the data.
	 * @throws  NullPointerException If file is null.
	 * @throws IOException If an IOException occurs.
	 *//*
	public void copyTo( GeneralFile file, boolean forceOverwrite )
		throws IOException
	{
		if (file == null) {
			throw new NullPointerException();
		}

		if (isDirectory()) {
			//recursive copy
			GeneralFile[] fileList = listFiles();

			file.mkdir();
			if (fileList != null) {
				for (int i=0;i<fileList.length;i++) {
					fileList[i].copyTo(
						FileFactory.newFile( file.getFileSystem(), file.getAbsolutePath(),
              fileList[i].getName()), forceOverwrite );
				}
			}
		}
		else {
			if (file.isDirectory()) {
				//change the destination from a directory to a file
				file = FileFactory.newFile( file, getName() );
			}
      if (file instanceof LocalFile) {
        super.copyTo(file);
      }
      else if (file instanceof HTTPFile) {
//TODO
      }
      else {
        super.copyTo( file );
      }
		}    
  }
  */

	/**
	 * Copies this file to another file. This object is the source file.
	 * The destination file is given as the argument.
	 * If the destination file, does not exist a new one will be created.
	 * Otherwise the source file will be appended to the destination file.
	 * Directories will be copied recursively.
	 *
	 * @param file	The file to receive the data.
	 * @throws  NullPointerException If file is null.
	 * @throws IOException If an IOException occurs.
	 */
/*	public void copyFrom( GeneralFile file, boolean forceOverwrite )
		throws IOException
	{
		if (file == null) {
			throw new NullPointerException();
		}

		if (file.isDirectory()) {
			//recursive copy
			GeneralFile[] fileList = file.listFiles();

			mkdir();
			if (fileList != null) {
				for (int i=0;i<fileList.length;i++) {
					FileFactory.newFile( this, fileList[i].getName() ).copyFrom(
						fileList[i], forceOverwrite );
				}
			}
		}
		else {
			if (isDirectory()) {
				//change the destination from a directory to a file
				GeneralFile subFile = FileFactory.newFile( this, file.getName() );
				subFile.copyFrom( file );
				return;
			}
      if (file instanceof LocalFile) {
//TODO
      }
      else if (file instanceof HTTPFile) {
//TODO
      }
      else {
        super.copyFrom( file );
      }
		}
	}
*/
	/**
	 * @return resource the physical resource where this RemoteFile is stored.
	 * 		Returns null if this abstract pathname is a directory or does not exist.
	 *
	 * @throws IOException If an IOException occurs during the system query.
	 */
	public String getResource( )
		throws IOException
  {
		throw new UnsupportedOperationException();    
  }
  
  
	/**
	 * Replicates this RemoteFile to a new resource. 
	 * Directories/collections will be recursively replicated.
	 *<P>
	 * In some remote systems, one can make copies of a data set and store the 
   * copies in different locations. But, all these copies are considered to be
	 * identifiable by the same identifier. That is, each copy is considered to
	 * be equivalent to each other.
	 *<P>
	 * When a user reads a replicated data set, the remote system cycles through 
   * all the copies of the datset and reads the one that is accessible at that
   * time.
	 *
	 * @param	newResource The storage resource name of the new copy.
	 * @throws IOException If an IOException occurs.
	 */
	public void replicate( String newResource )
		throws IOException
  {
		throw new UnsupportedOperationException();    
  }
  
//----------------------------------------------------------------------
// java.io.File Methods
//----------------------------------------------------------------------	
  /**
   * Tests this abstract pathname for equality with the given object. 
   * Returns true if and only if the argument is not null and is an 
   * abstract pathname that denotes the same file or directory as this 
   * abstract pathname on the same filesystem. Whether or not two 
   * abstract pathnames are equal depends upon the underlying system, 
   * but does not compare other user information of the filesystems. 
   * Alphabetic case may or may not be significant depending on the filesystem.
   *
   * @param   obj   The object to be compared with this abstract pathname
   *
   * @return  <code>true</code> if and only if the objects are the same;
   *          <code>false</code> otherwise
   */
  public boolean equals( Object obj )
  {
    try {
      if (obj == null)
        return false; 

      if (obj instanceof HTTPFile) {
        HTTPFile temp = (HTTPFile) obj;

        if (temp.httpFileSystem.getHost().equals(httpFileSystem.getHost())) {
          if (temp.httpFileSystem.getPort() == httpFileSystem.getPort())
          {
            return getAbsolutePath().equals(temp.getAbsolutePath());
          }
        }
      }
    } catch (ClassCastException e) {
      //TODO
    }
    return false;
  }

    
  
  /**
	 * Tests whether the application can read the file denoted by
	 * this abstract pathname.
	 *
	 * @return  <code>true</code> if and only if the file specified by this
	 * 	abstract pathname exists <em>and</em> can be read; otherwise
	 *  <code>false</code>.
	 */
	public boolean canRead()
	{
		//TODO
    return true;
	}


	/**
	 * Tests whether the application can modify to the file denoted by
	 * this abstract pathname.
	 *
	 * @return  <code>true</code> if and only if the file system actually
	 * 	contains a file denoted by this abstract pathname <em>and</em>
	 * 	the application is allowed to write to the file; otherwise
	 * <code>false</code>.
	 */
	public boolean canWrite()
	{
    //TODO
    return false;
	}
	/**
	 * Atomically creates a new, empty file named by this abstract
	 * pathname if and only if a file with this name does not yet exist.
	 */
	public boolean createNewFile() throws IOException
	{
		//TODO possible?
    return false;
	}

	/**
	 * Cannot delete using HTTP protocol.
	 */
	public boolean delete()
	{
    //TODO httpURLConnection.setRequestMethod(DELETE)
    return false;
	}
  

	/**
	 * @return This abstract pathname as a pathname string.
	 */
	public String getPath()
	{
		return getAbsolutePath();
	}
  

	/**
	 * Tests whether the file denoted by this abstract pathname exists.
	 *
	 * @return  <code>true</code> if and only if the file denoted by this
	 * 	abstract pathname exists; <code>false</code> otherwise
	 */
	public boolean exists()
	{
//TODO
    return true;
	}


	/**
	 * Tests whether this abstract pathname is absolute in a system-dependant way.
	 *
	 * @return  <code>true</code> if this abstract pathname is absolute,
	 *          <code>false</code> otherwise
	 */
	public boolean isAbsolute()
	{
		return true;
	}


	/**
	 * Tests whether the file denoted by this abstract pathname is a directory.
	 *
	 * @return <code>true</code> if and only if the file denoted by this
	 *          abstract pathname exists <em>and</em> is a directory;
	 *          <code>false</code> otherwise
	 */
	public boolean isDirectory()
	{
		//TODO...
    return false;
	}


	/**
	 * Tests whether the file denoted by this abstract pathname is a normal file.
	 *
	 * @return  <code>true</code> if and only if the file denoted by this
	 *          abstract pathname exists <em>and</em> is a normal file;
	 *          <code>false</code> otherwise
	 */
	public boolean isFile()
	{
		//TODO...
    return true;
	}


	/**
	 * Tests whether the file named by this abstract pathname is a hidden file.
	 *
	 * @return  <code>true</code> if and only if the file denoted by this
	 *          abstract pathname is hidden.
	 */
	public boolean isHidden()
	{
		//TODO...
    return false;
	}


  /**
   * Returns the time that the file denoted by this abstract pathname
   * was last modified.
   *
   * @return  A <code>long</code> value representing the time the file was
   *          last modified, measured in system-dependent way.
   */
  public long lastModified()
  {
    return httpFileSystem.conn.getLastModified();
  }


  /**
   * Returns the length of the file denoted by this abstract pathname.
   *
   * @return  The length, in bytes, of the file denoted by this abstract
   *          pathname, or <code>0L</code> if the file does not exist
   */
  public long length()
  {
    long length = httpFileSystem.conn.getContentLength();
    if (length < 0)
      return 0;

    return length;
  }

	/**
	 * Returns an array of strings naming the files and directories in
	 * the directory denoted by this abstract pathname.
	 *<P>
	 * There is no guarantee that the name strings in the resulting array
	 * will appear in any specific order; they are not, in particular,
	 * guaranteed to appear in alphabetical order.
	 *<P>
	 * If this GeneralFile object denotes a file, the results are unspecified.
	 *
	 * @return  An array of strings naming the files and directories in the
	 *          directory denoted by this abstract pathname.
	 */
	public String[] list()
	{
/*TODO there is a way to do it sometimes at least
//    try {
      Vector list = null;//TODO can depending on server
      Object[] listO = list.toArray();
      String[] listS = new String[listO.length];
      for (int i=0;i<listO.length;i++) {
        listS[i] = listO[i].toString();
      }
      return listS;
//    } catch( IOException e ) {
//      return null;
//    }*/return null;
	}
  
  
	/**
	 * Creates the directory named by this abstract pathname.
	 */
	public boolean mkdir()
	{
//TODO probably can't
    return false;
	}
  
  
	/**
	 * Renames the file denoted by this abstract pathname.
	 *<P>
	 * Whether or not this method can move a file from one filesystem to
	 * another is platform-dependent. The return value should always be
	 * checked to make sure that the rename operation was successful.
	 *
	 * @param  dest  The new abstract pathname for the named file
	 *
	 * @throws  IllegalArgumentException
	 *          If parameter <code>dest</code> is not a <code>GeneralFile</code>.
	 * @throws NullPointerException - If dest is null
	 */
	public boolean renameTo(GeneralFile dest)
		throws IllegalArgumentException, NullPointerException
	{
    try {
      if (dest instanceof HTTPFile) {
        /*TODO probably can't
        if (httpFileSystem.conn.equals(((HTTPFile)dest).httpFileSystem.conn)) {

        }
        else {
          //TODO some http to http copy...

          if (!dest.exists()) {
            copyTo( dest );
            delete();
          }
          else
            return false;
        }
        */ return false;
      }
      else {
        if (!dest.exists()) {
          copyTo( dest );
          delete();
        }
        else
          return false;      
      }
    } catch( IOException e ) {
      return false;
    }
    
//    return true;
//TODO can't really
return false;
	}
   
  
	/**
	 *	Returns the pathname string of this abstract pathname.
	 */
	public String toString()
	{
    String username = httpFileSystem.getUserName(), portString;
    int port = httpFileSystem.getPort();
    
    if (username != null)
      username += "@";
    else
      username = "";
    if (port > 0 && port != 80)
      portString = ":" + port;
    else
      portString = ""; //looks better without the port 80
    
    return "http://"+username+httpFileSystem.getHost()+portString+
      getAbsolutePath();
	}
  
  
  
	/**
	 * Constructs a <tt>file:</tt> URI that represents this abstract pathname.
	 *
	 * <p> The exact form of the URI is system-dependent.  If it can be
	 * determined that the file denoted by this abstract pathname is a
	 * directory, then the resulting URI will end with a slash.
	 *
	 * <p> For a given abstract pathname <i>f</i>, it is guaranteed that
	 *
	 * <blockquote><tt>
	 * new {@link #GeneralFile(java.net.URI) GeneralFile}
	 * (</tt><i>&nbsp;f</i><tt>.toURI()).equals(</tt><i>&nbsp;f</i><tt>)
	 * </tt></blockquote>
	 *
	 * so long as the original abstract pathname, the URI, and the new abstract
	 * pathname are all created in (possibly different invocations of) the same
	 * Java virtual machine.  However, this relationship typically does not hold
	 * when a <tt>file:</tt> URI that is created in a virtual machine on one
	 * operating system is converted into an abstract pathname in a virtual
	 * machine on a different operating system.
	 *
	 * @return  An absolute, hierarchical URI with a scheme equal to
	 *          <tt>"file"</tt>, a path representing this abstract pathname,
	 *          and undefined authority, query, and fragment components
	 *
	 * @see #GeneralFile(java.net.URI)
	 * @see java.net.URI
	 * @see java.net.URI#toURL()
	 */
	public URI toURI()
	{
    try {
  		return new URI( toString() );
    } catch (URISyntaxException e) {      
      if (DEBUG > 0)
        e.printStackTrace();
    }
    return null;
	}


	/**
	 * Converts this abstract pathname into a <code>file:</code> URL.  The
	 * exact form of the URL is system-dependent.  If it can be
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
		return new URL( toString() );
	}
}

