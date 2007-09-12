#
# modules/properties/Makefile
#
# Build the iRODS properties module
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
MSObjDir =	$(modulesDir)/properties/microservices/obj
MSSrcDir =	$(modulesDir)/properties/microservices/src
MSIncDir =	$(modulesDir)/properties/microservices/include





#
# Source files
#
OBJECTS =	$(MSObjDir)/propertiesMS.o
INCLUDES +=	-I$(MSIncDir)





#
# Compile and link flags
#
INCLUDES +=	$(LIB_INCLUDES) $(SVR_INCLUDES)
CFLAGS +=	$(MY_CFLAG) $(INCLUDES)





.PHONY: all rules microservices server clean
.PHONY: server_ldflags client_ldflags

# Build everything
all:	microservices
	@true

# List module's objects for inclusion in the clients
client_ldflags:
	@true

# List module's objects for inclusion in the server
server_ldflags:
	@echo $(OBJECTS)

# Build microservices
microservices:	$(OBJECTS)
	@true

# Build server additions
server:
	@true

# Build rules
rules:
	@true

# Clean
clean:
	@echo "Clean properties module..."
	@rm -f $(OBJECTS)





#
# Compilation targets
#
$(OBJECTS): $(MSObjDir)/%.o: $(MSSrcDir)/%.c $(DEPEND)
	@echo "Compile properties module `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

