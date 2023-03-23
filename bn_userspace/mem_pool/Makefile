CC = gcc
CPP = g++
GCCFLAG = -Wall -lpthread
SRCS = mem_pool.c
MAIN_SRCS = test.c
MAIN_OUTPUT = test


check:
	$(CC) $(GCCFLAG) $(MAIN_SRCS) $(SRCS) -o $(MAIN_OUTPUT).out
	./$(MAIN_OUTPUT).out

.PHONY: clean
clean:
	rm *.out