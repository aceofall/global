# Local additions to Autoconf macros.

AC_DEFUN(AG_STRUCT_DP_D_NAMLEN,
[AC_CACHE_CHECK([for dp_d_namlen in struct dirent], ac_cv_struct_dp_d_namlen,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <dirent.h>], [struct dirent s; s.d_namlen;],
ac_cv_struct_dp_d_namlen=yes, ac_cv_struct_dp_d_namlen=no)])
if test $ac_cv_struct_dp_d_namlen = yes; then
  AC_DEFINE(HAVE_DP_D_NAMLEN)
fi
])

AC_DEFUN(AG_STRUCT_DP_D_TYPE,
[AC_CACHE_CHECK([for dp_d_type in struct dirent], ac_cv_struct_dp_d_type,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <dirent.h>], [struct dirent s; s.d_type;],
ac_cv_struct_dp_d_type=yes, ac_cv_struct_dp_d_type=no)])
if test $ac_cv_struct_dp_d_type = yes; then
  AC_DEFINE(HAVE_DP_D_TYPE)
fi
])

dnl derived from acspecific.m4 and fix warning.
AC_DEFUN(AG_BYTE_ORDER,
[AC_CACHE_CHECK(whether byte ordering is bigendian, ac_cv_c_bigendian,
[ac_cv_c_bigendian=unknown
# See if sys/param.h defines the BYTE_ORDER macro.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/param.h>], [
#if !BYTE_ORDER || !BIG_ENDIAN || !LITTLE_ENDIAN
 bogus endian macros
#endif], [# It does; now see whether it defined to BIG_ENDIAN or not.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/param.h>], [
#if BYTE_ORDER != BIG_ENDIAN
 not big endian
#endif], ac_cv_c_bigendian=yes, ac_cv_c_bigendian=no)])
if test $ac_cv_c_bigendian = unknown; then
AC_TRY_RUN([main () {
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[sizeof (long) - 1] == 1);
}], ac_cv_c_bigendian=no, ac_cv_c_bigendian=yes, :)
fi])
if test $ac_cv_c_bigendian = yes; then
  AC_DEFINE(WORDS_BIGENDIAN)
fi
])