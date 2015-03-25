clang -O3 -o KNN KNN.c -lpthread -lm

clang -D__MIGRATE__ -O3 -o KNN_mig KNN.c -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib -lhwloc

clang -emit-llvm -O0 -c -o KNN.bc KNN.c

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm KNN.bc -o KNN.spm.bc
opt -O3 KNN.spm.bc -o KNN.final.bc

llc KNN.final.bc -o KNN.s

clang++ -O3 -o KNN_spm KNN.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma -lm

clang++ -O3 -o KNN_spm.nl KNN.s ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma -lm


rm -f *.bc KNN.s
