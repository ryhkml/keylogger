CFLAGS = -O2 -Wall -Wextra -Wformat -Wformat-security \
         -fstack-protector-strong -D_FORTIFY_SOURCE=2 \
         -pipe -std=c11 -MMD -MP
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

TEST_SOURCES = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SOURCES))

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

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BUILD_DIR)/%: $(TEST_DIR)/%.c $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) $^ $(LIBS) -lcmocka -o $@

test: $(TEST_BINS)
	for test_bin in $^; do $$test_bin; done

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
