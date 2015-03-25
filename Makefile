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


# ./BucketSort 3000000 64 0
# ./easy_add 1 20000 64
