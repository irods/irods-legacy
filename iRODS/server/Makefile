#
# server/Makefile
#
# Build the iRODS server, agent, and rule engine server.  This Makefile
# is intended to be invoked only through the top-level iRODS Makefile.
#
# Principal targets include:
#
# 	all		build the servers
#	server		build the servers
# 	clean		clean out all object files
#

ifndef buildDir
buildDir =	$(CURDIR)/..
endif

include $(buildDir)/config/config.mk
include $(buildDir)/config/platform.mk
include $(buildDir)/config/directories.mk
include $(buildDir)/config/common.mk




#
# Source files
# 	core	shared functions for all servers
# 	api	server API functions (SVR_API_OBJS is set by api.mk)
#	drivers	file system driver functions
#	icat	ICAT server
#	re	rule engine
#
# Core
SVR_CORE_OBJS =	\
		$(svrCoreObjDir)/dataObjOpr.o \
		$(svrCoreObjDir)/fileOpr.o \
		$(svrCoreObjDir)/initServer.o \
		$(svrCoreObjDir)/miscServerFunct.o \
		$(svrCoreObjDir)/objMetaOpr.o \
		$(svrCoreObjDir)/readServerConfig.o \
		$(svrCoreObjDir)/rsApiHandler.o \
		$(svrCoreObjDir)/rsIcatOpr.o \
		$(svrCoreObjDir)/rsLog.o \
		$(svrCoreObjDir)/rsRe.o
INCLUDES +=	-I$(svrCoreIncDir)


# Servers
CFLAGS +=	-DRODS_SERVER
SERVER_OBJS =	\
		$(svrCoreObjDir)/rodsServer.o \
		$(svrAgentObjDir)/rodsAgent.o
SERVER_BINS =	$(serverBinDir)/irodsServer \
		$(serverBinDir)/irodsAgent


# API (see api.mk, which is automatically generated)
include $(buildDir)/config/api.mk
INCLUDES +=	-I$(svrApiIncDir)


# Drivers
SVR_DRIVERS_OBJS = \
		$(svrDriversObjDir)/bundleDriver.o \
		$(svrDriversObjDir)/fileDriver.o \
		$(svrDriversObjDir)/haawBunSubDriver.o \
		$(svrDriversObjDir)/unixFileDriver.o
INCLUDES +=	-I$(svrDriversIncDir)


# XXXXX add svrIcatIncDir for now. needed by non-icat server. mw
INCLUDES +=	-I$(svrIcatIncDir)

# icat
ifdef RODS_CAT
CFLAGS +=	-DRODS_CAT
SVR_ICAT_OBJS =	\
		$(svrIcatObjDir)/icatMidLevelHelpers.o \
		$(svrIcatObjDir)/icatMidLevelRoutines.o \
		$(svrIcatObjDir)/icatHighLevelRoutines.o \
		$(svrIcatObjDir)/icatGeneralQuery.o \
		$(svrIcatObjDir)/icatGeneralQuerySetup.o \
		$(svrIcatObjDir)/icatGeneralUpdate.o
INCLUDES +=	-I$(svrIcatIncDir)

ifdef NEW_ODBC
# Use the 'new' database handling
CFLAGS +=	-DNEW_ODBC
endif


ifdef PSQICAT
# Support Postgres
SVR_ICAT_OBJS += $(svrIcatObjDir)/icatLowLevelPostgres.o
INCLUDES +=	-I$(POSTGRES_HOME)/include
endif

ifdef ORAICAT
# Support Oracle
CFLAGS +=	-DORA_ICAT
SVR_ICAT_OBJS += $(svrIcatObjDir)/icatLowLevelOracle.o
INCLUDES +=	-I$(ORACLE_HOME)/rdbms/public
endif
endif

ifdef DBMS_LIB
LDFLAGS +=	$(DBMS_LIB)
endif

ifdef GEN_QUERY_AC
CFLAGS +=	-DGEN_QUERY_AC
endif


