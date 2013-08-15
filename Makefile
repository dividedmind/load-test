CPPFLAGS += -std=gnu99 -W -Wall $(shell pkg-config --cflags libcurl)
LDFLAGS += $(shell pkg-config --libs libcurl)

all: load-authn

load-authn: load-authn.o conjur/authn.o
	gcc $^ $(LDFLAGS) -o $@

.PHONY: all
