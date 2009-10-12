//
//  Copyright (c) 2003  San Diego Supercomputer Center (SDSC),
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
//  FILE
//  SRBFileInputStream.java  -  edu.sdsc.grid.io.SRBFileInputStream
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.GeneralRandomAccessFile
//          |
//          +-.RemoteRandomAccessFile
//              |
//              +-.srb.SRBRandomAccessFile
//                    |
//                    +.SRBShadowFile
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.srb;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.*;

import java.io.IOException;
import java.io.FileNotFoundException;

import java.io.*;
import java.util.Vector;


/**
 * If the SRBShadowFile refers to a file, this class can obtain
 * input bytes from a shadow file in a SRB file system. This SRBShadowFile
 * can also represent a shadow directory.
 *<P>
 * @author  Lucas Gilbert
 * @since   JARGON1.4
 */
public class SRBShadowFile extends SRBRandomAccessFile
{
//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------



//----------------------------------------------------------------------
//  Fields
//----------------------------------------------------------------------
  /**
   * This is a SRBFile object instead of a string just to use some of the
   * path manipulation methods. Obviously it does not point to a real
   * file, (that is the point of <code>this</code> SRBShadowFile object.)
   */
  private String shadowPath;

  //0 = unknown, 1 = file, 2 = directory
  private int fileType = 0;



//----------------------------------------------------------------------
//  Constructors and Destructors
//----------------------------------------------------------------------
  /**
   * Creates a random access file stream to read from,
   * a shadow file object with the specified name. A new
   * file descriptor is obtained from the SRB which represents the
   * connection to the file.
   *<P>
   * On construction a check is made to see if read access to the file
   * is allowed.
   *
   * @param file the SRB abstract filepath
   * @param shadowPath the shadow path
   * @throws IOException If an I/O error occurs
   * @throws FileNotFoundException If the file exists but is a regular file
   *                   rather than a shadow object, or cannot be opened
   *                   for any other reason.
   * @throws SecurityException If denied read access to the file.
   */
  public SRBShadowFile( SRBFile file, String shadowPath )
    throws IllegalArgumentException, FileNotFoundException,
      SecurityException, IOException
  {
    super( file, "r" );

    if (shadowPath == null)
      shadowPath = "";

    this.shadowPath = shadowPath;
    open(file);
  }


  /**
   * Creates a random access file stream to read from,
   * a shadow file object with the specified name. A new
   * file descriptor is obtained from the SRB which represents the
   * connection to the file.
   *<P>
   * On construction a check is made to see if read access to the file
   * is allowed.
   *
   * @param parent a parent abstract shadow filepath
   * @param child the child shadow path
   * @throws IOException If an I/O error occurs
   * @throws FileNotFoundException If the file exists but is a regular file
   *                   rather than a shadow object, or cannot be opened
   *                   for any other reason.
   * @throws SecurityException If denied read access to the file.
   */
  public SRBShadowFile( SRBShadowFile parent,  String child )
    throws IllegalArgumentException, FileNotFoundException,
      SecurityException, IOException
  {
    super( parent.getSRBFile(), "r" );

    if (child == null)
      parent.getShadowPath();
    else
      shadowPath = parent.getShadowPath() + SRBFile.separator + child;

    open(parent.getSRBFile());
  }



//----------------------------------------------------------------------
// Setters and Getters
//----------------------------------------------------------------------
  /**
   * Opens the given file for use by this stream.
   *
   * @exception  IOException  if an I/O error occurs.
   */
  protected void open( GeneralFile file )
    throws IOException
  {
    //can't do ths from super because shadowPath isn't set yet.
    if (shadowPath == null) return;

    this.file = file;

    fd = fileSystem.srbObjOpen(
      file.getName()+"&SHADOW="+shadowPath,
      SRBRandomAccessFile.O_RDONLY, file.getParent() );
    fileType = 1;
  }


  public SRBFile getSRBFile( )
  {
    if ( file != null )
      return (SRBFile) file;

    throw new NullPointerException();
  }


  public String getShadowPath( )
  {
    return shadowPath;
  }


//----------------------------------------------------------------------
//  Random access
//----------------------------------------------------------------------
  /**
   * Returns the length of this file.
   *
   * @return     the length of this file, measured in bytes.
   * @throws  IOException  if an I/O error occurs.
   */
  public long length( )
    throws IOException
  {
    throw new UnsupportedOperationException();
//doesn't work SRBFile.exists() and getStat() don't seem to work
//    return FileFactory.newFile(file.getParentFile(),
//      file.getName()+"&SHADOW="+shadowPath).length();
  }