# re
ifdef RULE_ENGINE
CFLAGS +=	-DRULE_ENGINE
SVR_RE_OBJS =	\
		$(svrReObjDir)/extractAvuMS.o \
		$(svrReObjDir)/genQueryMS.o \
		$(svrReObjDir)/icatAdminMS.o \
		$(svrReObjDir)/icatGeneralMS.o \
		$(svrReObjDir)/keyValPairMS.o \
		$(svrReObjDir)/mailMS.o \
		$(svrReObjDir)/miscMS.o \
		$(svrReObjDir)/printMS.o \
		$(svrReObjDir)/reDataObjOpr.o \
		$(svrReObjDir)/reEraActions.o \
		$(svrReObjDir)/reEraHelpers.o \
		$(svrReObjDir)/reHelpers1.o \
		$(svrReObjDir)/reHelpers2.o \
		$(svrReObjDir)/reIn2p3SysRule.o \
		$(svrReObjDir)/reLib1.o	\
		$(svrReObjDir)/reLib2.o \
		$(svrReObjDir)/reNaraMetaData.o \
		$(svrReObjDir)/reStruct.o \
		$(svrReObjDir)/reSysDataObjOpr.o \
		$(svrReObjDir)/reVariableMap.o \
		$(svrReObjDir)/regExpMatch.o \
		$(svrReObjDir)/ruleAdminMS.o \
		$(svrReObjDir)/systemMS.o \
		$(svrReObjDir)/testMS.o \
		$(svrReObjDir)/reDataRel.o
INCLUDES +=	-I$(svrReIncDir)
SERVER_BINS +=	$(serverBinDir)/irodsReServer
endif


# icat Tests
ifdef RODS_CAT
TEST_OBJS =	\
		$(svrTestObjDir)/test_chl.o \
		$(svrTestObjDir)/test_cll.o \
		$(svrTestObjDir)/test_genq.o \
		$(svrTestObjDir)/test_genu.o

TEST_BINS =	\
		$(svrTestBinDir)/test_chl \
		$(svrTestBinDir)/test_cll \
		$(svrTestBinDir)/test_genq \
		$(svrTestBinDir)/test_genu
endif

# reTest only works on Solaris
#TEST_OBJS +=	$(svrTestObjDir)/reTest.o
#TEST_BINS +=	$(svrTestBinDir)/reTest




# All objects (except tests)
OBJS =		$(SVR_CORE_OBJS) $(SVR_DRIVERS_OBJS) $(SVR_API_OBJS) \
		$(SVR_ICAT_OBJS) $(SVR_RE_OBJS)


# Server configuration
CONFIG +=	$(svrConfigDir)/server.config $(svrConfigDir)/irodsHost


# Compilation and link flags
CFLAGS +=	$(MY_CFLAG) $(INCLUDES) $(LIB_INCLUDES) $(MODULE_CFLAGS)
LDFLAGS +=	$(LDADD) $(LIBRARY) $(MODULE_LDFLAGS)





#
# Princpal Targets
#
.PHONY:	all clean config server servers bin test tests
vpath %c	$(svrCoreSrcDir) $(svrDriversSrcDir) $(svrApiSrcDir) \
		$(svrIcatSrcDir) $(svrReSrcDir) 





# Build everything
all:	server
	@true





# Build the servers
servers: server
	@true

bin:	server
	@true

server:	config $(OBJS) $(SERVER_BINS)
	@true





# Server configuration
config:	$(CONFIG)
	@true

$(svrConfigDir)/server.config: $(svrConfigDir)/server.config.in
	@echo "Warning, using default server.config file"
	@touch $(svrConfigDir)/server.config
	@mv $(svrConfigDir)/server.config $(svrConfigDir)/server.config.sav
	@cp $(svrConfigDir)/server.config.in $(svrConfigDir)/server.config
	@chmod 600 $(svrConfigDir)/server.config

$(svrConfigDir)/irodsHost: $(svrConfigDir)/irodsHost.in
	@echo "Note, making irodsHost file from irodsHost.in"
	@touch $(svrConfigDir)/irodsHost
	@mv $(svrConfigDir)/irodsHost $(svrConfigDir)/irodsHost.sav
	@cp $(svrConfigDir)/irodsHost.in $(svrConfigDir)/irodsHost
	@chmod 600 $(svrConfigDir)/irodsHost




# Tests
tests:	test
	@true

retest:	test
	@true

test:	$(TEST_OBJS) $(TEST_BINS)
	@true





# Clean
clean:
	@echo "Cleaning server..."
	@rm -f $(OBJS)
	@rm -f $(SERVER_OBJS) $(SERVER_BINS)
	@rm -f $(TEST_OBJS) $(TEST_BINS)
	@rm -f $(svrCoreObjDir)/ruleAdmin.o $(serverBinDir)/ruleAdmin




