/*
 * Copyright (C) 2013 Paul Kocialkowski <contact@paulk.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <linux/ioctl.h>
#include <linux/input.h>

#define LOG_TAG "orientationd"
#include <utils/Log.h>

#include "orientationd.h"

void input_event_set(struct input_event *event, int type, int code, int value)
{
	if (event == NULL)
		return;

	memset(event, 0, sizeof(struct input_event));

	event->type = type,
	event->code = code;
	event->value = value;

	gettimeofday(&event->time, NULL);
}

int64_t timestamp(struct timeval *time)
{
	if (time == NULL)
		return -1;

	return (int64_t) (time->tv_sec * 1000000000LL + time->tv_usec * 1000);
}

int input_open(char *name)
{
	DIR *d;
	struct dirent *di;

	char input_name[80] = { 0 };
	char path[PATH_MAX];
	char *c;
	int fd;
	int rc;

	if (name == NULL)
		return -EINVAL;

	d = opendir("/dev/input");
	if (d == NULL)
		return -1;

	while ((di = readdir(d))) {
		if (di == NULL || strcmp(di->d_name, ".") == 0 || strcmp(di->d_name, "..") == 0)
			continue;

		snprintf(path, PATH_MAX, "/dev/input/%s", di->d_name);
		fd = open(path, O_RDWR | O_NONBLOCK);
		if (fd < 0)
			continue;

		rc = ioctl(fd, EVIOCGNAME(sizeof(input_name) - 1), &input_name);
		if (rc < 0)
			continue;

		c = strstr((char *) &input_name, "\n");
		if (c != NULL)
			*c = '\0';

		if (strcmp(input_name, name) == 0)
			return fd;
		else
			close(fd);
	}

	return -1;
}
