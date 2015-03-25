#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/time.h>
#include <errno.h>
#include <vector>

#include "arglib.h"


#ifdef __MIGRATE__
	#include <hwloc.h>
	hwloc_topology_t topology;
#endif


using namespace std;


std::vector<double*> matricesA;
std::vector<double*> matricesB;
std::vector<double*> matricesC;

// clarg::argInt nmats("-nm", "Number of matrix multiplications", 1);
// clarg::argInt matsz("-msz", "Matrix size = N x N", 1000);
// clarg::argInt nthreads("-nt", "Number of threads", 1);

int nmats = 1;
int matsz = 1000;
int nthreads = 8;

pthread_mutex_t lock;
int next_matrix = 0;


double* new_matrix(double* src, unsigned N) {
	double* m = (double*) malloc(N*N*sizeof(double));

	if (src != NULL)
		memcpy( m, src, N*N*sizeof(double) );
	else
		memset( m, 0, N*N*sizeof(double) );
	return m;
}


void init_matrices() {
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


int get_matrix()
{
	int id;

	if (next_matrix >= nmats)
		return -1;

	pthread_mutex_lock(&lock);
		id = next_matrix++;
	pthread_mutex_unlock(&lock);

	return id;
}


void* worker_thread(void* m) {
	int matrix_id;

	while ( (matrix_id = get_matrix()) >= 0 ) {
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
#endif

		for (unsigned i=0; i<N; i++) {
			for (unsigned j=0; j<N; j++) {
				double tmp=0;
				for (unsigned k=0; k<N; k++)
					tmp += A[i*N + k] * B[k*N + j];

				C[i*N + j] = tmp;
			}
		}

	} //while ( (matrix_id = get_matrix()) >= 0 )

	return NULL;
}


int main(int argc, char *argv[]) {
#ifdef __MIGRATE__
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
#endif

	// if (clarg::parse_arguments(argc, argv)) {
	// 	cerr << "Error when parsing the arguments!" << endl;
	// 	return 1;
	// }

	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init failed\n");
		return 1;
	}

	if (nmats < 1) {
		cerr << "Error, nm must be >= 1" << endl;
		return 1;
	}

	init_matrices();

	int n_threads = nthreads;
	std::vector<pthread_t> threads(n_threads);

    double tempo = mysecond();

	for(int t=0; t<n_threads; t++)
		pthread_create(&threads[t], NULL, worker_thread, (void*)t);

	for(int t=0; t<n_threads; t++)
		pthread_join(threads[t], NULL);

    tempo = mysecond() - tempo;
	printf("%d %f \n", matsz, tempo);

	free_matrices();

#ifdef __MIGRATE__
	hwloc_topology_destroy(topology);
#endif

	return 0;
}
