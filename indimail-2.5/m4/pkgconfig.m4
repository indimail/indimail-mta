# PKG_INSTALLDIR(DIRECTORY)
# -------------------------
# Substitutes the variable pkgconfigdir as the location where a module
# should install pkg-config .pc files. By default the directory is
# $libdir/pkgconfig, but the default can be changed by passing
# DIRECTORY. The user can override through the --with-pkgconfigdir
# parameter.
AC_DEFUN([PKG_INSTALLDIR],
[m4_pushdef([pkg_default], [m4_default([$1], ['${libdir}/pkgconfig'])])
m4_pushdef([pkg_description],
    [pkg-config installation directory @<:@]pkg_default[@:>@])
AC_ARG_WITH([pkgconfigdir],
    [AS_HELP_STRING([--with-pkgconfigdir], pkg_description)],,
    [with_pkgconfigdir=]pkg_default)
AC_SUBST([pkgconfigdir], [$with_pkgconfigdir])
m4_popdef([pkg_default])
m4_popdef([pkg_description])
]) dnl PKG_INSTALLDIR
