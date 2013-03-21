/*
 *
 * dbus-ping-systemd.c D-Bus benchmarking test client
 *
 * Copyright (C) 2013 BMW AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <libsystemd-bus/sd-bus.h>
#include <libsystemd-bus/bus-message.h>
#include <libsystemd-bus/bus-type.h>

#include "dbus-ping-cmdline.h"
#include "dbus-ping-common.h"


static usec_t start_time;
static usec_t message_duplicate_time;
static usec_t message_send_time;

static sd_bus_message* dbus_ping_message_new(sd_bus *bus, const struct gengetopt_args_info *args_info) {
	int r;
	sd_bus_message* message;

	r = sd_bus_message_new_method_call(bus,
			args_info->destination_arg,
			args_info->path_arg,
			args_info->interface_arg,
			args_info->member_arg,
			&message);

	if (args_info->contents_multiply_arg > 0) {
		int i;

		if (args_info->contents_multiply_arg > 1) {
			r = sd_bus_message_open_container(message, 'a', "(idds)");
			assert_error(r >= 0, "OOM: sd_bus_message_open_container() failed!");
		}

		for (i = 0; i < args_info->contents_multiply_arg; i++) {
			r = sd_bus_message_append(message, "(idds)", 1, 12.6, 1e40, "XXXXXXXXXXXXXXXXXXXX");
			assert_error(r >= 0, "OOM: sd_bus_message_append() failed!");
		}

		if (args_info->contents_multiply_arg > 1) {
			r = sd_bus_message_close_container(message);
			assert_error(r >= 0, "OOM: sd_bus_message_close_container() failed!");
		}
	}

	return message;
}

static int dbus_ping_send_message(sd_bus *bus, const struct gengetopt_args_info *args_info) {
	int r;
	usec_t time;
	sd_bus_message *message;
	sd_bus_message *message_reply;
	sd_bus_error error = SD_BUS_ERROR_INIT;

	message = dbus_ping_message_new(bus, args_info);

	time = time_now(CLOCK_MONOTONIC);

	r = sd_bus_send_with_reply_and_block(bus, message, (uint64_t) -1, &error, &message_reply);
	if (r < 0) {
		fprintf(stderr, "send error: %s\n", error.message);
		sd_bus_error_free(&error);
	} else {
		sd_bus_message_unref(message_reply);
	}

	sd_bus_message_unref(message);

	message_send_time += time_now(CLOCK_MONOTONIC) - time;

	return r;
}

static inline void update_progress(int count, const struct gengetopt_args_info *args_info) {
	static usec_t last_update_time;
	usec_t elapsed_time = time_now(CLOCK_MONOTONIC) - start_time;

	if (!args_info->verbose_given)
		return;

	if (last_update_time > 0) {
		if (elapsed_time - last_update_time < 2 * USEC_PER_SEC)
			return;

		fprintf(stderr, "Sent %d message%s in %llu seconds (%llu msgs/sec, %u%% done)\n",
				count, count != 1 ? "s" : "",
				elapsed_time / USEC_PER_SEC,
				elapsed_time > 0 && count > 0 ? USEC_PER_SEC / (elapsed_time / count) : 0,
				(100 * count) / args_info->count_arg);
	}

	last_update_time = elapsed_time;
}

static void show_summary(int sent, int received, const struct gengetopt_args_info *args_info) {
	usec_t elapsed_time = time_now(CLOCK_MONOTONIC) - start_time;
	usec_t elapsed_time_sec = elapsed_time / USEC_PER_SEC;
	int total = sent + received;
	usec_t msgs_per_sec = elapsed_time > 0 ? USEC_PER_SEC / (elapsed_time / total) : total;

	if (args_info->bash_given)
		printf("DBUS_PING_SYSTEMD_SENT=%d;\n"
				"DBUS_PING_SYSTEMD_RECEIVED=%d;\n"
				"DBUS_PING_SYSTEMD_TOTAL=%d;\n"
				"DBUS_PING_SYSTEMD_TIME=%llu;\n"
				"DBUS_PING_SYSTEMD_TIME_SEC=%llu;\n"
				"DBUS_PING_SYSTEMD_MSGS_PER_SEC=%llu;\n"
				"DBUS_PING_SYSTEMD_DUPLICATE_TIME=%llu;\n"
				"DBUS_PING_SYSTEMD_SEND_TIME=%llu;\n",
				sent, received, total,
				elapsed_time, elapsed_time_sec,
				msgs_per_sec,
				message_duplicate_time,
				message_send_time);

	if (!args_info->bash_given || args_info->verbose_given)
		fprintf(args_info->bash_given ? stderr : stdout,
				"sent       received   total      "
				"time (sec)   time (usec)   msgs/sec (total) "
				"duplicate time  send time\n"
				"%-10d %-10d %-10d "
				"%-12llu %-13llu %-16llu "
				"%-15llu %llu\n",
				sent, received, total,
				elapsed_time_sec, elapsed_time, msgs_per_sec,
				message_duplicate_time, message_send_time);

	fflush(stdout);
}

int main(int argc, char *argv[]) {
	struct gengetopt_args_info args_info;
	sd_bus *bus;
	long int count;
	int r;

	if (cmdline_parser(argc, argv, &args_info) != 0)
		return -1;

    r = sd_bus_open_user(&bus);
    assert_error(r >= 0, "Failed to connect to user bus: %s", strerror(-r));

	start_time = time_now(CLOCK_MONOTONIC);

	for (count = 0; count < args_info.count_arg; count++) {
		dbus_ping_send_message(bus, &args_info);
		update_progress(count, &args_info);
	}

	show_summary(count, count, &args_info);

	sd_bus_flush(bus);
	sd_bus_unref(bus);

	return 0;
}