#
# Compilation Targets
#
# Build server API
$(SVR_API_OBJS):	$(svrApiObjDir)/%.o: $(svrApiSrcDir)/%.c $(DEPEND)
	@echo "Compile api `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# Build server core
$(SVR_CORE_OBJS):	$(svrCoreObjDir)/%.o: $(svrCoreSrcDir)/%.c $(DEPEND)
	@echo "Compile core `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# Build server drivers
$(SVR_DRIVERS_OBJS):	$(svrDriversObjDir)/%.o: $(svrDriversSrcDir)/%.c $(DEPEND)
	@echo "Compile drivers `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# Build server ICAT
$(SVR_ICAT_OBJS):	$(svrIcatObjDir)/%.o: $(svrIcatSrcDir)/%.c $(DEPEND)
	@echo "Compile icat `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# Builder server rule engine
$(SVR_RE_OBJS):		$(svrReObjDir)/%.o: $(svrReSrcDir)/%.c $(DEPEND)
	@echo "Compile re `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<





#
# Servers
#
# Agent
$(svrCoreObjDir)/rodsAgent.o: $(svrCoreObjDir)/%.o: $(svrCoreSrcDir)/%.c $(LIBRARY) $(OBJS)
	@echo "Compile agent server `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

$(serverBinDir)/irodsAgent: $(svrCoreObjDir)/rodsAgent.o $(LIBRARY) $(OBJS)
	@echo "Link agent server `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)

# Rule engine
$(svrCoreObjDir)/irodsReServer.o: $(svrCoreObjDir)/%.o: $(svrCoreSrcDir)/%.c $(LIBRARY) $(OBJS)
	@echo "Compile rule engine server `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

$(serverBinDir)/irodsReServer: $(svrCoreObjDir)/irodsReServer.o $(LIBRARY) $(OBJS)
	@echo "Link rule engine server `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)

# RODS
$(svrCoreObjDir)/rodsServer.o: $(svrCoreObjDir)/%.o: $(svrCoreSrcDir)/%.c $(LIBRARY) $(OBJS)
	@echo "Compile rods server `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

$(serverBinDir)/irodsServer: $(svrCoreObjDir)/rodsServer.o $(LIBRARY) $(OBJS)
	@echo "Link rods server `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)





#
# Tests
#
$(TEST_OBJS):	$(svrTestObjDir)/%.o: $(svrTestSrcDir)/%.c $(DEPEND)
	@echo "Compile server test `basename $@`..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# reTest
$(svrTestBinDir)/reTest: $(svrTestObjDir)/reTest.o $(OBJS)
	@echo "Link server test `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)

# cll and chl
$(svrTestBinDir)/test_cll: $(svrTestObjDir)/test_cll.o $(SVR_ICAT_OBJS)
	@echo "Link server test `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)

$(svrTestBinDir)/test_chl: $(svrTestObjDir)/test_chl.o $(SVR_ICAT_OBJS) $(svrCoreObjDir)/readServerConfig.o
	@echo "Link server test `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)

# genq and genu
$(svrTestBinDir)/test_genq: $(svrTestObjDir)/test_genq.o $(SVR_ICAT_OBJS) $(svrCoreObjDir)/readServerConfig.o
	@echo "Link server test `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)

$(svrTestBinDir)/test_genu: $(svrTestObjDir)/test_genu.o $(SVR_ICAT_OBJS) $(svrCoreObjDir)/readServerConfig.o
	@echo "Link server test `basename $@`..."
	@$(LDR) -o $@ $^ $(LDFLAGS)





#
# Rule Admin Targets
#
ruleadmin:: $(serverBinDir)/ruleAdmin 
	@cp $(serverBinDir)/ruleAdmin /misc/www/projects/srb-secure/cgi-bin/ruleAdminRaja.cgi

$(svrCoreObjDir)/ruleAdmin.o: $(svrCoreObjDir)/%.o: %.c $(LIBRARY) $(OBJS)
	$(CC) -c $(CFLAGS) -o $@ $<

$(serverBinDir)/ruleAdmin: $(svrCoreObjDir)/ruleAdmin.o $(LIBRARY) $(OBJS)
	$(LDR) -o $@ $^ $(LDFLAGS)

