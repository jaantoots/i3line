/* i3line utils header
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
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include "block.h"

static int fscan_value(const char *dir, const char *fname,
        const char *fmt, void *value) {
    /* read value from specified file in directory */
    char name[MAX_LEN];
    snprintf(name, sizeof name, "%s/%s", dir, fname);
    FILE *file = fopen(name, "r");
    if (file == NULL) {
        perror(name);
        return -1;
    }
    int err = 0;
    if (fscanf(file, fmt, value) < 0) err = -1;
    if (ferror(file)) {
        perror(name);
        return -1;
    }
    fclose(file);
    return err;
}

#endif /* UTILS_H */
