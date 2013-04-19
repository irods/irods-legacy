DIRECTORY
	iRODS/server/tests/tbox - TinderBox test scripts and input files.

DESCRIPTION 
        This directory contains test scripts and input files for the
	iRODS TinderBox automatic build/test system.  This tbox system
	tests the installation and operation of the IRODS system as a
	whole, including the server, ICAT, and C clients.  The results
	are visible under the "Testing IRODS" page on the IRODS web
	site.

        tinderclient.pl is a major part of this, but is not included
        here as I am not sure if the licensing would be OK with iRODS
        distributions.  It's a slightly modified version of the
        standard tinderbox script.

        These files are not used here, but are put in place in
        a working directory on a host, along with tinderclient.pl, 
        and then started via 'nice ./start_tinderbox'.

        These are only used by the developers at DICE at UCSD, but I
        wanted to put them into CVS for maintanence.
 
        We have been using various versions of these for some time
        (both SRB and IRODS), but I think mantaining them in CVS will
        help us keep them organized.

        Wayne Schroeder
        Initially added March 2008.

Test scripts ran by TinderBox:

1) testiCommands.pl - every time.

2) irodsctl devtest every time.

3) poundtest - once a day

4) build with boost and run devtest - once a day
   (This no longer builds boost, but uses boost that was a previously
   installed via a package mangager.)

5) download and build Postgres and reinstall the ICAT - once a day

6) use MySQL for the ICAT - once a day
   (MySQL has been previously installed via a package manager)

7) test iRODS FUSE - once a day
   (FUSE has been previously installed via a package manager.)
