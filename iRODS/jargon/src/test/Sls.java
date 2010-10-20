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

import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.*;
import java.net.*;

public class Sls
{

//----------------------------------------------------------------------
// Main
//----------------------------------------------------------------------
	/**
	 * Testing
	 */
	public static void main(String args[])
	{
		SRBFileSystem srbFileSystem = null;

		GeneralFile file = null;

		String[] list = null;
    
		try{
			if ((args.length == 2) && (args[0].equals("-c"))) {
				file = new SRBContainer( new SRBFileSystem( ), args[1] );
			}
			else if ((args.length == 2) && (args[0].equals("-uri"))) {
				file = FileFactory.newFile( new URI( args[1] ) );
			}
			else if (args.length == 1) {
				file = new SRBFile( new SRBFileSystem( ), args[0] );
			}
			else if (args.length == 0) {
				srbFileSystem = new SRBFileSystem();

				file =
					new SRBFile( srbFileSystem, srbFileSystem.getHomeDirectory() );
			}
			else {
				throw new IllegalArgumentException(
					"\nUsage: Sls fileName/directory\nUsage: Sls -c containerName"+
					"\nUsage: Sls -uri uri\nUsage: Sls [-r] dirName");
			}

			list = file.list();

			if (list != null) {
				for (int i=0;i<list.length;i++) {
					System.out.println(list[i]);
				}
			}
		}
		catch ( Throwable e ) {
			System.out.println( "\nJava Error Message: "+ e.toString() );
			e.printStackTrace();
		}

		System.exit(0);
	}


/* Arun's way
	public static void main(String args[])
	{
		SRBFileSystem srbFileSystem = null;
		SRBFile srbFile = null;
		String[] list = null;
		try {
			// command line processing setup
			Options options = new Options();
			options.addOption("C", "container", true, "Container to list");
			options.addOption("c", "collection", true, "Collection to list");
			options.addOption("r", "recursive", false, "recursive listing");
			options.addOption("h", "help", false, "this help message");
			// command line parsing
			CommandLineParser parser = new PosixParser();
			CommandLine cmd = parser.parse(options, args);
			// command line process
			if (cmd.hasOption('r')) {
					srbFileSystem = new SRBFileSystem();
					if (cmd.hasOption('c') ||
							(! (cmd.hasOption('c') || cmd.hasOption('C')))) {
							if (cmd.hasOption('c')) {
									srbFile = new SRBFile(srbFileSystem,
																				cmd.getOptionValue("c"));
							} else {
									srbFile =
										new SRBFile(srbFileSystem,
																srbFileSystem.getHomeDirectory());
							}
					} else if (cmd.hasOption('C')) {
							srbFile = new SRBContainer(srbFileSystem,
								cmd.getOptionValue("C"));
								System.err.println("Recursive container listing not supported yet");
					} else { //must not come here
							printHelp(options);
					}
					MetaDataCondition conditions[] = {
								MetaDataSet.newCondition(
								SRBMetaDataSet.DIRECTORY_NAME,
								MetaDataCondition.LIKE,
								srbFile.getAbsolutePath() + "%"),
							};
							MetaDataSelect selects[] = {MetaDataSet.newSelection(
								SRBMetaDataSet.FILE_NAME),
								MetaDataSet.newSelection(SRBMetaDataSet.DIRECTORY_NAME)
							};
					MetaDataRecordList[] rl = srbFileSystem.query(conditions, selects);
					rl = MetaDataRecordList.getAllResults(rl);
					if (rl != null) { //Nothing in the database matched the query
							for (int i = 0; i < rl.length; i++) {
									System.out.print("\n" + rl[i].getValue(1) +
																		SRBFile.PATH_SEPARATOR +
																		rl[i].getValue(0));
							}
					}
			} else { //no recursive ness
					if (! (cmd.hasOption('c') || cmd.hasOption('C'))) { //no cmdline options
							if (args.length == 1) {
									srbFileSystem = new SRBFileSystem();
									srbFile = new SRBFile(srbFileSystem, args[0]);
									list = srbFile.list();
									printList(list);
							} else if (args.length == 0) {
									srbFileSystem = new SRBFileSystem();
									srbFile =
										new SRBFile(srbFileSystem,
																srbFileSystem.getHomeDirectory());
									list = srbFile.list();
									printList(list);
							}
					} else { //use only cmd line options
							if (cmd.hasOption('C')) {
									srbFileSystem = new SRBFileSystem();
									srbFile = new SRBContainer(srbFileSystem,
										cmd.getOptionValue("C"));
									list = srbFile.list();
									printList(list);
							}
							if (cmd.hasOption('c')) {
									srbFileSystem = new SRBFileSystem();
									srbFile = new SRBFile(srbFileSystem,
																				cmd.getOptionValue("c"));
									list = srbFile.list();
									printList(list);
							}
					}
			}
			if (cmd.hasOption('h')) {
					printHelp(options);
			}
		} catch (SRBException e) {
			//Get the standard error string
			String bar = e.getStandardMessage();
			System.out.println("Standardized SRB Server Error Message: " + bar);
			//The original error message is still available through
			System.out.println("\nGridTools Error Message: " + e.getMessage());
			e.printStackTrace();
		} catch (Throwable e) {
			e.printStackTrace();
		}
		System.exit(0);
	}

	public static void printList(String header, String[] list)
	{
		System.out.println(header);
		printList(list);
	}

	public static void printList(String[] list)
	{
		if (list != null) {
			for (int i = 0; i < list.length; i++) {
					System.out.println(list[i]);
			}
		}
	}

	public static void printHelp(Options opts)
	{
		HelpFormatter formatter = new HelpFormatter();
		formatter.printHelp("Sls [Ccrh] <collectionName|containerName>", opts);
	}
*/
}
