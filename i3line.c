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
#include <time.h>
#include <signal.h>
#include <sys/select.h>

static int cont = 1;
static void handler(int signum) {
    if (signum != SIGUSR1) cont = 0;
}

int wait_read(int (*fp)(const char *line), const struct timespec *timeout) {
    /* wait upto timeout to run fp with optional line from stdin */
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    int r = pselect(1, &rfds, NULL, NULL, timeout, NULL);
    if (r < 0) {
        perror("select()");
        return fp(NULL);
    }
    /* no input */
    if (r == 0) return fp(NULL);
    /* stdin should be ready */
    char *line = NULL;
    size_t n = 0;
    int val = 0;
    if (getline(&line, &n, stdin) != -1) val = fp(line);
    free(line);
    return val;
}

int update(const char *line) {
    /* update output */
    if (line != NULL) return printf("got line: %s---\n", line);
    return printf("meh\n");
}

int interval_loop(const int interval) {
    /* update on input and aligned seconds */
    struct timespec now, next, timeout;
    if (clock_gettime(CLOCK_REALTIME, &next)) {
        perror("clock_gettime()");
        exit(EXIT_FAILURE);
    }
    next.tv_nsec = 0;
    while (cont) {
        if (clock_gettime(CLOCK_REALTIME, &now)) {
            perror("clock_gettime()");
            exit(EXIT_FAILURE);
        }
        /* ensure next is in the future */
        if (now.tv_sec >= next.tv_sec && now.tv_nsec >= next.tv_nsec) {
            next.tv_sec = now.tv_sec + interval;
        }
        timeout.tv_sec = next.tv_sec - now.tv_sec;
        timeout.tv_nsec = next.tv_nsec - now.tv_nsec;
        if (timeout.tv_nsec < 0) {
            timeout.tv_sec -= 1;
            timeout.tv_nsec += 1000000000;
        }
        /* wait and update */
        char buff[100];
        strftime(buff, sizeof buff, "%F %T", gmtime(&now.tv_sec));
        printf("%s.%09ld\n", buff, now.tv_nsec);
        wait_read(update, &timeout);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    struct sigaction sa, oa;
    /* use handler to end loop and exit on SIGHUP or SIGTERM */
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, NULL, &oa);
    if (oa.sa_handler != SIG_IGN) sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, NULL, &oa);
    if (oa.sa_handler != SIG_IGN) sigaction(SIGTERM, &sa, NULL);
    /* ignore SIGUSR1 to only return from pselect */
    sigaction(SIGUSR1, NULL, &oa);
    if (oa.sa_handler != SIG_IGN) sigaction(SIGUSR1, &sa, NULL);

    /* run loop */
    const int interval = 4;
    return interval_loop(interval);
}
