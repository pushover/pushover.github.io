#!/bin/sh

set -e

export DARCS_DONT_ESCAPE_ISPRINT=1
darcs changes > ChangeLog

echo po/*.po | sed 's,po/\([^ ]*\)\.po,\1,g' >po/LINGUAS
sed -n '/^\(Hint\|Name\)$/,/^[^|]/ s,^| \(.*\)$,_("\1"),p' levels/*/*.level >po/leveltexts.txt

autoreconf -i -s
./configure "$@"

# regenerate the domino.png file, this requires povray
cd data/sources
./dominos_render
./dominos_assemble
mv dominos.png ..
