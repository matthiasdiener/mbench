SRC=BucketSort.cpp easy_add.cpp easy_ch.cpp easy_lu.cpp easy_prod.cpp kMeans.cpp KNN.cpp partitionStrSearch.cpp

BIN=$(subst .cpp,,$(SRC))

CFLAGS=-pthread -O2 -lm
CXXFLAGS=-pthread -O2

CC=gcc
CXX=g++
DEPS =

all: $(BIN)

clean:
	rm -f $(BIN)

#### Parameters (~ 5 seconds execution time on turing):
# ./BucketSort 3000000 64 0
# ./easy_add 64 5000 64     # large serial phase
# ./easy_ch 64 1000 64
# ./easy_lu 64 600 64
# ./easy_prod 64 600 64
# ./kMeans ?????
# ./KNN 64 500000 20
# ./partitionStrSearch 64 10000000 10000 0
