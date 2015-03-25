
#clang++ -O3 -D__THREAD_LOG__ -Wno-int-to-pointer-cast -c easy_ch.cpp -o easy_ch.o
clang++ -O3  -Wno-int-to-pointer-cast -c easy_ch.cpp -o easy_ch.o
clang++ -D__MIGRATE__ -O3 -Wno-int-to-pointer-cast -c easy_ch.cpp -o easy_ch_mig.o

clang++ -O3 -Wall -o easy_ch easy_ch.o arglib.o -lpthread -lm
clang++ -O3 -Wall -o easy_ch_mig easy_ch_mig.o arglib.o -lpthread -lm -L/local/guilherme/guilherme/hwloc-1.8/build/lib -lhwloc

rm -f easy_ch.o
rm -f easy_ch_mig.o

clang++ -emit-llvm -O0 -Wno-int-to-pointer-cast -c easy_ch.cpp -o easy_ch.bc

opt -mem2reg -load /local/guilherme/llvm/Debug+Asserts/lib/SelectivePageMigration.so -spm easy_ch.bc -o easy_ch.spm.bc
opt -O3 easy_ch.spm.bc -o easy_ch.final.bc

#llvm-dis easy_ch.final.bc -o easy_ch.final.ll
llc easy_ch.final.bc -o easy_ch.s

clang++ -O3 -o easy_ch_spm easy_ch.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

clang++ -O3 -o easy_ch_spm.nl easy_ch.s arglib.o ~/guilherme/selective-page-migration-ccnuma/src/SelectivePageMigration/Runtime/SPM_nolock.o -L/local/guilherme/guilherme/hwloc-1.8/build/lib/ -lhwloc -lpthread -lnuma

rm -f *.bc
rm -f easy_ch.s

#llvm-link -o easy_ch.rbc easy_ch.bc arglib.bc
