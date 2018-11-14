/* i3line block: temperature
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
#include <dirent.h>

#include "block.h"

#define HWMON "/sys/devices/platform/coretemp.0/hwmon"
#define TEMP_HIGH 70
#define TEMP_CRITICAL 85
#define TEMP_URGENT 95

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

static int set_temperature_path(struct block *b) {
    /* find directory in HWMON as these end with a non-standard integer */
    DIR *dir = opendir(HWMON);
    if (dir == NULL) {
        perror("opendir()");
        return -1;
    }
    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (dp->d_type == DT_DIR && !(strncmp(dp->d_name, "hwmon", 5))) {
            snprintf(b->path, sizeof b->path,
                    HWMON "/%s/temp1_input", dp->d_name);
            closedir(dir);
            return 0;
        }
    }
    closedir(dir);
    return -1;
}

int temperature(struct block *b) {
    if (b->state == BLOCK_RESET &&
            (set_temperature_path(b) || !strlen(b->path) || (b->state = 0)))
        return -1;

    /* read temperature */
    FILE *ftemp = fopen(b->path, "r");
    if (ftemp == NULL) {
        perror("fopen()");
        return -1;
    }
    int temp;
    fscanf(ftemp, "%d", &temp);
    fclose(ftemp);
    temp /= 1000;

    /* print and format temperature */
    int high = TEMP_HIGH;
    int critical = TEMP_CRITICAL;
    int urgent = TEMP_URGENT;

    snprintf(b->full_text, sizeof b->full_text,
            "%s%d°C", temp_icons[MAX(0, MIN(4, (temp - 40)/10))], temp);
    snprintf(b->short_text, sizeof b->short_text, "%d°C", temp);
    snprintf(b->color, sizeof b->color, "%s",
            (temp >= critical) ? base08 : ((temp >= high) ? base0A : ""));
    b->urgent = (temp >= urgent) ? 1 : 0;
    return 0;
}
