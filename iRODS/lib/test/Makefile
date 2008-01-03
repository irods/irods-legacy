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
l3structFile.o xmsgtest.o

TARGETS = lowlevtest packtest l1test l1rm testrule xmltest l3structFile xmsgtest
ifdef PHP_LIB
TARGETS+= phptest phpexttest
TESTOBJS+= phptest.o phpexttest.o
CFLAGS+=-I$(PHP_INCLUDE_DIR) -I$(PHP_INCLUDE_DIR)/main -I$(PHP_INCLUDE_DIR)/Zend -I$(PHP_INCLUDE_DIR)/TSRM -I$(PHP_INCLUDE_DIR)/regex -I$(PHP_INCLUDE_DIR)/sapi/embed
AG_LDADD+= -L/data/mwan/php/libxml2-2.6.30/.libs -lrt -lresolv -lm -ldl -lnsl -lxml2 -lz -lcrypt -liconv
endif


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

l3structFile: l3structFile.o
	$(LDR) -o $@ $^ $(LDADD)

xmsgtest: xmsgtest.o
	$(LDR) -o $@ $^ $(LDADD)

phptest: phptest.o
	$(LDR) -o $@ $^ $(LDADD) $(AG_LDADD)

phpexttest: phpexttest.o
	$(LDR) -o $@ $^ $(LDADD) $(AG_LDADD)

