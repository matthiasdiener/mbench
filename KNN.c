/*
   This program implements KNN, the K-Nearest-Neighbors algorithm.
   Each thread finds K nearest neighbors, and sorts them.
   In a second stage, we choose the first K neighbors out of all the
   possible candidates.

   Usage: KNN num_threads num_points num_K
    - num_threads is the number of threads that we will be using.
    - num_points is the total number of points that we will be using.
    - num_K is the 'K' in the KNN algorithm.

    When benchmarking, make sure to use a 'num_points' that is much larger than
    the parameter 'num_k'.
 */

#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 

#ifdef __MIGRATE__
	#include <hwloc.h>
	hwloc_topology_t topology;
#endif

#ifdef __THREAD_LOG__
	#include <sys/syscall.h>
	#include <unistd.h>
	pid_t *thread_ids;
	pthread_mutex_t tid_lock;
	char work_busy_lock;
	int thread_done;
	FILE *threads_fp;
#endif

//============================ BEGIN INPUT SIZE ==============================//
// Number of threads in the application.
long NUM_THREADS;

// Number of points in our universe.
long NUM_POINTS;

// The number of K nearest neighbors that we shall consider
long NUM_K;
//============================== END INPUT SIZE ==============================//

// Size of the largest X and Y coordinates:
#define LARGEST_X 32766
#define LARGEST_Y 32766

// The size of the buffer of private points:
long NUM_PRIVATE_POINTS;

// The X coordinate of the points:
long* points_X;

// The Y coordinate of the points:
long* points_Y;

// The buffer where each thread will keep their nearest points:
long* private_X;
long* private_Y;

// The point that must be found:
long pivot_x;
long pivot_y;

/*
   This function initializes an array with random characters. Each thread is
   in charge of initializing one block of the array.
 */
void *initArray(long* points_X, long* points_Y, int num_points) {
  int i;
  for (i = 0; i < num_points; i++) {
    long x = random() % LARGEST_X;
    long y = random() % LARGEST_Y;
    points_X[i] = x;
    points_Y[i] = y;
  }
	return NULL;
}

double distance(int x, int y, int xx, int yy) {
  return sqrt(abs(xx - x) * abs(xx - x) + abs(yy - y) * abs(yy - y));
}

int j_is_closer_than_i(long* array_X, long* array_Y, int i, int j,
    long point_x, long point_y) {
  double distance_i = distance(array_X[i], array_Y[i], point_x, point_y);
  double distance_j = distance(array_X[j], array_Y[j], point_x, point_y);
  return distance_j < distance_i;
}

/*
   This function implements the KNN algorithm.
 */
 
 
const unsigned long PAGESZ = 4096; //bytes
const unsigned long PAGEMSK = ~(4095);

void* get_page(void* array, int i) {
  unsigned long page = (unsigned long) array;
  page = page + i*PAGESZ;
  page = page & (~(4095));
  return (void*) page;
}

void *KNN(void *threadIdentifier) {

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
				
				int end_loop = NUM_THREADS;
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

  const long TID             = (long)threadIdentifier; 
  const long MY_REGION       = NUM_POINTS / NUM_THREADS;
  const long MY_START        = TID * MY_REGION;
  const long MY_END          = (TID + 1) * MY_REGION;
  const long MY_BUFFER_START = NUM_K * TID;
  int i, j;
 #ifdef __MIGRATE__
		hwloc_bitmap_t set = hwloc_bitmap_alloc();
		
		hwloc_get_cpubind(topology, set, HWLOC_CPUBIND_THREAD);
		hwloc_get_last_cpu_location(topology, set, HWLOC_CPUBIND_THREAD);
		
		hwloc_bitmap_singlify(set);

		hwloc_set_area_membind ( topology, (const void*)(points_X+MY_START*sizeof(long)), MY_REGION*sizeof(long), (hwloc_const_cpuset_t)set, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_MIGRATE );		

		hwloc_set_area_membind ( topology, (const void*)(points_Y+MY_START*sizeof(long)), MY_REGION*sizeof(long), (hwloc_const_cpuset_t)set, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_MIGRATE );		
		
		hwloc_bitmap_free(set);			
#endif	 
 
  

  // Sort the points by distance:
  for (i = MY_START; i < MY_END - 1; i++) {
    for (j = i + 1; j < MY_END; j++) {
      if (j_is_closer_than_i(points_X, points_Y, i, j, pivot_x, pivot_y)) {
        long tmp = points_X[i];
        points_X[i] = points_X[j];
        points_X[j] = tmp;
        tmp = points_Y[i];
        points_Y[i] = points_Y[j];
        points_Y[j] = tmp;
      }
    }
  }
  // Copy the K nearest points to the private buffer:
  for (i = 0; i < NUM_K; i++) {
    private_X[MY_BUFFER_START + i] = points_X[MY_START + i];
    private_Y[MY_BUFFER_START + i] = points_Y[MY_START + i];
  }
	return NULL;
}

