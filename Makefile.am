AM_CFLAGS = @dbus_ping_CFLAGS@ @DBUS_CFLAGS@
AM_LDFLAGS = @dbus_ping_LIBS@ @DBUS_LIBS@

bin_PROGRAMS = dbus-ping dbus-test-service

dbus_ping_SOURCES = src/dbus-ping.c \
					src/dbus-ping-cmdline.c \
					src/dbus-ping-cmdline.h \
					src/dbus-print-message.c \
					src/dbus-print-message.h
dbus_ping_CFLAGS = $(AM_CFLAGS)

dbus_test_service_SOURCES = src/dbus-test-service.c
dbus_test_service_CFLAGS = $(AM_CFLAGS)
libexec_PROGRAMS = dbus-test-service

servicedir =  @dbus_session_service_dir@
service_DATA = com.bmw.Test.service

com.bmw.Test.service: com.bmw.Test.service.in
	sed -e 's,@libexecdir\@,$(libexecdir),g' $< >$@


dist_pkgdata_DATA = dbus-genivi-benchmarking.sh

EXTRA_DIST = com.bmw.Test.service.in

CLEANFILES = *~ com.bmw.Test.service

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess \
		config.sub configure depcomp install-sh \
		ltmain.sh Makefile.in missing \
		config.h config.h.in \
		*-poky-linux-libtool
