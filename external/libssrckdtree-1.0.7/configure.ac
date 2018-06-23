dnl Copyright 2003-2005 Daniel F. Savarese
dnl Copyright 2006-2009 Savarese Software Research Corporation
dnl
dnl Licensed under the Apache License, Version 2.0 (the "License");
dnl you may not use this file except in compliance with the License.
dnl You may obtain a copy of the License at
dnl
dnl     http://www.savarese.com/software/ApacheLicense-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.

AC_INIT(libssrckdtree, 1.0.7)
AC_PREREQ(2.50)
AC_CONFIG_AUX_DIR(config)
AC_CANONICAL_HOST
AC_CANONICAL_SYSTEM
AC_COPYRIGHT([
Copyright 2003-2005 Daniel F. Savarese
Copyright 2006-2009 Savarese Software Research Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.savarese.com/software/ApacheLicense-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
])

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl
dnl Define custom functions
dnl

dnl
dnl Checks for programs.
dnl

AC_PROG_CXX
AM_CONDITIONAL(KD_USING_GCC, [test "$ac_cv_prog_gxx" = yes])

dnl
dnl Do not build shared libraries by default.  This is in order
dnl to speed up the build process; otherwise both shared and
dnl static libraries get built simultaneously, requiring two
dnl compiles for every .o.
dnl 

AC_DISABLE_SHARED

AC_PROG_LN_S

dnl
dnl Disable libtool fortran checks
dnl

F77=no

AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)
AC_PROG_INSTALL()
AC_PROG_MAKE_SET

dnl
dnl Adds options
dnl

AC_ARG_ENABLE(debug,
 AC_HELP_STRING([--enable-debug],
                [enable compilation with debug flags [default=no]]),
 [ ],
 [ enable_debug=no; CFLAGS=`echo "$CFLAGS" | sed -e 's/-g//g'`;
   CXXFLAGS=`echo "$CXXFLAGS" | sed -e 's/-g//g'`; ])
if test "$enable_debug" = yes; then
   AC_DEFINE(DEBUG, 1, [ Enable debug code. ])
   CFLAGS=-g
   CXXFLAGS=-g
fi
AM_CONDITIONAL(KD_DEBUG, [test "$enable_debug" = yes])

AC_ARG_ENABLE(namespace-versioning,
 AC_HELP_STRING([--disable-namespace-versioning],
                [disable the appending of a version string to the top-level namespace [[default=no]]]),
 [ ],
 [ enable_namespace_versioning=yes; ])

dnl
dnl Set default language for checks to C++
dnl

AC_LANG_CPLUSPLUS

if test "$ac_cv_prog_gcc" = yes; then
    CFLAGS="$CFLAGS -std=c99 -pipe -fno-gnu-keywords"
    CXXFLAGS="$CXXFLAGS -std=c++0x -pipe -fno-gnu-keywords"
    AC_LIBTOOL_COMPILER_OPTION(
      [checking if compiler recognizes -fvisibility-inlines-hidden],
      [kd_flag], [-fvisibility-inlines-hidden], ,
      [ CXXFLAGS="$CXXFLAGS -fvisibility-inlines-hidden" ])
fi

AC_WITH_BOOST_TEST
AM_CONDITIONAL(KD_BOOST_TEST, [test "$with_boost"])

AC_WITH_DOXYGEN
AM_CONDITIONAL(KD_DOXYGEN, [test "$with_doxygen"])

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl

AC_CXX_NAMESPACES
if test "$ac_cv_cxx_namespaces" = no; then
AC_MSG_ERROR([
    The source code can be compiled only with a C++ compiler that
    supports namespaces.  Your compiler does not support namespaces.
    Please try again with a compiler that supports namespaces, such
    as the latest version of GCC at http://gcc.gnu.org/

])
fi
AC_TR1_TUPLE
AC_STD_TUPLE

dnl
dnl Output commands and files
dnl 

kd_build="_v`echo $VERSION | sed -e 's/\./_/g'`"

if test "$enable_namespace_versioning" = no; then
  kd_namespace="kd"
else
  kd_namespace="kd${kd_build}"
fi

AC_SUBST(kd_namespace)

AH_BOTTOM(
[
#if !defined(NS_KD)
#  define NS_KD $kd_namespace
#endif

#if !defined(KD_DEFINE_NAMESPACE)
#  define KD_DEFINE_NAMESPACE(name) NS_KD::name
#endif

#if !defined(NS_KD_DECL_PREFIX)
#  define NS_KD_DECL_PREFIX \
namespace $kd_namespace {
#endif

#if !defined(NS_KD_DECL_SUFFIX)
#  define NS_KD_DECL_SUFFIX \
}
#endif

#if defined(LIBSSRCKDTREE_HAVE_STD_TUPLE)
#  define NS_TR1 std
#elif defined(LIBSSRCKDTREE_HAVE_TR1_TUPLE)
#  define NS_TR1 std::tr1
#endif
])

PACKAGEDIR=ssrc

MOVE_CONFIG_HEADER='
moveConfigHeader() {
  srcfile="$PACKAGE-config.h"
  if test -f "$srcfile"; then
    test -d $PACKAGEDIR || mkdir $PACKAGEDIR
    destfile="$PACKAGEDIR/$srcfile"
    if diff -q "$srcfile" "$destfile" > /dev/null 2>&1; then
      echo "$srcfile and $destfile are the same."
      echo "Removing $srcfile."
      rm -f "$srcfile"
    else
      echo "$srcfile and $destfile differ."
      echo "Overwriting $destfile with $srcfile."
      sed -e "s/\$kd_build/$kd_build/g" \
          -e "s/\$kd_namespace/$kd_namespace/g" \
          "$srcfile" > "$destfile"
      rm -f "$srcfile"
    fi
  fi
}'

AC_CONFIG_COMMANDS([ moveConfigHeader ],
	[ eval "$MOVE_CONFIG_HEADER"; ],
	[ MOVE_CONFIG_HEADER='$MOVE_CONFIG_HEADER' ])

AC_CONFIG_FILES([
Makefile
tests/Makefile
tests/spatial/Makefile
]
,
[
 eval "$MOVE_CONFIG_HEADER";
 moveConfigHeader
]
)

AC_OUTPUT

AC_CREATE_PREFIX_CONFIG_H

[
 #
 # Moves the config header to the source include directory.
 #

 eval "$MOVE_CONFIG_HEADER"; moveConfigHeader
]
