CPPFLAGS += -std=gnu99 -W -Wall $(shell pkg-config --cflags libcurl) -fopenmp -g
LDFLAGS += $(shell pkg-config --libs libcurl) -fopenmp

all: load-authn

load-authn: load-authn.o
	gcc $^ $(LDFLAGS) -o $@

.PHONY: all clean

clean:
	rm -f *.o
	rm -f load-authn
