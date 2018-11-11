# i3line

A small and fast status line generator for i3bar written in C.

Yet another i3status replacement:
- i3status does not support click events
- i3blocks is a scheduler for shell scripts

The source code should be modular but the actual i3line program will be a
single compiled executable for efficiency.

## Design

- Update immediately on click events
- Always update every second

Wifi block depends on [wireless_tools][wireless_tools] which is somewhat
outdated and ideally should be rewritten for the new nl80211 API but
unfortunately this seems a bit more work than it is worth considering its lack
of high level methods.

New mail is checked using [notmuch][notmuch].

## License

Copyright (C) 2018 Jaan Toots <jaan@jaantoots.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

[wireless_tools]: https://hewlettpackard.github.io/wireless-tools/Tools.html
[notmuch]: http://notmuchmail.org/
