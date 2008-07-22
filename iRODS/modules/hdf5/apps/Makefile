# Makefile - This is the makefile for libjh5srb.so, jh5srb.class. 
SHELL=/bin/csh

# these parameters needs to be changed
OS_platform=linux_platform
irodsDir = /data/mwan/rods/iRODS
hdf5Dir = /data/mwan/hdf5/hdf5-1.8.0/hdf5
PROG = islice

include $(irodsDir)/config/config.mk
include $(irodsDir)/config/platform.mk
include $(irodsDir)/config/directories.mk
include $(irodsDir)/config/common.mk

# these are derived parameters
irodsLibDir = $(irodsDir)/lib/core/obj
hdf5LibIncDir = $(irodsDir)/modules/hdf5/lib/include
hdf5LibObjDir = $(irodsDir)/modules/hdf5/lib/obj
hdf5LibSrcDir = $(irodsDir)/modules/hdf5/lib/src

PROG_OBJECTS = $(PROG).o
PROG_SRC = $(PROG).c
HDF_LIB_OBJECTS=$(hdf5LibObjDir)/h5ClHandler.o 	\
    $(hdf5LibObjDir)/clH5Dataset.o         \
    $(hdf5LibObjDir)/clH5File.o $(hdf5LibObjDir)/clH5Group.o	\
    $(hdf5LibObjDir)/h5Ctor.o $(hdf5LibObjDir)/h5Dtor.o

#   $(hdf5LibObjDir)/h5LocalHandler.o	\

INCLUDES = -I$(hdf5Dir)/include -I$(hdf5LibIncDir)
INCLUDES +=     -I$(irodsDir)/lib/core/include -I$(irodsDir)/lib/api/include
INCLUDES +=     -I$(irodsDir)/lib/md5/include
INCLUDES +=     -I$(irodsDir)/server/core/include
INCLUDES +=     -I$(irodsDir)/server/icat/include
INCLUDES +=     -I$(irodsDir)/server/drivers/include
INCLUDES +=     -I$(irodsDir)/server/re/include

CFLAGS = $(MY_CFLAG) $(INCLUDES)

LDFLAGS =      -L$(irodsLibDir) -lRodsAPIs -lpthread -lm


all:: $(PROG) 

clean:
	rm -f $(HDF_LIB_OBJECTS) $(PROG) $(PROG_OBJECTS)

$(HDF_LIB_OBJECTS): $(hdf5LibObjDir)/%.o: $(hdf5LibSrcDir)/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(PROG): $(HDF_LIB_OBJECTS) $(PROG_OBJECTS)
	$(CC) $(PROG_OBJECTS) $(HDF_LIB_OBJECTS) $(LDFLAGS) -o $(PROG)

$(PROG_OBJECTS): $(PROG_SRC)
	$(CC) -c $(CFLAGS) -o $@ $<

