/* i3line block: battery
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
#include <stdlib.h>
#include <string.h>

#include "block.h"

#define BAT "/sys/class/power_supply/"
#define BAT0 "BAT0"
#define BAT_LOW 25
#define BAT_CRITICAL 15
#define BAT_URGENT 10

#define BATREAD(dir, fmt, var) batread(dir, #var, fmt, &var)

int batread(const char *dir, const char *fname, const char *fmt, void *value) {
    /* read value from specified file */
    char name[MAX_LEN];
    snprintf(name, sizeof name, "%s/%s", dir, fname);
    FILE *file = fopen(name, "r");
    if (file == NULL) {
        perror(name);
        return -1;
    }
    fscanf(file, fmt, value);
    if (ferror(file)) perror(name);
    fclose(file);
    return 0;
}

int battery(struct block *b) {
    /* set battery path */
    if (b->state == BLOCK_RESET &&
            ((snprintf(b->path, sizeof b->path, BAT "/%s",
                       (strlen(b->instance) > 0) ? b->instance : BAT0) <= 0) ||
             (b->state = 0)))
        return -1;

    /* read battery status values */
    char *status;
    long long capacity, charge_full, charge_now, current_now, voltage_now;
    if (BATREAD(b->path, "%ms", status)) return 0;
    BATREAD(b->path, "%Ld", capacity);
    BATREAD(b->path, "%Ld", charge_full);
    BATREAD(b->path, "%Ld", charge_now);
    BATREAD(b->path, "%Ld", current_now);
    BATREAD(b->path, "%Ld", voltage_now);

    /* format block */
    const char *icon = power_icons[0];
    const char *color = "";
    long long time = 0;
    int low = BAT_LOW;
    int critical = BAT_CRITICAL;
    int urgent = BAT_URGENT;
    if (!strcmp(status, "Charging")) {
        icon = power_icons[1];
        time = 3600*(charge_full - charge_now)/current_now;
    }
    else if (!strcmp(status, "Full")) {
        icon = power_icons[2];
        color = base0B;
    }
    else if (!strcmp(status, "Discharging")) {
        icon = battery_icons[(capacity + 12)/25];
        time = 3600*charge_now/current_now;
        if (capacity <= critical) color = base08;
        else if (capacity <= low) color = base0A;
    }
    free(status);
    double percent = (double)100*charge_now/charge_full;
    double power = (double)voltage_now*current_now/1e12;
    if (time)
        snprintf(b->full_text, sizeof b->full_text,
                "%s%.2lf%% %02lld:%02lld %.1fW",
                icon, percent, time/3600, (time % 3600)/60, power);
    else
        snprintf(b->full_text, sizeof b->full_text, "%s%.2lf%%", icon, percent);
    snprintf(b->short_text, sizeof b->short_text, "%.2lf%%", percent);
    snprintf(b->color, sizeof b->color, "%s", color);
    b->urgent = (capacity <= urgent) ? 1 : 0;
    return 0;
}
