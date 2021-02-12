#include "CacheHelper.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <chrono>
#include <fstream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <thread>

#define TYPE int
using namespace std;

int num_threads, program;
unsigned long length = 0;
const int progress_depth = 4;
char* datafile;
TYPE* dst;

int length1, length2, length11, length12, length21, length22;

void mm_scan( TYPE* x, TYPE* u, TYPE* v, TYPE* y, int n0, int n)
{
	if ( n <= CacheHelper::MM_BASE_SIZE )
	{
		for ( int i = 0; i < n; i++ )
		{
			TYPE* vv = v;
			for ( int j = 0; j < n; j++ )
			{
				TYPE t = 0;

				for ( int k = 0; k < n; k++ )
					t += u[ k ] * vv[ k ];

				( *x++ ) += t;
				vv += n;
			}
			u += n;
		}
	}
	else
	{
		std::string depth_trace = "";
		int n3 = length;
		int limit = 0;
		while (n3 > n || n3 == 1){
			n3 >>= 1;
			depth_trace += " ";
			limit++;
		}
	    if (limit < progress_depth){
	      	std::cout << depth_trace << "Running matrix multiply with depth: " << limit;
	  		std::cout << " value of n: " << n << std::endl;
	    }
	    int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;

	    int n2 = n0;
	    TYPE* y2 = y;
	    while (n2 > n){
	      y2 += n2*n2;
	      n2 >>= 1;
	    }
	    //cout << "y2-y in this call: " << y2-y << endl;
		mm_scan( x + m11, u + m11, v + m11, y, n0, nn );
		mm_scan( x + m12, u + m11, v + m12, y, n0, nn );
		mm_scan( x + m21, u + m21, v + m11, y, n0, nn );
		mm_scan( x + m22, u + m21, v + m12, y, n0, nn );

		mm_scan( y2 + m11, u + m12, v + m21, y, n0, nn );
		mm_scan( y2 + m12, u + m12, v + m22, y, n0, nn );
		mm_scan( y2 + m21, u + m22, v + m21, y, n0, nn );
		mm_scan( y2 + m22, u + m22, v + m22, y, n0, nn );

		for (int i = 0; i < n*n; i++){
			x[i] += y2[i];
			y2[i] = 0;
	    }
	}
}





//x is the output, u and v are the two inputs
void mm_inplace( TYPE* x, TYPE* u, TYPE* v, int n )
{
	if ( n <= CacheHelper::MM_BASE_SIZE )
	{
		for ( int i = 0; i < n; i++ )
		{
			TYPE* vv = v;
			for ( int j = 0; j < n; j++ )
			{
				TYPE t = 0;

				for ( int k = 0; k < n; k++ )
					t += u[ k ] * vv[ k ];

				( *x++ ) += t;
				vv += n;
			}
			u += n;
		}
	}
	else
	{
		std::string depth_trace = "";
		int n3 = length;
		int limit = 0;
		while (n3 > n || n3 == 1){
			n3 /= 2;
			depth_trace += " ";
			limit++;
		}
    if (limit < progress_depth){
      std::cout << depth_trace << "Running matrix multiply with depth: " << limit;
  		std::cout << " value of n: " << n << std::endl;
    }
		int nn = ( n >> 1 );
		int nn2 = nn * nn;

		const int m11 = 0;
		int m12 = m11 + nn2;
		int m21 = m12 + nn2;
		int m22 = m21 + nn2;
		mm_inplace( x + m11, u + m11, v + m11, nn );
		mm_inplace( x + m12, u + m11, v + m12, nn );
		mm_inplace( x + m21, u + m21, v + m11, nn );
		mm_inplace( x + m22, u + m21, v + m12, nn );

		mm_inplace( x + m11, u + m12, v + m21, nn );
		mm_inplace( x + m12, u + m12, v + m22, nn );
		mm_inplace( x + m21, u + m22, v + m21, nn );
		mm_inplace( x + m22, u + m22, v + m22, nn );
	}
}


