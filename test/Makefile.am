## Process this file with automake to produce Makefile.in
##
 AM_CPPFLAGS = -I\$(top_builddir)/include -I$(srcdir)/../include @OPENTRACING_C_WRAPPER_CPPFLAGS@
   AM_CFLAGS = @OPENTRACING_C_WRAPPER_CFLAGS@
  AM_LDFLAGS = @OPENTRACING_C_WRAPPER_LDFLAGS@

if WANT_DEBUG
                 bin_PROGRAMS = ot-c-wrapper-test_dbg
ot_c_wrapper_test_dbg_SOURCES = opentracing.c test.c util.c
  ot_c_wrapper_test_dbg_LDADD = -lstdc++ -lm @OPENTRACING_C_WRAPPER_LIBS@ $(top_builddir)/src/libopentracing-c-wrapper_dbg.la
ot_c_wrapper_test_dbg_LDFLAGS = @OPENTRACING_C_WRAPPER_LDFLAGS@

else

             bin_PROGRAMS = ot-c-wrapper-test
ot_c_wrapper_test_SOURCES = opentracing.c test.c util.c
  ot_c_wrapper_test_LDADD = -lstdc++ -lm @OPENTRACING_C_WRAPPER_LIBS@ $(top_builddir)/src/libopentracing-c-wrapper.la
ot_c_wrapper_test_LDFLAGS = @OPENTRACING_C_WRAPPER_LDFLAGS@
endif

CLEANFILES = a.out
##
## Makefile.am ends here
