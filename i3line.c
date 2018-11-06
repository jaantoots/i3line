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
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include "block.h"

#define DEBUG

#define JSON_OBJECT_STRING_ADD(obj, key, from) \
    json_object_object_add(obj, #key, json_object_new_string(from->key))

struct block blocks[] = { { "date", "", date } };

static int cont = 1;
static void handler(int signum) {
    /* end loop and exit on SIGHUP or SIGTERM
     * return from pselect on SIGUSR1 */
    if (signum != SIGUSR1) cont = 0;
}

int process_event(const char *line) {
    /* process input */
    if (line == NULL) return 0;
#ifdef DEBUG
    fprintf(stderr, "%s", line);
#endif /* DEBUG */
    while (*line != '\0' && *line != '{') ++line;
    if (!strlen(line)) return 0;
    /* parse the input into a JSON object */
    json_tokener *tok = json_tokener_new();
    json_object *event = json_tokener_parse_ex(tok, line, -1);
    enum json_tokener_error err = json_tokener_get_error(tok);
    if (err != json_tokener_success) {
        fprintf(stderr, "json_tokener: %s\n", json_tokener_error_desc(err));
        return -1;
    }
    json_tokener_free(tok);
    if (!json_object_is_type(event, json_type_object)) {
        fprintf(stderr, "json_object: wrong object type\n");
        return -1;
    }
    /* set button for given block */
    const char *name = json_object_get_string(
            json_object_object_get(event, "name"));
    const char *instance = json_object_get_string(
            json_object_object_get(event, "instance"));
    for (struct block *b = blocks;
            b - blocks < sizeof blocks / sizeof (struct block); ++b) {
        if (!strcmp(b->name, name) && !strcmp(b->instance, instance)) {
            b->button = json_object_get_int(
                    json_object_object_get(event, "button"));
            break;
        }
    }
    json_object_put(event);
    return 0;
}

void update(const char *line) {
    /* reset buttons and process click event */
    for (struct block *b = blocks;
            b - blocks < sizeof blocks / sizeof (struct block); ++b)
        b->button = 0;
    process_event(line);
    /* update output */
    json_object *arr = json_object_new_array();
    for (struct block *b = blocks;
            b - blocks < sizeof blocks / sizeof (struct block); ++b) {
        /* update block and append block JSON */
        b->update(b);
        json_object *obj = json_object_new_object();
        JSON_OBJECT_STRING_ADD(obj, name, b);
        JSON_OBJECT_STRING_ADD(obj, instance, b);
        JSON_OBJECT_STRING_ADD(obj, full_text, b);
        if (strlen(b->short_text)) JSON_OBJECT_STRING_ADD(obj, short_text, b);
        if (strlen(b->color)) JSON_OBJECT_STRING_ADD(obj, color, b);
        json_object_object_add(obj, "urgent",
                json_object_new_boolean(b->urgent));
        json_object_array_add(arr, obj);
    }
    const char *str = json_object_to_json_string(arr);
    printf(",%s\n", str);
    json_object_put(arr);
    fflush(stdout);
}

int interval_loop(const int interval) {
    /* initialize output */
    printf("{\"version\": 1, \"click_events\": true}\n[\n");
    json_object *arr = json_object_new_array();
    const char *str = json_object_to_json_string(arr);
    printf("%s\n", str); // placeholder so that commas can be prepended
    json_object_put(arr);
    fflush(stdout);

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
    /* finalize output */
    printf("]\n");
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
    const int interval = (argc > 1) ? atoi(argv[1]) : 1;
    return interval_loop(interval);
}
