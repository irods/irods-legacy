#
# modules/image/Makefile
#
# Build the iRODS image module
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
MSObjDir =	$(modulesDir)/image/microservices/obj
MSSrcDir =	$(modulesDir)/image/microservices/src
MSIncDir =	$(modulesDir)/image/microservices/include





#
# Source files
#
IMAGEMAGICK =	$(buildDir)/../ImageMagick-6.3.5
OBJECTS =	$(MSObjDir)/imageMS.o $(MSObjDir)/imageMSutil.o
INCLUDES +=	-I$(MSIncDir) -I$(IMAGEMAGICK)/include
LIBS =		-L$(IMAGEMAGICK)/lib -lMagick





#
# Compile and link flags
#
INCLUDES +=	$(LIB_INCLUDES) $(SVR_INCLUDES)
CFLAGS +=	$(MY_CFLAG) $(INCLUDES)





.PHONY: all server microservices clean
.PHONY: server_ldflags client_ldflags

# Build everytying
all:	microservices
	@true

# List module's objects and needed libs for inclusion in clients
client_ldflags:
	@true

# List module's objects and needed libs for inclusion in the server
server_ldflags:
	@echo $(OBJECTS) $(LIBS)

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
	@echo "Clean image module..."
	@rm -f $(OBJECTS)





#
# Compile targets
#
$(OBJECTS): $(MSObjDir)/%.o: $(MSSrcDir)/%.c $(DEPEND)
	@echo "Compile image module `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

