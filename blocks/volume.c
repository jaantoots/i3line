/* i3line block: volume
 * Copyright (C) 2019 Jaan Toots <jaan@jaantoots.org>
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
#include <alsa/asoundlib.h>

#include "block.h"

#define INC 0.05

int volume(struct block *b) {
    int err = -1;
    /* open alsa mixer */
    snd_mixer_t *mixer;
    if (snd_mixer_open(&mixer, 0)) goto erro;
    if (snd_mixer_attach(
                mixer, (strlen(b->instance) > 0) ? b->instance : "default"))
        goto err;
    if (snd_mixer_selem_register(mixer, NULL, NULL)) goto err;
    if (snd_mixer_load(mixer)) goto err;

    /* find the mixer */
    snd_mixer_selem_id_t *sid;
    if (snd_mixer_selem_id_malloc(&sid)) goto err;
    err = -2;
    const char *selem_name = "Master";
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(mixer, sid);
    snd_mixer_selem_id_free(sid);
    if (!(elem)) goto err;

    /* get the volume */
    long min, max;
    if (snd_mixer_selem_get_playback_volume_range(elem, &min, &max)) goto err;
    long volume;
    if (snd_mixer_selem_get_playback_volume(elem, 0, &volume)) goto err;
    int unmuted;
    if (snd_mixer_selem_get_playback_switch(elem, 0, &unmuted)) goto err;
    double volp = (double)(volume - min)/(max - min);

    /* handle button */
    switch (b->button) {
        case 3:
            /* mute/unmute volume */
            if (snd_mixer_selem_set_playback_switch_all(
                        elem, unmuted = !(unmuted))) goto err;
            break;
        case 4:
            /* decrease/increase volume */
            volp -= 2*INC;
        case 5:
            volp += INC;
            volume = volp*(max - min) + min;
            if (snd_mixer_selem_set_playback_volume_all(elem, volume))
                goto err;
            break;
    }

    /* print backlight */
    const char *icon = volume_icons[2];
    if (!(unmuted)) icon = volume_icons[0];
    else if (volp <= 0.3) icon = volume_icons[1];
    snprintf(b->full_text, sizeof b->full_text,
            "%s%.0lf%%", icon, 100*volp);
    snprintf(b->short_text, sizeof b->short_text,
            "%.0lf%%", 100*volp);
    snprintf(b->color, sizeof b->color, "%s", (unmuted) ? "" : base0A);
    b->urgent = 0;
    err = 0;

err:
    if (err) fprintf(stderr, "snd_mixer: error\n");
    snd_mixer_close(mixer);
erro:
    return err;
}
