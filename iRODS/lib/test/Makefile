ifndef buildDir
buildDir = ../..
endif

include $(buildDir)/config/config.mk
include $(buildDir)/config/platform.mk
include $(buildDir)/config/directories.mk
include $(buildDir)/config/common.mk

vpath %.c src

CFLAGS=$(MY_CFLAG)
CFLAGS+= -g -I$(buildDir)/server/core/include \
-I$(buildDir)/server/icat/include \
-I$(buildDir)/server/drivers/include \
-I$(buildDir)/server/re/include \
-I$(buildDir)/lib/core/include  \
-I$(buildDir)/lib/api/include -I$(buildDir)/lib/md5/include

# LDADD+= -L$(clObjDir) -lRodsAPIs -L$(buildDir)/../SRB3_3plus/obj
LDADD+= -L$(buildDir)/lib/core/obj -l$(LIBRARY_NAME)

TESTOBJS = lowlevtest.o packtest.o l1test.o l1rm.o testrule.o xmltest.o \
l3bundle.o xmsgtest.o

TARGETS = lowlevtest packtest l1test l1rm testrule xmltest l3bundle xmsgtest

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

xmsgtest: xmsgtest.o
	$(LDR) -o $@ $^ $(LDADD)
