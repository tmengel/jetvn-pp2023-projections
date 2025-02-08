AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib64

pkginclude_HEADERS = \
  JetVnTree.h

lib_LTLIBRARIES = \
  libjetflow.la

libjetflow_la_SOURCES = \
  JetVnTree.cc

libjetflow_la_LIBADD = \
  -lfun4all \
  -lepd_io \
  -lffamodules \
  -lg4dst \
  -lg4eval \
  -lphhepmc \
  -lphool \
  -lcdbobjects \
  -lglobalvertex_io \
  -lSubsysReco \
  -lg4jets


BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libjetflow.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f $(BUILT_SOURCES)
