SRC=easy_add.cpp easy_ch.cpp easy_lu.cpp easy_prod.cpp kMeans.cpp KNN.cpp partitionStrSearch.cpp
BIN=$(subst .cpp,,$(SRC))

# SRC = $(patsubst %,$(BIN)/%,$(BIN).cpp))

CFLAGS=-pthread -O2 -lm
CXXFLAGS=-pthread -O2

CC=gcc
CXX=g++
DEPS =

# %.o: %.c $(DEPS)
# 	$(CC) -c -o $@ $< $(CFLAGS)

# %.o: %.cpp $(DEPS)
# 	$(CXX) -c -o $@ $< $(CFLAGS)

all: $(BIN)

# $(BIN): $(DEPS)
# 	$(CXX) -c -o $@ $< $(CFLAGS)


clean:
	rm -f $(BIN)
