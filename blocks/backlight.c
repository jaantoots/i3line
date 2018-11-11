/* i3line block: backlight
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

#define BACKLIGHT "/sys/class/backlight/intel_backlight"

int backlight(struct block *b) {
    /* read backlight level */
    FILE *fcur = fopen(BACKLIGHT "/brightness", "r");
    if (fcur == NULL) {
        perror("fopen()");
        return -1;
    }
    int cur;
    fscanf(fcur, "%d", &cur);
    if (ferror(fcur)) cur = 0;
    fclose(fcur);
    FILE *fmax = fopen(BACKLIGHT "/max_brightness", "r");
    if (fmax == NULL) {
        perror("fopen()");
        return -1;
    }
    int max;
    fscanf(fmax, "%d", &max);
    if (ferror(fmax)) max = 0;
    fclose(fmax);

    /* print backlight */
    snprintf(b->full_text, sizeof b->full_text,
            "%s%.2g%%", backlight_icon, (double)100*cur/max);
    snprintf(b->short_text, sizeof b->short_text,
            "%.2g%%", (double)100*cur/max);
    snprintf(b->color, sizeof b->color, "%s", "");
    b->urgent = (cur == 0 || max == 0) ? 1 : 0;
    return 0;
}
