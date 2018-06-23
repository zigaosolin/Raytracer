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

include $(top_srcdir)/config/Make-rules

if KD_BOOST_TEST

AM_CPPFLAGS += $(kd_with_boost_cflags)
LDADD = $(kd_with_boost_ldflags) $(BOOST_DIR)/lib/libboost_unit_test_framework.a

TESTS             = distance_test kd_tree_test
check_PROGRAMS    = $(TESTS)

distance_test_SOURCES = distance_test.cc
kd_tree_test_SOURCES = kd_tree_test.cc

endif
