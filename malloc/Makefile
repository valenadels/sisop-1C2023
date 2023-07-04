CFLAGS := -ggdb3 -Wall -Wextra -std=gnu11
#CFLAGS += -Wmissing-prototypes

# To compile using different strategies:
# - For First Free
#     make -B -e USE_FF=true
# - For Best Free
#     make -B -e USE_BF=true
ifdef USE_FF
	CFLAGS += -D FIRST_FIT
endif
ifdef USE_BF
	CFLAGS += -D BEST_FIT
endif

TESTS := malloc.test
SRCS := $(filter-out malloc.test.c, $(wildcard *.c))
OBJS := $(SRCS:%.c=%.o)

all: $(TESTS)

%.test: $(OBJS) %.test.o
	cc $(CFLAGS) -o $@ $^

test: $(TESTS)
	./$(TESTS)

format: .clang-files .clang-format
	xargs -r clang-format -i <$<

clean:
	rm -f *.o $(TESTS)

.PHONY: clean format test
