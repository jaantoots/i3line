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

#define DEBUG

static int cont = 1;
static void handler(int signum) {
    /* end loop and exit on SIGHUP or SIGTERM
     * return from pselect on SIGUSR1 */
    if (signum != SIGUSR1) cont = 0;
}

int update(const char *line) {
    /* update output */
    if (line != NULL) return printf("got line: %s", line);
    return printf("meh\n");
}

int interval_loop(const int interval) {
    /* update on input, signals, or aligned seconds */
    struct timespec now, next, timeout;
    if (clock_gettime(CLOCK_REALTIME, &next)) {
        perror("clock_gettime()");
        exit(EXIT_FAILURE);
    }
    next.tv_nsec = 0;
    /* cont is set to zero on SIGHUP or SIGTERM */
    while (cont) {
        update(NULL); // this is here to handle signals correctly
wait:;
        if (clock_gettime(CLOCK_REALTIME, &now)) {
            perror("clock_gettime()");
            exit(EXIT_FAILURE);
        }
        /* ensure next is in the future */
        if (now.tv_sec >= next.tv_sec && now.tv_nsec >= next.tv_nsec)
            next.tv_sec = now.tv_sec + interval;
        timeout.tv_sec = next.tv_sec - now.tv_sec;
        timeout.tv_nsec = next.tv_nsec - now.tv_nsec;
        if (timeout.tv_nsec < 0) {
            timeout.tv_sec -= 1;
            timeout.tv_nsec += 1000000000;
        }

#ifdef DEBUG
        char buff[100];
        strftime(buff, sizeof buff, "%F %T", gmtime(&now.tv_sec));
        fprintf(stderr, "%s.%09ld\n", buff, now.tv_nsec);
#endif /* DEBUG */

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        /* wait upto timeout for a signal or stdin to be readable */
        if (pselect(1, &rfds, NULL, NULL, &timeout, NULL) > 0) {
            /* positive return value means stdin should be ready */
            char *line = NULL;
            size_t n = 0;
            if (getline(&line, &n, stdin) != -1) update(line);
            free(line);
            goto wait; // skip the empty update
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    /* specify signal handler to exit cleanly or trigger updates */
    struct sigaction sa, oa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, NULL, &oa);
    if (oa.sa_handler != SIG_IGN) sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, NULL, &oa);
    if (oa.sa_handler != SIG_IGN) sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, NULL, &oa);
    if (oa.sa_handler != SIG_IGN) sigaction(SIGUSR1, &sa, NULL);

    /* run loop */
    const int interval = (argc > 1) ? atoi(argv[1]) : 4;
    return interval_loop(interval);
}
