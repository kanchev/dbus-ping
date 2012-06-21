AC_PREREQ(2.61)

AC_INIT(dbus-ping, 1)
AM_INIT_AUTOMAKE([foreign 1.11])
AC_CANONICAL_HOST
AM_SILENT_RULES([yes])
AC_CONFIG_HEADERS([config.h])

LT_PREREQ(2.2)
LT_INIT

AM_PROG_CC_C_O
AC_PROG_INSTALL

PKG_CHECK_MODULES(DBUS, [dbus-1 >= 1.4.6])

AC_ARG_WITH([dbus-session-service-dir],
		AS_HELP_STRING([--with-dbus-session-service-dir=DIR], [D-Bus session service directory]),
		[],
		[with_dbus_session_service_dir=`pkg-config --variable=session_bus_services_dir dbus-1`])

AC_SUBST(dbus_ping_CFLAGS)
AC_SUBST(dbus_ping_LIBS)
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
