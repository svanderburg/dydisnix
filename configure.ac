AC_PREREQ(2.61)
AC_INIT([dydisnix], m4_esyscmd([echo -n $(cat ./version)$VERSION_SUFFIX]))
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.9 tar-pax -Wall -Werror -Wno-extra-portability])

# Checks for installed programs
AC_DEFUN([NEED_PROG],
[
AC_PATH_PROG($1, $2)
if test -z "$$1"; then
    AC_MSG_ERROR([$2 is required])
fi
])

# Checks for programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG
NEED_PROG(bash, bash)
NEED_PROG(getopt, getopt)

AC_PATH_PROGS(readlink, [greadlink readlink], [na])
if test "$readlink" = na; then
    AC_MSG_ERROR([We need a GNU compatible readlink command, i.e. greadlink or readlink needed])
fi

# Checks for libxml2 libraries
LIBXML2_REQUIRED=2.5.10
PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= $LIBXML2_REQUIRED)
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)	

# Checks for glib libraries
GLIB_REQUIRED=2.20.0
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# Checks for Disnix libraries
DISNIX_REQUIRED=0.9
PKG_CHECK_MODULES(DISNIX, disnix >= $DISNIX_REQUIRED)
DISNIX_PREFIX=`pkg-config --variable=prefix disnix`
AC_SUBST(DISNIX_PREFIX)
AC_SUBST(DISNIX_CFLAGS)
AC_SUBST(DISNIX_LIBS)

# State should be stored in /nix/var, unless the user overrides it explicitly.
test "$localstatedir" = '${prefix}/var' && localstatedir=/nix/var

# Output
AC_CONFIG_FILES([
Makefile
scripts/dydisnix-env
scripts/dydisnix-gendist
scripts/dydisnix-self-adapt
scripts/dydisnix-augment-infra
scripts/dydisnix-xml
scripts/dydisnix-generate-services-docs
scripts/Makefile
src/Makefile
src/dydisnix.pc
src/libdymain/Makefile
src/libgraph/Makefile
src/filter-buildable/Makefile
src/divide/Makefile
src/multiwaycut/Makefile
src/minsetcover/Makefile
src/graphcol/Makefile
src/idassign/Makefile
src/libdistribution/Makefile
src/libservices/Makefile
src/libinfrastructure2/Makefile
src/visualize-services/Makefile
src/document-services/Makefile
src/previous-distribution/Makefile
nix/Makefile
xsl/Makefile
css/Makefile
])
AC_OUTPUT
