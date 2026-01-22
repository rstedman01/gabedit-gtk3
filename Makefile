CC = gcc
CFLAGS = -std=c17 -O2 -pipe -Wno-deprecated-declarations -w

GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0)

EPOXY_LIBS := $(shell pkg-config --libs epoxy)
PANGO_LIBS := $(shell pkg-config --libs pangoft2)

GL_LIBS = -lGL -lGLU
GL2PS_LIBS = -lgl2ps
SPGLIBS = -lsymspg
LDFLAGS = -lm -lpthread

SRC := $(shell find src -name '*.c')
OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: gabedit-gtk3

gabedit-gtk3: $(OBJ)
	$(CC) -o $@ $^ $(PANGO_LIBS) $(GTK_LIBS) $(EPOXY_LIBS) $(GL_LIBS) $(GL2PS_LIBS) $(SPGLIBS) $(LDFLAGS)

# Compile rule for .c -> .o
%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) $(INCDIR) -c $< -o $@

clean:
	find . -name '*.o' -delete
	rm -f gabedit-gtk3