This is iRODS version 0.9.2.1.


Web site:

For information, please see http://irods.sdsc.edu


Release notes:

http://irods.sdsc.edu/index.php/Release_Notes_0.9

0.9.1 was a patch release with some small changes from 0.9, some
improvements for running on Macintosh hosts:
1) A segfault in 'icp' on Macs has been fixed, and
2) iRODS now installs on Intel-based Macintosh systems.

0.9.2 was another small patch to correct a fatal error building
non-ICAT servers.

0.9.2.1 is another small patch to correct two problems that show up
only with non-ICAT servers on Solaris: a fatal linking error and a bug
in installServer.pl.  Only two files are updated from the 0.9.2
version: rsDataObjRename.c and installServer.pl (plus this README).
