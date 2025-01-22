CFLAGS = -O2 -Wall -Wextra -Wformat -Wformat-security \
		 -fstack-protector-strong -D_FORTIFY_SOURCE=2 \
		 -pipe -std=c11
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
