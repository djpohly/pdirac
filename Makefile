BINS = pdirac ffdirac foo

DIRAC_BINS = pdirac foo
FFMPEG_BINS = ffdirac

CFLAGS += -std=c99 -Werror -Wall -Wno-unused -ggdb
CXXFLAGS += -Werror -Wall -Wno-unused -ggdb
$(DIRAC_BINS): CFLAGS += -m32
$(DIRAC_BINS): LDLIBS += -lDiracLE -lstdc++ -lm
$(FFMPEG_BINS): LDLIBS += -lavcodec -lavformat -lavutil

.PHONY: all install clean

all: $(BINS)

install: all
	install -d $(DESTDIR)/usr/bin/ffdirac
	install -t $(DESTDIR)/usr/bin/ffdirac $(BINS)

clean:
	$(RM) $(BINS)

pdirac: options.o
