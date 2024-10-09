CC = gcc
CFLAGS = -Wall -Wextra -g -pthread -Iinclude

SRCDIR = src
INCDIR = include
OBJDIR = bin
BINDIR = server

# The target executable
TARGET = $(BINDIR)/server

SRCS = $(wildcard $(SRCDIR)/*.c)

OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/*

.PHONY: all clean