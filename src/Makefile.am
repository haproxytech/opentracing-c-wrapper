## Process this file with automake to produce Makefile.in
##
##
AM_CPPFLAGS = @OPENTRACING_C_WRAPPER_CPPFLAGS@
  AM_CFLAGS = @OPENTRACING_C_WRAPPER_CFLAGS@
AM_CXXFLAGS = @OPENTRACING_C_WRAPPER_CXXFLAGS@
 AM_LDFLAGS = @OPENTRACING_C_WRAPPER_LDFLAGS@
       LIBS = @LIBS@ @OPENTRACING_C_WRAPPER_LIBS@

if WANT_DEBUG
lib_LTLIBRARIES = libopentracing-c-wrapper_dbg.la
pkgconfig_DATA  = ../opentracing-c-wrapper_dbg.pc

libopentracing_c_wrapper_dbg_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/../include
libopentracing_c_wrapper_dbg_la_CFLAGS   = $(AM_CFLAGS)
libopentracing_c_wrapper_dbg_la_CXXFLAGS = $(AM_CXXFLAGS)
libopentracing_c_wrapper_dbg_la_LDFLAGS  = $(AM_LDFLAGS) -version-info @LIB_VERSION@ -Wl,--version-script=$(srcdir)/export_dbg.map
libopentracing_c_wrapper_dbg_la_SOURCES  = \
	dbg_malloc.cpp \
	span.cpp \
	tracer.cpp \
	util.cpp

else

lib_LTLIBRARIES = libopentracing-c-wrapper.la
pkgconfig_DATA  = ../opentracing-c-wrapper.pc

libopentracing_c_wrapper_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)/../include
libopentracing_c_wrapper_la_CFLAGS   = $(AM_CFLAGS)
libopentracing_c_wrapper_la_CXXFLAGS = $(AM_CXXFLAGS)
libopentracing_c_wrapper_la_LDFLAGS  = $(AM_LDFLAGS) -version-info @LIB_VERSION@ -Wl,--version-script=$(srcdir)/export.map
libopentracing_c_wrapper_la_SOURCES  = \
	span.cpp \
	tracer.cpp \
	util.cpp
endif

pkginclude_HEADERS = \
	../include/opentracing-c-wrapper/common.h \
	../include/opentracing-c-wrapper/dbg_malloc.h \
	../include/opentracing-c-wrapper/define.h \
	../include/opentracing-c-wrapper/include.h \
	../include/opentracing-c-wrapper/propagation.h \
	../include/opentracing-c-wrapper/span.h \
	../include/opentracing-c-wrapper/tracer.h \
	../include/opentracing-c-wrapper/util.h \
	../include/opentracing-c-wrapper/value.h

CLEANFILES = a.out
##
## Makefile.am ends here
