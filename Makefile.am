AM_CXXFLAGS = -Wall -DTOP_DATADIR=\"$(pkgdatadir)\"
SUBDIRS = m4
ACLOCAL_AMFLAGS = -I m4

PACKAGE = pushover

bin_PROGRAMS = pushover

pushover_LDFLAGS = $(LIBINTL)
pushover_SOURCES =                  \
  src/ant.cpp src/ant.h \
  src/decompress.cpp src/decompress.h \
  src/graphics.cpp src/graphics.h \
  src/graphicsn.cpp src/graphicsn.h \
  src/graphicso.cpp src/graphicso.h \
  src/level.cpp src/level.h \
  src/main.cpp


