CFLAGS := -O2 -Wall -I.

blocks := $(wildcard blocks/*.c)
ALL := i3line
OBJS := $(blocks:.c=.o) $(ALL:=.o)

.PHONY: all
all: $(ALL)

$(OBJS): block.h

i3line: LDLIBS := $(shell pkg-config --libs json-c)
i3line: $(OBJS)

.PHONY: clean
clean:
	-rm $(ALL) $(OBJ)