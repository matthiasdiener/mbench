clang++ -O3 -Wno-int-to-pointer-cast -c easy_prod.cpp -o easy_prod.o
clang++ -D__MIGRATE__ -O3 -Wno-int-to-pointer-cast -c easy_prod.cpp -o easy_prod_mig.o

clang++ -O3 -Wall -o easy_prod easy_prod.o arglib.o -lpthread -lm
clang++ -O3 -Wall -o easy_prod_mig easy_prod_mig.o arglib.o -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc

rm -f easy_prod.o easy_prod_mig.o


clang++ -emit-llvm -O0 -Wno-int-to-pointer-cast -c easy_prod.cpp -o easy_prod.bc

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm easy_prod.bc -o easy_prod.spm.bc
opt -O3 easy_prod.spm.bc -o easy_prod.final.bc

llc easy_prod.final.bc -o easy_prod.s

clang++ -O3 -o easy_prod_spm easy_prod.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

clang++ -O3 -o easy_prod_spm.nl easy_prod.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma


rm -f *.bc
rm -f easy_prod.s

