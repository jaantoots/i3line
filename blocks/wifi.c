/* i3line block: wifi
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
#include <iwlib.h>

#include "block.h"

#define NET "/proc/net/wireless"
#define BITRATE_LOW 100*1e6
#define BITRATE_HIGH 300*1e6

/* from wireless_tools.29/iwconfig.c */
static int get_info(int skfd, char *ifname, struct wireless_info *info) {
    struct iwreq wrq;
    memset((char *)info, 0, sizeof (struct wireless_info));
    /* get basic information */
    if(iw_get_basic_config(skfd, ifname, &(info->b)) < 0) {
        /* if no wireless name : no wireless extensions */
        /* but let's check if the interface exists at all */
        struct ifreq ifr;
        memcpy(ifr.ifr_name, ifname, IFNAMSIZ);
        if(ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
            perror("ioctl()");
            return -ENODEV;
        }
        else return -ENOTSUP;
    }
    /* get bit rate */
    if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0) {
        info->has_bitrate = 1;
        memcpy(&(info->bitrate), &(wrq.u.bitrate), sizeof (iwparam));
    }
    return 0;
}

int get_wireless_device(struct block *b) {
    if (strlen(b->instance)) {
        strncpy(b->path, b->instance, sizeof b->path);
        return 0;
    }
    /* find wireless devices */
    FILE *fp = fopen(NET, "r");
    if (fp == NULL) {
        perror("fopen()");
        return -1;
    }
    /* discard first two lines */
    if (fscanf(fp, "%*[^\n]\n%*[^\n]\n") == EOF) goto scan_error;
    /* pick first wireless device */
    if (fscanf(fp, "%" MAX_LEN_STR "[^:]", b->path) == EOF) goto scan_error;
    fclose(fp);
    return 0;
scan_error:
    perror(NET);
    fclose(fp);
    return -1;
}

int wifi(struct block *b) {
    if (b->state == BLOCK_NOTAVAIL) return -1;
    /* get device */
    if (b->state == BLOCK_RESET && (get_wireless_device(b) || (b->state = 0))) {
        b->state = BLOCK_NOTAVAIL;
        return -1;
    }
    /* create a channel to the NET kernel. */
    int skfd;
    if((skfd = iw_sockets_open()) < 0) {
        perror("socket");
        return -1;
    }
    /* get info */
    wireless_info info;
    int err = get_info(skfd, b->path, &info);
    /* close the socket. */
    iw_sockets_close(skfd);

    /* format output */
    long low = BITRATE_LOW;
    long high = BITRATE_HIGH;
    b->urgent = 0;
    if (!err && info.b.essid_on) {
        snprintf(b->full_text, sizeof b->full_text, "%s%s %.3gGHz %.3gMbps",
                wifi_icon, info.b.essid, info.b.freq/1e9, info.bitrate.value/1e6);
        snprintf(b->short_text, sizeof b->short_text, "%s", info.b.essid);
        snprintf(b->color, sizeof b->color, "%s",
                (info.bitrate.value < low) ? base0A : (info.bitrate.value < high) ? "" : base0B);
    }
    else {
        snprintf(b->full_text, sizeof b->full_text, "%s", wifi_icon);
        snprintf(b->short_text, sizeof b->short_text, "%s", "");
        snprintf(b->color, sizeof b->color, "%s", base08);
        if (err) b->urgent = 1;
    }
    return err;
}
