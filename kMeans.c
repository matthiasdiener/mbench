/*
 * kMeans.c
 *
 *  Created on: Feb 20, 2014
 *      Author: raphael
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#ifdef __MIGRATE__
	#include <hwloc.h>
	hwloc_topology_t topology;
#endif

//Maximum value for X and Y. Empty means unlimited.
#define MAX_DIM_LENGHT % 100000


typedef struct {

	int x;
	int y;

	int cluster;

} Point;

typedef struct {

	Point* data;
	int size;

} PointVector;


PointVector generateRandomVector(int numElements){

	PointVector result;
	result.size = numElements;
	result.data = (Point*)calloc(numElements, sizeof(Point));

	int i;
	for (i = 0; i < numElements; i++){
		result.data[i].x = rand() MAX_DIM_LENGHT;
		result.data[i].y = rand() MAX_DIM_LENGHT;
		result.data[i].cluster = -1;
	}

	return result;
}

typedef int bool;

#define false 0
#define true 1

bool hasChanged;

typedef struct {

	int threadId;
	int numThreads;
	PointVector points;
	PointVector means;

} ThreadArg;


void* assignPointsToCluster(void* arg){
	ThreadArg* data = (ThreadArg*)arg;

	int first_idx = (data->threadId/data->numThreads) * data->points.size;
	int next_first_idx = ((data->threadId + 1)/data->numThreads) * data->points.size;

#ifdef __MIGRATE__

		hwloc_bitmap_t set = hwloc_bitmap_alloc();
		
		hwloc_get_cpubind(topology, set, HWLOC_CPUBIND_THREAD);
		hwloc_get_last_cpu_location(topology, set, HWLOC_CPUBIND_THREAD);
		
		hwloc_bitmap_singlify(set);
		
		hwloc_set_area_membind ( topology, (const void*)(data->points.data + first_idx*sizeof(Point)), (next_first_idx - first_idx)*sizeof(Point), (hwloc_const_cpuset_t)set, HWLOC_MEMBIND_BIND, HWLOC_MEMBIND_MIGRATE );		
	
		hwloc_bitmap_free(set);			
#endif	

	//iterate over a subset of points
	int i;
	for(i = first_idx; i < next_first_idx; i++){

		Point currentPoint = data->points.data[i];

		//identify the nearest mean
		int nearest_mean;
		double nearestDistance;
		int j;
		for(j = 0; j < data->means.size; j++){

			Point currentMean = data->means.data[j];

			double deltaX = abs(currentPoint.x - currentMean.x);
			double deltaY = abs(currentPoint.y - currentMean.y);

			double distance = sqrt( (deltaX * deltaX) + (deltaY * deltaY) );

			//the first is always the nearest
			if(!j){

				nearest_mean = 0;
				nearestDistance = distance;

			} else {

				if(distance < nearestDistance){
					nearest_mean = j;
					nearestDistance = distance;
				}

			}

		}

		//Check if the point belongs to a different cluster now
		if(currentPoint.cluster != nearest_mean){

			data->points.data[i].cluster = nearest_mean;
			hasChanged = true;

		}


	}

	pthread_exit((void*) arg);
}



void* updateClusterCentroid(void* arg){
	ThreadArg* data = (ThreadArg*)arg;

	//iterate over a subset of points
	int totalX = 0;
	int totalY = 0;
	int numPoints = 0;

	int i;
	for(i = 0; i < data->points.size; i++){

		Point currentPoint = data->points.data[i];

		if (currentPoint.cluster == data->threadId){

			totalX += currentPoint.x;
			totalY += currentPoint.y;
			numPoints++;

		}

	}

	if(numPoints){
		data->means.data[data->threadId].x = totalX/numPoints;
		data->means.data[data->threadId].y = totalY/numPoints;
	}

//	pthread_exit((void*) arg);
	return NULL;
}

void printVector(PointVector vector){

	int i;
	for(i = 0; i < vector.size; i++){
		printf("%d:	(%d,%d) - %d  \n", i, vector.data[i].x, vector.data[i].y, vector.data[i].cluster);
	}

}

void kMeans(PointVector vector, int NumClusters, int NumThreads){

	int i;

	PointVector means = generateRandomVector(NumClusters);
	for(i = 0; i < NumClusters; i++){
		means.data[i].cluster = i;
	}


	do {
		hasChanged = false;

		//assign points to nearest mean
		pthread_t thread[NumThreads];
		pthread_attr_t attr;

		/* Initialize and set thread detached attribute */
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		//Sort Each Bucket Individually
		for (i = 0; i < NumThreads; i++) {

			ThreadArg Arg;
			Arg.threadId = i;
			Arg.numThreads = NumThreads;
			Arg.points = vector;
			Arg.means = means;

			int rc = pthread_create(&thread[i], &attr, assignPointsToCluster, &(Arg));
			if (rc) {
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			 }
		}

		/* Free attribute and wait for the other threads */
		void *status;
		pthread_attr_destroy(&attr);
		for(i=0; i<NumThreads; i++) {
			int rc = pthread_join(thread[i], &status);
			if (rc) {
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
		}

		//update means to the centroid of the points of the cluster
		for (i = 0; i < NumClusters; i++) {

			ThreadArg Arg2;
			Arg2.threadId = i;
			Arg2.numThreads = NumClusters;
			Arg2.points = vector;
			Arg2.means = means;

			updateClusterCentroid(&(Arg2));
		}

	} while(hasChanged);

	free(means.data);

}


int main(int argc, char** argv){

	if (argc < 4) {

		char* programName = argv[0];
		printf("Invalid number of arguments\n\nUSAGE: %s NumElements NumClusters NumThreads [seed]\n", programName);

		exit(1);
	}

#ifdef __MIGRATE__
	hwloc_topology_init(&topology);
	hwloc_topology_load(topology);
#endif

	if(argc > 4){
		//we will use the same seed in order to generate always the same input
		srand(atoi(argv[4]));
	} else {
		//we will use the same seed in order to generate always the same input
		srand(0);
	}

	PointVector vector = generateRandomVector(atoi(argv[1]));

//	printVector(vector);
	kMeans(vector, atoi(argv[2]), atoi(argv[3]));
//	printVector(vector);

	free(vector.data);

#ifdef __MIGRATE__  
	hwloc_topology_destroy(topology);
#endif 
	return 0;

}
