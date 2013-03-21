/*
 *
 * dbus-ping-common.h D-Bus benchmarking test client
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

#ifndef DBUS_PING_COMMON_H_
#define DBUS_PING_COMMON_H_

#include "dbus-ping-cmdline.h"

#include <stdarg.h>
#include <stdint.h>
#include <time.h>


void assert_error(int expression, const char *fmt, ...);
const char *get_next_data_item(char **items);
int type_from_name(const char *arg);


#define USEC_PER_SEC  1000000ULL
#define USEC_PER_MSEC 1000ULL
#define NSEC_PER_USEC 1000ULL

typedef uint64_t usec_t;

static inline usec_t time_now(clockid_t clock_id) {
	struct timespec ts;
	assert_error(clock_gettime(clock_id, &ts) == 0, "Unable to get clock time");
	return (usec_t) ts.tv_sec * USEC_PER_SEC + (usec_t) ts.tv_nsec / NSEC_PER_USEC;
}

#endif /* DBUS_PING_COMMON_H_ */
