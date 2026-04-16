CC = gcc
CFLAGS = -Wall -Wextra -I./include


TARGET=bin/citysup
SRCS = src/main.c #src/parser.c

all: $(TARGET)

$(TARGET) : $(SRCS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
