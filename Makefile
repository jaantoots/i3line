CFLAGS := -O2 -Wall -I.

blocks := $(wildcard blocks/*.c)
ALL := i3line
OBJS := $(blocks:.c=.o) $(ALL:=.o)

.PHONY: all
all: $(ALL)

$(OBJS): block.h icons.h colors.h utils.h

i3line: LDLIBS := -ljson-c -lasound -liw -lnotmuch
i3line: $(OBJS)

.PHONY: clean
clean:
	-rm $(ALL) $(OBJS)
