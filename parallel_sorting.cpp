#include "CacheHelper.h"
#include "funnelSort.h"
#include <algorithm>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <ctime>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <queue>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <vector>
#include <pthread.h>
#include <thread>

using namespace std;
#define TYPE int

typedef pair<int, pair<int, int> > ppi;
time_t start, finish; // introduced to measure wall clock time
double duration;
char* datafile;
std::ofstream out, out_sorting, logfile;
std::vector<long> io_stats = {0,0};
int num_threads, data_in_megabytes, memory_given_MB, k_logical, logical_block_size_KB, actual_block_size_KB, program;
vector<int> random_memory;
unsigned long long num_elements, base_case, memory;

TYPE* arr; 

class Integer_comparator
{
  public:
    bool operator () (const int& a, const int& b) const
    {
      return (a < b);
    }
};

/* M/B-way merge, to be precise M/4B */
void merge(int arr[], int temp_arr[], unsigned long long l, unsigned long long m, unsigned long long r) {
  unsigned long long itr = 0ULL;
  priority_queue<ppi, vector<ppi>, greater<ppi> > pq;
  for (int i = 0; i < k_logical; i++) {
    for (unsigned long long j = 0; j < 1024ULL * actual_block_size_KB / sizeof(TYPE); j++) {
        pq.push({arr[l + i*m + j], {i, j}});
    }
  }
  //cout << "done now" << endl;
  while (!pq.empty()) {
    ppi curr = pq.top();// cout << sizeof(curr) << endl;
    pq.pop();
    temp_arr[itr] = curr.first; itr++;
    int i = curr.second.first;   
    unsigned long long j = curr.second.second;
    if (j + 1 < m && (j + 1) % (1024ULL * actual_block_size_KB / sizeof(TYPE)) == 0) {
      //cout << "here " << j + 1 << " " << 1048576ULL * actual_block_size_MB / sizeof(TYPE) << endl;
      for (unsigned long long p = 0ULL; p < 1024ULL * actual_block_size_KB / sizeof(TYPE); ++p) {
        pq.push({arr[m*i + j + p], {i, j + p + 1}});
      }
    } 
  }
  for (unsigned long long i = 0 ; i < m * k_logical; i++)
    arr[i + l] = temp_arr[i];
}


/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(int arr[], unsigned long long l, unsigned long long r, int temp_arr[]) { 
  if (l < r && r - l + 1 > base_case) {
    logfile << "recursive call" << endl;
    unsigned long long m = (r - l + 1) / k_logical; 
    for (int i = 0; i < k_logical; ++i) {
      mergeSort(arr, l + i*m, l + i*m + m - 1, temp_arr); 
    }
    logfile << "merging elements from " << l << " to " << r << endl;
    merge(arr, temp_arr, l, m, r); 
  }
  else if (l < r && r - l + 1 <= base_case) {
    logfile << "in-memory sort" << endl;
    actual_block_size_KB = (r - l + 1) * sizeof(TYPE) / 1024;
    sort(arr + l, arr + (r + 1));
  }
} 


/* root function to call merge sort */
void rootMergeSort(int inp) {
  if (inp == 1) {
    int* temp_arr1 = NULL;
    temp_arr1 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr1);
    delete [] temp_arr1; temp_arr1 = NULL;
  }
  else if (inp == 2) {
    int* temp_arr2 = NULL;
    temp_arr2 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr2);
    delete [] temp_arr2; temp_arr2 = NULL;
  }
  else if (inp == 3) {
    int* temp_arr3 = NULL;
    temp_arr3 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr3);
    delete [] temp_arr3; temp_arr3 = NULL;
  }
  else if (inp == 4) {
    int* temp_arr4 = NULL;
    temp_arr4 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr4);
    delete [] temp_arr4; temp_arr4 = NULL;
  }
  else if (inp == 5) {
    int* temp_arr5 = NULL;
    temp_arr5 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr5);
    delete [] temp_arr5; temp_arr5 = NULL;
  }
  else if (inp == 6) {
    int* temp_arr6 = NULL;
    temp_arr6 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr6);
    delete [] temp_arr6; temp_arr6 = NULL;
  }
  else if (inp == 7) {
    int* temp_arr7 = NULL;
    temp_arr7 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr7);
    delete [] temp_arr7; temp_arr7 = NULL;
  }
  else if (inp == 8) {
    int* temp_arr8 = NULL;
    temp_arr8 = new int[num_elements / num_threads];
    mergeSort(arr, (inp - 1) * num_elements / num_threads, inp * num_elements / num_threads - 1, temp_arr8);
    delete [] temp_arr8; temp_arr8 = NULL;
  }
  else {
    printf ("some error in input\n");
  }
}

/* root function to call merge sort */
void rootFunnelSort(int inp) {
  Integer_comparator comp;
  FunnelSort::sort<int, class Integer_comparator>(&arr[(inp - 1) * num_elements / num_threads], &arr[inp * num_elements / num_threads - 1], comp);  
}

