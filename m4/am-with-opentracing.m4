dnl am-with-opentracing.m4 by Miroslav Zagorac <mzagorac@haproxy.com>
dnl
AC_DEFUN([AX_WITH_OPENTRACING], [
	AC_ARG_WITH([opentracing],
		[AS_HELP_STRING([--with-opentracing@<:@=DIR@:>@], [use OPENTRACING library @<:@default=yes@:>@])],
		[with_opentracing="${withval}"],
		[with_opentracing=yes]
	)

	AX_CHECK_NOEXCEPT([])

	if test "${with_opentracing}" != "no"; then
		HAVE_OPENTRACING=
		OPENTRACING_CFLAGS=
		OPENTRACING_CXXFLAGS=
		OPENTRACING_CPPFLAGS=
		OPENTRACING_LDFLAGS=
		OPENTRACING_LIBS="-lopentracing"

		if test -n "${with_opentracing}" -a "${with_opentracing}" != "yes" -a "${with_opentracing}" != "yes"; then
			OPENTRACING_CPPFLAGS="-I${with_opentracing}/include"

			if test "${with_opentracing}" != "/usr"; then
				if test "`uname`" = "Linux"; then
					OPENTRACING_LDFLAGS="-L${with_opentracing}/lib -Wl,--rpath,${with_opentracing}/lib"
				else
					OPENTRACING_LDFLAGS="-L${with_opentracing}/lib -R${with_opentracing}/lib"
				fi
			fi
		fi

		AX_VARIABLES_STORE

		LIBS="${LDFLAGS} ${OPENTRACING_LIBS}"
		LDFLAGS="${LDFLAGS} ${OPENTRACING_LDFLAGS}"
		CFLAGS="${CFLAGS} ${OPENTRACING_CFLAGS}"
		CXXFLAGS="${CXXFLAGS} ${OPENTRACING_CXXFLAGS}"
		CPPFLAGS="${CPPFLAGS} ${OPENTRACING_CPPFLAGS}"

		AC_LANG_PUSH([C++])
		AC_LINK_IFELSE([
			AC_LANG_PROGRAM(
				[[#include <opentracing/dynamic_load.h>]],
				[[std::string e;] [auto a = opentracing::DynamicallyLoadTracingLibrary("", e);]]
			)],
			[],
			[AC_MSG_ERROR([OPENTRACING library not found])]
		)
		AC_CHECK_HEADER([opentracing/version.h], [], [AC_MSG_ERROR([OPENTRACING library headers not found])])
		AC_LANG_POP([C++])

		HAVE_OPENTRACING=yes

		AX_VARIABLES_RESTORE

		AC_MSG_NOTICE([OPENTRACING environment variables:])
		AC_MSG_NOTICE([  OPENTRACING_CFLAGS=${OPENTRACING_CFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_CXXFLAGS=${OPENTRACING_CXXFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_CPPFLAGS=${OPENTRACING_CPPFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_LDFLAGS=${OPENTRACING_LDFLAGS}])
		AC_MSG_NOTICE([  OPENTRACING_LIBS=${OPENTRACING_LIBS}])

		AC_SUBST([OPENTRACING_CFLAGS])
		AC_SUBST([OPENTRACING_CXXFLAGS])
		AC_SUBST([OPENTRACING_CPPFLAGS])
		AC_SUBST([OPENTRACING_LDFLAGS])
		AC_SUBST([OPENTRACING_LIBS])
	fi
])
