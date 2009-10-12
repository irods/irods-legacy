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
import edu.sdsc.grid.io.local.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;


/**
 * Ideas I thought might be useful.
 *
 * @author iktome
 */
public class Utils
{


  /**
   * Creates a new directory of symbolic links.
   * The conditions given are used for a query, the results define which links
   * get created. Thus the new directory will [appear to] contain all files
   * on the filesystem that match the given query.
   * Clearly this will only work on fileSystem which support querying
   * and symbolic links. (Currently only works with SRB actually)
   */
  public static GeneralFile newFile( GeneralFileSystem fileSystem,
    String filePath, MetaDataCondition[] conditions )
		throws NullPointerException, IOException
  {
    GeneralFile dir = FileFactory.newFile( fileSystem, filePath );
    dir.mkdir();

    MetaDataSelect[] selects = MetaDataSet.newSelection(
      new String[]{SRBMetaDataSet.FILE_NAME, SRBMetaDataSet.DIRECTORY_NAME});
    MetaDataRecordList[] rl = fileSystem.query(conditions, selects);

    if (rl == null) {
      return dir;
    }
    for (int i=0;i<rl.length;i++) {
      //oldfile.link(newfilepath_i)
      //avoids filename collisions by appending the increment
      ((SRBFile) FileFactory.newFile(fileSystem,
          rl[i].getStringValue(1), rl[i].getStringValue(0))).link(
        ((SRBFile) FileFactory.newFile(dir, rl[i].getStringValue(0)+"_"+i)));
    }

    return dir;
  }
}
