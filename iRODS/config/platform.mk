#
# config/platform.mk
#
# Set platform-specific variables for building iRODS.  These variables
# include the name of the C compiler, loader, archiver, and ranlib, plus
# standard compile and link flags.  Many of these variables are set
# differently depending upon the current OS platform.
#

# default to cc
CC=cc
LDR=cc
PERL=perl

ifeq ($(OS_platform), sunos_platform)
# CC=gcc
CC=gcc
LDR=gcc
endif

ifdef CCMALLOC
LDADD+=$(CCMALLOC_DIR)/obj/ccmalloc-gcc.o -L$(CCMALLOC_DIR)/lib -lccmalloc -ldl
endif

LDADD= -lm -lpthread

ifeq ($(OS_platform), solaris_platform)
ifeq ($(ADDR_64BITS), 1)
# This compiled 64 bits and linked with 64 bit Oracle 2003.01.100 
# at SDSC and NASA on Solaris 9
#
CC=/opt/SUNWspro/bin/cc
AR=/usr/xpg4/bin/ar
LDR=/opt/SUNWspro/bin/cc -xarch=v9
else
ifdef PARA_OPR
CC=/opt/SUNWspro/bin/cc
LDR=/opt/SUNWspro/bin/cc
else
CC=cc
LDR=cc
endif
endif
endif

ifeq ($(OS_platform), aix_platform)
ifdef HPSS
CC=xlc_r4
LDR=xlc_r4
endif
endif

ifeq ($(OS_platform), linux_platform)
CC=gcc
LDR=gcc
endif

ifeq ($(OS_platform), sgi_platform)
CC=cc
LDR=cc
endif

ifeq ($(OS_platform), alpha_platform)
CC=cc -pthread  -std0 -verbose
LDR=cc
endif

ifeq ($(GMAKE_EXISTS), 1)
 ifeq ($(OS_platform), osx_platform)
 MAKE="make"
 else
 MAKE="gmake"
 endif
else
MAKE="make"
endif

ifeq ($(OS_platform), solaris_platform)
ifeq ($(ADDR_64BITS), 1)
AR=/usr/xpg4/bin/ar
else
AR=ar
endif
else
AR=ar
endif

AROPT= -crs
ifeq ($(OS_platform), osx_platform)
AROPT= -cr
else
AROPT= -crs
endif

ifeq ($(OS_platform), osx_platform)
RANLIB=                ranlib
else
RANLIB=                touch
endif

MY_CFLAG+= -g

ifeq ($(OS_platform), solaris_platform)
ifeq ($(ADDR_64BITS), 1)
MY_CFLAG+=-xarch=v9
endif
endif

ifeq ($(FILE_64BITS), 1)
ifeq ($(OS_platform), solaris_platform)
MY_CFLAG+=-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
endif
ifeq ($(OS_platform), aix_platform)
MY_CFLAG+=-D_LARGE_FILES
endif
ifeq ($(OS_platform), linux_platform)
MY_CFLAG+=-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
endif
ifeq ($(OS_platform), osx_platform)
MY_CFLAG+=-D_FILE_OFFSET_BITS=64
endif
endif

ifeq ($(ADDR_64BITS), 1)
MY_CFLAG+= -DADDR_64BITS
endif

MY_CFLAG+= -D$(OS_platform)

ifdef PARA_OPR
MY_CFLAG+= -DPARA_OPR=1
endif

ifeq ($(OS_platform), solaris_platform)
LDADD+=-lnsl -lsocket -lm -lpthread
endif

ifdef GSI_AUTH
ifeq ($(OS_platform), aix_platform)
LDADD+= $(LIB_GSI_AUTH) $(KRB_LIBS)
else
LDADD+= $(LIB_GSI_AUTH) $(KRB_LIBS) -z muldefs
endif
endif
