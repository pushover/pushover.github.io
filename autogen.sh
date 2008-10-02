#!/bin/sh

set -e
autoreconf -i -s
./configure "$@"
