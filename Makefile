CC=gcc
CFLAGS=-Wall -Wextra -pedantic -ggdb2
DEFINES=
INCLUDES=-Isrc
LIBS=

SRCDIR=src
BUILDDIR=build

ifeq ($(BUILD_TYPE), DEBUG)
CFLAGS += -g -ggdb2
endif

SRC=$(wildcard $(SRCDIR)/*.c)
OBJ=$(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC))

TIREDIR=$(BUILDDIR)/tire
TIRESRC=$(wildcard $(SRCDIR)/tire/*.c)
TIREOBJ=$(patsubst $(SRCDIR)/tire/%.c, $(TIREDIR)/%.o, $(TIRESRC))

TIRENAME=tire
TIRE=$(TIREDIR)/$(TIRENAME)

TASMDIR=$(BUILDDIR)/tasm
TASMSRC=$(wildcard $(SRCDIR)/tasm/*.c)
TASMOBJ=$(patsubst $(SRCDIR)/tasm/%.c, $(TASMDIR)/%.o, $(TASMSRC))

TASMNAME=tasm
TASM=$(TASMDIR)/$(TASMNAME)

.PHONY: all setup clean destroy test

all: $(BUILDDIR)/$(OBJ) $(TIRE) $(TASM)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(TIRE): $(BUILDDIR)/$(TIREOBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(TIREOBJ) $(OBJ) -o $(TIRE) $(LIBS)

$(BUILDDIR)/tire/%.o: $(SRCDIR)/tire/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(TASM): $(BUILDDIR)/$(TASMOBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(TASMOBJ) $(OBJ) -o $(TASM) $(LIBS)

$(BUILDDIR)/tasm/%.o: $(SRCDIR)/tasm/%.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

setup:
	mkdir -p $(BUILDDIR)
	mkdir -p $(TIREDIR)
	mkdir -p $(TASMDIR)

clean:
	rm -rf $(TIRE)
	rm -rf $(TIREOBJ)
	rm -rf $(TASM)
	rm -rf $(TASMOBJ)
	rm -rf $(OBJ)

destroy:
	rm -rf $(BUILDDIR)

test:
	$(foreach file, $(wildcard tests/*.tasm), ./build/tasm/tasm $(file);)