  /**
   * Sets the length of this file.
   *
   * <p> Truncating a file on the SRB Shadow Fileis not supported, so
   * an UnsupportedOperationException will be thrown.
   * (If the present length of the file as returned by the
   * <code>length</code> method is greater than the <code>newLength</code>
   * argument then the file will be truncated. In this case, if the file
   * offset as returned by the <code>getFilePointer</code> method is greater
   * then <code>newLength</code> then after this method returns the offset
   * will be equal to <code>newLength</code>.)
   *
   * <p> If the present length of the file as returned by the
   * <code>length</code> method is smaller than the <code>newLength</code>
   * argument then the file will be extended.  In this case, the contents of
   * the extended portion of the file are not defined.
   *
   * @param newLength The desired length of the file
   * @throws IOException If an I/O error occurs
   * @throws UnsupportedOperationException on truncate
   */
  public void setLength( long newLength )
    throws IOException
  {
    throw new UnsupportedOperationException();
  }



  public String toString( )
  {
    switch (rw) {
      case 0:
        return file.getAbsolutePath()+"/"+shadowPath+" : r";
      case 1:
        return file.getAbsolutePath()+"/"+shadowPath+" : rw";
      case 2:
        return file.getAbsolutePath()+"/"+shadowPath+" : rws";
      case 3:
        return file.getAbsolutePath()+"/"+shadowPath+" : rwd";
    }

    return file.getAbsolutePath()+"/"+shadowPath;
  }


//----------------------------------------------------------------------
//  File Methods
//----------------------------------------------------------------------
  boolean exists( )
  {
//real SRBFile.exists() isn't working for some reason
    MetaDataRecordList[] rl = null;
    int operator = MetaDataCondition.EQUAL;

    //if it is a directory
    if (fileType == 2)
      return true;

    //if it is a file
    if (fileType == 1)
      return true;

    return false;
  }

  boolean isDirectory( )
  {
    if (fileType == 2)
      return true;

    return false;
  }

  boolean isFile( )
  {
    if (fileType == 1)
      return true;

    return false;
  }

  /**
   * Returns an array of strings naming the files and directories in
   * the directory denoted by this abstract pathname.
   *<P>
   * There is no guarantee that the name strings in the resulting
   * array will appear in any specific order; they are not, in particular,
   * guaranteed to appear in alphabetical order.
   *<P>
   * This method will return all the files in the directory. Listing
   * directories with a large number of files may take a very long time.
   *
   * @return  An array of strings naming the files and directories in the
   *          directory denoted by this abstract pathname. The array will be
   *           empty if the directory is empty. Returns null if an I/O error
   *           occurs.
   */
  public String[] list( )
  {
    Vector list = new Vector();
    String temp = "", physicalPath;
    int index = 0;

    try {
      physicalPath = ((SRBFile)file).getServerLocalPath();
      index = physicalPath.indexOf("/?SHADOW");
      if (index >= 0) {
        physicalPath = physicalPath.substring( 0, index );
      }
      InputStream in = fileSystem.executeProxyCommand(
        "ls", physicalPath + SRBFile.separator +
        shadowPath );

      int result = in.read();
      while (result != -1) {
        if ((char)result == '\n') {
          index = temp.lastIndexOf(SRBFile.separator);
          if (index >= 0) {
            temp = temp.substring( index );
          }
          list.add(temp);
          temp = "";
        }
        else {
          temp += (char)result;
        }
        result = in.read();
      }
    } catch ( SRBException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
    }

    return (String[]) list.toArray(new String[0]);
  }

  /**
   * Returns an array of strings naming the files and directories in
   * the directory denoted by this abstract pathname.
   *<P>
   * There is no guarantee that the name strings in the resulting
   * array will appear in any specific order; they are not, in particular,
   * guaranteed to appear in alphabetical order.
   *<P>
   * This method will return all the files in the directory. Listing
   * directories with a large number of files may take a very long time.
   *
   * @return  An array of strings naming the files and directories in the
   *          directory denoted by this abstract pathname. The array will be
   *           empty if the directory is empty. Returns null if an I/O error
   *           occurs.
   */
  public SRBShadowFile[] listFiles( )
  {
    String[] list = list();
    int length = list.length;
    SRBShadowFile[] shadows = new SRBShadowFile[length];

    try {
      for (int i=0;i<length;i++) {
        shadows[i] = new SRBShadowFile( this, list[i] );
      }
    } catch ( IOException e ) {
      if (SRBCommands.DEBUG > 0) e.printStackTrace();
      return null;
    }

    return shadows;
  }
}
