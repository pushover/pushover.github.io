#!/bin/bash

echo "$0: running some preparations ..."
(
	version="`cat VERSION`"
	extra_dist="`cd m4/ && ls -1 *.m4 | tr '\n' ' '`"
	all_linguas="`echo -n "$linguas" | tr '\n' ' '`"

	echo "Creating m4/Makefile.am"
	echo "EXTRA_DIST = $extra_dist" >m4/Makefile.am

	echo "Updating configure.ac"
	sed \
		-e 's/\(ALL_LINGUAS="\)[^"]*/\1'"$all_linguas"'/' \
		-e 's/\(AC_INIT([^,]*,\)[^,]*\([,)].*\)/\1'"[$version]"'\2/' \
		-i configure.ac
)

echo "$0: running aclocal ..."
aclocal || exit 1

echo "$0: running libtoolize ..."
libtoolize --force || exit 1

echo "$0: running autoconf ..."
autoconf || exit 1

echo "$0: running autoheader ..."
autoheader || exit 1

echo "$0: running automake ..."
automake --add-missing || exit 1

echo "$0: running configure ..."
./configure $@ || exit 1
