#
# modules/ERA/Makefile
#
# Builds the iRODS ERA module
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
MSObjDir =	$(modulesDir)/ERA/microservices/obj
MSSrcDir =	$(modulesDir)/ERA/microservices/src
MSIncDir =	$(modulesDir)/ERA/microservices/include





#
# Source files
#
OBJECTS =	$(MSObjDir)/eraMS.o $(MSObjDir)/eraUtil.o $(MSObjDir)/auditMS.o
INCLUDE_FLAGS =	-I$(MSIncDir)





#
# Compile and link flags
#
INCLUDES +=	$(INCLUDE_FLAGS) $(LIB_INCLUDES) $(SVR_INCLUDES)
CFLAGS +=	$(MY_CFLAG) $(INCLUDES) $(MODULE_CFLAGS)
CFLAGS +=	-DRODS_SERVER





.PHONY: all rules microservices server client clean
.PHONY: server_ldflags client_ldflags server_cflags client_cflags

# Build everything
all:	microservices
	@true

# List module's objects for inclusion in the clients
client_ldflags:
	@true

# List module's includes for inclusion in the clients
client_cflags:
	@true

# List module's objects for inclusion in the server
server_ldflags:
	@echo $(OBJECTS)

# List module's includes for inclusion in the server
server_cflags:
	@echo $(INCLUDE_FLAGS)

# Build microservices
microservices:	$(OBJECTS)
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
	@echo "Clean ERA module..."
	@rm -f $(OBJECTS)





#
# Compilation targets
#
$(OBJECTS): $(MSObjDir)/%.o: $(MSSrcDir)/%.c $(DEPEND)
	@echo "Compile ERA module `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

