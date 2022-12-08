#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(libartnet, [1.1.2])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([config])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([endian.h ppc/endian.h arpa/inet.h netinet/in.h stddef.h \
                  stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h \
                  unistd.hi linux/if_packet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_HEADER_STDBOOL

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([inet_ntoa inet_aton memset select socket strchr strdup \
                strerror getifaddrs])

# check for ipv6 support - taken from unp
AC_MSG_CHECKING(for IPv6 support)
AC_CACHE_VAL(ac_cv_ipv6,
        AC_TRY_RUN([
#       include <sys/types.h>
#       include <sys/socket.h>
#       include <netinet/in.h>
        /* Make sure the definitions for AF_INET6 and struct sockaddr_in6
         * are defined, and that we can actually create an IPv6 TCP socket.
         */
        main()
        {
                int fd;
                struct sockaddr_in6 foo;
                fd = socket(AF_INET6, SOCK_STREAM, 0);
                exit(fd >= 0 ? 0 : 1);
        }],
        ac_cv_ipv6=yes,
        ac_cv_ipv6=no,
        ac_cv_ipv6=no))
AC_MSG_RESULT($ac_cv_ipv6)
if test $ac_cv_ipv6 = yes ; then
        AC_DEFINE(IPV6, 1, Define to 1 if the system supports IPv6)
fi


# Check if sockaddr{} has sa_len member.
AC_CHECK_MEMBER([struct sockaddr.sa_len],
 AC_DEFINE(HAVE_SOCKADDR_SA_LEN, 1,
           define if socket address structures have length fields),,
           [
#include <sys/types.h>
#include <sys/socket.h>])

# check for win32
AM_CONDITIONAL(USING_WIN32, test "$host_os" = 'mingw32')

AC_OUTPUT(Makefile \
          artnet/Makefile \
          debian/Makefile \
          libartnet.pc)
