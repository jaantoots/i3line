CFLAGS := -O2 -Wall
DEPS := block.h
ALL := i3line

.PHONY: all
all: $(ALL)

%: %.c $(DEPS)

i3line: LDLIBS := $(shell pkg-config --libs json-c)

.PHONY: clean
clean:
	-rm $(ALL)
