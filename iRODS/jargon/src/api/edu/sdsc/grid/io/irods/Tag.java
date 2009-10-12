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
//  FILE
//  Rule.java  -  edu.sdsc.grid.io.irods.Rule
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.irods.Rule
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

/**
 * Was a shortcut just to get started, now it is permanent. Eh, it happens.
 * Might prefer it this way anyhow.
 */
class Tag implements Cloneable
{
  static final char OPEN_START_TAG = '<';
  static final char CLOSE_START_TAG = '>';
  static final String OPEN_END_TAG = "</";
  static final char CLOSE_END_TAG = '>';
  
  /**
   * iRODS name of the tag
   */
  String tagName;

  /**
   * all the sub tags
   */
  Tag[] tags;

  /**
   * probably a string...
   */
  String value;


  Tag( String tagName )
  {
    this.tagName = tagName;
  }

  Tag( String tagName, int value )
  {
      this.tagName = tagName;
      this.value = ""+value;
  }

  Tag( String tagName, long value )
  {
      this.tagName = tagName;
      this.value = ""+value;
  }

  Tag( String tagName, String value )
  {
      this.tagName = tagName;
      this.value = value;
  }

  Tag( String tagName, Tag tag )
  {
      this(tagName, new Tag[]{tag});
  }
  Tag( String tagName, Tag[] tags )
  {
    this.tagName = tagName;
    this.tags = tags;
  }
  
  void setTagName( String tagName )
  {
    this.tagName = tagName;
  }

  void setValue( int value )
  {
    this.value = ""+value;
  }

  void setValue( long value )
  {
    this.value = ""+value;
  }

  void setValue( String value, boolean decode )
  {
    if (value == null) {
      this.value = null;
      return;
    }
    if (decode) {
      //decode escaped characters
      value = value.replaceAll("&amp;", "&");
      value = value.replaceAll("&lt;", "<");
      value = value.replaceAll("&gt;", ">");
      value = value.replaceAll("&quot;", "\"");
      value = value.replaceAll("&apos;", "`");
    }
    this.value = value;
  }


/*
  void setTagValues( Object[] values )
  {
    if (values instanceof Tag[]) {
      tags = (Tag[]) values;
    }
    else if (tags.length != values.length) {
      throw new IllegalArgumentException("Schema mismatch "+this+
        " does not have these "+values+"   "+values.length+" values.");
    }
    else {
      for (int i=0;i<tags.length;i++) {
        if (values[i] instanceof String) {
          //just set this leaf value
          tags[i].setValue((String)values[i]);
        }
        else if (values[i] instanceof Tag) {
          tags[i] = (Tag) values[i];
        }
        else if (values[i] instanceof Object[]) {
          tags[i].setTagValues((Object[])values[i]);
        }
        else {
          throw new IllegalArgumentException("Protocol error: This "+
            values[i]+" shouldn't be here");
        }
      }
    }
  }
*/

  Object getValue( )
  {
//TODO clone so it can't over write when set value is called?
    if (tags != null)
      return tags.clone();
    else
      return value;
  }

  int getIntValue( )
  {
    return Integer.parseInt(value);
  }

  String getStringValue( )
  {
    return value;
  }

  String getName( )
  {
    return tagName;
  }

  int getLength( ) {
    return tags.length;
  }

  Tag getTag( String tagName )
  {
    if (tags == null) return null;

    //see if tagName exists in first level
    //if it isn't the toplevel, just leave it.
    for (int i=0;i<tags.length;i++) {
      if (tags[i].getName().equals(tagName))
        return tags[i];
    }
    return null;
  }
  
  /**
   * Get the <code>index</code>-th sub-tag, from the first level down, 
   * with the name of <code>tagName</code>. Index count starts at zero.
   *
   * So if tagname = taggy, and index = 2, 
   * get the 3rd subtag with the name of 'taggy'.
   */
  Tag getTag( String tagName, int index )
  {
    if (tags == null) return null;

    //see if tagName exists in first level
    //if it isn't the toplevel, just leave it.
    for (int i=0,j=0;i<tags.length;i++) {
      if (tags[i].getName().equals(tagName)) {
        if (index == j) {
          return tags[i];
        }
        else {
          j++;
        }
      }
    }
    return null;
  }

  Tag[] getTags( )
  {
    //clone so it can't over write when set value is called?
    if (tags != null)
      return tags;
    else
      return null;
  }

  /**
   * Returns the values of this tags subtags. Which are probably more
   * tags unless we've finally reached a leaf.
   */
  Object[] getTagValues()
  {
      if (tags == null) return null;

      Object[] val = new Object[tags.length];
      for (int i=0;i<tags.length;i++) {
          val[i] = tags[i].getValue();
      }
      return val;
  }

  /**
   * Convenience for addTag( new Tag(name, val) ) 
   */
  void addTag( String name, String val )
  {
    addTag( new Tag( name, val ) );
  }

  void addTag( Tag add )
  {
      if (tags != null) {
          Tag[] temp = tags;
          tags = new Tag[temp.length+1];
          System.arraycopy(temp, 0, tags, 0, temp.length);
          tags[temp.length] = add;
      }
      else {
          tags = new Tag[]{add};
      }
  }

  void addTags( Tag[] add )
  {
      if (tags != null) {
          Tag[] temp = tags;
          tags = new Tag[temp.length+add.length];
          System.arraycopy(temp, 0, tags, 0, temp.length);
          System.arraycopy(add, 0, tags, temp.length, add.length);
      }
      else {
          tags = add;
      }
  }

  public Object clone()
    throws CloneNotSupportedException
  {
    return super.clone();
  }

  public boolean equals( Object obj )
  {
    if (obj instanceof Tag) {
      Tag newTag = (Tag) obj;
      if (newTag.getName().equals(tagName)) {
        if (newTag.getValue().equals(value)) {
          if (newTag.getTags().equals(tags)) {
            return true;
          }
        }
      }
    }
    return false;
  }

  public String toString()
  {
    return tagName;
  }

  /**
   * Outputs a string to send communications (function calls) to the
   * iRODS server. All values are strings
   */
  String parseTag( )
  {
//TODO though if something isn't a string and you try to send a
//non-printable character this way, it will get all messed up.
//so...not sure if should be converted to Base64
    StringBuffer parsed =
      new StringBuffer(OPEN_START_TAG + tagName + CLOSE_START_TAG);
    if (tags != null) {
      for (int i=0;i<tags.length;i++) {
        parsed.append(tags[i].parseTag());
      }
    }
    else {
      parsed.append(escapeChars(value));
    }
    parsed.append(OPEN_END_TAG + tagName + CLOSE_END_TAG + "\n");

    return parsed.toString();
  }

  String escapeChars( String out )
  {
    if (out == null) return null;
    out = out.replaceAll("&", "&amp;");
    out = out.replaceAll( "<", "&lt;" );
    out = out.replaceAll( ">", "&gt;" );
    out = out.replaceAll( "\"", "&quot;" );
    return out.replaceAll( "`", "&apos;" );
  }
/*
  boolean isLeaf( )
  {
    if (tags != null)
      return false;
    else
      return true;
  }
*/
}
