/* i3line
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
#include <sys/select.h>

int wait_read(const int secs) {
    struct timeval timeout;
    timeout.tv_sec = secs;
    timeout.tv_usec = 0;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    int r = select(1, &rfds, NULL, NULL, &timeout);
    if (r < 0) {
        perror("select()");
        return r;
    }
    if (r == 0) return 0;
    /* stdin should be ready */
    if (FD_ISSET(0, &rfds)) printf("stdin ready\n");
    char *line = NULL;
    size_t n = 0;
    if (getline(&line, &n, stdin) != -1) printf("got line: %s---\n", line);
    free(line);
    return 0;
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < 8; ++i) {
        printf("waiting\n");
        wait_read(4);
    }
    return 0;
}
