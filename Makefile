CFLAGS = -O2 -Wall -Wextra -std=c11
LIBS =

TARGET = out/keylogger

SOURCES = main.c keylogger.c behavior_subject.c

ifdef USE_LIBWEBSOCKETS
	CFLAGS += -DUSE_LIBWEBSOCKETS
	LIBS += -lwebsockets
	SOURCES += websocket.c
endif

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
