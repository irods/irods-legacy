DIRECTORY
	iRODS/lib/sha1	- SHA1 hash functions

DESCRIPTION
	This directory contains hash functions used by the clients
	and servers.

This is the freeware version of sha-1 provided by Paul E. Jones (see
license.txt), with minor modifications to fit into iRODS.  Where I've
modified a .h or .c file, I've kept the original in a _.h.orig or
_.c.orig file.  The sha.c and shatest.c are test programs in the
original that can be linked in with it.  sha1.c contains the sha
library functions used in iRODS.  Wayne Schroeder, January 2013

