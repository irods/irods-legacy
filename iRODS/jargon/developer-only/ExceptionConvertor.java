//
//	Copyright (c) 2003  San Diego Supercomputer Center (SDSC),
//	University of California, San Diego (UCSD), San Diego, CA, USA.
//
//	Users and possessors of this source code are hereby granted a
//	nonexclusive, royalty-free copyright and design patent license
//	to use this code in individual software.  License is not granted
//	for commercial resale, in whole or in part, without prior written
//	permission from SDSC/UCSD.  This source is provided "AS IS"
//	without express or implied warranty of any kind.
//
//
//  FILE
//	ExceptionConvertor.java	-
//
//  CLASS HIERARCHY
//	java.lang.Object
//	    |
//	    +-.ExceptionCovertor
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//

// default package (no package)

/**
	* <p>Title: ExceptionConvertor </p>
	* <p>Description: This class converts the SRB Exceptions from the C code
	* to the java code. </p>
	* <p>Copyright: Copyright (c) 2003</p>
	* <p>Company: SDSC </p>
	* @author Arun swaran Jagatheesan (arun@sdsc.edu)
	* @version 1.0
	*/

import java.io.*;
import java.util.*;

public class ExceptionConvertor
{
	public static String readFile(String fileName)
	{
		String fileString = null; // = (String)fileNameToString.get(fileName);
		//System.err.println("The file size is ..."+fileString.length());
		if (fileString == null){
			BufferedInputStream br = null;
			try{
				File file = new File(fileName);
				int length = (int) file.length();
				br = new BufferedInputStream(new
						FileInputStream(file));
				byte b[] = new byte[length];
				br.read(b);
				String tempfileString = new String(b);
				//fileNameToString.put(fileName,tempfileString);
				return tempfileString;
			} catch (Exception e){
				e.printStackTrace();
				return null;
			} finally{
				try{
							if (br != null)
										br.close();
				} catch (Exception e){
				}
			}
		}
		else
			return fileString;
	}

	public static void convert(String cFileName, String javaFileName,
		String outFileName)
		throws Exception
	{
		//if (cFileName.endsWith("srb_error.h")) throw new Exception("Dude, where is the c file?");
		String cFileContent = readFile(cFileName);
		String javaFileContent = readFile(javaFileName);
		StringBuffer newFile = new StringBuffer();

		// copy original data above
		int startPtr = javaFileContent.indexOf("MAGIC_LINE");
		newFile.append(javaFileContent.substring(0, startPtr));
		newFile.append("MAGIC_LINE (DONT CHANGE THIS LINE - it has to come above the following static declarations)");
		//	newFile.append(javaFileContent.substring(startPtr,
		//			javaFileContent.indexOf("\n\r\f", startPtr)-1));

		// copy next two lines
		newFile.append("\n/* ERROR TABLE DECLARATIONS */ \n");

		//get other lines from c code
		int c_start = cFileContent.indexOf("srb_errtbl srb_errent[]") + "srb_errtbl srb_errent[]".length();
		if (c_start < 0 ) throw new Exception (" C File is not valid");

		StringTokenizer str = new StringTokenizer( cFileContent.substring(c_start),"\n\r\f");
		if (str.hasMoreTokens()) str.nextToken(); //igonre current line
		boolean continueMore = true; //some like continue
		while (str.hasMoreTokens() && continueMore ){
			String currStr = str.nextToken().trim();
			if (currStr.equals("")){
				newFile.append("\n");
			}else if (currStr.equals("{")){
				//
			}else if (currStr.equals("};")) {
				//newFile.append("}\n");
				continueMore = false;
			}else{
				newFile.append( convertLine(currStr));
				newFile.append("\n");
			}
		}//while

		//end of file
		//for now assume the end of file is the same content
		int endPtr = javaFileContent.indexOf("MAGIC_LINE", startPtr + "MAGIC_LINE".length());
		newFile.append( "//" +	javaFileContent.substring( endPtr));

		//write the file newFile to disk
		File file = new File(outFileName);
		FileWriter fw = new FileWriter(file);
		fw.write(newFile.toString());
		fw.close();
	}

	private static String convertLine(String cString)
		throws Exception
	{
		String cToken = "";
		try {
			StringBuffer javaString = new StringBuffer();
			javaString.append("\t\t srbException.setProperty( \"");
			StringTokenizer str2 = new StringTokenizer(cString, ",\"" , true);
			if (str2.hasMoreTokens()){
				cToken = str2.nextToken().trim();
				javaString.append(cToken + "\""); // 0"
			} else throw new Exception("Format Error");
			if (str2.hasMoreTokens() && ((cToken = str2.nextToken().trim()).equals(","))){
				//cToken = str2.nextToken().trim();
				javaString.append(", "); //,_
			} else throw new Exception("Format Error");
			boolean insideStr = false;

			javaString.append("\"");
			while (str2.hasMoreTokens()){
				cToken = str2.nextToken().trim();
				if (cToken.equals("\"")){
					insideStr = !insideStr;
					if (!insideStr) javaString.append(" ");
				}else if(cToken.equals(",")){
					if (insideStr)			javaString.append(cToken);
				}else{
					javaString.append(cToken);
				}
			} //while
			javaString.append("\" );");
			return javaString.toString();
		} catch(Exception e) {
			System.err.println("Problem at "+cString);
			System.err.println("Problem at "+cToken);
			throw e;
		}
	}

	public static void main(String a[])
	{
		try{
			//System.out.println( convertLine("-1500, \"HPSS_SRB_TIMEOUT_OPEN1\", \"HPSS timeout error. Cannot finish HPSS hpss_Open() operation in the alloted time\", \"HPSS timeout error. Cannot finish HPSS hpss_Open() operation in alloted time, called in paraHPSStoFAPCopy(), (parallel HPSS code)\", \"\","));
			if (a.length == 3){
				convert(a[0], a[1], a[2]);
			} else if (a.length == 1){
				convert("../../src/include/srb_error.h", "../src/api/edu/sdsc/grid/io/srb/SRBException.java", a[0]);
			}
		} catch (Exception e){
					e.printStackTrace();
		}
	}
}//class
