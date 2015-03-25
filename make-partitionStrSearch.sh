clang -g -O3 -o partitionStrSearch partitionStrSearch.c -lpthread 

clang -g -D__MIGRATE__ -O3 -o partitionStrSearch_mig partitionStrSearch.c -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib -lhwloc

clang -g -emit-llvm -O0 -c -o partitionStrSearch.bc partitionStrSearch.c

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm partitionStrSearch.bc -o partitionStrSearch.spm.bc
opt -O3 partitionStrSearch.spm.bc -o partitionStrSearch.final.bc

llc partitionStrSearch.final.bc -o partitionStrSearch.s

clang++ -O3 -o partitionStrSearch_spm partitionStrSearch.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

clang++ -O3 -o partitionStrSearch_spm.nl partitionStrSearch.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

rm -f *.bc partitionStrSearch.s
