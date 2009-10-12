/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.sdsc.grid.io.irods;


import java.io.IOException;

/**
 *
 * @author iktome
 */
public class Domain 
{
  protected final String name, typeName, tableName;
//  protected List<Attribute> attributes;
          
/*  //So...this is probably dumb, I should go to bed.
  protected final Attribute.Type TYPE = 
          new Attribute(this, "type").new Type(this, "type");
  //
  protected int attributeNum = 1;
  */
  
  Domain( String name, String typeName, String tableName ) 
  { 
    this.name = name; 
    this.typeName = typeName;
    this.tableName = tableName;
  }
    
  String getName( ) { return name; }
  String getTypeName( ) { return typeName; }
  String getTableName( ) { return tableName; }
  
  public String toString() { return name; }   
  
  
  //------------------------------------------------------------------------   
  /**
   * Queries the fileSystem to aqcuire all the types for this domain.
   * @return
   */
  public String[] listTypes( ) 
    throws IOException
  {
    return IRODSAdmin.fileSystem.commands.simpleQuery(       
      "select token_name from r_tokn_main where token_namespace = ?",
      typeName ); 
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
      "select "+name+" from "+tableName, null ); 
  }
  
  
  //------------------------------------------------------------------------ 
  public void addType( String newType )
    throws IOException
  {
    addType( newType, "null", "null", "null");
  }
  
  // at tokenNamespace Name Value [Value2] [Value3] (add token)
  //TODO don't know what the 3 values are
  public void addType( String newType,
    String value, String value2, String value3 )
    throws IOException
  {
    String[] args = { "add", "token", 
      getTypeName(), newType, value, value2, value3 };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  // rt tokenNamespace Name Value (remove token)
  public void deleteType( String newType )
    throws IOException
  {
    String[] args = { "rm", "token", getTypeName(), newType };
    IRODSAdmin.fileSystem.commands.admin( args );
  }
  
  public void modifyType( String subjectName, String newType )
    throws IOException
  {
    String[] args = { 
      "modify", name, subjectName, "type", newType };
    IRODSAdmin.fileSystem.commands.admin( args );
  }  
}