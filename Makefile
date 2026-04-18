CC = gcc
CFLAGS = -Wall -I./include -g


TARGET=bin/citysup
SRCS = src/main.c src/parser.c src/utils.c src/commands.c

all: $(TARGET)

$(TARGET) : $(SRCS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
