/*
 * Copyright 2003-2005 Daniel F. Savarese
 * Copyright 2006-2009 Savarese Software Research Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.savarese.com/software/ApacheLicense-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * This header contains documentation only and is not part of the
 * actual source code.
 */

#include <ssrc/libssrckdtree-packages.h>

__BEGIN_NS_SSRC_SPATIAL

/**
@mainpage

@section licensing Licensing

libssrckdtree is
Copyright 2003-2005 by Daniel F. Savarese and
Copyright 2006-2009 by
<a href="http://www.savarese.com/">Savarese Software Research
Corporation</a> and is licensed under the 
<a href="http://www.savarese.com/software/ApacheLicense-2.0">Apache
License 2.0</a> as described in the files accompanying the source
distribution:
  - LICENSE
  - NOTICE

@section contact Contact

For inquiries about the software see http://www.savarese.com/contact.html

@section overview Overview

libssrckdtree is a C++ header-only template library of spatial data
structures, currently containing only an implementation of a kd-tree.
Additional spatial data structures may be added in the future.

@section build Compiling and Installation Instructions

@subsection dependencies Dependencies

libssrckdtree depends on the GNU development tool chain (g++, gmake,
autoconf, automake, and libtool)&mdash;specifically GCC 4.3.x or
greater is required for compilation on account of dependence on
the 2003 TR1 tuple_size, get, and array extensions.  Otherwise,
the code is platform-independent and should compile on any operating
system with a TR1-capable compiler.  In addition, k-nearest neighbors
search is conditionally compiled based on the availability of the
Boost.Iterator library.

The unit tests require the Boost %Test library (www.boost.org) to compile
and run.  Generation of %test code coverage data requires lcov
(ltp.sourceforge.net/coverage/lcov.php).  Generation of documentation
requires doxygen (www.doxygen.org).  None of these packages is
required to build and install the library or compile code using
the library.

@subsection compiling Compiling

Run the configure script to generate the Makefiles.
<pre>  configure --help</pre>
will list the configuration options.

Also, you may decide to use the <code>--disable-namespace-versioning</code>
option to avoid having to recompile dynamically linked programs after
upgrading the library.

@subsection installation Installation

To install:
<pre>  make install</pre>

If Doxygen is available on your system, you can generate API documentation
with:
<pre>  make apidocs</pre>

If cppunit is available on your system, you can compile and run the unit
tests with:
<pre>  make test</pre>

If lcov is available on your system in addition to cppunit, then you can
generate the code coverage data with:
<pre>  make coverage</pre>

*/

__END_NS_SSRC_SPATIAL
