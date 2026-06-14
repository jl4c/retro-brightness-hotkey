# Makefile for retro-brightness-hotkey

CC      ?= clang
CFLAGS  ?= -Os -Wall -Wextra
LDFLAGS ?= -s

PREFIX   := /usr
BINDIR   := $(DESTDIR)$(PREFIX)/bin
DINITDIR := $(DESTDIR)$(PREFIX)/lib/dinit.d
 
TARGET := retro-brightness-hotkey
SRC    := retro-brightness-hotkey.c
 
all: $(TARGET)
 
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
 
install: $(TARGET)
	install -Dm755 $(TARGET) $(BINDIR)/$(TARGET)
	install -Dm644 dinit.d/$(TARGET) $(DINITDIR)/$(TARGET)
 
clean:
	rm -f $(TARGET)
 
.PHONY: all install clean
