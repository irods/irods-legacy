DESCRIPTION
	This directory contains the source code and documentation for
	the SDSC JARGON Project.

URLS
	https://www.irods.org/index.php/Jargon - Project home page


AUTHOR
	Principal Author,
	Lucas Ammon Gilbert, San Diego Supercomputer Center

	Designers,
		Lucas Ammon Gilbert, San Diego Supercomputer Center
	version 1.0
		Lucas Ammon Gilbert, San Diego Supercomputer Center
		David R. Nadeau, San Diego Supercomputer Center
		John Moreland, San Diego Supercomputer Center


CONTACT
	Mike Conway, DICE Center, UNC Chapel Hill
	michael_conway@unc.edu


CONTENT
	build.xml	                 -  ANT build, Makes the API and test applications
	build.properties             -  Customization properties for build behavior
	README.txt	                 -  This file
	RELEASE_NOTES.txt	         -  Text copy of release notes
	doc/		                 -  Documentation
	lib/		                 -  Compile, test, and run libraries
	src/		                 -  Source code
	target/                      -  Build-generated directory for output of build tasks
	   target/dist               -  Distribution directory for built Jargon .jar files


To build Jargon:

1) Download JVM from SUN (other versions of java do not seem to work).
2) cd jargon; ant build

To test jargon

1) set up the ~/.irods/.irodsEnv file (same as the icommand) and put the
plain text password in the ~/.irods/.irodsA file (scrambled password does
not work).
2)run the run_irods or run_irods_fulltest (for the full test) ANT tasks


LICENSE - BSD
Copyright (c) 2009, Regents of the University of California All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    * Neither the name of the University of California, San Diego (UCSD) nor
      the names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
