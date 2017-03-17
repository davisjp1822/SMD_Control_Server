#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([SMD_Control_Server], [1.0], [smd_server], [jd@paudlavisautomation.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_AUX_DIR([autotools-files])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([modbus], [modbus_new_tcp])
AC_CHECK_LIB([m], [floor])
AC_CHECK_LIB([cjson], [cJSON_Parse], [], [AC_MSG_ERROR([You must have cJSON installed. See https://github.com/DaveGamble/cJSON for info.])])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

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
AC_CHECK_FUNCS([memset socket strchr strcspn strdup strstr strtol])

# Create Makefiles
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_OUTPUT
