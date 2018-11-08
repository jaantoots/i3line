/* i3line block: notmuch
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
#include <notmuch.h>

#include "block.h"

int notmuch(struct block *b) {
    /* get notmuch database path */
    if (b->state == BLOCK_RESET &&
            ((snprintf(b->path, sizeof b->path, "%s/%s",
                       (b->instance[0] != '/') ? getenv("HOME") : "",
                       strlen(b->instance) ? b->instance : "mail") <= 0) ||
             (b->state = 0)))
        return -1;

    /* query notmuch database */
    notmuch_database_t *db;
    if (notmuch_database_open(b->path, NOTMUCH_DATABASE_MODE_READ_ONLY, &db)
            != NOTMUCH_STATUS_SUCCESS)
        return -1;
    unsigned int count;
    notmuch_query_t *query =
        notmuch_query_create(db, "folder:INBOX tag:unread");
    notmuch_status_t st = notmuch_query_count_messages(query, &count);
    notmuch_query_destroy(query);
    notmuch_database_close(db);
    if (st != NOTMUCH_STATUS_SUCCESS) return -1;

    /* format block */
    const unsigned int high = 8;
    const unsigned int critical = 16;
    snprintf(b->full_text, sizeof b->full_text, "%s%d", mail_icon, count);
    snprintf(b->short_text, sizeof b->short_text, "%d", count);
    snprintf(b->color, sizeof b->color, "%s",
            (count > critical) ? base08 : (count > high) ? base0A : "");
    b->urgent = 0;
    return 0;
}
