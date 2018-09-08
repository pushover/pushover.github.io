PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
DESTDIR ?=
CROSS ?=

CONVERT ?= convert
CXX ?= $(CROSS)g++
CXXFLAGS ?= -Wall -Wextra -g -O2
INSTALL ?= install
LDFLAGS ?=
MSGFMT ?= msgfmt
MSGMERGE ?= msgmerge
PKG_CONFIG ?= $(CROSS)pkg-config
POVRAY ?= povray
XGETTEXT ?= xgettext

PKG_LUA_DEFAULT := $(shell $(PKG_CONFIG) --exists lua-5.2 && echo lua-5.2 || echo lua)
PKG_LUA ?= $(PKG_LUA_DEFAULT)

MSGID_BUGS_ADDRESS := roever@users.sf.net

.DELETE_ON_ERROR:

.PHONY: default
default: all

VERSION := $(shell cat src/version)
FILES_DIST += src/version

PKGS += $(PKG_LUA)
PKGS += SDL_mixer
PKGS += SDL_ttf
PKGS += fribidi
PKGS += libpng
PKGS += sdl
PKGS += zlib

LIBS += -lboost_filesystem
LIBS += -lboost_system

DEFS += -DVERSION='"$(VERSION)"'
DEFS += -DDATADIR='"$(DATADIR)"'