void scan_add( TYPE* x, TYPE* y) {
	for (int i = 0; i < length * length; i++) {
		x[i] += y[i];
	}
}


void mm_root( int inp ) {
	if (program == 0) {
		if (inp == 1) {
			mm_inplace( dst + length11, dst + length*length + length11, dst + length*length*2 + length11, length1 );
		}
		else if (inp == 2) {
			mm_inplace( dst + length12, dst + length*length + length11, dst + length*length*2 + length12, length1 );
		}
		else if (inp == 3) {
			mm_inplace( dst + length21, dst + length*length + length21, dst + length*length*2 + length11, length1 );
		}
		else if (inp == 4) {
			mm_inplace( dst + length22, dst + length*length + length21, dst + length*length*2 + length12, length1 );
		}
		else if (inp == 5) {
			mm_inplace( dst + length11, dst + length*length + length12, dst + length*length*2 + length21, length1 );
		}
		else if (inp == 6) {
			mm_inplace( dst + length12, dst + length*length + length12, dst + length*length*2 + length22, length1 );
		}
		else if (inp == 7) {
			mm_inplace( dst + length21, dst + length*length + length22, dst + length*length*2 + length21, length1 );
		}
		else if (inp == 8) {
			mm_inplace( dst + length22, dst + length*length + length22, dst + length*length*2 + length22, length1 );
		}
		else {
			cout << "invalid" << endl;
		}
	}
	else if (program == 1) {
		if (inp == 1) {
			mm_scan( dst + length11, dst + length*length + length11, dst + length*length*2 + length11, dst + length*length*3, length, length1 );
		}
		else if (inp == 2) {
			mm_scan( dst + length12, dst + length*length + length11, dst + length*length*2 + length12, dst + length*length*3, length, length1 );
		}
		else if (inp == 3) {
			mm_scan( dst + length21, dst + length*length + length21, dst + length*length*2 + length11, dst + length*length*3, length, length1 );
		}
		else if (inp == 4) {
			mm_scan( dst + length22, dst + length*length + length21, dst + length*length*2 + length12, dst + length*length*3, length, length1 );
		}
		else if (inp == 5) {
			mm_scan( dst + length11, dst + length*length + length12, dst + length*length*2 + length21, dst + length*length*3, length, length1 );
		}
		else if (inp == 6) {
			mm_scan( dst + length12, dst + length*length + length12, dst + length*length*2 + length22, dst + length*length*3, length, length1 );
		}
		else if (inp == 7) {
			mm_scan( dst + length21, dst + length*length + length22, dst + length*length*2 + length21, dst + length*length*3, length, length1 );
		}
		else if (inp == 8) {
			mm_scan( dst + length22, dst + length*length + length22, dst + length*length*2 + length22, dst + length*length*3, length, length1 );
		}
		else {
			cout << "invalid" << endl;
		}
	}
}


int main(int argc, char *argv[]){
	if (argc < 4){
	std::cout << "cgexec -g memory:cache-test-arghya ./executables/parallel_mm <0=MM-INPLACE/1=MM-SCAN> matrix-width data_files/nullbytes <num_threads(1/4)>\n";
	exit(1);
	}
	std::ofstream mm_out = std::ofstream("out_mm.csv",std::ofstream::out | std::ofstream::app);
	program = atoi(argv[1]); length = std::stol(argv[2]); num_threads = atoi(argv[4]); 
	std::string datafilename = argv[3];
	std::cout << "Running cache_adaptive matrix multiply with matrices of size: " << (int)length << "x" << (int)length << "\n";
	std::vector<long> io_stats = {0,0};
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics at program start @@@@@ \n");

	int fdout;
	datafile = new char[strlen(argv[3]) + 1](); strncpy(datafile,argv[3],strlen(argv[3]));
	if ((fdout = open (datafile, O_RDWR, 0x0777 )) < 0){
		printf ("can't create nullbytes for writing\n");
		return 0;
	}

	if (program == 0) {
		if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*3, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
		   printf ("mmap error for output with code");
		   return 0;
		}
	}
	else if (program == 1) {
		if (((dst = (TYPE*) mmap(0, sizeof(TYPE)*length*length*5, PROT_READ | PROT_WRITE, MAP_SHARED , fdout, 0)) == (TYPE*)MAP_FAILED)){
		   printf ("mmap error for output with code");
		   return 0;
		}
	}
	else {
		cout << "program can be only 0 or 1" << endl;
	}
	/*
	for (unsigned int i = 0 ; i < 3*length*length; i++){
		std::cout << dst[i] << "\t";
	}
	std::cout << "\n";
	*/
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics AFTER loading output matrix to cache @@@@@ \n");
	std::cout << "===========================================\n";

	//MODIFY MEMORY WITH CGROUP
	//CacheHelper::limit_memory(std::stol(argv[3])*1024*1024,argv[4]);

	std::chrono::system_clock::time_point t_start = std::chrono::system_clock::now();
	std::clock_t start = std::clock();
	if (num_threads == 1) {
		if (program == 0) {
			mm_inplace(dst,dst+length*length,dst+length*length*2,length);
		}
		else if (program == 1) {
			mm_scan(dst,dst+length*length,dst+length*length*2,dst+length*length*3,length,length);
		}
		else {
			cout << "program can be only 0 or 1" << endl;
		}
	}
	else if (num_threads == 4) {
		length1 = ( length >> 1 );
		length2 = length1 * length1;
		length11 = 0;
		length12 = length11 + length2;
		length21 = length12 + length2;
		length22 = length21 + length2;
		
		std::thread th1(mm_root, 1);
		std::thread th2(mm_root, 3);
		std::thread th3(mm_root, 5);
		std::thread th4(mm_root, 7);
		th1.join();
		th2.join();
		th3.join();
		th4.join();
		std::thread th5(mm_root, 2);
		std::thread th6(mm_root, 4);
		std::thread th7(mm_root, 6);
		std::thread th8(mm_root, 8);
		th5.join();
		th6.join();
		th7.join();
		th8.join();
		if (program == 1) {
			scan_add(dst, dst + length*length*3);
		}
	}
	else {
		cout << "invalid num_threads" << endl;
	}
	
	std::chrono::system_clock::time_point t_end = std::chrono::system_clock::now();
	double cpu_time = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	auto wall_time = std::chrono::duration<double, std::milli>(t_end-t_start).count();

	std::cout << "===========================================\n";
	std::cout << "Total wall time: " << wall_time << "\n";
	std::cout << "Total CPU time: " << cpu_time << "\n";
	std::cout << "===========================================\n";

	std::cout << "Data: " << (unsigned int)dst[length*length/2/2+length] << std::endl;
	std::cout << "===========================================\n";
	CacheHelper::print_io_data(io_stats, "Printing I/O statistics AFTER matrix multiplication @@@@@ \n");

	if (program == 0)
		mm_out << "MM_INPLACE" << "," << datafilename.substr(11,21) << "," << length << "," << num_threads << "," << wall_time << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
	else if (program == 1)
		mm_out << "MM_SCAN" << "," << datafilename.substr(11,21) << "," << length << "," << num_threads << "," << wall_time << "," << (float)io_stats[0]/1000000.0 << "," << (float)io_stats[1]/1000000.0 << "," << (float)(io_stats[0] + io_stats[1])/1000000.0 << std::endl;
	else
		cout << "program can be only 0 or 1" << endl;
	/*std::cout << "Result array\n";
	for (unsigned int i = 0 ; i < length*length; i++){
	std::cout << dst[i] << " ";
	}
	std::cout << std::endl;
	*/
  	return 0;
}
