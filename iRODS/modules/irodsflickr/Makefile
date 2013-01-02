#
# modules/irodsflickr/Makefile
#
# Builds the iRODS irodsflickr module
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
MSObjDir =	$(modulesDir)/irodsflickr/microservices/obj
MSSrcDir =	$(modulesDir)/irodsflickr/microservices/src
MSIncDir =	$(modulesDir)/irodsflickr/microservices/include



#
# Objects
#
OBJECTS =	$(MSObjDir)/flickrMS.o $(MSObjDir)/cJSON.o



#
# Include flags
#
INCLUDE_FLAGS =	-I$(MSIncDir)



#
# Include files and libraries for libcurl
# Please run 'curl-config --cflags' and 'curl-config --libs'
# on your system and edit the following two lines accordingly.
#
INCLUDE_FLAGS += -I/opt/local/include
LIBS = -L/opt/local/lib -lcurl -lidn -lssl -lcrypto -lz -loauth
#LIBS = -L/usr/lib -lcurl -loauth



#
# Compile and link flags
#
INCLUDES +=	$(INCLUDE_FLAGS) $(LIB_INCLUDES) $(SVR_INCLUDES)
CFLAGS_OPTIONS := -DRODS_SERVER $(CFLAGS) $(MY_CFLAG)
CFLAGS =	$(CFLAGS_OPTIONS) $(INCLUDES) $(MODULE_CFLAGS)




.PHONY: all rules microservices server client clean
.PHONY: server_ldflags client_ldflags server_cflags client_cflags
.PHONY: print_cflags

# Build everything
all:	microservices
	@true

# List module's objects for inclusion in the clients
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
	@echo $(INCLUDE_FLAGS)

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
	@echo "Clean irodsflickr module..."
	@rm -f $(OBJECTS)





# Show compile flags
print_cflags:
	@echo "Compile flags:"
	@echo "    $(CFLAGS_OPTIONS)"





#
# Compilation targets
#
$(OBJECTS): $(MSObjDir)/%.o: $(MSSrcDir)/%.c $(DEPEND)
	@echo "Compile irodsflickr module `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

