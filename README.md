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

## License

i3line is licensed under GPLv3.
