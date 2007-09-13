#
# clients/fuse/Makefile
#
# Build the iRODS fuse interface.
#
# The principal targets include:
#
#	all		build all of the tools
#	clean		clean out all object files
#

ifndef buildDir
buildDir =	$(CURDIR)/../..
endif

include $(buildDir)/config/config.mk
include $(buildDir)/config/platform.mk
include $(buildDir)/config/directories.mk
include $(buildDir)/config/common.mk




#
# Directories
#
fuseDir =	$(buildDir)/clients/fuse

objDir =	$(fuseDir)/obj
binDir =	$(fuseDir)/bin
srcDir =	$(fuseDir)/src
incDir =	$(fuseDir)/include





#
# Source files
#
OBJS =		\
		$(objDir)/irodsFs.o

BINS =		\
		$(binDir)/irodsFs \

LIB_OBJS =	\
		$(objDir)/iFuseOper.o \
		$(objDir)/iFuseLib.o

INCLUDES +=	-I$(incDir)





#
# Compile and link flags
#
# Is this needed?
#ifndef ADDR_64BITS
#CC =		gcc
#endif

ifdef IRODS_FS
CFLAGS +=	-D_REENTRANT -I$(fuseHomeDir)/include
endif

CFLAGS +=	-g $(MY_CFLAG) $(INCLUDES) $(LIB_INCLUDES) $(SVR_INCLUDES)
LDFLAGS +=	$(LDADD) $(MODULE_LDFLAGS)
LDFLAGS +=	-L$(fuseHomeDir)/lib -lfuse -ldl -lrt

# for checking memleak
# LDFLAGS +=	-L/data/mwan/rods/ccmalloc/ccmalloc-0.4.0/lib





#
# Principal Targets
#
.PHONY: all fuse clients clean
all:	fuse
	@true

clients: fuse
	@true

fuse:	$(LIB_OBJS) $(OBJS) $(BINS)
	@true

$(objDir)/%.o:	$(srcDir)/%.c $(LIBRARY)
	@echo "Compile fuse `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

$(binDir)/%:	$(objDir)/%.o
	@echo "Link fuse `basename $@`..."
	@$(LDR) -o $@ $< $(LIB_OBJS) $(LDFLAGS) $(LIBRARY)


# Clean
clean:
	@echo "Cleaning fuse..."
	@rm -f $(BINS) $(OBJS) $(LIB_OBJS)

