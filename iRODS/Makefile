#
# Makefile
#
# Build the iRODS libraries, servers, and client commands.
#
# Principal targets include:
#
# 	all		build the libraries, server, and client commands
#	libs		build the libraries
#	server		build the server
#	clients		build the clients
#	modules		build the modules
# 	clean		clean out all object and library files
#

include config/config.mk
include config/platform.mk
include config/directories.mk
include config/common.mk




#
# Varibles
#
# The name of temporary files containing LDFLAGS assembled from
# the installed modules.
SERVER_LDFLAGS_TMP =	.server_ldflags
CLIENT_LDFLAGS_TMP =	.client_ldflags





#
# Principal Targets
#
.PHONY: all lib libs modules server clients $(MODULES) icommands fuse
.PHONY: clean distclean mostlyclean maintainer-clean
.PHONY: cleanlib cleanclients cleanserver cleanmodules
.PHONY: config server_ldflags client_ldflags
.PHONY: dist check test tests
.PHONY: install install-html install-dvi install-pdf install-ps
.PHONY: install-strip uninstall
.PHONY: installdirs installcheck
.PHONY: TAGS info dvi html pdf ps





# Build everything
all:	libs modules server clients tests
	@true





# Build the libraries
lib:	libs
	@true

libs:	modules
	@echo "Libraries"
	@echo "------------------------------------------------------------------------"
	@$(MAKE) -C lib all
	@echo " "





# Build the servers
server:	server_ldflags libs modules
	@echo "Server"
	@echo "------------------------------------------------------------------------"
	@MODULE_LDFLAGS=`cat $(SERVER_LDFLAGS_TMP)` && export MODULE_LDFLAGS && $(MAKE) -C server all
	@echo " "





# Build the clients
clients: icommands

icommands: client_ldflags libs modules
	@echo "I-Commands"
	@echo "------------------------------------------------------------------------"
	@MODULE_LDFLAGS=`cat $(CLIENT_LDFLAGS_TMP)` && export MODULE_LDFLAGS && $(MAKE) -C clients/icommands all
	@echo " "

fuse:	client_ldflags libs modules
	@echo "Fuse"
	@echo "------------------------------------------------------------------------"
	@MODULE_LDFLAGS=`cat $(CLIENT_LDFLAGS_TMP)` && export MODULE_LDFLAGS && $(MAKE) -C clients/fuse all
	@echo " "





# Build the modules
modules: $(MODULES)

ifdef MODULES
$(MODULES):
	@echo "Module $@"
	@echo "------------------------------------------------------------------------"
	@$(MAKE) -C modules/$@ server microservices
	@echo " "

cleanmodules:
	@for i in modules/*; do $(MAKE) -C $$i clean; done

# Get LDFLAGS added by all the modules for the servers
server_ldflags:
	@touch $(SERVER_LDFLAGS_TMP)
	@for i in $(MODULES); do $(MAKE) -s -C modules/$$i server_ldflags; done | \
		perl $(configDir)/joinlines.pl > $(SERVER_LDFLAGS_TMP)

# Get LDFLAGS added by all the modules for the clients
client_ldflags:
	@touch $(CLIENT_LDFLAGS_TMP)
	@for i in $(MODULES); do $(MAKE) -s -C modules/$$i client_ldflags; done | \
		perl $(configDir)/joinlines.pl > $(CLIENT_LDFLAGS_TMP)
else
cleanmodules:
	@true

server_ldflags:
	@touch $(SERVER_LDFLAGS_TMP)

client_ldflags:
	@touch $(CLIENT_LDFLAGS_TMP)
endif





# Build the documentation
doc:
	@echo "The iRODS Makefile does not support building documentation yet."

dvi:	doc
	@true

html:	doc
	@true

pdf:	doc
	@true

ps:	doc
	@true





# Build the server configuration
config:
	@echo "Server Configuration"
	@echo "------------------------------------------------------------------------"
	@$(MAKE) -C server config
	@echo " "

TAGS:
	@true





# Clean out everything that's been built
clean:
	@echo "Clean"
	@echo "------------------------------------------------------------------------"
	@rm -f $(SERVER_LDFLAGS_TMP) $(CLIENT_LDFLAGS_TMP)
	@$(MAKE) cleanlib
	@$(MAKE) cleanclients
	@$(MAKE) cleanserver
	@$(MAKE) cleanmodules

cleanlib:
	@$(MAKE) -C lib clean

cleanserver:
	@$(MAKE) -C server clean

cleanclients:
	@$(MAKE) -C clients/icommands clean

distclean:	clean
	@echo "Cleaning configuration files..."
	@rm -f config/config.mk
	@rm -f server/config/server.config
	@echo "Cleaning logs..."
	@rm -f installLogs/*
	@rm -f server/log/*

mostlyclean:	clean
	@true

maintainer-clean:	clean
	@true





# Build a distribution
dist:
	@echo "The iRODS Makefile does not support building a distribution yet."

info:
	@true

check:	test
	@true

tests:	test
	@true

test:
	@echo "Tests"
	@echo "------------------------------------------------------------------------"
	@$(MAKE) -C server test





# Install
install:
	@echo "Please use the install.pl script to install iRODS"

installdirs:
	@true

installcheck:
	@true

install-html:	install
	@true

install-dvi:	install
	@true

install-pdf:	install
	@true

install-ps:	install
	@true

install-strip:	install
	@true

uninstall:
	@echo "The iRODS Makefile does not support an uninstall operation."
