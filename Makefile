CFLAGS = -O2 -Wall -Wextra -std=c11

TARGET = out/keylogger

SOURCES = main.c keylogger.c behavior_subject.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
