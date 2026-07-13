CC = gcc
CFLAGS = -O2 -Iinclude -Wall
LDFLAGS = -lgme

TARGET = test_runner
SRC = src/chiptune_gme.c src/gme_helper.c tests/test_runner.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
