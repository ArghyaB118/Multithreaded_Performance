#!/bin/bash
set -ex
mkdir -p executables
g++  -std=c++11 -pthread ./sorting.cpp -o ./executables/parallel_sorting
chmod a+x ./executables/parallel_sorting
g++ ./make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data

NUMRUNS=1
NUMINSTANCE=2

declare -a data_size=( 256 512 1024 2048 4096 ) 
declare -a memory_given=( 256 256 256 256 256 )

#creating nullbytes
mkdir -p data_files
for i in `seq 1 $NUMINSTANCE`;
do
	if [ ! -f "data_files/nullbytes$i" ]
	then
	  echo "First creating file for storing data."
	  dd if=/dev/urandom of=data_files/nullbytes$i count=32768 bs=1048576
	fi
done

if [ -f "out_sorting.csv" ]
then
  echo "out_sorting.csv already exists. Deleting it first."
  rm out_sorting.csv && touch out_sorting.csv
fi

echo "Program,MemoryMiB,InputMiB,NumThreads,Runtime,ReadIO,WriteIO,TotalIO" > out_sorting.csv

if [ -f "log.txt" ]
then
  echo "log.txt already exists. Deleting it first."
  rm log.txt && touch out-sorting.txt
fi

for i in `seq 1 $NUMRUNS`;
do
	for (( index=0; index<=${#data_size[@]}-1; index++ ));
	do
		data_size_run=${data_size[$index]}
		memory_given_run=${memory_given[$index]}
		TOTALMEMORY=$(($memory_given_run*1024*1024))

		#fanout=8 //decided in parallel_sorting.cpp
		#block_size=8192 //transfered to CacheHelper

		#code for parallel threaded funnel sort, instances with variable parallelism are run together
		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 0 1 data_files/nullbytes1
		wait

		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 0 8 data_files/nullbytes1
		wait

		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		./executables/make-unsorted-data $data_size_run data_files/nullbytes2
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 0 1 data_files/nullbytes1 &
		cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 0 8 data_files/nullbytes2
		wait

		./cgroup_creation.sh cache-test-arghya
		./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		./executables/make-unsorted-data $data_size_run data_files/nullbytes2
		sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 0 1 data_files/nullbytes2 &
		cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 0 8 data_files/nullbytes1
		wait

		# #code for parallel threaded merge sort, instances with variable parallelism are run together
		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 1 1 data_files/nullbytes1
		# wait

		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 1 8 data_files/nullbytes1
		# wait

		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes2
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 1 1 data_files/nullbytes1 &
		# cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 1 8 data_files/nullbytes2
		# wait

		# ./cgroup_creation.sh cache-test-arghya
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes1
		# ./executables/make-unsorted-data $data_size_run data_files/nullbytes2
		# sudo sh -c "sync; echo 3 > /proc/sys/vm/drop_caches; echo 0 > /proc/sys/vm/vfs_cache_pressure"
		# echo $TOTALMEMORY > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
		# cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 1 1 data_files/nullbytes2 &
		# cgexec -g memory:cache-test-arghya ./executables/parallel_sorting $memory_given_run $data_size_run 1 8 data_files/nullbytes1
		# wait		
	done
done