void print_first_k(long* array_x, long* array_y, const int K) {
  int i;
  for (i = 0; i < K; i++) {
    printf("(%ld, %ld - %.2f) ", array_x[i], array_y[i],
        distance(array_x[i], array_y[i], pivot_x, pivot_y));
  }
  printf("\n");
}

int main (int argc, char *argv[]) { 

  int t; 

  if (argc != 4) {
    fprintf(stderr, "Usage: %s num_threads num_points num_K\n", argv[0]);
    fprintf(stderr, "Observation: num_points >> num_K\n");
    return -1;
  } else {
#ifdef __MIGRATE__
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
#endif

    int i, j;
    pthread_t* threads;
    pthread_attr_t attr;

    // Read input arguments:
    NUM_THREADS = atoi(argv[1]);
    NUM_POINTS  = atoi(argv[2]);
    NUM_K  = atoi(argv[3]);

    // Initialize the array of points:
    points_X = (long*) malloc(NUM_POINTS * sizeof(long));
    points_Y = (long*) malloc(NUM_POINTS * sizeof(long));
    initArray(points_X, points_Y, NUM_POINTS);

    // Initialize the private buffer:
    NUM_PRIVATE_POINTS = NUM_K * NUM_THREADS;
    private_X = (long*) malloc(NUM_PRIVATE_POINTS * sizeof(long));
    private_Y = (long*) malloc(NUM_PRIVATE_POINTS * sizeof(long));

    // Initialize the pivot:
    pivot_x = 0;
    pivot_y = 0;

#ifdef __THREAD_LOG__
	thread_done = NUM_THREADS;
	thread_ids = (pid_t*)calloc(NUM_THREADS,sizeof(pid_t));
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


    // Initialize the thread attributes:
    threads = (pthread_t*) malloc(NUM_THREADS * sizeof(pthread_t));
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Initialize the array of KNN threads:
    for(t = 0; t < NUM_THREADS; t++){ 
      int rc = pthread_create(&threads[t], &attr, KNN, (void *)t); 
      if (rc) { 
        fprintf(stderr, "ERROR: pthread_create(KNN) is %d\n", rc); 
        fprintf(stderr, "Trying to create %ld threads\n", NUM_THREADS);
        return -1; 
      } 
    } 

    // Wait for the other threads. Also, free the resources used in attr.
    pthread_attr_destroy(&attr);
    for(t = 0; t < NUM_THREADS; t++) { 
      void* status;
      int rc = pthread_join(threads[t], &status);
      if (rc) { 
        printf("ERROR: return code from pthread_join() is %d\n", rc); 
        return -1; 
      } 
    }

    // Sort the points in the private buffers by distance:
    for (i = 0; i < NUM_PRIVATE_POINTS - 1; i++) {
      for (j = i + 1; j < NUM_PRIVATE_POINTS; j++) {
        if (j_is_closer_than_i(private_X, private_Y, i, j, pivot_x, pivot_y)) {
          long tmp = private_X[i];
          private_X[i] = private_X[j];
          private_X[j] = tmp;
          tmp = private_Y[i];
          private_Y[i] = private_Y[j];
          private_Y[j] = tmp;
        }
      }
    }

    // Print the results:
    // printf("Universe of points:\n");
    // print_first_k(points_X, points_Y, NUM_POINTS);
    //printf("K nearest points:\n");
    //print_first_k(private_X, private_Y, NUM_K);

    pthread_exit(NULL); 
  
#ifdef __MIGRATE__  
	hwloc_topology_destroy(topology);
#endif 
  
  }
  
#ifdef __THREAD_LOG__
	free(thread_ids);
#endif
  
	return 0;
	
}
