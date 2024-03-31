CC=gcc
CFLAGS=-Wall -Wextra -pedantic -ggdb2 -Werror
DEFINES=
INCLUDES=
LIBS=

SRCDIR=src
BUILDDIR=build

ifeq ($(BUILD_TYPE), DEBUG)
CFLAGS += -g -ggdb2
endif

SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC))

BINARYNAME=main
BINARY=$(BUILDDIR)/$(BINARYNAME)

.PHONY: all setup clean destroy

all: $(BINARY)

$(BINARY): $(BUILDDIR)/$(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJ) -o $(BINARY) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

setup:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BINARY)
	rm -rf $(OBJ)

destroy:
	rm -rf $(BUILDDIR)
