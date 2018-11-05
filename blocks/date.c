/* i3line block: date
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
#include <time.h>

#include "block.h"

int date(struct block *b) {
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
        perror("clock_gettime()");
        return 1;
    }
    char str[256];
    if (!strftime(str, sizeof str, "%F %T", localtime(&now.tv_sec))) return 1;
    snprintf(b->full_text, sizeof b->full_text, "%s%s", "time_icon", str);
    snprintf(b->short_text, sizeof b->short_text, "%s", str);
    snprintf(b->color, sizeof b->color, "%s", "");
    b->urgent = 0;
    return 0;
}
