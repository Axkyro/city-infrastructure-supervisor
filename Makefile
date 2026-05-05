CC = gcc
CFLAGS = -Wall -I./include -g 

TARGET1=bin/city_manager
TARGET2=bin/monitor_reports
SRCS1 = src/main.c src/parser.c src/utils.c src/commands.c
SRCS2 = src/monitor.c src/parser.c src/utils.c src/commands.c

all: $(TARGET1) $(TARGET2)

$(TARGET1) : $(SRCS1)
	
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS1) -o $(TARGET1)

$(TARGET2) : $(SRCS2)
	
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS2) -o $(TARGET2)

clean:
	rm -f $(TARGET1) $(TARGET2)
