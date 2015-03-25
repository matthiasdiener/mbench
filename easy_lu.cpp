// LU

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/time.h>
#include <errno.h>
#include <vector>

// #include "arglib.h"


#ifdef __MIGRATE__

	#include <hwloc.h>
	hwloc_topology_t topology;
/*
extern "C" {
  void __spm_init();
  void __spm_end();
  void __spm_get (void *Array, long Start, long End, long Reuse);
}
*/
#endif

#ifdef __THREAD_LOG__
	#include <sys/syscall.h>
	pid_t *thread_ids;
	pthread_mutex_t tid_lock;
	char work_busy_lock;
	int thread_done;
	FILE *threads_fp;
#endif

using namespace std;


std::vector<double*> matricesA;
std::vector<double*> matricesB;
std::vector<double*> matricesC;


pthread_mutex_t lock;
int next_matrix = 0;

// clarg::argInt nmats("-nm", "Number of matrix multiplications", 1);
// clarg::argInt matsz("-msz", "Matrix size = N x N", 1000);
// clarg::argInt nthreads("-nt", "Number of threads", 1);

int nmats = 1;
int matsz = 1000;
int nthreads = 8;


double* new_matrix(double* src, unsigned N) {

  double *m = (double*) malloc( N*N*sizeof(double) );
  if (m == NULL) {
	printf("\n\nError: new_matrix()\n\n");
	exit( -999 );
  }

  if (src != NULL) { memcpy( m, src, N*N*sizeof(double) ); }

  else { memset( m, 0, N*N*sizeof(double) ); }

  return m;
}


void init_matrices(void) {

  for (unsigned i=0; i<nmats; i++) {
    matricesA.push_back( new_matrix(0, matsz) );
    matricesB.push_back( new_matrix(0, matsz) );
    matricesC.push_back( new_matrix(0, matsz) );
  }
}


void free_matrices() {

  for (unsigned i=0; i<nmats; i++) {
    free(matricesA[i]);
    free(matricesB[i]);
    free(matricesC[i]);
  }
}


double mysecond()
{

  struct timeval tp;
  struct timezone tzp;
  gettimeofday(&tp,&tzp);
  return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}


int get_matrix(void) {

  int id;

  if (next_matrix >= nmats)
    return -1;

  pthread_mutex_lock(&lock);
		id = next_matrix++;
  pthread_mutex_unlock(&lock);

  return id;
}



void* worker_thread(void* m) {
#ifdef __THREAD_LOG__
		pthread_mutex_lock(&tid_lock);
		//{
			thread_done--;
			thread_ids[thread_done] = syscall(SYS_gettid);

			if (thread_done == 0) {
				threads_fp = fopen("thread_TIDs.tmp","w");
				if (threads_fp == NULL) {
					printf("\nError in threads_fp...ABORTING!");
					exit(1);
				}

				int end_loop = nthreads;
				//fprintf(threads_fp,"%d\n",end_loop);

				for (int tmp_loop=0; tmp_loop < end_loop; ++tmp_loop)
					fprintf(threads_fp,"%d\n", (int)thread_ids[tmp_loop]);

				fclose(threads_fp);
				rename("thread_TIDs.tmp", "thread_TIDs");
				work_busy_lock = 0;
			}
		//}
		pthread_mutex_unlock(&tid_lock);

		while (work_busy_lock != 0) {}
#endif

    int matrix_id;

    while ( (matrix_id = get_matrix()) >= 0) {

		double* A = matricesA[matrix_id];
		double* B = matricesB[matrix_id];
		double* C = matricesC[matrix_id];
		unsigned N = matsz;

#ifdef __MIGRATE__

		hwloc_bitmap_t set = hwloc_bitmap_alloc();

		hwloc_get_cpubind(topology, set, HWLOC_CPUBIND_THREAD);
		hwloc_get_last_cpu_location(topology, set, HWLOC_CPUBIND_THREAD);

		hwloc_bitmap_singlify(set);

		hwloc_set_area_membind ( topology, (const void*)A, N*N*sizeof(double), (hwloc_const_cpuset_t)set, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_MIGRATE );
		hwloc_set_area_membind ( topology, (const void*)B, N*N*sizeof(double), (hwloc_const_cpuset_t)set, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_MIGRATE );
		hwloc_set_area_membind ( topology, (const void*)C, N*N*sizeof(double), (hwloc_const_cpuset_t)set, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_MIGRATE );

		hwloc_bitmap_free(set);
/*
		__spm_get ((void*)A, 0, N*N*sizeof(double), 1000000000000);
		__spm_get ((void*)B, 0, N*N*sizeof(double), 1000000000000);
		__spm_get ((void*)B, 0, N*N*sizeof(double), 1000000000000);
*/
#endif

	int i,j,k;
	memset( A, 1, N*N*sizeof(double) );

//LU decomposition of A
//B = L and C = U
		for (i=0 ;i<N;++i ) {
			B[i*N + i] = 1;

			for( j=(i+1);j<N;++j ) {
				B[j*N + i]= A[j*N +i]/A[i*N + i];

				for(k=(i+1);k<N;++k)
					A[j*N + k] = A[j*N + k] - B[j*N + i]*A[i*N + k];
			}

			for (k=i;k<N;++k)
				C[i*N + k] = A[i*N + k];
		}

	}

    return NULL;
}



int main(int argc, char *argv[]) {

#ifdef __MIGRATE__
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
	//__spm_init();
#endif

	if (argc <= 1)
		return -10;

	if (argc == 4) {
		nmats = atoi(argv[1]);
		matsz = atoi(argv[2]);
		nthreads = atoi(argv[3]);
	}

    if (nmats < 1) {
        cerr << "Error, nm must be >= 1" << endl;
        return 1;
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
      printf("\n mutex init failed\n");
      return 1;
    }

    init_matrices();

    int n_threads = nthreads;
	vector<pthread_t> threads(n_threads);

#ifdef __THREAD_LOG__
	thread_done = n_threads;

	thread_ids = (pid_t*)calloc(n_threads,sizeof(pid_t));

	if (thread_ids == NULL) {
		printf("\nError allocating thread_ids!");
		return -1;
	}

	if (pthread_mutex_init(&tid_lock, NULL) != 0) {
      printf("\n mutex init failed\n");
      return 1;
    }
	work_busy_lock = 1;
#endif

    double tempo = mysecond();

//pthread invoke
    for(long t=0; t<n_threads; t++)
		pthread_create(&threads[t], NULL, worker_thread, (void*) t);

    for(int t=0; t<n_threads; t++)
      pthread_join(threads[t], NULL);

//
    tempo = mysecond() - tempo;

    printf("%d %f \n", matsz, tempo);
    free_matrices();

#ifdef __MIGRATE__
    hwloc_topology_destroy(topology);
    //__spm_end();
#endif

#ifdef __THREAD_LOG__
	free(thread_ids);
#endif

    return 0;
}
