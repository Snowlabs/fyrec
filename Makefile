CXX		= clang++
LDFLAGS =
CFLAGS	= -g -O2
SRC		= src/main.c
OBJ		= $(SRC:.c=.o)

fyrec: $(OBJ)
	$(CXX) -o $@ $?

%.o: %.c
	$(CXX) $(CFLAGS) -o $@ $?

clean:
	-rm $(OBJ)

distclean:
	-rm fyrec

.PHONY: clean distclean
