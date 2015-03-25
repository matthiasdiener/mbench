clang -O3 -o BucketSort BucketSort.c -lpthread 

clang -D__MIGRATE__ -O3 -o BucketSort_mig BucketSort.c -lpthread -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc


clang -emit-llvm -O0 -c -o BucketSort.bc BucketSort.c

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm BucketSort.bc -o BucketSort.spm.bc
opt -O3 BucketSort.spm.bc -o BucketSort.final.bc

llc BucketSort.final.bc -o BucketSort.s

clang++ -O3 -o BucketSort_spm BucketSort.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

clang++ -O3 -o BucketSort_spm.nl BucketSort.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma



rm -f *.bc BucketSort.s
