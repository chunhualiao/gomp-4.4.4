AC_PREREQ(2.59)
AC_INIT([GNU OpenMP Runtime Library], 1.0,,[libgomp])
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([1.9.0 foreign -Wall -Wno-portability -Wno-override])
AC_PROG_CC
AC_SUBST(CFLAGS)


save_CFLAGS="$CFLAGS"
if test "x$GCC" = "xyes"; then
    XCFLAGS="$XCFLAGS -Wall -Werror"
fi

# Find other programs we need.
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib, ranlib-not-found-in-path-error)
AC_PATH_PROG(PERL, perl, perl-not-found-in-path-error)
AC_PROG_MAKE_SET
AC_PROG_INSTALL


# Configure libtool
#AM_PROG_LIBTOOL
AC_PROG_LIBTOOL
AC_SUBST(enable_shared)
AC_SUBST(enable_static)

# We need gfortran to compile parts of the library
# We can't use AC_PROG_FC because it expects a fully working gfortran.
#AC_PROG_FC(gfortran)
FC="$GFORTRAN"
AC_PROG_FC(gfortran)
FCFLAGS="$FCFLAGS -Wall"

# For libtool versioning info, format is CURRENT:REVISION:AGE
libtool_VERSION=1:0:0
AC_SUBST(libtool_VERSION)


# Check header files.
AC_STDC_HEADERS
AC_HEADER_TIME
ACX_HEADER_STRING
AC_CHECK_HEADERS(unistd.h semaphore.h sys/loadavg.h sys/time.h sys/time.h)

GCC_HEADER_STDINT(gstdint.h)

# Check to see if -pthread or -lpthread is needed.  Prefer the former.
# In case the pthread.h system header is not found, this test will fail.
XPCFLAGS=""
CFLAGS="$CFLAGS -pthread"
AC_LINK_IFELSE(
 [AC_LANG_PROGRAM(
  [#include <pthread.h>
   void *g(void *d) { return NULL; }],
  [pthread_t t; pthread_create(&t,NULL,g,NULL);])],
 [XPCFLAGS=" -Wc,-pthread"],
 [CFLAGS="$save_CFLAGS" LIBS="-lpthread $LIBS"
  AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
    [#include <pthread.h>
     void *g(void *d) { return NULL; }],
    [pthread_t t; pthread_create(&t,NULL,g,NULL);])],
   [],
   [AC_MSG_ERROR([Pthreads are required to build libgomp])])])

# Check for functions needed.
AC_CHECK_FUNCS(getloadavg clock_gettime strtoull)

# Check for broken semaphore implementation on darwin.
# sem_init returns: sem_init error: Function not implemented.
case "$host" in
  *-darwin*)
    AC_DEFINE(HAVE_BROKEN_POSIX_SEMAPHORES, 1,
        Define if the POSIX Semaphores do not work on your system.)
    ;;
esac

GCC_LINUX_FUTEX(:)

