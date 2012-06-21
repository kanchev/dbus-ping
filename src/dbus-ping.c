/*
 *
 * dbus-ping.c D-Bus benchmarking test client
 *
 * Copyright (C) 2011 BMW AG
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

#include <dbus/dbus.h>

#include "dbus-ping-cmdline.h"
#include "dbus-print-message.h"


#define USEC_PER_SEC  1000000ULL
#define USEC_PER_MSEC 1000ULL
#define NSEC_PER_USEC 1000ULL

typedef uint64_t usec_t;

typedef union {
	dbus_int16_t i16;
	dbus_uint16_t u16;
	dbus_int32_t i32;
	dbus_uint32_t u32;
	dbus_int64_t i64;
	dbus_uint64_t u64;
	double dbl;
	unsigned char byt;
	char *str;
} DBusBasicValue;


static usec_t start_time;
static usec_t message_duplicate_time;
static usec_t message_send_time;


static void assert_error(int expression, const char *fmt, ...) {
	if (!expression) {
		va_list ap;

		va_start(ap, fmt);
		fprintf(stderr, CMDLINE_PARSER_PACKAGE ": ERROR: ");
		vfprintf(stderr, fmt, ap);
		fputc('\n', stderr);
		va_end(ap);

		exit(1);
	}
}

static int type_from_name(const char *arg) {
	if (!strcmp(arg, "string"))			return DBUS_TYPE_STRING;
	else if (!strcmp(arg, "int16"))		return DBUS_TYPE_INT16;
	else if (!strcmp(arg, "uint16"))	return DBUS_TYPE_UINT16;
	else if (!strcmp(arg, "int32"))		return DBUS_TYPE_INT32;
	else if (!strcmp(arg, "uint32"))	return DBUS_TYPE_UINT32;
	else if (!strcmp(arg, "int64"))		return DBUS_TYPE_INT64;
	else if (!strcmp(arg, "uint64"))	return DBUS_TYPE_UINT64;
	else if (!strcmp(arg, "double"))	return DBUS_TYPE_DOUBLE;
	else if (!strcmp(arg, "byte"))		return DBUS_TYPE_BYTE;
	else if (!strcmp(arg, "boolean"))	return DBUS_TYPE_BOOLEAN;
	else if (!strcmp(arg, "objpath"))	return DBUS_TYPE_OBJECT_PATH;
	else if (!strcmp(arg, "variant"))	return DBUS_TYPE_VARIANT;
	else if (!strcmp(arg, "array"))		return DBUS_TYPE_ARRAY;
	else if (!strcmp(arg, "struct"))	return DBUS_TYPE_STRUCT;
	else if (!strcmp(arg, "dict"))		return DBUS_TYPE_DICT_ENTRY;

	assert_error(FALSE, "Unknown type '%s'", arg);
	return DBUS_TYPE_INVALID;
}

static const char *get_next_data_item(char **items) {
	char *item = *items;
	char *delim;

	if (item == NULL || *item == '\0') {
		fprintf(stderr, CMDLINE_PARSER_PACKAGE ": Data item is badly formed\n");
		exit(1);
	}

	delim = strchr(item, ':');
	if (delim != NULL) {
		*delim = '\0';
		*items = delim + 1;
	} else
		*items = item + strlen(item);

	return item;
}

static void append_arg(DBusMessageIter *iter, int type, const char *value) {
	dbus_uint16_t uint16;
	dbus_int16_t int16;
	dbus_uint32_t uint32;
	dbus_int32_t int32;
	dbus_uint64_t uint64;
	dbus_int64_t int64;
	double d;
	unsigned char byte;
	dbus_bool_t v_BOOLEAN;

	/* FIXME - we are ignoring OOM returns on all these functions */
	switch (type) {
	case DBUS_TYPE_BYTE:
		byte = strtoul(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &byte);
		break;

	case DBUS_TYPE_DOUBLE:
		d = strtod(value, NULL);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_DOUBLE, &d);
		break;

	case DBUS_TYPE_INT16:
		int16 = strtol(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16, &int16);
		break;

	case DBUS_TYPE_UINT16:
		uint16 = strtoul(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &uint16);
		break;

	case DBUS_TYPE_INT32:
		int32 = strtol(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &int32);
		break;

	case DBUS_TYPE_UINT32:
		uint32 = strtoul(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &uint32);
		break;

	case DBUS_TYPE_INT64:
		int64 = strtoll(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_INT64, &int64);
		break;

	case DBUS_TYPE_UINT64:
		uint64 = strtoull(value, NULL, 0);
		dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &uint64);
		break;

	case DBUS_TYPE_STRING:
		dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &value);
		break;

	case DBUS_TYPE_OBJECT_PATH:
		dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &value);
		break;

	case DBUS_TYPE_BOOLEAN:
		assert_error(!strcmp(value, "true") || !strcmp(value, "false"),
				"Expected 'true' or 'false' instead of '%s'", value);
		v_BOOLEAN = !strcmp(value, "true") ? TRUE : FALSE;
		dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &v_BOOLEAN);
		break;

	default:
		assert_error(FALSE, "Unsupported data type %c", (char) type);
		break;
	}
}

