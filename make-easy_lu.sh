
clang++ -g -O3  -Wno-int-to-pointer-cast -c easy_lu.cpp -o easy_lu.o
clang++ -D__MIGRATE__ -O3 -Wno-int-to-pointer-cast -c easy_lu.cpp -o easy_lu_mig.o

clang++ -O3 -Wall -o easy_lu easy_lu.o arglib.o -lpthread -lm
clang++ -O3 -Wall -o easy_lu_mig easy_lu_mig.o arglib.o -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc

rm -f easy_lu.o
rm -f easy_lu_mig.o

clang++ -emit-llvm -O0 -Wno-int-to-pointer-cast -c easy_lu.cpp -o easy_lu.bc

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm easy_lu.bc -o easy_lu.spm.bc
opt -O3 easy_lu.spm.bc -o easy_lu.final.bc

#llvm-dis easy_lu.final.bc -o easy_lu.final.ll
llc easy_lu.final.bc -o easy_lu.s

clang++ -O3 -o easy_lu_spm easy_lu.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

clang++ -O3 -o easy_lu_spm.nl easy_lu.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

rm -f *.bc
rm -f easy_lu.s

#llvm-link -o easy_lu.rbc easy_lu.bc arglib.bc

