Currently i-commands are built successfully and partially tested.
Two library files need to be built before building s-commands. They are: 
  (1) iRODSLib.lib - iRODS library built from main iRODS code from core, 
  (2) iRODSNtUtilLib.lib - Windows handy utility functions for porting UNIX application to Windows.

The building sequence should be: iRODSLib.lib, iRODSNtUtilLib.lib, i-commands.

To build "iRODSLib.lib", open the .net project file in the "iRODSLib" directory and build the library file in the MS visual studio.
Take the similar procedure for building "iRODSNtUtilLib.lib".

To build i-commands, a "Makfile" was created in the "icmds" directory for batch build of all i-commands. Start a "Visual Studio Command Prompt" from the "Visual Studio Tools", which can be found in Visual Studio group of startup menu. Within the Visual Studio command prompt, change the current working directory to ......\IRODS\nt\icmds and then run the command, nmake.
The icommands, ils.exe, iget.exe, iput.exe, etc, can be found in the "Debug" directory".

Please contact me via email if encountering any build problem.

Bing Zhu, Ph.D.
SRB Team
San Diego Supercomputer Center
9500 Gilman Drive, MC 0505
La Jolla, CA 92093
email: bzhu@sdsc.edu
Phone: (858)534-8373
