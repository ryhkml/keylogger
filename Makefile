CFLAGS = -O2 -Wall -Wextra -Wformat -Wformat-security \
         -fstack-protector-strong -D_FORTIFY_SOURCE=2 \
         -pipe -std=c17 -MMD -MP
LDFLAGS = -fstack-protector-strong
LIBS =

TARGET = out/keylogger
BUILD_DIR = obj
SRC_DIR = src
TEST_DIR = test

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/util.c \
          $(SRC_DIR)/keylogger.c \
          $(SRC_DIR)/behavior_subject.c

OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)

ifdef USE_LIBWEBSOCKETS
    CFLAGS += -DUSE_LIBWEBSOCKETS
    LIBS += -lwebsockets
    SOURCES += $(SRC_DIR)/websocket.c
endif

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

TEST_SOURCES = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/test_%.c,$(BUILD_DIR)/test_%,$(TEST_SOURCES))

$(BUILD_DIR)/test_%: $(TEST_DIR)/test.c $(TEST_DIR)/test_%.c $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $^ $(LIBS) -o $@

test: $(TEST_BINS)
	@set -e; \
	for test_bin in $(TEST_BINS); do \
	    $$test_bin; \
	done

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
