CC = gcc
INC = -I.
FLAGS += -O3 -Wextra -Wall
FLAGS += -Wno-unused
TESTERS = $(patsubst %.c, %, $(wildcard testers/*))

all: alloc.so contest-alloc.so mreplace mcontest testers

alloc.so: alloc.c
	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC

# contest-alloc.so: contest-alloc.c
# 	$(CC) $^ $(FLAGS) -o $@ -shared -fPIC -ldl

mreplace: mreplace.c
	$(CC) $^ $(FLAGS) -o $@

mcontest: mcontest.c
	$(CC) $^ $(FLAGS) -o $@ -ldl -lpthread

testers: $(subst testers, ., $(TESTERS))

%: testers/%.c
	$(CC) $^ $(FLAGS) -o $@

.PHONY : clean
clean:
	-rm -f *.o alloc.so mreplace mcontest tester-*
