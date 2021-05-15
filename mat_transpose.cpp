#include <stdio.h>
#include <chrono>
#include <fstream>
#include "CacheHelper.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <cstring>

#define TYPE int
using namespace std;

int num_threads, program, memory;
unsigned long length = 0;
char* datafile; 
char* cgroup_name;
TYPE* dst;


void iterative_mat_transpose()
{
	unsigned long i, j;
	for (i = 0; i < length; i++)
		for (j = 0; j < length; j++)
			dst[i*length + j] = dst[i + j*length];
}

int main(int argc, char *argv[])
{
	if (argc < 4){
	std::cout << "cgexec -g memory:cache-test-arghya ./executables/parallel_mm <0=MM-INPLACE/1=MM-SCAN> matrix-width data_files/nullbytes <num_threads(1/4)>\n";
	exit(1);
	}
	std::ofstream mm_out = std::ofstream("out_mt.csv",std::ofstream::out | std::ofstream::app);
	program = atoi(argv[1]); length = std::stol(argv[2]); memory = std::stol(argv[3]); num_threads = atoi(argv[6]); 
	std::string datafilename = argv[4];
	datafile = new char[strlen(argv[4]) + 1](); strncpy(datafile,argv[4],strlen(argv[4]));
	cgroup_name = new char[strlen(argv[5]) + 1](); strncpy(cgroup_name,argv[5],strlen(argv[5]));
	
	int fdout;
	if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}

	if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*2, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
	   printf ("mmap error for output with code");
	   return 0;
	}

	std::vector<long> io_stats = {0,0};
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics AFTER loading output matrix to cache @@@@@ \n");
	std::cout << "===========================================\n";
	std::chrono::system_clock::time_point t_start = std::chrono::system_clock::now();
	std::clock_t start = std::clock();
	
	if (num_threads == 1)
		if (program == 0)
			iterative_mat_transpose();


	std::chrono::system_clock::time_point t_end = std::chrono::system_clock::now();
	double cpu_time = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	auto wall_time = std::chrono::duration<double, std::milli>(t_end-t_start).count();

	std::cout << "===========================================\n";
	std::cout << "Total wall time: " << wall_time << "\n";
	std::cout << "Total CPU time: " << cpu_time << "\n";
	std::cout << "===========================================\n";

	CacheHelper::print_io_data(io_stats, "Printing I/O statistics AFTER matrix multiplication @@@@@ \n");
	mm_out << wall_time << endl;

	return 0;
}

