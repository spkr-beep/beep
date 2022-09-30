#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([beep],
        [1.4.13],
        [https://github.com/spkr-beep/beep/issues],
        [beep],
        [https://github.com/spkr-beep/beep])
AC_CONFIG_AUX_DIR([auto-aux])
AC_CONFIG_MACRO_DIR([auto-m4])
AC_CONFIG_SRCDIR([beep-log.h])
AC_CONFIG_HEADERS([auto-config.h])

AM_INIT_AUTOMAKE([
  1.16
  foreign
  serial-tests
  subdir-objects
  -Wall
  -Werror
])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_SED

dnl Catch our project specific (*s*pkr-*b*beep) macros
m4_pattern_forbid([^SB_])dnl

dnl While we are still using the old run-tests script, check for tput
dnl and /bin/bash

SB_PROG([BASH],     [bash])
SB_PROG([DIFF],     [diff])
SB_PROG([DOXYGEN],  [doxygen])
SB_PROG([PANDOC],   [pandoc])
SB_PROG([PYTHON3],  [python3])
SB_PROG([TPUT],     [tput])

# TODO: Use the proper C standard for compiling.
# TODO: Produce as many warnings and errors as possible when compiling.

# Checks for libraries.
saved_LIBS="$LIBS"
LIBS=""
AC_CHECK_LIB([m], [lrint])
AC_SUBST([M_LIBS], ["$LIBS"])
LIBS="$saved_LIBS"

# Checks for header files.
AC_CHECK_HEADERS(m4_flatten([
  fcntl.h
  limits.h
  stddef.h
  stdint.h
  stdlib.h
  string.h
  sys/ioctl.h
  sys/time.h
  unistd.h
]))

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS(m4_flatten([
  clock_gettime
  memset
  strdup
  strerror
  strrchr
  strtol
  strtoul
]))

AC_SUBST([DEFAULT_FREQ],   [440])
AC_SUBST([DEFAULT_LENGTH], [200])
AC_SUBST([DEFAULT_DELAY],  [100])

SB_SUMMARY_VAR([DEFAULT_FREQ])
SB_SUMMARY_VAR([DEFAULT_LENGTH])
SB_SUMMARY_VAR([DEFAULT_DELAY])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
SB_SUMMARY

dnl Local Variables:
dnl indent-tabs-mode: nil
dnl End:
