BINS = pdirac
OBJS = options.o

DIRAC_BINS = pdirac

CFLAGS += -std=c99 -Werror -Wall -ggdb
CXXFLAGS += -Werror -Wall -ggdb
$(DIRAC_BINS): CFLAGS += -m32
$(DIRAC_BINS): LDLIBS += -lDiracLE -lstdc++ -lm

.PHONY: all install clean

all: $(BINS)

install: all
	install -D pdirac $(DESTDIR)/usr/bin/pdirac

clean:
	$(RM) $(BINS) $(OBJS)

pdirac: options.o