static void append_array(DBusMessageIter *iter, int type, const char *value) {
	const char *val;
	char *dupval = strdup(value);

	val = strtok(dupval, ",");
	while (val != NULL) {
		append_arg(iter, type, val);
		val = strtok(NULL, ",");
	}
	free(dupval);
}

static void append_struct(DBusMessageIter *iter, char *expr) {
	while (*expr != '\0') {
		int type = type_from_name(get_next_data_item(&expr));
		const char *value = *expr != '\0' ? get_next_data_item(&expr) : "";

		append_arg(iter, type, value);
	}
}

static void append_dict(DBusMessageIter *iter, int keytype, int valtype, const char *value) {
	const char *val;
	char *dupval = strdup(value);

	val = strtok(dupval, ",");
	while (val != NULL) {
		DBusMessageIter subiter;

		dbus_message_iter_open_container(iter, DBUS_TYPE_DICT_ENTRY, NULL, &subiter);

		append_arg(&subiter, keytype, val);
		val = strtok(NULL, ",");
		assert_error(val != NULL, "Malformed dictionary");
		append_arg(&subiter, valtype, val);

		dbus_message_iter_close_container(iter, &subiter);
		val = strtok(NULL, ",");
	}
	free(dupval);
}

static void message_append_args(DBusMessageIter *iter, DBusMessageIter *append_iter) {
	do {
		int type = dbus_message_iter_get_arg_type(iter);
		if (type == DBUS_TYPE_INVALID)
			break;

		if (dbus_type_is_basic(type)) {
			DBusBasicValue value;

			dbus_message_iter_get_basic(iter, &value);
			dbus_message_iter_append_basic(append_iter, type, &value);

		} else { // container type
			char *signature = NULL;
			DBusMessageIter subiter, append_subiter;

			dbus_message_iter_recurse (iter, &subiter);
			if (type != DBUS_TYPE_DICT_ENTRY && type != DBUS_TYPE_STRUCT)
				signature = dbus_message_iter_get_signature(&subiter);
			dbus_message_iter_open_container(append_iter, type, signature, &append_subiter);

			switch (type) {
			case DBUS_TYPE_DICT_ENTRY:
			case DBUS_TYPE_VARIANT:
				message_append_args(&subiter, &append_subiter);

				if (type == DBUS_TYPE_DICT_ENTRY) {
					dbus_message_iter_next(&subiter);
					message_append_args(&subiter, &append_subiter);
				}
				break;

			default:
				while ((type = dbus_message_iter_get_arg_type(&subiter)) != DBUS_TYPE_INVALID) {
					message_append_args(&subiter, &append_subiter);
					dbus_message_iter_next(&subiter);
				}
				break;
			}

			dbus_message_iter_close_container(append_iter, &append_subiter);
		}
	} while (dbus_message_iter_next(iter));
}

