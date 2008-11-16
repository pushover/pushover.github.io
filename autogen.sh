#!/bin/sh

set -e

export DARCS_DONT_ESCAPE_ISPRINT=1
darcs changes > ChangeLog

autoreconf -i -s
./configure "$@"
