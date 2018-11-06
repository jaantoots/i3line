/* i3line block: load
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
#include <time.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "block.h"

#define TERMINAL "i3-sensible-terminal"

void htop() {
    /* fork htop */
    int pid = fork();
    if (!(pid)) {
        if (setpgid(0, 0)) exit(EXIT_FAILURE);
        pid = fork();
        if (pid < 0) exit(EXIT_FAILURE);
        if (pid > 0) exit(EXIT_SUCCESS);
        for (long i = sysconf(_SC_OPEN_MAX); i >= 0; --i) close(i);
        execlp(TERMINAL, TERMINAL, "-e", "htop", NULL);
    }
}

int nelem = 1;

int loadavg(struct block *b) {
    /* handle button */
    switch (b->button) {
        case 1: nelem = (nelem == 1) ? 3 : 1;
                break;
        case 3: htop();
                break;
    }

    double load[3];
    if (getloadavg(load, nelem) != nelem) return 1;

    if (nelem == 3)
        snprintf(b->full_text, sizeof b->full_text, "%s%.2f %.2f %.2f",
                load_icon, load[0], load[1], load[2]);
    else
        snprintf(b->full_text, sizeof b->full_text, "%s%.2f",
                load_icon, load[0]);
    snprintf(b->short_text, sizeof b->short_text, "%.2f", load[0]);
    snprintf(b->color, sizeof b->color, "%s",
            (load[0] > get_nprocs()) ? base08 : (
                (load[0] > get_nprocs()/2) ? base0A : ""));
    b->urgent = 0;
    return 0;
}
