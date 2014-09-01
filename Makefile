# Copyleft Williham Totland <williham.totland@gmail.com>
#
# This file is in the public domain, or under Creative Commons' CC0 where required by law.

CC=clang++
CFLAGS=-Wall -Iinc -I/usr/X11/include
LFLAGS=-L/usr/X11/lib -L./inc
LDFLAGS=-lpng

TDIR=
TODIR=
ODIR=obj
DDIR=obj

SRC_DIR=src

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
_OBJS = $(SOURCES:$(SRC_DIR)/%.cpp=%.o)

_TEST = 
TEST = $(patsubst %,$(TODIR)/%.test,$(_TEST))

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJS))

BioSim: $(ODIR) $(OBJ)
	$(CC) -o $@ $(OBJ) $(LFLAGS) $(LDFLAGS)

documentation : Doxyfile
	doxygen >/dev/null

$(ODIR):
	mkdir -p $(ODIR)

$(ODIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -MMD -c -g -std=c++11 -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -rf obj doc BioSim

-include $(SOURCES:$(SRC_DIR)/%.cpp=$(DEPDIR)/%.P)
-include ${OBJ:.o=.d}