static DBusMessage *dbus_message_clone_header(DBusMessage *message) {
	const int type = dbus_message_get_type(message);
	const char *path = dbus_message_get_path(message);
	const char *interface = dbus_message_get_interface(message);
	const char *member = dbus_message_get_member(message);
	const char *bus_name = (type == DBUS_MESSAGE_TYPE_METHOD_CALL) ? dbus_message_get_destination(message) : NULL;
	DBusMessage *clone;

	clone = (type == DBUS_MESSAGE_TYPE_METHOD_CALL) ?
			dbus_message_new_method_call(bus_name, path, interface, member) :
			dbus_message_new_signal(path, interface, member);
	assert_error(clone != NULL, "Unable to create message clone (out of memory)");

	return clone;
}

static DBusMessage *message_multiply_contents(DBusMessage *old_message, int count) {
	DBusMessage *message = dbus_message_clone_header(old_message);
	const char *signature = dbus_message_get_signature(old_message);
	DBusMessageIter append_iter, append_array_iter;

	dbus_message_iter_init_append(message, &append_iter);
	dbus_message_iter_open_container(&append_iter, DBUS_TYPE_ARRAY, signature, &append_array_iter);

	while (count-- > 0) {
		DBusMessageIter iter;

		dbus_message_iter_init(old_message, &iter);
		message_append_args(&iter, &append_array_iter);
	}

	dbus_message_iter_close_container(&append_iter, &append_array_iter);

	return message;
}

static DBusMessage *message_create_contents(struct gengetopt_args_info *args_info, DBusMessage *message) {
	int i;
	DBusMessageIter message_iter;

	dbus_message_iter_init_append(message, &message_iter);

	for (i = 0; i < args_info->inputs_num; i++) {
		char *arg = args_info->inputs[i];
		int type;

		type = type_from_name(get_next_data_item(&arg));
		if (dbus_type_is_container(type)) {
			int container_type = type;
			DBusMessageIter container_iter;

			if (container_type == DBUS_TYPE_STRUCT) {
				dbus_message_iter_open_container(&message_iter, DBUS_TYPE_STRUCT, 0, &container_iter);
				append_struct(&container_iter, arg);
			} else {
				type = arg[0] == 0 ? DBUS_TYPE_STRING : type_from_name(get_next_data_item(&arg));

				if (container_type == DBUS_TYPE_DICT_ENTRY) {
					int secondary_type = type_from_name(get_next_data_item(&arg));
					char sig[] = { DBUS_DICT_ENTRY_BEGIN_CHAR, type, secondary_type, DBUS_DICT_ENTRY_END_CHAR, '\0' };

					dbus_message_iter_open_container(&message_iter, DBUS_TYPE_ARRAY, sig, &container_iter);
					append_dict(&container_iter, type, secondary_type, arg);
				} else {
					char sig[2] = { type, '\0' };

					dbus_message_iter_open_container(&message_iter, container_type, sig, &container_iter);

					if (container_type == DBUS_TYPE_ARRAY)
						append_array(&container_iter, type, arg);
					else
						append_arg(&container_iter, type, arg);
				}
			}

			dbus_message_iter_close_container(&message_iter, &container_iter);
		} else
			append_arg(&message_iter, type, arg);
	}

	if (args_info->contents_multiply_arg > 1) {
		DBusMessage *old_message = message;
		message = message_multiply_contents(old_message, args_info->contents_multiply_arg);
		dbus_message_unref(old_message);
	}

	return message;
}

static DBusMessage *message_create(struct gengetopt_args_info *args_info) {
	int type = dbus_message_type_from_string(args_info->type_arg);
	DBusMessage *message;

	assert_error(type == DBUS_MESSAGE_TYPE_METHOD_CALL || type == DBUS_MESSAGE_TYPE_SIGNAL,
			"Message type '%s' is not supported\n", args_info->type_arg);

	message = type == DBUS_MESSAGE_TYPE_METHOD_CALL ?
			dbus_message_new_method_call(args_info->destination_arg, args_info->path_arg, args_info->interface_arg, args_info->member_arg) :
			dbus_message_new_signal(args_info->path_arg, args_info->interface_arg, args_info->member_arg);
	assert_error(message != NULL, "Unable to allocate message (out of memory)");

	return message_create_contents(args_info, message);
}

