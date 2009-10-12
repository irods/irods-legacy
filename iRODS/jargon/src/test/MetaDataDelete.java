//	Copyright (c) 2005, Regents of the University of California
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

// Created on June 3, 2005, 4:11 PM


import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;

import java.io.*;

/**
 * There isn't currently a simple way to delete one row of the 
 * user definable metadata. The most current Jargon API can delete a 
 * file's entire set of such metadata like so,
 *<br><br><code>
 * MetaDataRecordList rl = new SRBMetaDataRecordList(
 *  SRBMetaDataSet.getField(
 *   SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES ),
 *   (MetaDataTable)null );
 *<br><br> 
 * new SRBFile( srbFileSystem, "myDirectory" ).modifyMetaData( rl );
 *<br><br>
 * This class provides a static utility which can delete a single row.
 *
 * @author Lucas Gilbert
 */
public class MetaDataDelete {
  
  /** Creates a new instance of MetaDataDelete */
  public MetaDataDelete() {
  }
  
  
  /**
   * 
   */
  public static void deleteRow( SRBFile file, String attribute )
    throws IOException
  {
    MetaDataTable metaDataTable = null;
    MetaDataRecordList[] rl = null;
    String fieldName = null;
    MetaDataField field = null;
    if (file.isDirectory()) {
        fieldName = SRBMetaDataSet.DEFINABLE_METADATA_FOR_DIRECTORIES;
    }
    else {
        fieldName = SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES;      
    }
    field = SRBMetaDataSet.getField( fieldName );
    
    rl = file.query( fieldName );
    metaDataTable = (MetaDataTable)rl[0].getValue(field);
    
    rl[0] = new SRBMetaDataRecordList( field, (MetaDataTable)null );       
    file.modifyMetaData( rl[0] );
    
    for (int i=0;i<metaDataTable.getRowCount();i++)
    {
      if (metaDataTable.getStringValue(i,0).equals(attribute))
      {
        metaDataTable.removeRow(i);
      }
    }
     
    rl[0] = new SRBMetaDataRecordList( field, metaDataTable );       
    file.modifyMetaData( rl[0] );
  }
}
