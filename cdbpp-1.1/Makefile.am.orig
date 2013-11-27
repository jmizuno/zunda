# $Id: Makefile.am 11 2009-07-13 15:58:29Z naoaki $

docdir = $(prefix)/share/doc/@PACKAGE@
doc_DATA = README INSTALL COPYING AUTHORS ChangeLog NEWS

cdbppincludedir = $(includedir)

cdbppinclude_HEADERS = \
	include/cdbpp.h

noinst_PROGRAMS = cdbpp-sample

cdbpp_sample_SOURCES = \
	sample/sample.cpp

EXTRA_DIST = \
	autogen.sh \
	win32/stdint.h \
	cdbpp.sln \
	sample/sample.vcproj
