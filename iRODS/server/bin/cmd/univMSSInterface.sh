#!/bin/sh

# This script is a template which must be updated if one wants to use the universal MSS driver.
# Functions to modify: syncToArch, stageToCache, mkdir, chmod, rm
# These functions need one or two input parameters which should be named $1 and $2.
# If some of these functions are not implemented for your MSS, just let this function as it is.
#	
# Jean-Yves Nief - CCIN2P3 - 09/2009
 
# function for the synchronization of file $1 on local disk resource to file $2 in the MSS
syncToArch () {
	# <your command or script to copy from cache to MSS> $1 $2 
	# e.g: /usr/local/bin/rfcp $1 rfioServerFoo:$2
	return
}

# function for staging a file $1 from the MSS to file $2 on disk
stageToCache () {
	# <your command to stage from MSS to cache> $1 $2	
	# e.g: /usr/local/bin/rfcp rfioServerFoo:$1 $2
	return
}

# function to create a new directory $1 in the MSS logical name space
mkdir () {
	# <your command to make a directory in the MSS> $1
	# e.g.: /usr/local/bin/rfmkdir -p rfioServerFoo:$1
	return
}

# function to modify ACLs $2 (octal) in the MSS logical name space for a given directory $1 
chmod () {
	# <your command to modify ACL> $1 $2
	# e.g: /usr/local/bin/rfchmod $2 rfioServerFoo:$1
	return
}

# function to remove a file $1 from the MSS
rm () {
	# <your command to remove a file from the MSS> $1
	# e.g: /usr/local/bin/rfrm rfioServerFoo:$1
	return
}

#############################################
# below this line, nothing should be changed.
#############################################

case "$1" in
	syncToArch ) $1 $2 $3 ;;
	stageToCache ) $1 $2 $3 ;;
	mkdir ) $1 $2 ;;
	chmod ) $1 $2 $3 ;;
	rm ) $1 $2 ;;
esac

exit $?
