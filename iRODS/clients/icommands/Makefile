#
# clients/icommands/Makefile
#
# Build the iRODS command-line tools
#
# The principal targets include:
#
#	all		build all of the commands
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
icommandsDir =	$(buildDir)/clients/icommands

objDir =	$(icommandsDir)/obj
binDir =	$(icommandsDir)/bin
srcDir =	$(icommandsDir)/src





#
# Source files
#
OBJS =		\
		$(objDir)/iadmin.o \
		$(objDir)/icd.o \
		$(objDir)/ichksum.o   \
		$(objDir)/ichmod.o \
		$(objDir)/icp.o \
		$(objDir)/ienv.o \
		$(objDir)/iexecmd.o \
		$(objDir)/iexit.o \
		$(objDir)/iget.o \
		$(objDir)/iinit.o \
		$(objDir)/ils.o \
		$(objDir)/ilsresc.o \
		$(objDir)/imeta.o \
		$(objDir)/imiscsvrinfo.o \
		$(objDir)/imkdir.o \
		$(objDir)/imv.o \
		$(objDir)/iphymv.o \
		$(objDir)/iput.o \
		$(objDir)/ipwd.o  \
		$(objDir)/iqdel.o \
		$(objDir)/iqmod.o \
		$(objDir)/iqstat.o \
		$(objDir)/iquest.o \
		$(objDir)/ireg.o \
		$(objDir)/irepl.o \
		$(objDir)/irm.o \
		$(objDir)/irmtrash.o \
		$(objDir)/irsync.o \
		$(objDir)/irule.o \
		$(objDir)/isysmeta.o \
		$(objDir)/itrim.o \
		$(objDir)/iuserinfo.o

BINS =		\
		$(binDir)/iadmin \
		$(binDir)/icd \
		$(binDir)/ichksum \
		$(binDir)/ichmod \
		$(binDir)/icp \
		$(binDir)/ienv \
		$(binDir)/iexecmd \
		$(binDir)/iexit \
		$(binDir)/iget \
		$(binDir)/iinit \
		$(binDir)/ils \
		$(binDir)/ilsresc \
		$(binDir)/imeta \
		$(binDir)/imiscsvrinfo \
		$(binDir)/imkdir \
		$(binDir)/imv \
		$(binDir)/iphymv \
		$(binDir)/iput \
		$(binDir)/ipwd \
		$(binDir)/iqdel \
		$(binDir)/iqmod \
		$(binDir)/iqstat \
		$(binDir)/iquest \
		$(binDir)/ireg \
		$(binDir)/irepl \
		$(binDir)/irm \
		$(binDir)/irmtrash \
		$(binDir)/irsync \
		$(binDir)/irule \
		$(binDir)/isysmeta \
		$(binDir)/itrim \
		$(binDir)/iuserinfo \





#
# Compile and link flags
#
# Is this needed?
#ifndef ADDR_64BITS
#CC =		gcc
#endif

CFLAGS +=	-g $(MY_CFLAG) $(LIB_INCLUDES) $(SVR_INCLUDES) $(MODULE_CFLAGS)
LDFLAGS +=	$(LDADD) $(MODULE_LDFLAGS)

# for checking memleak
# LDFLAGS +=	-L/data/mwan/rods/ccmalloc/ccmalloc-0.4.0/lib





#
# Principal Targets
#
.PHONY: all icommands clients clean
all:	icommands
	@true

clients: icommands
	@true

icommands:	$(OBJS) $(BINS)
	@true

$(objDir)/%.o:	$(srcDir)/%.c $(LIBRARY)
	@echo "Compile icommand `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

$(binDir)/%:	$(objDir)/%.o
	@echo "Link icommand `basename $@`..."
	@$(LDR) -o $@ $< $(LDFLAGS) $(LIBRARY)





# Clean
clean:
	@echo "Cleaning icommands..."
	@rm -f $(BINS) $(OBJS)

