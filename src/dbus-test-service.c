/*
 *
 * dbus-test-service.c D-Bus benchmarking test service
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

#include <stdio.h>
#include <string.h>
#include <dbus/dbus.h>

#define DEFAULT_BUS_NAME		"com.bmw.Test"
#define	DEFAULT_OBJECT_PATH		"/com/bmw/Test"

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


static DBusMessage *last_method_reply;

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

static DBusHandlerResult echo_send(DBusConnection *connection, DBusMessage *reply) {
	if (!dbus_connection_send (connection, reply, NULL)) {
		dbus_message_unref(reply);
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	if (last_method_reply)
		dbus_message_unref(last_method_reply);

	last_method_reply = reply;

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult echo_method_call(DBusConnection *connection, DBusMessage *message) {
	DBusMessage *reply = dbus_message_new_method_return(message);
	DBusMessageIter iter, append_iter;

	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init(message, &iter);
	dbus_message_iter_init_append(reply, &append_iter);
	message_append_args(&iter, &append_iter);

	return echo_send(connection, reply);
}

static DBusHandlerResult echo_last_method_reply(DBusConnection *connection, DBusMessage *message) {
	DBusMessage *reply;

	if (!last_method_reply)
		return echo_method_call(connection, message);

	reply = dbus_message_copy(last_method_reply);
	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_set_destination(reply, dbus_message_get_sender(message));
	dbus_message_set_reply_serial(reply, dbus_message_get_serial(message));

	return echo_send(connection, reply);
}

static void path_unregistered_func(DBusConnection *connection, void *user_data) {
	/* connection was finalized */
}

static DBusHandlerResult path_message_func(DBusConnection *connection, DBusMessage *message, void *user_data) {
	const char *member;

	if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL) {
		member = dbus_message_get_member(message);

		if (!strcmp(member, "getLastReply"))
			return echo_last_method_reply(connection, message);

		if (!strcmp(member, "getEcho"))
			return echo_method_call(connection, message);
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static DBusObjectPathVTable echo_vtable = {
		path_unregistered_func,
		path_message_func,
		NULL,
};

int main(void) {
	int ret = -1;
	DBusConnection *connection;
	DBusError error;

	dbus_error_init(&error);

	connection = dbus_bus_get(DBUS_BUS_STARTER, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "Unable to get session bus connection: %s", error.message);
		dbus_error_free(&error);
		goto end;
	}

	dbus_bus_request_name(connection, DEFAULT_BUS_NAME, 0, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "Failed to get bus name '%s': %s", DEFAULT_BUS_NAME, error.message);
		dbus_error_free(&error);
		goto end;
	}

	if (!dbus_connection_register_object_path(connection, DEFAULT_OBJECT_PATH, &echo_vtable, NULL)) {
		fprintf(stderr, "Unable to register object path '%s'!", DEFAULT_OBJECT_PATH);
		goto end;
	}

	while (dbus_connection_read_write_dispatch(connection, -1))
		;

	if (last_method_reply)
		dbus_message_unref(last_method_reply);

	ret = 0;
end:
	if (connection != NULL)
		dbus_connection_unref(connection);

	dbus_shutdown();

	return ret;
}
