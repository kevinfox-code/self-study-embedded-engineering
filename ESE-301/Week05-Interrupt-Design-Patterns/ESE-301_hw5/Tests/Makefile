CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c11 -I../Core/Inc

SRCS    := test_pubsub.c \
           ../Core/Src/pubsub.c \
           ../Core/Src/scheduler.c

TARGET  := test_pubsub

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
