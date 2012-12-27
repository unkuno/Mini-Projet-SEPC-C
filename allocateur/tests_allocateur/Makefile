CC=gcc


# variables utilises pour les regles de compilation implicites

LIBCUNIT_INSTALL = $(shell ls /usr/include/CUnit/CUnit.h 2> /dev/null)
ifeq ($(LIBCUNIT_INSTALL),/usr/include/CUnit/CUnit.h)
# cunit default install
CFLAGS += -Wall -g -Werror
CXXFLAGS += -Wall -g -Werror -ftemplate-depth-30
LDLIBS= -lcunit
else
LIBCUNIT_INSTALL = $(shell ls /usr/local/src/CUnit-2.0-2/include/CUnit/CUnit.h 2> /dev/null)
ifeq ($(LIBCUNIT_INSTALL),/usr/local/src/CUnit-2.0-2/include/CUnit/CUnit.h)
# cunit default install
CFLAGS += -I/usr/local/src/CUnit-2.0-2/include -Wall -g -Werror
CXXFLAGS += -I/usr/local/src/CUnit-2.0-2/include -Wall -g -Werror -ftemplate-depth-30
LDLIBS= -L/usr/local/src/CUnit-2.0-2/lib -lcunit
else
LIBCUNIT_INSTALL = $(shell ls /usr/local/include/CUnit/CUnit.h 2> /dev/null)
ifeq ($(LIBCUNIT_INSTALL),/usr/local/include/CUnit/CUnit.h)
# cunit default install
CFLAGS += -I/usr/local/include -Wall -g -Werror
CXXFLAGS += -I/usr/local/include -Wall -g -Werror -ftemplate-depth-30
LDLIBS= -L/usr/local/lib -lcunit
else
# ensibm/ensibull: installation dans un repertoire d'un enseignant
CFLAGS += -Wall -g -Werror -I/home/perms/mounie/local/include
CXXFLAGS += -Wall -g -Werror -ftemplate-depth-30
LDLIBS+= /home/perms/mounie/local/lib/libcunit.a 
endif
endif
endif

SKELDIR = tests_allocateur


all: build test

build: ../mem.o
	( cd ..; make mem.o )

test: utest
	./utest

usage:
	@ echo \"make test\" build and execute utest
	@ echo \"make all\" build utest
	@ echo "NB: requirement C++ compiler, cunit installed."

tar-tests:
	cd .. ; tar czvf $(SKELDIR)-1.1.tar.gz  $(SKELDIR)/test_buddy.c $(SKELDIR)/test_cff.c $(SKELDIR)/test_cff.h $(SKELDIR)/test_bf.c $(SKELDIR)/test_bf.h $(SKELDIR)/test_buddy.h $(SKELDIR)/Makefile  $(SKELDIR)/test_generic.c $(SKELDIR)/test_generic.h $(SKELDIR)/utest.c $(SKELDIR)/README $(SKELDIR)/test_run_cpp.cc $(SKELDIR)/test_run_cpp.h $(SKELDIR)/test_run.H


# Utilisation des regles implicites de compilation de make


utest: utest.o ../mem.o test_generic.o test_buddy.o test_cff.o test_bf.o test_run_cpp.o
	g++ -g -Wall -Werror -o $@ $+ $(LDLIBS)

clean:
	rm *.o utest

