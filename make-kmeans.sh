clang -O3 -o kMeans kMeans.c -lpthread -lm

clang -D__MIGRATE__ -O3 -o kMeans_mig kMeans.c -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib -lhwloc

clang -emit-llvm -O0 -c -o kMeans.bc kMeans.c

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm kMeans.bc -o kMeans.spm.bc
opt -O3 kMeans.spm.bc -o kMeans.final.bc

llc kMeans.final.bc -o kMeans.s

clang++ -O3 -o kMeans_spm kMeans.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma -lm

clang++ -O3 -o kMeans_spm.nl kMeans.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma -lm


rm -f *.bc kMeans.s
