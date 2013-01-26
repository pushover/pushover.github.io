#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.13])
AC_INIT([pushover], [0.0.4])
AC_CONFIG_AUX_DIR([config_aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

# General information.
AC_SUBST([FULLNAME], ["Pushover"])
AC_SUBST([URL],      ["http://pushover.sourceforge.net/"])
AC_DEFINE_UNQUOTED([FULLNAME], ["$FULLNAME"], [Define to the full name of this package.])
AC_DEFINE_UNQUOTED([URL],      ["$URL"],      [Define to the URL of this package's homepage.])

# Compiler options.

# Additional variables.

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

# Checks for i18n.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.16.1])
LIBS="$LIBS $LIBINTL"

# Checks for libraries.
AC_MSG_CHECKING([for Lua package name])
PKG_CHECK_EXISTS([lua5.1], [LUA="lua5.1"],
    [PKG_CHECK_EXISTS([lua-5.2], [LUA="lua-5.2"], [LUA="lua"])])
AC_MSG_RESULT([$LUA])

PKG_CHECK_EXISTS([zlib], [ZLIB="zlib >= 1.2.3"],
    [AC_CHECK_LIB([z], [uncompress], [], [AC_MSG_ERROR([Unable to find zlib.])])])

PKG_CHECK_EXISTS([SDL_mixer], [SDL_MIXER="SDL_mixer >= 1.2.0"],
    [AC_CHECK_LIB([SDL_mixer], [main], [], [AC_MSG_ERROR([Unable to find SDL_mixer.])])])

PKG_CHECK_EXISTS([SDL_ttf], [SDL_TTF="SDL_ttf >= 2.0.0"],
    [AC_CHECK_LIB([SDL_ttf], [main], [], [AC_MSG_ERROR([Unable to find SDL_ttf.])])])

PKG_CHECK_MODULES([DEPS], [$ZLIB libpng >= 1.2.40 sdl >= 1.2.0 $SDL_MIXER $SDL_TTF $LUA >= 5.1])
CPPFLAGS="$CPPFLAGS $DEPS_CFLAGS"
LIBS="$LIBS $DEPS_LIBS"

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_BIGENDIAN
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit malloc realloc memmove memset mkdir setlocale strcasecmp strchr strstr])

# Output.
AC_CONFIG_FILES([Makefile po/Makefile.in])
AC_OUTPUT
