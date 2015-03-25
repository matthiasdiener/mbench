BIN=BucketSort easy_add easy_ch easy_lu easy_prod kMeans KNN partitionStrSearch
BIN_X=$(addsuffix .x,$(BIN))

CFLAGS=-pthread -O2
CXXFLAGS=-pthread -O2

CC=gcc
CXX=g++
LDLIBS=-lm
DEPS =

all: $(BIN)
	@for bin in $(BIN); do cp $$bin $$bin.x; done

clean:
	rm -f $(BIN) $(BIN_X)

#### Parameters (~ 5 seconds execution time on turing):
# ./BucketSort 3000000 64 0
# ./easy_add 64 5000 64     # large serial phase
# ./easy_ch 64 1000 64
# ./easy_lu 64 600 64
# ./easy_prod 64 600 64
# ./kMeans ?????
# ./KNN 64 500000 20
# ./partitionStrSearch 64 10000000 10000 0
