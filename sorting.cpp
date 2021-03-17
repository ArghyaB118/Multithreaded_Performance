#include "CacheHelper.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <fstream>
#include <sys/mman.h>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>

#include <pthread.h>
#include <thread>
#include <time.h>
using namespace std;

int* arr;
int memory_given_MB, data_in_megabytes, num_ways, program, num_threads;
unsigned long long num_elements, base_case, base_case_mod;
time_t start, finish; // introduced to measure wall clock time
double duration;
std::ofstream out_sorting;
std::vector<long> io_stats = {0,0};


struct Position {
	int sorted_run;
	unsigned long long position_run;
};

struct MinHeapNode
{
	int* element;
	Position psn;
};

struct MinHeapNodeComparator
{
public:
	bool operator()(const MinHeapNode l, const MinHeapNode r) const {
	      return *(l.element) > *(r.element);
	}
};

void merge(int* l, int* r, unsigned long long num_elts, int temp_arr[]) {
	//cout << l << " " << r << endl;
	priority_queue<MinHeapNode, vector<MinHeapNode>, MinHeapNodeComparator> pq;
	for (int i = 0; i < num_ways; i++) {
	    for (unsigned long long j = 0; j < base_case_mod; j++) {
	    	Position psn = {i, j}; 
	    	int* value = l + i * num_elts / num_ways + j; //cout << i << " " << j << " " << value << endl;
	    	MinHeapNode node = {value, psn};
	    	pq.push(node);
	    }
	}
	unsigned long long itr = 0ULL;
	while(!pq.empty()) {
		MinHeapNode curr = pq.top(); pq.pop();
		int* value = curr.element; Position psn = curr.psn; 
		temp_arr[itr] = *value; itr++;
		int i = psn.sorted_run; 
		unsigned long long j = psn.position_run;
		if ((j < num_elts / num_ways - 1) && ((j + 1) % base_case_mod == 0)) {
			for (unsigned long long p = 1; p <= base_case_mod; p++) {
				Position psn = {i, j + p};
				int* value = l + i * num_elts / num_ways + j + p;
		    	MinHeapNode node = {value, psn};
		    	pq.push(node);
			}
		}
	}

	for (unsigned long long i = 0 ; i < num_elts; i++)
    	*(l + i) = temp_arr[i];
}


void mergeSort(int* l, int* r, unsigned long long num_elts) {
	//cout << "recursive call" << endl;
	if (l < r && num_elts > base_case) {
		for (int i = 0; i < num_ways; i++) {
			mergeSort(l + i * num_elts / num_ways, l + (i + 1) * num_elts / num_ways, num_elts / num_ways); 
	    }
	    int* temp_arr = NULL;
    	temp_arr = new int[num_elts];
	    merge(l, r, num_elts, temp_arr); 
	    delete [] temp_arr; temp_arr = NULL;
	}
	else if (l < r && num_elts <= base_case) {
		base_case_mod = num_elts;
		sort(l , r);
	}
}

void rootMergeSort(int input) {
	mergeSort(&arr[(input - 1) * num_elements / num_threads], &arr[input * num_elements / num_threads], num_elements / num_threads);
}


int main(int argc, char *argv[]) {
	srand (time(NULL));
	memory_given_MB = atoi(argv[1]), data_in_megabytes = atoi(argv[2]);
	program = atoi(argv[3]); num_threads = atoi(argv[4]);
	num_elements = 1048576ULL * data_in_megabytes / sizeof(int);
	char* datafile = new char[strlen(argv[5]) + 1](); strncpy(datafile,argv[5],strlen(argv[5]));
	num_ways = (memory_given_MB * 1024) / (4 * CacheHelper::EM_BASE_SIZE);
	base_case = 1024ULL * CacheHelper::EM_BASE_SIZE / sizeof(int);

	int fdout;
	if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
		cout << "can't create nullbytes for writing" << endl;
		return 0;
	}

	if (((arr = (int*) mmap(0, sizeof(int)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (int*)MAP_FAILED)){
	    printf ("mmap error for output with code");
	    return 0;
  	}

  	// to check if the input array is actually unsorted
  	if (!is_sorted(arr, arr + num_elements - 1))
  		cout << "not sorted" << endl;

	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");
  	start = time(NULL); 

  	if (num_threads == 1)
  		mergeSort(&arr[0], &arr[num_elements - 1], num_elements);
  	else if (num_threads == 8) {
  		std::thread th1(rootMergeSort, 1);
		std::thread th2(rootMergeSort, 2);
		std::thread th3(rootMergeSort, 3);
		std::thread th4(rootMergeSort, 4);
		std::thread th5(rootMergeSort, 5);
		std::thread th6(rootMergeSort, 6);
		std::thread th7(rootMergeSort, 7);
		std::thread th8(rootMergeSort, 8);
		th1.join(); th2.join(); th3.join(); th4.join();
		th5.join(); th6.join(); th7.join(); th8.join();
		int* temp_arr = NULL;
		temp_arr = new int[num_elements];
		merge(arr, arr + num_elements, num_elements, temp_arr);
		delete [] temp_arr; temp_arr = NULL;
  	}

	finish = time(NULL);
  	duration = (finish - start);
  	CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");

  	out_sorting = std::ofstream("out_sorting.csv",std::ofstream::out | std::ofstream::app);
	if (program == 0) {
		out_sorting << "MergeSort," << memory_given_MB << "," << data_in_megabytes << "," << num_threads << "," << duration << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
	}
	else if (program == 1) {
		out_sorting << "FunnelSort," << memory_given_MB << "," << data_in_megabytes << "," << num_threads << "," << duration << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
	}
	else
		cout << "wrong input" << endl;

	out_sorting.close();
  	// to check if the sorting is perfect
  	if (!is_sorted(arr, arr + num_elements - 1))
  		cout << "not sorted" << endl;

  	//for (unsigned long long i = 1; i < num_elements; i++)
  	//	if (arr[i] < arr[i - 1])
  	//		cout << i << " " << arr[i - 1] << " " << arr[i] << " " << arr[i + 1] << endl;

  	return 0;
}