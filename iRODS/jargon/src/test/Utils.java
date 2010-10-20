//	Copyright (c) 2006, Regents of the University of California
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;

import java.io.IOException;


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
      new String[]{StandardMetaData.FILE_NAME, StandardMetaData.DIRECTORY_NAME});
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
