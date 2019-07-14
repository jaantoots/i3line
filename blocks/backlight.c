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
#include <errno.h>

#include "block.h"
#include "utils.h"

#define BACKLIGHT "/sys/class/backlight"
#define INC 0.01
#define REPEAT 5

static int set_backlight(const char *dir, int *cur, int new) {
    char name[MAX_LEN];
    snprintf(name, sizeof name, "%s/%s", dir, "brightness");
    FILE *fset = fopen(name, "w");
    if (fset == NULL) {
        perror(name);
        return (errno == EACCES) ? -EACCES : -1;
    }
    fprintf(fset, "%d", new);
    if (ferror(fset)) new = *cur;
    fclose(fset);
    return *cur = new;
}

int backlight(struct block *b) {
    /* set backlight path */
    if (b->state == BLOCK_RESET &&
            ((snprintf(b->path, sizeof b->path, BACKLIGHT "/%s",
                       (strlen(b->instance) > 0) ? b->instance :
                       "intel_backlight") <= 0) ||
             (b->state = 0)))
        return -1;

    /* read backlight level */
    int cur, max;
    if (fscan_value(b->path, "brightness", "%d", &cur) ||
            fscan_value(b->path, "max_brightness", "%d", &max))
        return -1;

    /* handle button */
    if (b->state >= 0) --b->state;
    int new = max;
    float inc = INC;
    switch (b->button) {
        case 3:
            if (cur > max/3) new = max/3;
            else if (b->state < 0) new = 1;
            if (set_backlight(b->path, &cur, new) > 0) b->state = REPEAT;
            break;
        case 4:
            inc = -INC;
        case 5:
            new = (1 + inc)*cur;
            if (new == cur) new += (inc > 0) ? 1 : -1;
            if (new <= 0) new = 1;
            if (new > max) new = max;
            set_backlight(b->path, &cur, new);
            break;
    }

    /* print backlight */
    snprintf(b->full_text, sizeof b->full_text,
            "%s%.0lf%%", backlight_icon, (double)100*cur/max);
    snprintf(b->short_text, sizeof b->short_text,
            "%.0lf%%", (double)100*cur/max);
    snprintf(b->color, sizeof b->color, "%s", "");
    b->urgent = (cur == 0 || max == 0) ? 1 : 0;
    return 0;
}
