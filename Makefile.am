EXTRA_DIST = com.bmw.Test.service.in

# ------------------------------------------------------------------------------
CLEANFILES = *~ com.bmw.Test.service

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess \
		config.sub configure depcomp install-sh \
		ltmain.sh Makefile.in missing \
		config.h config.h.in \
		*-poky-linux-libtool

AM_CFLAGS = \
		${DBUS_CFLAGS} \
		-include $(top_builddir)/config.h

AM_LDFLAGS = ${DBUS_LIBS}

# ------------------------------------------------------------------------------
noinst_LTLIBRARIES = libdbus-ping-common.la

libdbus_ping_common_la_SOURCES = \
					src/dbus-ping-cmdline.c \
					src/dbus-ping-cmdline.h \
					src/dbus-ping-common.c \
					src/dbus-ping-common.h

# ------------------------------------------------------------------------------
bin_PROGRAMS = \
		dbus-ping

dbus_ping_SOURCES = \
		src/dbus-ping.c \
		src/dbus-print-message.c \
		src/dbus-print-message.h


dbus_ping_LDADD = \
		libdbus-ping-common.la

dbus_ping_CFLAGS = \
		$(AM_CFLAGS)

# ------------------------------------------------------------------------------
libexec_PROGRAMS = \
		dbus-test-service

dbus_test_service_SOURCES = \
		src/dbus-test-service.c

dbus_test_service_CFLAGS = \
		$(AM_CFLAGS)

# ------------------------------------------------------------------------------
noinst_LTLIBRARIES += \
		libsystemd-shared.la

libsystemd_shared_la_SOURCES = \
		systemd/src/shared/env-util.c \
		systemd/src/shared/env-util.h \
		systemd/src/shared/exit-status.c \
		systemd/src/shared/exit-status.h \
		systemd/src/shared/fileio.c \
		systemd/src/shared/fileio.h \
		systemd/src/shared/hashmap.c \
		systemd/src/shared/hashmap.h \
		systemd/src/shared/ioprio.h \
		systemd/src/shared/label.c \
		systemd/src/shared/label.h \
		systemd/src/shared/list.h \
		systemd/src/shared/log.c \
		systemd/src/shared/log.h \
		systemd/src/shared/macro.h \
		systemd/src/shared/missing.h \
		systemd/src/shared/mkdir.c \
		systemd/src/shared/mkdir.h \
		systemd/src/shared/path-util.c \
		systemd/src/shared/path-util.h \
		systemd/src/shared/set.c \
		systemd/src/shared/set.h \
		systemd/src/shared/socket-util.c \
		systemd/src/shared/socket-util.h \
		systemd/src/shared/strv.c \
		systemd/src/shared/strv.h \
		systemd/src/shared/time-util.c \
		systemd/src/shared/time-util.h \
		systemd/src/shared/utf8.c \
		systemd/src/shared/utf8.h \
		systemd/src/shared/util.c \
		systemd/src/shared/util.h

libsystemd_shared_la_CFLAGS = \
		$(AM_CFLAGS) \
		-Isystemd/src/ \
		-Isystemd/src/systemd

# ------------------------------------------------------------------------------
noinst_LTLIBRARIES += \
		libsystemd-id128.la

libsystemd_id128_la_SOURCES = \
		systemd/src/systemd/sd-id128.h \
		systemd/src/libsystemd-id128/sd-id128.c

libsystemd_id128_la_CFLAGS = \
		$(AM_CFLAGS) \
		-Isystemd/src/ \
		-Isystemd/src/shared \
		-Isystemd/src/systemd

libsystemd_id128_la_LIBADD = \
		libsystemd-shared.la

# ------------------------------------------------------------------------------
noinst_LTLIBRARIES += \
		libsystemd-bus.la

libsystemd_bus_la_SOURCES = \
		systemd/src/libsystemd-bus/sd-bus.c \
		systemd/src/libsystemd-bus/sd-bus.h \
		systemd/src/libsystemd-bus/sd-bus-protocol.h \
		systemd/src/libsystemd-bus/bus-control.c \
		systemd/src/libsystemd-bus/bus-error.c \
		systemd/src/libsystemd-bus/bus-error.h \
		systemd/src/libsystemd-bus/bus-internal.c \
		systemd/src/libsystemd-bus/bus-internal.h \
		systemd/src/libsystemd-bus/bus-message.c \
		systemd/src/libsystemd-bus/bus-message.h \
		systemd/src/libsystemd-bus/bus-signature.c \
		systemd/src/libsystemd-bus/bus-signature.h \
		systemd/src/libsystemd-bus/bus-type.c \
		systemd/src/libsystemd-bus/bus-type.h

libsystemd_bus_la_CFLAGS = \
		$(AM_CFLAGS) \
		-Isystemd/src/ \
		-Isystemd/src/shared \
		-Isystemd/src/systemd

libsystemd_bus_la_LIBADD = \
		libsystemd-shared.la \
		libsystemd-id128.la

# ------------------------------------------------------------------------------
bin_PROGRAMS += \
		dbus-ping-systemd

dbus_ping_systemd_SOURCES = \
		src/dbus-ping-systemd.c

dbus_ping_systemd_LDADD = \
		libsystemd-bus.la \
		libdbus-ping-common.la

dbus_ping_systemd_CFLAGS = \
		$(AM_CFLAGS) \
		-Isystemd/src \
		-Isystemd/src/shared \
		-Isystemd/src/systemd

# ------------------------------------------------------------------------------
libexec_PROGRAMS += \
		dbus-test-service-systemd

dbus_test_service_systemd_SOURCES = \
		src/dbus-test-service-systemd.c

dbus_test_service_systemd_LDADD = \
		libsystemd-bus.la

dbus_test_service_systemd_CFLAGS = \
		$(AM_CFLAGS) \
		-Isystemd/src \
		-Isystemd/src/shared \
		-Isystemd/src/systemd

# ------------------------------------------------------------------------------
servicedir =  @dbus_session_service_dir@
service_DATA = com.bmw.Test.service

com.bmw.Test.service: com.bmw.Test.service.in
	sed -e 's,@libexecdir\@,$(libexecdir),g' $< >$@


dist_pkgdata_DATA = dbus-genivi-benchmarking.sh
