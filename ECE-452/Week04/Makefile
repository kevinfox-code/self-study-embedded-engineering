CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
LDFLAGS = -lm

TARGET = test_svpwm
SRCS   = test_svpwm.c svpwm.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
