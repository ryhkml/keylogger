CFLAGS = -O2 -Wall -std=c11 -lwebsockets

TARGET = out/keylogger

SOURCES = main.c keylogger.c behavior_subject.c websocket.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
