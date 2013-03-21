AC_PREREQ([2.61])

AC_INIT([dbus-ping], [2])

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign 1.11])
AM_SILENT_RULES([yes])

LT_PREREQ(2.2)
LT_INIT

AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
AC_PROG_INSTALL

PKG_CHECK_MODULES(DBUS, [dbus-1 >= 1.4.6])

AC_CHECK_FUNCS([fanotify_init fanotify_mark])
AC_CHECK_FUNCS([__secure_getenv secure_getenv])
AC_CHECK_DECLS([gettid, pivot_root, name_to_handle_at], [], [], [[#include <sys/types.h>
#include <unistd.h>
#include <sys/mount.h>
#include <fcntl.h>]])

AC_ARG_WITH([dbus-session-service-dir],
		AS_HELP_STRING([--with-dbus-session-service-dir=DIR], [D-Bus session service directory]),
		[],
		[with_dbus_session_service_dir=`pkg-config --variable=session_bus_services_dir dbus-1`])

AC_SUBST([dbus_session_service_dir], [$with_dbus_session_service_dir])

AC_CONFIG_FILES([
	Makefile
])

AC_OUTPUT

AC_MSG_RESULT([
        $PACKAGE_NAME $VERSION

        prefix:                  ${prefix}
        D-Bus session dir:       ${with_dbus_session_service_dir}
        CFLAGS:                  ${CFLAGS}
        LDFLAGS:                 ${LDFLAGS}
])