# Check for pthread_{,attr_}[sg]etaffinity_np.
AC_LINK_IFELSE(
 [AC_LANG_PROGRAM(
  [#define _GNU_SOURCE
   #include <pthread.h>],
  [cpu_set_t cpuset;
   pthread_attr_t attr;
   pthread_getaffinity_np (pthread_self (), sizeof (cpu_set_t), &cpuset);
   if (CPU_ISSET (0, &cpuset))
     CPU_SET (1, &cpuset);
   else
     CPU_ZERO (&cpuset);
   pthread_setaffinity_np (pthread_self (), sizeof (cpu_set_t), &cpuset);
   pthread_attr_init (&attr);
   pthread_attr_getaffinity_np (&attr, sizeof (cpu_set_t), &cpuset);
   pthread_attr_setaffinity_np (&attr, sizeof (cpu_set_t), &cpuset);])],
  AC_DEFINE(HAVE_PTHREAD_AFFINITY_NP, 1,
[       Define if pthread_{,attr_}{g,s}etaffinity_np is supported.]))

# At least for glibc, clock_gettime is in librt.  But don't pull that
# in if it still doesn't give us the function we want.
if test $ac_cv_func_clock_gettime = no; then
  AC_CHECK_LIB(rt, clock_gettime,
    [LIBS="-lrt $LIBS"
     AC_DEFINE(HAVE_CLOCK_GETTIME, 1,
               [Define to 1 if you have the `clock_gettime' function.])])
fi

# See if we support thread-local storage.
GCC_CHECK_TLS

# See what sort of export controls are availible.
LIBGOMP_CHECK_ATTRIBUTE_VISIBILITY
LIBGOMP_CHECK_ATTRIBUTE_DLLEXPORT
LIBGOMP_CHECK_ATTRIBUTE_ALIAS
LIBGOMP_ENABLE_SYMVERS

if test $enable_symvers = gnu; then
  AC_DEFINE(LIBGOMP_GNU_SYMBOL_VERSIONING, 1,
            [Define to 1 if GNU symbol versioning is used for libgomp.])
fi

# Get target configury.
. ${srcdir}/configure.tgt
CFLAGS="$save_CFLAGS $XCFLAGS"

# Check for __sync_val_compare_and_swap, but only after the target has
# had a chance to set XCFLAGS.
LIBGOMP_CHECK_SYNC_BUILTINS

XCFLAGS="$XCFLAGS$XPCFLAGS"

AC_SUBST(config_path)
AC_SUBST(XCFLAGS)
AC_SUBST(XLDFLAGS)

# Cleanup and exit.
CFLAGS="$save_CFLAGS"
AC_CACHE_SAVE

# Set up the set of libraries that we need to link against for libgomp.
# Note that the GOMP_SELF_SPEC in gcc.c will force -pthread for -fopenmp,
# which will force linkage against -lpthread (or equivalent for the system).
# That's not 100% ideal, but about the best we can do easily.
if test $enable_shared = yes; then
  link_gomp="-lgomp %{static: $LIBS}"
else
  link_gomp="-lgomp $LIBS"
fi
AC_SUBST(link_gomp)

AM_CONDITIONAL([USE_FORTRAN], [test "$ac_cv_fc_compiler_gnu" = yes])

# ??? 2006-01-24: Paulo committed to asking autoconf folk to document
# and export AC_COMPUTE_INT.  If that happens, then we'll need to remove
# the underscore here and update the PREREQ.  If it doesn't, then we'll
# need to copy this macro to our acinclude.m4.
save_CFLAGS="$CFLAGS"
for i in $config_path; do
  if test -f $srcdir/config/$i/omp-lock.h; then
    CFLAGS="$CFLAGS -include confdefs.h -include $srcdir/config/$i/omp-lock.h"
    break
  fi
done

_AC_COMPUTE_INT([sizeof (omp_lock_t)], [OMP_LOCK_SIZE],,
  [AC_MSG_ERROR([unsupported system, cannot find sizeof (omp_lock_t)])])
_AC_COMPUTE_INT([__alignof (omp_lock_t)], [OMP_LOCK_ALIGN])
_AC_COMPUTE_INT([sizeof (omp_nest_lock_t)], [OMP_NEST_LOCK_SIZE])
_AC_COMPUTE_INT([__alignof (omp_nest_lock_t)], [OMP_NEST_LOCK_ALIGN])
_AC_COMPUTE_INT([sizeof (omp_lock_25_t)], [OMP_LOCK_25_SIZE],,
  [AC_MSG_ERROR([unsupported system, cannot find sizeof (omp_lock_25_t)])])
_AC_COMPUTE_INT([__alignof (omp_lock_25_t)], [OMP_LOCK_25_ALIGN])
_AC_COMPUTE_INT([sizeof (omp_nest_lock_25_t)], [OMP_NEST_LOCK_25_SIZE])
_AC_COMPUTE_INT([__alignof (omp_nest_lock_25_t)], [OMP_NEST_LOCK_25_ALIGN])

# If the lock fits in an integer, then arrange for Fortran to use that
# integer.  If it doesn't, then arrange for Fortran to use a pointer.
# Except that we don't have a way at present to multi-lib the installed
# Fortran modules, so we assume 8 bytes for pointers, regardless of the
# actual target.
OMP_LOCK_KIND=$OMP_LOCK_SIZE
OMP_NEST_LOCK_KIND=$OMP_NEST_LOCK_SIZE
if test $OMP_LOCK_SIZE -gt 8 || test $OMP_LOCK_ALIGN -gt $OMP_LOCK_SIZE; then
  OMP_LOCK_KIND=8
fi
if test $OMP_NEST_LOCK_SIZE -gt 8 || test $OMP_NEST_LOCK_ALIGN -gt $OMP_NEST_LOCK_SIZE; then
  OMP_NEST_LOCK_KIND=8
fi
OMP_LOCK_25_KIND=$OMP_LOCK_25_SIZE
OMP_NEST_LOCK_25_KIND=$OMP_NEST_LOCK_25_SIZE
if test $OMP_LOCK_25_SIZE -gt 8 || test $OMP_LOCK_25_ALIGN -gt $OMP_LOCK_25_SIZE; then
  OMP_LOCK_25_KIND=8
fi
if test $OMP_NEST_LOCK_25_SIZE -gt 8 || test $OMP_NEST_LOCK_25_ALIGN -gt $OMP_NEST_LOCK_25_SIZE; then
  OMP_NEST_LOCK_25_KIND=8
fi

AC_SUBST(OMP_LOCK_SIZE)
AC_SUBST(OMP_LOCK_ALIGN)
AC_SUBST(OMP_NEST_LOCK_SIZE)
AC_SUBST(OMP_NEST_LOCK_ALIGN)
AC_SUBST(OMP_LOCK_KIND)
AC_SUBST(OMP_NEST_LOCK_KIND)
AC_SUBST(OMP_LOCK_25_SIZE)
AC_SUBST(OMP_LOCK_25_ALIGN)
AC_SUBST(OMP_NEST_LOCK_25_SIZE)
AC_SUBST(OMP_NEST_LOCK_25_ALIGN)
AC_SUBST(OMP_LOCK_25_KIND)
AC_SUBST(OMP_NEST_LOCK_25_KIND)
CFLAGS="$save_CFLAGS"

AC_CONFIG_FILES(omp.h omp_lib.h omp_lib.f90 libgomp_f.h)
AC_CONFIG_FILES(Makefile libgomp.spec)
AC_OUTPUT


