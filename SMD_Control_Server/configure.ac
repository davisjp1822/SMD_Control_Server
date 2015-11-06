#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([SMDServer], [2.0], [jd@linecardapp.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([modbus_free], [libmodbus modbus], [], [
				  AC_MSG_ERROR([unable to find libmodbus])
				  ])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h float.h limits.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

AC_CHECK_HEADERS([modbus.h], [], [],
				      [[#ifdef HAVE_MODBUS_H
					    # include <modbus.h>
						#endif
						]])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor memset pow socket strchr strcspn strdup strrchr strstr strtol])

AC_OUTPUT(Makefile src/Makefile)
