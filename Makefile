CC = gcc
CFLAGS = -Wall -I./include -g 

TARGET1=bin/city_manager
TARGET2=bin/monitor_reports
TARGET3=bin/city_hub
TARGET4=bin/scorer

SRCS1 = src/main.c src/parser.c src/utils.c src/commands.c
SRCS2 = src/monitor.c src/parser.c src/utils.c src/commands.c
SRCS3 = src/hub.c	src/utils.c	src/commands.c	src/parser.c
SRCS4 = src/scorer.c	src/utils.c	src/commands.c	src/parser.c

.PHONY: all clean

all: $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)

$(TARGET1) : $(SRCS1)
	
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS1) -o $(TARGET1)

$(TARGET2) : $(SRCS2)
	
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS2) -o $(TARGET2)

$(TARGET3) : $(SRCS3)
	
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS3) -o $(TARGET3)

$(TARGET4) : $(SRCS4)
	
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRCS4) -o $(TARGET4)


clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)
