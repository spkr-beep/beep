# sb-prog-doxygen.m4 - check for doxygen program, but let user override it -*- Autoconf -*-
# serial 2
dnl
dnl
dnl
AC_DEFUN([SB_PROG_DOXYGEN], [dnl
SB_PROG([DOXYGEN], [doxygen])dnl
m4_ifblank([$1], [dnl
  doxygen_version_sufficient=yes
], [dnl
  doxygen_version_sufficient=no
  AM_COND_IF([HAVE_TOOL_DOXYGEN], [dnl
    AC_MSG_CHECKING([doxygen version])
    doxygen_version="$(${DOXYGEN} -v)"
    AC_MSG_RESULT([${doxygen_version}])
    AS_VERSION_COMPARE([${doxygen_version}], [$1], [dnl
      doxygen_version_sufficient=no
    ], [dnl
      doxygen_version_sufficient=yes
    ], [dnl
      doxygen_version_sufficient=yes
    ])
  ])
  AC_MSG_CHECKING([if doxygen version is sufficient for our Doxyfile])
  AC_MSG_RESULT([${doxygen_version_sufficient}])
])dnl
AM_CONDITIONAL([HAVE_TOOL_VERSION_DOXYGEN],
               [test "x$doxygen_version_sufficient" = xyes])
])dnl
dnl
dnl
dnl
dnl Local Variables:
dnl indent-tabs-mode: nil
dnl End:
