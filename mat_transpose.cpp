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


/*
This function converts a matrix in row major layout to Z-Morton row major layout.
x is the output
xo is the input
no is the width of the entire matrix,
nn is the width of the matrix in the current recursive call
*/
void conv_RM_2_ZM_RM(TYPE* x,TYPE* xo, int n,int no ){
  /*
	std::string depth_trace = "";
	int n3 = length;
	int limit = 0;
	while (n3 > n || n3 == 1){
		n3 /= 2;
		depth_trace += " ";
		limit++;
	}
	std::cout << depth_trace << "Running conv with depth: " << limit;
	std::cout << " value of n: " << n << std::endl;
  */
	if ( n <= CacheHelper::MM_BASE_SIZE )
	{
		for ( int i = 0; i < n; i++ )
		{
			for ( int j = 0; j < n; j++ )
				( *x++ ) = ( *xo++ );

			xo += ( no - n );
		}
	}
	else
	{
		int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

		conv_RM_2_ZM_RM( x, xo, nn, no );
		conv_RM_2_ZM_RM( x+m12, xo + nn, nn, no );
		conv_RM_2_ZM_RM( x+m21, xo + nn * no, nn, no );
		conv_RM_2_ZM_RM( x+m22, xo + nn * no + nn, nn, no );
	}
}

void iterative_mat_transpose(TYPE* x) {
	unsigned long i, j;
	for (i = 0; i < length; i++)
		for (j = 0; j < length; j++)
			*(x + i*length + j) = *(x + i + j*length);
}

int main(int argc, char *argv[]) {
	if (argc < 6){
	std::cout << "cgexec -g memory:cache-test-arghya ./executables/parallel_mt <0=MT-ITERATIVE/1=MT-OBLIVIOUS> matrix-width data_files/nullbytes <cgroup_name> <num_threads(1/4)>\n";
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
	
	if (num_threads == 1) {
		if (program == 0) {
			for (unsigned int i = 0; i < length*length; i++) {
				dst[i] = i;
			}
			iterative_mat_transpose(dst);
		}
		if (program == 1) {
			for (unsigned int i = 0; i < length*length; i++) {
				dst[i] = i;
			}
			conv_RM_2_ZM_RM(dst+length*length,dst,length,length);
		}
	}
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

