## Copyright 2003-2005 Daniel F. Savarese
## Copyright 2006-2009 Savarese Software Research Corporation
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.savarese.com/software/ApacheLicense-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

AM_CPPFLAGS = -I$(top_srcdir)
AM_LDFLAGS =

# This gets rid of dangerous -I. and -I$(srcdir) that break builds when
# project headers alias system headers.
DEFAULT_INCLUDES = -I$(top_builddir)

if KD_USING_GCC
KD_CXX_FLAGS    =
KD_CXX_WARNINGS = -pedantic -Wall -Wno-long-long -Winline \
         -Woverloaded-virtual -Wsign-promo
KD_DEBUG_FLAGS  = -g -DKD_DEBUG
else
KD_DEBUG_FLAGS  = -DKD_DEBUG
endif

if KD_DEBUG
AM_CXXFLAGS = $(KD_CXX_FLAGS) $(KD_CXX_WARNINGS) $(KD_DEBUG_FLAGS)
else
AM_CXXFLAGS = $(KD_CXX_FLAGS) $(KD_CXX_WARNINGS)
endif
