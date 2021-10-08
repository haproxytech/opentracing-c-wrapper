dnl Process this file with autoconf to produce a configure script.
dnl
dnl configure.ac - Miroslav Zagorac <mzagorac@haproxy.com>

AC_PREREQ([2.69])
AC_COPYRIGHT([Copyright (C) 2020 HAProxy Technologies])
AC_INIT(m4_esyscmd_s([basename ${PWD}]), m4_esyscmd_s([awk '/Package/ { print $3 }' VERSION]), [mzagorac@haproxy.com])
AC_CONFIG_MACRO_DIR([m4])
AX_VARIABLE_SET([LIB_VERSION], m4_esyscmd_s([awk '/Library/ { print $3 }' VERSION]))

echo "*** configuring for ${PACKAGE_NAME} v${PACKAGE_VERSION}, lib v${LIB_VERSION} ***"

AC_PREFIX_DEFAULT([/usr])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.13])
AX_VARIABLES_INIT
AM_SILENT_RULES([yes])
AX_HOST

dnl Check for source directory.
dnl
AC_CONFIG_SRCDIR([src/tracer.cpp])
AC_CONFIG_HEADERS([config/config.h])

AX_PROG_PKGCONFIG
PKG_INSTALLDIR

dnl Set the compiler, preprocessor, and file extensions.
dnl
AC_LANG([C])

dnl Checking command line options.
dnl
AX_ENABLE_DEBUG
AX_ENABLE_GPROF
AX_ENABLE_THREADS
dnl
dnl Misc
dnl
AX_WITH_OPENTRACING

dnl Checks for programs.
dnl
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

dnl Initialize libtool
dnl
LT_INIT

dnl Checks for header files.
dnl
AC_CHECK_HEADERS([fcntl.h inttypes.h malloc.h stdint.h stdlib.h string.h sys/time.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT8_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

dnl Checks for libraries.
dnl

dnl Checks for library functions.
dnl
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime gettimeofday memmove memset strchr strdup strndup])
AC_CHECK_FUNCS([mallinfo])

dnl Set the compiler flags
dnl
AX_PROG_CC_SET([])
AX_PROG_CXX_SET([])

AM_CONDITIONAL([WANT_DEBUG], [test "${enable_debug}" = "yes"])

AX_VARIABLES_SET

AX_VARIABLE_SET([OPENTRACING_C_WRAPPER_CPPFLAGS], [ ${OPENTRACING_CPPFLAGS} ${THREADS_CPPFLAGS} ])
AX_VARIABLE_SET([OPENTRACING_C_WRAPPER_CFLAGS],   [ ${OPENTRACING_CFLAGS}   ${THREADS_CFLAGS}   ])
AX_VARIABLE_SET([OPENTRACING_C_WRAPPER_CXXFLAGS], [ ${OPENTRACING_CXXFLAGS} ${THREADS_CXXFLAGS} ])
AX_VARIABLE_SET([OPENTRACING_C_WRAPPER_LDFLAGS],  [ ${OPENTRACING_LDFLAGS}  ${THREADS_LDFLAGS}  ])
AX_VARIABLE_SET([OPENTRACING_C_WRAPPER_LIBS],     [ ${OPENTRACING_LIBS}     ${THREADS_LIBS}     ])

AC_SUBST([OPENTRACING_C_WRAPPER_CPPFLAGS])
AC_SUBST([OPENTRACING_C_WRAPPER_CFLAGS])
AC_SUBST([OPENTRACING_C_WRAPPER_CXXFLAGS])
AC_SUBST([OPENTRACING_C_WRAPPER_LDFLAGS])
AC_SUBST([OPENTRACING_C_WRAPPER_LIBS])

dnl version.h
dnl
CONFIGURE_OPTIONS="${ac_configure_args}"
AC_SUBST([DATE])
AC_SUBST([PACKAGE_VERSION])
AC_SUBST([LIB_VERSION])
AC_SUBST([CONFIGURE_OPTIONS])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile include/version.h opentracing-c-wrapper.pc opentracing-c-wrapper_dbg.pc])
AC_OUTPUT

AX_SHOW_CONFIG


AH_TOP([
#ifndef _COMMON_CONFIG_H
#define _COMMON_CONFIG_H
])

AH_BOTTOM([
#endif /* _COMMON_CONFIG_H */
])