int main(int argc, char *argv[]){
  srand (time(NULL));
  logfile = std::ofstream("log.txt",std::ofstream::out | std::ofstream::app);
  if (argc < 5){
    std::cout << "Insufficient arguments! Usage: ./a.out <memory_limit> <data_size> <cgroup_name> <type> <k_logical> <block_size>";
    exit(1);
  }

  memory_given_MB = atoi(argv[1]); data_in_megabytes = atoi(argv[2]); program = atoi(argv[3]); num_threads = atoi(argv[4]);
  logical_block_size_KB = CacheHelper::EM_BASE_SIZE;
  //float k_logical_float = (memory_given_MB * 1024) / (4 * logical_block_size_KB); cout << "here1" << endl;
  k_logical = (memory_given_MB * 1024) / (4 * logical_block_size_KB); //k_logical_float;
  //k_logical = atoi(argv[5]); logical_block_size_KB = atoi(argv[6]);
  datafile = new char[strlen(argv[5]) + 1](); strncpy(datafile,argv[5],strlen(argv[5]));
  num_elements = 1048576ULL * data_in_megabytes / sizeof(TYPE);
  base_case = 1024ULL * logical_block_size_KB / sizeof(TYPE);

  int fdout;
  if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
    printf ("can't create nullbytes for writing\n");
    return 0;
  }
  
  //some checks are required here
  if ((k_logical + 1) * logical_block_size_KB / 1024 > memory_given_MB) {
    cout << "constraint not met" << endl;
    return 0;
  }
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");
  logfile << "Running " << k_logical <<"-way merge sort on an array of size: " << num_elements << " with base case " << base_case << std::endl;

  if (((arr = (TYPE*) mmap(0, sizeof(TYPE)*num_elements, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
    printf ("mmap error for output with code");
    return 0;
  }
  out = std::ofstream("mem_profile.txt", std::ofstream::out); 
  memory = 1048576ULL * memory_given_MB;
  out << duration << " " << memory << std::endl;
  //CacheHelper::limit_memory(memory, cgroup_name);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just before sorting start @@@@@ \n");
  start = time(NULL); 

  if (program == 0) {
    if (num_threads == 1) {
      std::thread th1(rootFunnelSort, 1);
      th1.join();
    }
    if (num_threads == 8) {
      std::thread th1(rootFunnelSort, 1);
      std::thread th2(rootFunnelSort, 2);
      std::thread th3(rootFunnelSort, 3);
      std::thread th4(rootFunnelSort, 4);
      std::thread th5(rootFunnelSort, 5);
      std::thread th6(rootFunnelSort, 6);
      std::thread th7(rootFunnelSort, 7);
      std::thread th8(rootFunnelSort, 8);
      th1.join();
      th2.join();
      th3.join();
      th4.join();
      th5.join();
      th6.join();
      th7.join();
      th8.join();
      int* temp_arr = NULL;
      temp_arr = new int[num_elements];
      merge(arr, temp_arr, 0, num_elements / 8, num_elements - 1);
      delete [] temp_arr; temp_arr = NULL;
    }
  }
  else if (program == 1) {
    if (num_threads == 1) {
      std::thread th1(rootMergeSort, 1);
      th1.join();
    }
    if (num_threads == 8) {
      std::thread th1(rootMergeSort, 1);
      std::thread th2(rootMergeSort, 2);
      std::thread th3(rootMergeSort, 3);
      std::thread th4(rootMergeSort, 4);
      std::thread th5(rootMergeSort, 5);
      std::thread th6(rootMergeSort, 6);
      std::thread th7(rootMergeSort, 7);
      std::thread th8(rootMergeSort, 8);
      th1.join();
      th2.join();
      th3.join();
      th4.join();
      th5.join();
      th6.join();
      th7.join();
      th8.join();
      int* temp_arr = NULL;
      temp_arr = new int[num_elements];
      merge(arr, temp_arr, 0, num_elements / 8, num_elements - 1);
      delete [] temp_arr; temp_arr = NULL;
    }
  }
  else
    cout << "wrong input" << endl;
  
  finish = time(NULL);
  //duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  duration = (finish - start);
  std::cout << "\n==================================================================\n";
  CacheHelper::print_io_data(io_stats, "Printing I/O statistics just after sorting start @@@@@ \n");
  logfile << "Total sorting time: " << duration << "\n";
  out_sorting = std::ofstream("out_sorting.csv",std::ofstream::out | std::ofstream::app);
  if (program == 0) {
    out_sorting << "FunnelSort," << memory_given_MB << "," << data_in_megabytes << "," << num_threads << "," << duration << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
  }
  else if (program == 1) {
    logfile << "logical block size in KB is " << logical_block_size_KB << ", actual block size in KB is " << actual_block_size_KB << std::endl;
    out_sorting << "MergeSort," << memory_given_MB << "," << data_in_megabytes << "," << num_threads << "," << duration << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
  }
  else
    cout << "wrong input" << endl;
  //introduced code for checking the accuracy of sorting result
  std::ofstream test_out = std::ofstream("test_out.txt", std::ofstream::out);
  for (unsigned long long i = 1 ; i < num_elements; i++) {
   if ((int)arr[i - 1] > (int)arr[i]) {
      logfile << "bad result " << (unsigned long long)i << " " << (int)arr[i - 2] << " " << (int)arr[i - 1] << " " << (int)arr[i] << " " << (int)arr[i + 1] << endl;
   }
  }
  out.close(); out_sorting.close(); logfile.close();
  return 0;
}
