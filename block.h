/* i3line block header
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
#ifndef BLOCK_H
#define BLOCK_H

#include "icons.h"
#include "colors.h"

#define MAX_LEN 2048
#define MAX_LEN_STR "2048"

struct block {
    const char *name;
    const char *instance;
    int (*update)(struct block *);
    char full_text[MAX_LEN];
    char short_text[MAX_LEN];
    char color[MAX_LEN];
    int urgent;
    int button;
    /* optional variables */
    int state;
    char path[MAX_LEN];
};

enum block_state { BLOCK_RESET = -1, BLOCK_NOTAVAIL = -2 };

int date(struct block *);
int loadavg(struct block *);
int temperature(struct block *);
int battery(struct block *);
int backlight(struct block *);
int volume(struct block *);
int wifi(struct block *);
int disk(struct block *);
int notmuch(struct block *);

#endif /* BLOCK_H */
