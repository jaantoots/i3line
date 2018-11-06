/* i3line block: disk
 * Copyright (C) 2018 Jaan Toots <jaan@jaantoots.org>
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <string.h>
#include <sys/statvfs.h>

#include "block.h"

#define DISK_LOW 10
#define DISK_CRITICAL 5

typedef unsigned long long fssize_t;

int get_fssize(const char *path, fssize_t *avail, fssize_t *size) {
    struct statvfs buf;
    if (statvfs(path, &buf)) {
        perror(path);
        return -1;
    }
    *avail = (fssize_t)buf.f_bsize*buf.f_bavail;
    *size = (fssize_t)buf.f_frsize*buf.f_blocks;
    return 0;
}

int disk(struct block *b) {
    const char *path = b->instance;
    if (!strlen(path)) path = "/";

    fssize_t avail, size;
    if (get_fssize(path, &avail, &size)) {
        snprintf(b->full_text, sizeof b->full_text, "%s", disk_icon);
        snprintf(b->short_text, sizeof b->short_text, "%s", "");
        snprintf(b->color, sizeof b->color, "%s", "");
        b->urgent = 1;
        return 1;
    }

    int low = DISK_LOW;
    int critical = DISK_CRITICAL;
    fssize_t prop = 100*avail/size;

    snprintf(b->full_text, sizeof b->full_text, "%s%.2fG",
            disk_icon, (double)avail/(1L << 30));
    snprintf(b->short_text, sizeof b->short_text, "%.2fG",
            (double)avail/(1L << 30));
    snprintf(b->color, sizeof b->color, "%s",
            (prop < critical) ? base08 : (prop < low) ? base0A : "");
    b->urgent = 0;
    return 0;
}
