#!/bin/sh

set -e

export DARCS_DONT_ESCAPE_ISPRINT=1
darcs changes > ChangeLog

echo po/*.po | sed 's,po/\([^ ]*\)\.po,\1,g' >po/LINGUAS
sed -n '/^\(Hint\|Name\|Description\)$/,/^[^|]/ s,^| \(.*\)$,_("\1"),p' `find -name '*.level' | grep -v "levels/tests"` >po/leveltexts.txt

autoreconf -i -s
./configure "$@"

# regenerate the domino.png file, this requires povray
cd data/sources
sh ./dominos_render
sh ./dominos_assemble
mv dominos.png ..
