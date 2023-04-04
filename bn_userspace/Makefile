CC := gcc
CFLAGS := -Wall -O2

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

OBJS := \
	fib.o\
	bn.o
deps := $(OBJS:%.o=.%.o.d)

OBJS2 := \
	fib_measurement.o\
	bn.o
deps2 := $(OBJS2:%.o=.%.o.d)

OBJS3 := \
	fib_show.o\
	bn.o\
	mem_pool.o
deps3 := $(OBJS3:%.o=.%.o.d)

fib: $(OBJS)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^

fib_m: $(OBJS2)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^

fib_show: $(OBJS3)
	$(VECHO) "  LD\t$@\n"
	$(Q)$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(VECHO) "  CC\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<


.PHONY: check clean

# Check fib algorithm with python scipt
check: fib_show
	sh check_fibonacci.sh

# Generate output that perf measured
check_p: fib
	sh perf_test.sh $(ver)

# Generate output for plot compare picture
check_m: fib_m
	@echo VER $(ver)
	./fib_m >> out_$(ver)

clean:
	rm -f $(OBJS) $(OBJS2) $(OBJS3) $(deps) $(deps2) $(deps3)
	rm -f out
	$(RM) fib fib_m fib_show

-include $(deps)