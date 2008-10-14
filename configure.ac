#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.13)
AC_INIT(pushover, 0.0.1)
AC_CONFIG_AUX_DIR(config_aux)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(src/main.cpp)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# General information.
AC_SUBST(FULLNAME,"Pushover")
AC_SUBST(URL,"http://pushover.sourceforge.net/")
AC_DEFINE_UNQUOTED([FULLNAME],["$FULLNAME"],[Define to the full name of this package.])
AC_DEFINE_UNQUOTED([URL],["$URL"],[Define to the URL of this package's homepage.])

# Compiler options.

# Additional variables.

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
	AC_DEFINE_UNQUOTED(HAVE_SDL,[1],[Define to 1 if you have SDL.]),
	AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, ,
	AC_MSG_WARN([Can't find SDL_image.]))

AC_CHECK_LIB(SDL_mixer, main, ,
	AC_MSG_WARN([Can't find SDL_mixer.]))

AC_CHECK_LIB(z, uncompress, ,
	AC_MSG_ERROR([Can't find libz.]))

AC_CHECK_LIB(png, png_malloc, ,
	AC_MSG_ERROR([Can't find libpng.]))

AC_CHECK_LIB(lua, luaL_newstate, ,
	AC_MSG_ERROR([Can't find liblua.]))

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit malloc realloc memmove memset mkdir setlocale strcasecmp strchr strstr])

# Output.
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
