#
# modules/persistentArchives/Makefile
#
# Build the iRODS persistentArchives module
#

ifndef buildDir
buildDir = $(CURDIR)/../..
endif

include $(buildDir)/config/config.mk
include $(buildDir)/config/platform.mk
include $(buildDir)/config/directories.mk
include $(buildDir)/config/common.mk

#
# Directories
#
MSObjDir =	$(modulesDir)/persistentArchives/microservices/obj
MSSrcDir =	$(modulesDir)/persistentArchives/microservices/src
MSIncDir =	$(modulesDir)/persistentArchives/microservices/include

#
# Source files
#

OBJECTS =	$(MSObjDir)/reDataRel.o
#OBJECTS =	$(MSObjDir)/reDataRel.o $(MSObjDir)/srcEvBiWe2Ta3.o $(MSObjDir)/srcEvBiWe2Ta2.o $(MSObjDir)/srcEvBiWe2Ta1.o

INCLUDE_FLAGS = -I$(MSIncDir)
LIB_INCLUDES = -I$(buildDir)/lib/core/include -I$(buildDir)/lib/md5/include -I$(buildDir)/lib/api/include
#LIB_INCLUDES = -I$(buildDir)/lib/core/include -I$(buildDir)/lib/api/include

#
# Compile and link flags
#
INCLUDES +=	$(INCLUDE_FLAGS) $(LIB_INCLUDES) $(SVR_INCLUDES)
#CFLAGS_OPTIONS := $(CFLAGS) $(MY_CFLAG)
CFLAGS_OPTIONS = $(MY_CFLAG)
CFLAGS =	$(CFLAGS_OPTIONS) $(INCLUDES) $(MODULE_CFLAGS)


.PHONY: all server client microservices clean
.PHONY: server_ldflags client_ldflags server_cflags client_cflags
.PHONY: print_cflags

# Build everytying
all:	microservices
	@true

# List module's objects and needed libs for inclusion in clients
client_ldflags:
	@true

# List module's includes for inclusion in the clients
client_cflags:
	@true

# List module's objects and needed libs for inclusion in the server
server_ldflags:
	@echo $(OBJECTS) $(LIBS)

# List module's includes for inclusion in the server
server_cflags:
	@echo "gjk include = $(INCLUDE_FLAGS)"

# Build microservices

microservices:	print_cflags $(OBJECTS)
	@true

# Build client additions
client:
	@true

# Build server additions
server:
	@true

# Build rules
rules:
	@true

# Clean
clean:
	@echo "Clean persistentArchives module..."
	@rm -f $(OBJECTS)

# Show compile flags
print_cflags:
	@echo "Compile flags3:"
	@echo "   CFLAGS_OPTIONS  $(CFLAGS_OPTIONS)"
	@echo "   CFLAGS $(CFLAGS)"





#
# Compile targets
#
$(OBJECTS): $(MSObjDir)/%.o: $(MSSrcDir)/%.c $(DEPEND)
	@echo "Compile persistentArchives module `basename $@`..."
	@echo "$(CC) -c $(CFLAGS) -o $@ $<"
	@$(CC) -c $(CFLAGS) -o $@ $<

