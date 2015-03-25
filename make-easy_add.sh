
clang++ -O3 -Wno-int-to-pointer-cast -c easy_add.cpp -o easy_add.o
clang++ -D__MIGRATE__ -O3 -Wno-int-to-pointer-cast -c easy_add.cpp -o easy_add_mig.o

clang++ -O3 -Wall -o easy_add easy_add.o arglib.o -lpthread -lm
clang++ -O3 -Wall -o easy_add_mig easy_add_mig.o arglib.o -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc

rm -f easy_add.o easy_add_mig.o


clang++ -emit-llvm -O0 -Wno-int-to-pointer-cast -c easy_add.cpp -o easy_add.bc

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm easy_add.bc -o easy_add.spm.bc
opt -O3 easy_add.spm.bc -o easy_add.final.bc

llc easy_add.final.bc -o easy_add.s

clang++ -O3 -o easy_add_spm easy_add.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

clang++ -O3 -o easy_add_spm.nl easy_add.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma


rm -f *.bc easy_add.s