static DBusConnection *dbus_connect(const struct gengetopt_args_info *args_info) {
	DBusBusType type = args_info->system_given ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION;
	DBusConnection *connection;
	DBusError error;

	dbus_error_init (&error);

	connection = args_info->address_given ?
			dbus_connection_open(args_info->address_arg, &error) :
			dbus_bus_get(type, &error);
	assert_error(!dbus_error_is_set(&error), "Failed to open connection to '%s' message bus: %s",
			args_info->address_given ? args_info->address_arg : (type == DBUS_BUS_SYSTEM) ? "system" : "session",
			error.message);

	assert_error(dbus_bus_register(connection, &error), "Unable to register bus connection");

	return connection;
}

static DBusMessage *dbus_message_clone(DBusMessage *message) {
	DBusMessage *clone = dbus_message_clone_header(message);
	DBusMessageIter iter, append_iter;

	dbus_message_iter_init(message, &iter);
	dbus_message_iter_init_append(clone, &append_iter);

	message_append_args(&iter, &append_iter);

	return clone;
}

static inline usec_t time_now(clockid_t clock_id) {
	struct timespec ts;
	assert_error(clock_gettime(clock_id, &ts) == 0, "Unable to get clock time");
	return (usec_t) ts.tv_sec * USEC_PER_SEC + (usec_t) ts.tv_nsec / NSEC_PER_USEC;
}

static DBusMessage *dbus_message_duplicate(DBusMessage *message, int do_clone) {
	usec_t time = time_now(CLOCK_MONOTONIC);
	DBusMessage *clone = do_clone ? dbus_message_clone(message) : dbus_message_copy(message);

	message_duplicate_time += time_now(CLOCK_MONOTONIC) - time;

	return clone;
}

static int dbus_send_message(DBusConnection *connection, DBusMessage *message, int reply_timeout, int do_clone) {
	int ret = 1;
	DBusMessage *clone = dbus_message_duplicate(message, do_clone);
	usec_t time = time_now(CLOCK_MONOTONIC);
	DBusMessage *reply;
	DBusError error;

	dbus_message_set_auto_start(clone, TRUE);
	dbus_error_init(&error);

	reply = dbus_connection_send_with_reply_and_block(connection, clone, reply_timeout, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, CMDLINE_PARSER_PACKAGE ": Send error %s: %s\n", error.name, error.message);
		dbus_error_free(&error);
		ret = 0;
	}

	if (reply)
		dbus_message_unref(reply);

	dbus_message_unref(clone);
	message_send_time += time_now(CLOCK_MONOTONIC) - time;

	return ret;
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
		printf("DBUS_PING_SENT=%d;\n"
				"DBUS_PING_RECEIVED=%d;\n"
				"DBUS_PING_TOTAL=%d;\n"
				"DBUS_PING_TIME=%llu;\n"
				"DBUS_PING_TIME_SEC=%llu;\n"
				"DBUS_PING_MSGS_PER_SEC=%llu;\n"
				"DBUS_PING_DUPLICATE_TIME=%llu;\n"
				"DBUS_PING_SEND_TIME=%llu;\n",
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
	DBusMessage *message;
	DBusConnection *connection;
	long int count;

	if (cmdline_parser(argc, argv, &args_info) != 0)
		return -1;

	message = message_create(&args_info);

	if (args_info.verbose_given)
		print_message(message, FALSE);

	connection = dbus_connect(&args_info);
	start_time = time_now(CLOCK_MONOTONIC);

	for (count = 0; count < args_info.count_arg; count++) {
		dbus_send_message(connection, message, args_info.reply_timeout_arg, args_info.clone_given);
		update_progress(count, &args_info);
	}

	show_summary(count, count, &args_info);

	dbus_connection_unref(connection);
	dbus_message_unref(message);
	dbus_shutdown();

	return 0;
}
