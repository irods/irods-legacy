DIRECTORY
iRODS/clients/fuse	- iRODS/fuse implementation

DESCRIPTION
	This directory, and its subdirectories, contain modules for
	building the irodsFs binary which can be used to mount an
	iRods collection to a local directory. Then the files and
        sub-collections in this mounted iRods collection can be
	accessed using regular UNIX commands through this local directory.


Building irodsFs:
-----------------

a) Edit the config/config.mk file:

1) Uncomment the line:
    # IRODS_FS = 1

2) set fuseHomeDir to the directory where the fuse library and include
files are installed. e.g.,

    fuseHomeDir=/usr/local/fuse

b) Making irodsFs:

Type in:

cd clients/fuse
gmake

Running irodsFs:
----------------

1) cd clients/fuse/bin

2) make a local directory for mounting. e.g.,

    mkdir /usr/tmp/fmount

3) Setup the iRods client env (~/irods/.irodsEnv) so that iCommands will
work. Type in:
    iinit

and do the normal login.

3) Mount the home collection to the local directory by typing in:
./irodsFs /usr/tmp/fmount

The user's home collection is now mounted. The iRods files and sub-collections 
in the user's home collection should be accessible with normal UNIX commands 
through the /usr/tmp/fmount directory. 

