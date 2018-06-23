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

EXTRA_DIST = LICENSE NOTICE CHANGES doc/images/logoSmall.jpg \
	doc/index.h doc/api config/dox.footer config/dox.header \
	config/libssrckdtree.dox

nobase_include_HEADERS := $(subst $(top_srcdir)/,,$(shell find $(top_srcdir)/ssrc -name "*.h" -not -name libssrckdtree-config.h -not -name config.h -print)) ssrc/libssrckdtree-config.h

check_HEADERS := $(subst $(top_srcdir)/,,$(shell find $(top_srcdir)/tests -name "*.h" -print))

SUBDIRS = tests

TEST_AM_CXXFLAGS = $(AM_CXXFLAGS) -Wno-non-virtual-dtor -Wno-overloaded-virtual -fprofile-arcs -ftest-coverage
TEST_CXXFLAGS := $(patsubst -O%,,$(CXXFLAGS))
TEST_CFLAGS := $(patsubst -O%,,$(CFLAGS))

LCOV_DIRS = ssrc tests

test:
	make all-recursive AM_CXXFLAGS='$(TEST_AM_CXXFLAGS)' CXXFLAGS='$(TEST_CXXFLAGS)' CFLAGS='$(TEST_CFLAGS)'
	find $(LCOV_DIRS) -name "*.gcda*" -print -o -name AllTests.info -print | xargs rm -f
	(cd tests/ && $(MAKE) check AM_CXXFLAGS='$(TEST_AM_CXXFLAGS)' CXXFLAGS='$(TEST_CXXFLAGS)' CFLAGS='$(TEST_CFLAGS)')

coverage: test
	geninfo .
	find $(LCOV_DIRS) -name "*.info" -a -not -name AllTests.info -exec \
		lcov -r {} "/usr/include/*" -r {} "/net/*" -o {}.r ';'
	find $(LCOV_DIRS) -name "*.info.r" -exec \
		sh -c 'test=`basename {} .gcda.info.r`; perl -pi -e "s/^TN:/TN:$$test/" {}' ';'
	find $(LCOV_DIRS) -name "*.info.r" -print | sed 's/\(.*\)/ -a \1/' | \
		xargs lcov -o tests/AllTests.info
	genhtml -t "libssrckdtree $(VERSION) C++ Unit Tests " \
		--show-details -o $(top_srcdir)/doc/tests tests/AllTests.info
	for file in $$(find $(top_srcdir)/doc/tests -name "*.html" -print) ; do \
		imgdir=$(top_srcdir)/doc/tests ; \
		imgdir="`echo $$imgdir | sed -e 's/\\//\\\\\\//g'`" ; \
		imgdir="`echo $$file | sed -e \"s/$$imgdir//\"`" ; \
		imgdir="`echo $$imgdir | sed -e 's#^\./##' -e 's#[^/]\+/#../#g'`" ; \
		imgdir="`dirname $${imgdir} | sed -e 's#^\.\?/##' -e 's#\.\.$$#../#'`" ; \
		perl -pi \
			-e "s/<title>LCOV - /<title>/;" \
			-e "s#<tr><td class=\"versionInfo\">.*?</td></tr>##;" \
			-e "s#<td class=\"title\">.*?</td>#<td class=\"title\">libssrckdtree $(VERSION) C++ Unit Test Coverage</td>#;" "$$file" ; \
	done

package: apidocs dist
dist: $(top_srcdir)/doc/api

$(top_srcdir)/doc/api:
	mkdir $(top_srcdir)/doc/api

if KD_DOXYGEN

$(top_srcdir)/doc/api/%.jpg: doc/images/%.jpg $(top_srcdir)/doc/api
	cp $< $@

apidocs: $(top_srcdir)/doc/api/logoSmall.jpg
	(incdir="`pwd | sed -e 's/\\//\\\\\\//g'`" ; \
        cd $(top_srcdir); \
	sed -e "s/^\(INCLUDE_PATH.*=.*\)/\1 $${incdir}/" \
		-e "s/^\(PROJECT_NUMBER.*=.*\)/PROJECT_NUMBER = $(VERSION)/" \
		config/libssrckdtree.dox | $(DOXYGEN) -)
	find $(top_srcdir)/doc/api -type f -name "*.html" -print | \
		xargs perl -pi -e \
		's/\b$(kd_namespace)/ssrc/g'
	find $(top_srcdir)/doc/api -type f \
		-regex '.*\.\(md5\|map\|dot\)' -print | xargs rm -f
endif