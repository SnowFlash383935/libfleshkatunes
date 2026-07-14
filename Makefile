CC = gcc
CFLAGS = -O2 -Iinclude -Wall
LDFLAGS = -lgme -lz

TARGET = test_runner
SRC = src/chiptune.c src/chiptune_gme.c src/gme_helper.c tests/test_runner.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
