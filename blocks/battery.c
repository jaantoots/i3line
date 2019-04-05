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
#include "utils.h"

#define BAT "/sys/class/power_supply"
#define BAT0 "BAT0"
#define BAT_LOW 25
#define BAT_CRITICAL 15
#define BAT_URGENT 10

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
    if (fscan_value(b->path, "status", "%ms", &status)) return -1;
    if (fscan_value(b->path, "capacity", "%Ld", &capacity)) return -2;
    if (fscan_value(b->path, "charge_full", "%Ld", &charge_full) &&
            fscan_value(b->path, "energy_full", "%Ld", &charge_full)) return -2;
    if (fscan_value(b->path, "charge_now", "%Ld", &charge_now) &&
            fscan_value(b->path, "energy_now", "%Ld", &charge_now)) return -2;
    if (fscan_value(b->path, "voltage_now", "%Ld", &voltage_now)) return -1;
    if (fscan_value(b->path, "current_now", "%Ld", &current_now)) {
        if (fscan_value(b->path, "power_now", "%Ld", &current_now)) return -2;
        voltage_now = 1e6;
    }

    /* format block */
    const char *icon = power_icons[0];
    const char *color = "";
    long long time = 0;
    int low = BAT_LOW;
    int critical = BAT_CRITICAL;
    int urgent = BAT_URGENT;
    if (!strcmp(status, "Charging") && current_now > 0) {
        icon = power_icons[1];
        time = 3600*(charge_full - charge_now)/current_now;
    }
    else if (!strcmp(status, "Full")) {
        icon = power_icons[2];
        color = base0B;
    }
    else if (!strcmp(status, "Discharging") && current_now > 0) {
        icon = battery_icons[(capacity + 12)/25];
        time = 3600*charge_now/current_now;
        if (capacity <= critical) color = base08;
        else if (capacity <= low) color = base0A;
    }
    free(status);
    double percent = (charge_full > 0) ? (double)100*charge_now/charge_full : 0;
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
