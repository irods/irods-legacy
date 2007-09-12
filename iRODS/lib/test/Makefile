buildDir = ../..
include $(buildDir)/mk/mk.config
include $(buildDir)/mk/mk.common


svrSrcDir=$(buildDir)/server/src/server
agentSrcDir=$(buildDir)/server/src/agent
miscSrcDir=$(buildDir)/server/src/misc
cliSrcDir=$(buildDir)/clientLib/src
svrObjDir=$(buildDir)/server/obj
clObjDir=$(buildDir)/clientLib/obj
serverBinDir=$(buildDir)/server/bin
dataDir=$(buildDir)/data

vpath %.c $(svrSrcDir) $(agentSrcDir) $(miscSrcDir) $(cliSrcDir)

CFLAGS=$(MY_CFLAG)
CFLAGS+= -g -I$(buildDir)/server/include -I$(buildDir)/clientLib/include  \
-I$(buildDir)/clientLib/include/api -I$(buildDir)/clientLib/src/md5

LDADD+= -L$(clObjDir) -lRodsAPIs -L$(buildDir)/../SRB3_3plus/obj

TESTOBJS = lowlevtest.o packtest.o l1test.o l1rm.o testrule.o xmltest.o \
l3bundle.o

TARGETS = lowlevtest packtest l1test l1rm testrule xmltest l3bundle

all:: $(TARGETS)

clean:
	rm -f $(TARGETS) $(TESTOBJS)

$(TESTOBJS): %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

lowlevtest: lowlevtest.o
	$(LDR) -o $@ $^ $(LDADD)

packtest: packtest.o
	$(LDR) -o $@ $^ $(LDADD)

l1test: l1test.o
	$(LDR) -o $@ $^ $(LDADD)

testrule: testrule.o
	$(LDR) -o $@ $^ $(LDADD)

l1rm: l1rm.o
	$(LDR) -o $@ $^ $(LDADD)

xmltest: xmltest.o
	$(LDR) -o $@ $^ $(LDADD)

l3bundle: l3bundle.o
	$(LDR) -o $@ $^ $(LDADD)


