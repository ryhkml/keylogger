CFLAGS = -O2 -Wall -Wextra -std=c11
LIBS =

TARGET = out/keylogger

SOURCES = main.c keylogger.c behavior_subject.c

ifdef USE_LIBWEBSOCKETS
	CFLAGS += -DUSE_LIBWEBSOCKETS
	LIBS += -lwebsockets
	SOURCES += websocket.c
endif

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
