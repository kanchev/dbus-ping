/*
 *
 * dbus-ping-common.c D-Bus benchmarking test client
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
#include "dbus-ping-common.h"

#include <string.h>
#include <stdlib.h>


void assert_error(int expression, const char *fmt, ...) {
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

const char *get_next_data_item(char **items) {
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

int type_from_name(const char *arg) {
	if (!strcmp(arg, "string"))			return 's';
	else if (!strcmp(arg, "int16"))		return 'n';
	else if (!strcmp(arg, "uint16"))	return 'q';
	else if (!strcmp(arg, "int32"))		return 'i';
	else if (!strcmp(arg, "uint32"))	return 'u';
	else if (!strcmp(arg, "int64"))		return 'x';
	else if (!strcmp(arg, "uint64"))	return 't';
	else if (!strcmp(arg, "double"))	return 'd';
	else if (!strcmp(arg, "byte"))		return 'y';
	else if (!strcmp(arg, "boolean"))	return 'b';
	else if (!strcmp(arg, "objpath"))	return 'o';
	else if (!strcmp(arg, "variant"))	return 'v';
	else if (!strcmp(arg, "array"))		return 'a';
	else if (!strcmp(arg, "struct"))	return 'r';
	else if (!strcmp(arg, "dict"))		return 'e';

	assert_error(0, "Unknown type '%s'", arg);
	return 0;
}
