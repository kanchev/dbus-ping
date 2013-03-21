/*
 *
 * dbus-test-service-systemd.c D-Bus benchmarking test client
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


static int handleGetTestDataCopy(sd_bus *bus, sd_bus_message *message, sd_bus_message **reply) {
	int r;
	int32_t intValue;
	double doubleValue1;
	double doubleValue2;
	const char* stringValue;

	r = sd_bus_message_read(message, "(idds)", &intValue, &doubleValue1, &doubleValue2, &stringValue);
	if (r < 0) {
		log_error("Failed to get parameter: %s", strerror(-r));
		return 0;
	}

	r = sd_bus_message_new_method_return(bus, message, reply);
	if (r < 0) {
		log_error("Failed to allocate return: %s", strerror(-r));
		return r;
	}

	r = sd_bus_message_append(*reply, "(idds)", intValue, doubleValue1, doubleValue2, stringValue);
	if (r < 0) {
		log_error("Failed to append message: %s", strerror(-r));
		return r;
	}

    return r;
}

static int handleGetTestDataArrayCopy(sd_bus *bus, sd_bus_message *message, sd_bus_message **reply) {
	int r;

	r = sd_bus_message_open_container(message, 'a', "(idds)");
	if (r < 0) {
		log_error("Failed to open container: %s", strerror(-r));
		return 0;
	}

	r = sd_bus_message_new_method_return(bus, message, reply);
	if (r < 0) {
		log_error("Failed to allocate return: %s", strerror(-r));
		return r;
	}

	r = sd_bus_message_open_container(*reply, 'a', "(idds)");
	if (r < 0) {
		log_error("Failed to open reply container: %s", strerror(-r));
		return r;
	}

	for (;;) {
		int32_t intValue;
		double doubleValue1;
		double doubleValue2;
		const char* stringValue;

		r = sd_bus_message_read(message, "(idds)", &intValue, &doubleValue1, &doubleValue2, &stringValue);
		if (r < 0) {
			log_error("Failed to get parameter: %s", strerror(-r));
			sd_bus_message_unref(*reply);
			*reply = NULL;
			return 0;
		}

		if (!r) {
			break;
		}

		r = sd_bus_message_append(*reply, "(idds)", intValue, doubleValue1, doubleValue2, stringValue);
		if (r < 0) {
			log_error("Failed to append message: %s", strerror(-r));
			return r;
		}
	}

	r = sd_bus_message_close_container(*reply);
	if (r < 0) {
		log_error("Failed to close reply container: %s", strerror(-r));
		return r;
	}

	r = sd_bus_message_close_container(message);
	if (r < 0) {
		log_error("Failed to close container: %s", strerror(-r));
		return r;
	}

	return r;
}

int main() {
	sd_bus *bus;
	int r;

    r = sd_bus_open_user(&bus);
    if (r < 0) {
        log_error("Failed to connect to user bus: %s", strerror(-r));
        goto fail;
    }

    r = sd_bus_request_name(bus, "org.genivi.test.dbus.systemd.Ping", 0);
    if (r < 0) {
    	log_error("Failed to acquire name: %s", strerror(-r));
    	goto fail;
    }

    for (;;) {
		_cleanup_bus_message_unref_ sd_bus_message *message = NULL, *reply = NULL;

		r = sd_bus_process(bus, &message);
		if (r < 0) {
			log_error("Failed to process requests: %s", strerror(-r));
			goto fail;
		}
		if (r == 0) {
			r = sd_bus_wait(bus, (uint64_t) -1);
			if (r < 0) {
				log_error("Failed to wait: %s", strerror(-r));
				goto fail;
			}

			continue;
		}

		if (sd_bus_message_is_method_call(message, NULL, "getEmptyResponse")) {
			sd_bus_message_new_method_return(bus, message, &reply);
			if (r < 0) {
				log_error("Failed to allocate return: %s", strerror(-r));
				goto fail;
			}
		} else if (sd_bus_message_is_method_call(message, NULL, "getTestDataCopy")) {
			r = handleGetTestDataCopy(bus, message, &reply);
			if (r < 0) {
				goto fail;
			}
		} else if (sd_bus_message_is_method_call(message, NULL, "getTestDataArrayCopy")) {
			r = handleGetTestDataArrayCopy(bus, message, &reply);
			if (r < 0) {
				goto fail;
			}
		} else if (sd_bus_message_is_method_call(message, NULL, NULL )) {
			const sd_bus_error error = SD_BUS_ERROR_INIT_CONST(
					"org.freedesktop.DBus.Error.UnknownMethod",
					"Unknown method.");

			r = sd_bus_message_new_method_error(bus, message, &error, &reply);
			if (r < 0) {
				log_error("Failed to allocate return: %s", strerror(-r));
				goto fail;
			}
		}

		if (reply) {
			r = sd_bus_send(bus, reply, NULL );
			if (r < 0) {
				log_error("Failed to send reply: %s", strerror(-r));
				goto fail;
			}
		}
    }

	r = 0;

fail:
	if (bus)
		sd_bus_unref(bus);

	return r;
}