FILES_H := $(wildcard src/pushover/*.h src/pushover/linebreak/*.h src/pushover/sha1/*.hpp)
FILES_DIST += $(FILES_H)
FILES_CPP := $(wildcard src/pushover/*.cpp src/pushover/linebreak/*.c src/pushover/sha1/*.cpp)
FILES_DIST += $(FILES_CPP)
FILES_O := $(patsubst src/pushover/%,_tmp/pushover/%.o,$(FILES_CPP))

.SECONDARY: $(FILES_O)
_tmp/pushover/%.o: src/pushover/% src/version $(FILES_H)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) `$(PKG_CONFIG) --cflags $(PKGS)` $(DEFS) -c -o $@ $<

FILES_BINDIR += pushover

pushover: $(FILES_O)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(FILES_O) `$(PKG_CONFIG) --libs $(PKGS)` $(LIBS)

ASSEMBLER_PKGS += SDL_image
ASSEMBLER_PKGS += libpng
ASSEMBLER_PKGS += sdl

ASSEMBLER_SOURCES := src/dominoes/assembler.cpp src/dominoes/pngsaver.h
FILES_DIST += $(ASSEMBLER_SOURCES)

.SECONDARY: _tmp/dominoes/assembler
_tmp/dominoes/assembler: $(ASSEMBLER_SOURCES)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) `$(PKG_CONFIG) --cflags $(ASSEMBLER_PKGS)` -o $@ $< `$(PKG_CONFIG) --libs $(ASSEMBLER_PKGS)`

DOMINOES_SOURCES := src/dominoes/domino.ini $(wildcard src/dominoes/*.pov)
FILES_DIST += $(DOMINOES_SOURCES)

.SECONDARY: _tmp/dominoes/done
_tmp/dominoes/done: $(DOMINOES_SOURCES)
	mkdir -p $(dir $@)
	$(POVRAY) $<
	touch $@

FILES_DIST += generated/dominoes.png

generated/dominoes.png: _tmp/dominoes/assembler _tmp/dominoes/done
	mkdir -p $(dir $@)
	_tmp/dominoes/assembler $@ 58 2 200 src/dominoes/dominoes.lst

FILES_DATADIR += data/pushover/images/dominoes.png

data/pushover/images/dominoes.png: generated/dominoes.png
	mkdir -p $(dir $@)
	cp $< $@

FILES_IMAGES_SRC := $(wildcard src/images/*.png)
FILES_DIST += $(FILES_IMAGES_SRC)
FILES_DATADIR += $(patsubst src/images/%,data/pushover/images/%,$(FILES_IMAGES_SRC))

data/pushover/images/%: src/images/%
	mkdir -p $(dir $@)
	cp $< $@

FILES_SOUNDS_SRC := $(wildcard src/sounds/*.ogg)
FILES_DIST += $(FILES_SOUNDS_SRC)
FILES_DATADIR += $(patsubst src/sounds/%,data/pushover/sounds/%,$(FILES_SOUNDS_SRC))

data/pushover/sounds/%: src/sounds/%
	mkdir -p $(dir $@)
	cp $< $@

FILES_THEMES_SRC := $(wildcard src/themes/*)
FILES_DIST += $(FILES_THEMES_SRC)
FILES_DATADIR += $(patsubst src/themes/%,data/pushover/themes/%,$(FILES_THEMES_SRC))

data/pushover/themes/%: src/themes/%
	mkdir -p $(dir $@)
	cp $< $@

FILES_PO := $(wildcard src/po/*.po)
FILES_DIST += $(FILES_PO)
FILES_DATADIR += $(patsubst src/po/%.po,data/locale/%/LC_MESSAGES/pushover.mo,$(FILES_PO))

data/locale/%/LC_MESSAGES/pushover.mo: src/po/%.po
	mkdir -p $(dir $@)
	$(MSGFMT) -c -o $@ $<

FILES_DIST += src/description/pushover.appdata.xml
FILES_DATADIR += data/metainfo/pushover.appdata.xml

data/metainfo/pushover.appdata.xml: src/description/pushover.appdata.xml
	mkdir -p $(dir $@)
	cp $< $@

FILES_DIST += src/description/pushover.desktop
FILES_DATADIR += data/applications/pushover.desktop

data/applications/pushover.desktop: src/description/pushover.desktop
	mkdir -p $(dir $@)
	cp $< $@

FILES_DIST += src/description/pushover.ico
FILES_PNG_ICON += generated/pushover_16x16.png
FILES_PNG_ICON += generated/pushover_32x32.png
FILES_PNG_ICON += generated/pushover_48x48.png
FILES_PNG_ICON += generated/pushover_64x64.png
FILES_DIST += $(FILES_PNG_ICON)

generated/pushover_%.png: src/description/pushover.ico
	mkdir -p $(dir $@)
	$(CONVERT) $<[$(shell expr substr $* 1 2 / 16 - 1)] $@

FILES_DATADIR += $(patsubst generated/pushover_%.png,data/icons/hicolor/%/apps/pushover.png,$(FILES_PNG_ICON))

data/icons/hicolor/%/apps/pushover.png: generated/pushover_%.png
	mkdir -p $(dir $@)
	cp $< $@

FILES_DIST += src/description/pushover.6
FILES_DATADIR += data/man/man6/pushover.6.gz

data/man/man6/pushover.6.gz: src/description/pushover.6
	mkdir -p $(dir $@)
	gzip -9n <$< >$@

FILES_DIST += $(wildcard src/levels/*/*)
FILES_LEVELS_SRCDIRS := $(wildcard src/levels/*)
FILES_DATADIR += $(patsubst src/levels/%,data/pushover/levels/%.gz,$(FILES_LEVELS_SRCDIRS))

data/pushover/levels/%.gz: src/levels/%/*.level
	mkdir -p $(dir $@)
	cat src/levels/$*/*.level | gzip -9n >$@

_tmp/po/leveltexts.cpp: src/levels/*/*.level
	mkdir -p $(dir $@)
	sed -n '/^\(Description\|Hint\|Name\|Tutorial\)$$/,/^[^|]/ s,^| \(.*\)$$,_("\1"),p' src/levels/*/*.level | LC_ALL=C sort -u >$@

.PHONY: all
all: $(FILES_BINDIR) $(FILES_DATADIR)

FILES_DIST += $(wildcard src/recordings/*/*.rec src/recordings/finish/*/*.rec)

.PHONY: check
check: all
	./pushover -c src/recordings/finish/*
	./pushover -y src/recordings/fail
	./pushover -x src/recordings/crash
	@echo OK

DIST := pushover-$(VERSION).tgz

.PHONY: dist
dist: $(DIST)

FILES_DIST += AUTHORS
FILES_DIST += LICENSE
FILES_DIST += Makefile
FILES_DIST += NEWS
FILES_DIST += README

$(DIST): $(FILES_DIST)
	rm -rf _tmp/$(basename $@)
	mkdir -p _tmp/$(basename $@)
	tar c $^ | tar x -C _tmp/$(basename $@)
	tar c --owner=0 --group=0 --numeric-owner --mtime='1970-01-01T00:00:00Z' -C _tmp $(basename $@) | gzip -9n >$@
	rm -rf _tmp/$(basename $@)
	tar x -f $@ -C _tmp
	$(MAKE) -C _tmp/$(basename $@) all -j 8 CONVERT='exit 1;' POVRAY='exit 1;'
	$(MAKE) -C _tmp/$(basename $@) check
	$(MAKE) -C _tmp/$(basename $@) install DESTDIR=`pwd`/_tmp/$(basename $@)/_test_destdir
	rm -rf _tmp/$(basename $@)

FILES_INSTALL += $(patsubst %,$(DESTDIR)$(BINDIR)/%,$(FILES_BINDIR))

$(DESTDIR)$(BINDIR)/%: %
	$(INSTALL) -m 755 -d $(dir $@)
	$(INSTALL) -m 755 $< $@

FILES_INSTALL += $(patsubst data/%,$(DESTDIR)$(DATADIR)/%,$(FILES_DATADIR))

$(DESTDIR)$(DATADIR)/%: data/%
	$(INSTALL) -m 755 -d $(dir $@)
	$(INSTALL) -m 644 $< $@

.PHONY: install
install: $(FILES_INSTALL)

.PHONY: clean
clean:
	rm -f $(DIST) $(FILES_BINDIR) $(FILES_DEBUG)
	rm -rf _tmp/ data/
	@echo Not removing generated/

.PHONY: update-po
update-po: _tmp/po/messages.pot
	for PO_FILE in src/po/*.po; do $(MSGMERGE) -Uq --backup=none $$PO_FILE $<; done

_tmp/po/messages.pot: _tmp/po/leveltexts.cpp src/pushover/*.cpp
	mkdir -p $(dir $@)
	$(XGETTEXT) --msgid-bugs-address=$(MSGID_BUGS_ADDRESS) -cTRANSLATORS: -k_ -kN_ -o $@ $^
