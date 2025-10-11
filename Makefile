CC ?= gcc
CFLAGS ?= -std=c17 -O2 -pipe -Wall -Wextra -Wno-deprecated-declarations
INCDIR ?= $(pkg-config --cflags gtk+-3.0)
GTK_CFLAGS := $(pkg-config --cflags gtk+-3.0)
GTK_LIBS   := $(pkg-config --libs gtk+-3.0)
EPOXY_LIBS := $(pkg-config --libs epoxy)

SRC := $(shell find src -name '*.c')
OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: gabedit-gtk3

gabedit-gtk3: $(OBJ)
	$(CC) -o $@ $^ $(GTK_LIBS) $(EPOXY_LIBS) -lm

# Compile rule for .c -> .o
%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) $(INCDIR) -c $< -o $@

clean:
	find . -name '*.o' -delete
	rm -f gabedit-gtk3